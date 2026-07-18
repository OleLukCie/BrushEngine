#define NOMINMAX
#include "base/log.h"
#include "vk_brush_pipeline.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "vk_pipeline.h"
#include "../render/texture.h"
#include <cstring>
#include <cmath>

// 外部全局变量
extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;
extern VkCommandPool g_commandPool;
extern VkQueue g_graphicsQueue;

// 内部常量
static constexpr uint32_t MAX_DABS_PER_BATCH = 4096;     // 每批最大 dab 数量
static constexpr uint32_t DAB_BUFFER_RING_SIZE = 3;      // 三重缓冲（对应 swapchain 帧数）
static constexpr VkFormat BRUSH_TARGET_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

// 内部全局状态（单例模式）
static struct BrushGlobals {
    GpuBuffer dabStagingBuffer;                              // Staging buffer for SSBO upload
    GpuBuffer dabDeviceBuffer;                               // Device-local SSBO
    RingBuffer dabRingBuffer;                                // Per-frame SSBO ring buffer
    VkFramebuffer tempFramebuffer = VK_NULL_HANDLE;          // 临时 framebuffer（每帧重建）
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;       // 描述符池
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;          // 当前帧描述符集
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE; // 描述符集布局
    VkSampler defaultSampler = VK_NULL_HANDLE;             // 默认纹理采样器（笔刷纹理用）
    VkShaderModule vertModule = VK_NULL_HANDLE;              // 顶点着色器模块
    VkShaderModule fragModule = VK_NULL_HANDLE;              // 片段着色器模块
    uint32_t currentFrame = 0;                               // 当前帧索引（用于 ring buffer）
} g_brush;

// 辅助函数声明
static bool loadShaderModule(const char* filepath, VkShaderModule* outModule);
static bool createDefaultSampler(VkSampler* outSampler);
static bool createDescriptorPool(VkDescriptorPool* outPool);
static bool createDabBuffers();
static void destroyDabBuffers();
static bool uploadDabsToGPU(VkCommandBuffer cmd, const std::vector<DabInstance>& dabs, VkDeviceSize offset);
static bool createTempFramebuffer(VkImageView targetView, uint32_t width, uint32_t height, VkRenderPass renderPass, VkFramebuffer* outFramebuffer);
static void destroyTempFramebuffer(VkFramebuffer framebuffer);


// createBrushPipeline
bool createBrushPipeline(BrushPipelineState& outState, VkRenderPass sharedRenderPass) {
    // -------------------------------------------------------------------------
    // Step 1: 创建描述符集布局
    // Binding 0: SSBO (DabInstance 数组) - 顶点+片段阶段只读
    // Binding 1: Combined Image Sampler (笔刷纹理) - 片段阶段采样
    // -------------------------------------------------------------------------
    VkDescriptorSetLayoutBinding ssboBinding = {};
    ssboBinding.binding = 0;
    ssboBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboBinding.descriptorCount = 1;
    ssboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    ssboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding texBinding = {};
    texBinding.binding = 1;
    texBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texBinding.descriptorCount = 1;
    texBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    texBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[2] = { ssboBinding, texBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    VK_CHECK(vkCreateDescriptorSetLayout(g_device, &layoutInfo, nullptr, &g_brush.descriptorSetLayout));
    if (g_brush.descriptorSetLayout == VK_NULL_HANDLE) {
        BE_LOG_ERROR("createBrushPipeline: failed to create descriptor set layout");
        return false;
    }

    // Step 2: 创建 Push Constant Range
    // BrushPushConstants: 16 bytes, 顶点+片段阶段
    VkPushConstantRange pushRange = {};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(BrushPushConstants);

    // Step 3: 创建 Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &g_brush.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;

    VK_CHECK(vkCreatePipelineLayout(g_device, &pipelineLayoutInfo, nullptr, &outState.layout));
    if (outState.layout == VK_NULL_HANDLE) {
        BE_LOG_ERROR("createBrushPipeline: failed to create pipeline layout");
        vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
        g_brush.descriptorSetLayout = VK_NULL_HANDLE;
        return false;
    }

    // -------------------------------------------------------------------------
    // Step 4: 加载着色器模块
    // 着色器文件路径相对于可执行文件目录
    // -------------------------------------------------------------------------
    if (!loadShaderModule("shaders/brush_dab.vert.spv", &g_brush.vertModule)) {
        BE_LOG_ERROR("createBrushPipeline: failed to load brush_dab.vert.spv");
        vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
        outState.layout = VK_NULL_HANDLE;
        vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
        g_brush.descriptorSetLayout = VK_NULL_HANDLE;
        return false;
    }

    if (!loadShaderModule("shaders/brush_dab.frag.spv", &g_brush.fragModule)) {
        BE_LOG_ERROR("createBrushPipeline: failed to load brush_dab.frag.spv");
        vkDestroyShaderModule(g_device, g_brush.vertModule, nullptr);
        g_brush.vertModule = VK_NULL_HANDLE;
        vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
        outState.layout = VK_NULL_HANDLE;
        vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
        g_brush.descriptorSetLayout = VK_NULL_HANDLE;
        return false;
    }

    // -------------------------------------------------------------------------
    // Step 5: 创建 Render Pass（如果未提供 sharedRenderPass）
    // 笔刷渲染只需要一个 color attachment，无 depth/stencil
    // -------------------------------------------------------------------------
    if (sharedRenderPass == VK_NULL_HANDLE) {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = BRUSH_TARGET_FORMAT;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;      // 保留已有内容（dab 叠加）
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef = {};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK(vkCreateRenderPass(g_device, &renderPassInfo, nullptr, &outState.renderPass));
        if (outState.renderPass == VK_NULL_HANDLE) {
            BE_LOG_ERROR("createBrushPipeline: failed to create render pass");
            vkDestroyShaderModule(g_device, g_brush.fragModule, nullptr);
            g_brush.fragModule = VK_NULL_HANDLE;
            vkDestroyShaderModule(g_device, g_brush.vertModule, nullptr);
            g_brush.vertModule = VK_NULL_HANDLE;
            vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
            outState.layout = VK_NULL_HANDLE;
            vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
            g_brush.descriptorSetLayout = VK_NULL_HANDLE;
            return false;
        }
    } else {
        outState.renderPass = sharedRenderPass;
    }

    // -------------------------------------------------------------------------
    // Step 6: 使用 PipelineBuilder 创建图形管线
    // -------------------------------------------------------------------------
    PipelineBuilder builder;
    builder.setShaders(g_brush.vertModule, g_brush.fragModule);
    builder.setVertexInput(nullptr);                              // 无顶点输入，纯 SSBO 实例化
    builder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP); // 4 个顶点组成 quad
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE); // 2D 渲染关闭剔除
    builder.setMultisampling(VK_SAMPLE_COUNT_1_BIT);
    builder.setBlendPreset(BlendPreset::AlphaBlend());
    builder.disableDepthTest();
    builder.setLayout(outState.layout);
    builder.setRenderPass(outState.renderPass, 0);

    // 视口和裁剪区设为动态，在 recordBrushRender 中根据目标尺寸设置
    if (!builder.build(g_device, &outState.pipeline)) {
        BE_LOG_ERROR("createBrushPipeline: failed to build graphics pipeline");
        if (sharedRenderPass == VK_NULL_HANDLE && outState.renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(g_device, outState.renderPass, nullptr);
        }
        outState.renderPass = VK_NULL_HANDLE;
        vkDestroyShaderModule(g_device, g_brush.fragModule, nullptr);
        g_brush.fragModule = VK_NULL_HANDLE;
        vkDestroyShaderModule(g_device, g_brush.vertModule, nullptr);
        g_brush.vertModule = VK_NULL_HANDLE;
        vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
        outState.layout = VK_NULL_HANDLE;
        vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
        g_brush.descriptorSetLayout = VK_NULL_HANDLE;
        return false;
    }

    // -------------------------------------------------------------------------
    // Step 7: 创建 dab 缓冲区和描述符资源
    // -------------------------------------------------------------------------
    if (!createDabBuffers()) {
        BE_LOG_ERROR("createBrushPipeline: failed to create dab buffers");
        destroyBrushPipeline(outState);
        return false;
    }

    if (!createDefaultSampler(&g_brush.defaultSampler)) {
        BE_LOG_ERROR("createBrushPipeline: failed to create default sampler");
        destroyBrushPipeline(outState);
        return false;
    }

    if (!createDescriptorPool(&g_brush.descriptorPool)) {
        BE_LOG_ERROR("createBrushPipeline: failed to create descriptor pool");
        destroyBrushPipeline(outState);
        return false;
    }

    // 分配描述符集
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_brush.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &g_brush.descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(g_device, &allocInfo, &g_brush.descriptorSet));
    if (g_brush.descriptorSet == VK_NULL_HANDLE) {
        BE_LOG_ERROR("createBrushPipeline: failed to allocate descriptor set");
        destroyBrushPipeline(outState);
        return false;
    }

    // 初始化 SSBO 描述符（绑定 0）
    VkDescriptorBufferInfo ssboInfo = {};
    ssboInfo.buffer = g_brush.dabDeviceBuffer.buffer;
    ssboInfo.offset = 0;
    ssboInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet ssboWrite = {};
    ssboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ssboWrite.dstSet = g_brush.descriptorSet;
    ssboWrite.dstBinding = 0;
    ssboWrite.dstArrayElement = 0;
    ssboWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboWrite.descriptorCount = 1;
    ssboWrite.pBufferInfo = &ssboInfo;

    vkUpdateDescriptorSets(g_device, 1, &ssboWrite, 0, nullptr);

    outState.descSetLayout = g_brush.descriptorSetLayout;

    return true;
}

// ============================================================================
// destroyBrushPipeline
// ============================================================================
void destroyBrushPipeline(BrushPipelineState& state) {
    if (g_brush.descriptorSet != VK_NULL_HANDLE && g_brush.descriptorPool != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(g_device, g_brush.descriptorPool, 1, &g_brush.descriptorSet);
        g_brush.descriptorSet = VK_NULL_HANDLE;
    }

    if (g_brush.descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(g_device, g_brush.descriptorPool, nullptr);
        g_brush.descriptorPool = VK_NULL_HANDLE;
    }

    if (g_brush.defaultSampler != VK_NULL_HANDLE) {
        vkDestroySampler(g_device, g_brush.defaultSampler, nullptr);
        g_brush.defaultSampler = VK_NULL_HANDLE;
    }

    destroyDabBuffers();

    if (state.pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(g_device, state.pipeline, nullptr);
        state.pipeline = VK_NULL_HANDLE;
    }

    if (g_brush.fragModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(g_device, g_brush.fragModule, nullptr);
        g_brush.fragModule = VK_NULL_HANDLE;
    }

    if (g_brush.vertModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(g_device, g_brush.vertModule, nullptr);
        g_brush.vertModule = VK_NULL_HANDLE;
    }

    if (state.layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(g_device, state.layout, nullptr);
        state.layout = VK_NULL_HANDLE;
    }

    if (g_brush.descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(g_device, g_brush.descriptorSetLayout, nullptr);
        g_brush.descriptorSetLayout = VK_NULL_HANDLE;
        state.descSetLayout = VK_NULL_HANDLE;
    }

    if (state.renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(g_device, state.renderPass, nullptr);
        state.renderPass = VK_NULL_HANDLE;
    }
}

// ============================================================================
// recordBrushRender
// ============================================================================
uint32_t recordBrushRender(
    VkCommandBuffer cmd,
    const BrushPipelineState& state,
    const std::vector<DabInstance>& dabs,
    VkImage targetImage,
    VkImageView targetView,
    uint32_t targetWidth,
    uint32_t targetHeight,
    const BrushTexture* brushTex) {

    if (dabs.empty()) {
        return 0;
    }

    if (dabs.size() > MAX_DABS_PER_BATCH) {
        BE_LOG_DEBUG(
            "recordBrushRender: dab count {} exceeds MAX_DABS_PER_BATCH {}",
            dabs.size(),
            MAX_DABS_PER_BATCH
        );
        return 0;
    }

    // -------------------------------------------------------------------------
    // Step 1: 确保目标图像布局为 COLOR_ATTACHMENT_OPTIMAL
    // -------------------------------------------------------------------------
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // 假设从 undefined 进入，调用方应确保正确
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = targetImage;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    // -------------------------------------------------------------------------
    // Step 2: 上传 dab 数据到 GPU SSBO
    // -------------------------------------------------------------------------
    VkDeviceSize ssboOffset = g_brush.dabRingBuffer.currentOffset();
    if (!uploadDabsToGPU(cmd, dabs, ssboOffset)) {
        BE_LOG_ERROR("recordBrushRender: failed to upload dabs to GPU");
        return 0;
    }

    // -------------------------------------------------------------------------
    // Step 3: 更新描述符集（SSBO offset + 纹理）
    // -------------------------------------------------------------------------
    VkDescriptorBufferInfo ssboInfo = {};
    ssboInfo.buffer = g_brush.dabDeviceBuffer.buffer;
    ssboInfo.offset = ssboOffset;
    ssboInfo.range = dabs.size() * sizeof(DabInstance);

    VkWriteDescriptorSet ssboWrite = {};
    ssboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ssboWrite.dstSet = g_brush.descriptorSet;
    ssboWrite.dstBinding = 0;
    ssboWrite.dstArrayElement = 0;
    ssboWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboWrite.descriptorCount = 1;
    ssboWrite.pBufferInfo = &ssboInfo;

    VkWriteDescriptorSet writes[2] = { ssboWrite };
    uint32_t writeCount = 1;

    VkDescriptorImageInfo texInfo = {};
    if (brushTex != nullptr && brushTex->view != VK_NULL_HANDLE) {
        texInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        texInfo.imageView = brushTex->view;
        texInfo.sampler = brushTex->sampler != VK_NULL_HANDLE ? brushTex->sampler : g_brush.defaultSampler;

        VkWriteDescriptorSet texWrite = {};
        texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texWrite.dstSet = g_brush.descriptorSet;
        texWrite.dstBinding = 1;
        texWrite.dstArrayElement = 0;
        texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texWrite.descriptorCount = 1;
        texWrite.pImageInfo = &texInfo;

        writes[1] = texWrite;
        writeCount = 2;
    } else {
        // 无纹理时绑定一个 dummy 1x1 透明纹理，或保持上一帧绑定
        // 为简化，这里不处理，shader 中通过 useTexture 标志控制
    }

    vkUpdateDescriptorSets(g_device, writeCount, writes, 0, nullptr);

    // -------------------------------------------------------------------------
    // Step 4: 创建临时 framebuffer
    // -------------------------------------------------------------------------
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    if (!createTempFramebuffer(targetView, targetWidth, targetHeight, state.renderPass, &framebuffer)) {
        BE_LOG_ERROR("recordBrushRender: failed to create temp framebuffer");
        return 0;
    }

    // -------------------------------------------------------------------------
    // Step 5: 开始 Render Pass
    // -------------------------------------------------------------------------
    VkRenderPassBeginInfo rpBegin = {};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = state.renderPass;
    rpBegin.framebuffer = framebuffer;
    rpBegin.renderArea.offset = { 0, 0 };
    rpBegin.renderArea.extent = { targetWidth, targetHeight };
    rpBegin.clearValueCount = 0;
    rpBegin.pClearValues = nullptr;

    vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    // -------------------------------------------------------------------------
    // Step 6: 绑定管线和描述符集
    // -------------------------------------------------------------------------
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);

    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        state.layout,
        0,                          // firstSet
        1,                          // descriptorSetCount
        &g_brush.descriptorSet,
        0, nullptr
    );

    // -------------------------------------------------------------------------
    // Step 7: 设置动态视口和裁剪区
    // -------------------------------------------------------------------------
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(targetWidth);
    viewport.height = static_cast<float>(targetHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { targetWidth, targetHeight };
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // -------------------------------------------------------------------------
    // Step 8: 设置 Push Constants
    // -------------------------------------------------------------------------
    BrushPushConstants pc = {};
    pc.canvasSize = Vec2(static_cast<float>(targetWidth), static_cast<float>(targetHeight));
    pc.globalOpacity = 1.0f;  // 可由调用方传入，这里默认 1.0
    pc.numDabs = static_cast<uint32_t>(dabs.size());

    vkCmdPushConstants(
        cmd,
        state.layout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(BrushPushConstants),
        &pc
    );

    // -------------------------------------------------------------------------
    // Step 9: 绘制实例化 quad
    // 4 个顶点（triangle strip）× N 个实例（dab 数量）
    // -------------------------------------------------------------------------
    vkCmdDraw(cmd, 4, static_cast<uint32_t>(dabs.size()), 0, 0);

    // -------------------------------------------------------------------------
    // Step 10: 结束 Render Pass
    // -------------------------------------------------------------------------
    vkCmdEndRenderPass(cmd);

    // -------------------------------------------------------------------------
    // Step 11: 清理临时 framebuffer
    // -------------------------------------------------------------------------
    destroyTempFramebuffer(framebuffer);

    // -------------------------------------------------------------------------
    // Step 12: 推进 ring buffer 到下一帧
    // -------------------------------------------------------------------------
    g_brush.dabRingBuffer.nextFrame();

    return static_cast<uint32_t>(dabs.size());
}

// ============================================================================
// createBrushTexture
// ============================================================================
BrushTexture createBrushTexture(const Texture* texture) {
    BrushTexture result = {};

    if (texture == nullptr) {
        return result;
    }

    // TODO: 从 Texture 对象获取像素数据并上传到 GPU
    // 这需要 Texture 类提供像素访问接口
    // 当前实现返回空结构，需要后续补充

    BE_LOG_WARN("createBrushTexture: not yet implemented (requires Texture pixel access)");
    return result;
}

// ============================================================================
// destroyBrushTexture
// ============================================================================
void destroyBrushTexture(BrushTexture& tex) {
    if (tex.sampler != VK_NULL_HANDLE) {
        vkDestroySampler(g_device, tex.sampler, nullptr);
        tex.sampler = VK_NULL_HANDLE;
    }
    if (tex.view != VK_NULL_HANDLE) {
        vkDestroyImageView(g_device, tex.view, nullptr);
        tex.view = VK_NULL_HANDLE;
    }
    if (tex.image != VK_NULL_HANDLE) {
        vkDestroyImage(g_device, tex.image, nullptr);
        tex.image = VK_NULL_HANDLE;
    }
    if (tex.memory != VK_NULL_HANDLE) {
        vkFreeMemory(g_device, tex.memory, nullptr);
        tex.memory = VK_NULL_HANDLE;
    }
    tex.width = 0;
    tex.height = 0;
}

// ============================================================================
// 辅助函数实现
// ============================================================================

static bool loadShaderModule(const char* filepath, VkShaderModule* outModule) {
    FILE* file = nullptr;
    fopen_s(&file, filepath, "rb");
    if (!file) {
        BE_LOG_ERROR("loadShaderModule: failed to open {}", filepath);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    std::vector<char> buffer(fileSize);
    size_t readSize = fread(buffer.data(), 1, fileSize, file);
    fclose(file);

    if (readSize != static_cast<size_t>(fileSize)) {
        BE_LOG_ERROR("loadShaderModule: failed to read {}", filepath);
        return false;
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

    VK_CHECK(vkCreateShaderModule(g_device, &createInfo, nullptr, outModule));
    return *outModule != VK_NULL_HANDLE;
}

static bool createDefaultSampler(VkSampler* outSampler) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    VK_CHECK(vkCreateSampler(g_device, &samplerInfo, nullptr, outSampler));
    return *outSampler != VK_NULL_HANDLE;
}

static bool createDescriptorPool(VkDescriptorPool* outPool) {
    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = DAB_BUFFER_RING_SIZE;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = DAB_BUFFER_RING_SIZE;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = DAB_BUFFER_RING_SIZE;

    VK_CHECK(vkCreateDescriptorPool(g_device, &poolInfo, nullptr, outPool));
    return *outPool != VK_NULL_HANDLE;
}

static bool createDabBuffers() {
    VkDeviceSize perFrameSize = MAX_DABS_PER_BATCH * sizeof(DabInstance);

    if (!g_brush.dabRingBuffer.init(perFrameSize, DAB_BUFFER_RING_SIZE,
                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
        BE_LOG_ERROR("createDabBuffers: failed to create ring buffer");
        return false;
    }

    if (!g_brush.dabDeviceBuffer.create(
            perFrameSize * DAB_BUFFER_RING_SIZE,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
        BE_LOG_ERROR("createDabBuffers: failed to create device buffer");
        return false;
    }

    return true;
}

static void destroyDabBuffers() {
    g_brush.dabRingBuffer.gpuBuffer.destroy();
    g_brush.dabRingBuffer.frameSize = 0;
    g_brush.dabRingBuffer.frameCount = 0;
    g_brush.dabRingBuffer.frameIndex = 0;

    g_brush.dabDeviceBuffer.destroy();
}

static bool uploadDabsToGPU(VkCommandBuffer cmd, const std::vector<DabInstance>& dabs, VkDeviceSize offset) {
    VkDeviceSize dataSize = dabs.size() * sizeof(DabInstance);

    if (offset + dataSize > g_brush.dabDeviceBuffer.size) {
        BE_LOG_ERROR("uploadDabsToGPU: data exceeds buffer size");
        return false;
    }

    // 直接 memcpy 到 host-visible buffer
    void* mapped = g_brush.dabDeviceBuffer.mapped;
    if (mapped == nullptr) {
        BE_LOG_ERROR("uploadDabsToGPU: buffer not mapped");
        return false;
    }

    std::memcpy(static_cast<char*>(mapped) + offset, dabs.data(), dataSize);
    return true;
}

static bool createTempFramebuffer(VkImageView targetView, uint32_t width, uint32_t height,
                                   VkRenderPass renderPass, VkFramebuffer* outFramebuffer) {
    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = renderPass;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments = &targetView;
    fbInfo.width = width;
    fbInfo.height = height;
    fbInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(g_device, &fbInfo, nullptr, outFramebuffer));
    return *outFramebuffer != VK_NULL_HANDLE;
}

static void destroyTempFramebuffer(VkFramebuffer framebuffer) {
    if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(g_device, framebuffer, nullptr);
    }
}