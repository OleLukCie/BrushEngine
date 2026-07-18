#define NOMINMAX
#include "base/log.h"
#include "vk_composite_pipeline.h"
#include "vk_buffer.h"
#include "vk_utils.h"
#include "vk_pipeline.h"
#include <cstring>

// ============================================================================
// 外部全局变量
// ============================================================================
extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;

// ============================================================================
// 内部常量
// ============================================================================
static constexpr uint32_t MAX_LAYERS = 16;               // 最大图层数量
static constexpr VkFormat COMPOSITE_TARGET_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

// ============================================================================
// 内部全局状态
// ============================================================================
static struct CompositeGlobals {
	VkShaderModule vertModule = VK_NULL_HANDLE;
	VkShaderModule fragModule = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	GpuBuffer uboBuffer;                                     // Host-visible UBO
	VkFramebuffer tempFramebuffer = VK_NULL_HANDLE;
	VkSampler layerSampler = VK_NULL_HANDLE;               // 图层纹理通用采样器
} g_composite;

// ============================================================================
// 辅助函数声明
// ============================================================================
static bool loadShaderModule(const char* filepath, VkShaderModule* outModule);
static bool createLayerSampler(VkSampler* outSampler);
static bool createDescriptorPool(VkDescriptorPool* outPool);
static bool createUBOBuffer();
static void destroyUBOBuffer();
static bool updateUBO(const CompositeUBO& uboData);
static bool updateDescriptorSet(const std::vector<VkImageView>& layerViews, uint32_t layerCount);
static bool createTempFramebuffer(VkImageView outputView, uint32_t width, uint32_t height,
                                   VkRenderPass renderPass, VkFramebuffer* outFramebuffer);
static void destroyTempFramebuffer(VkFramebuffer framebuffer);

// ============================================================================
// createCompositePipeline
// ============================================================================
bool createCompositePipeline(CompositePipelineState& outState, VkRenderPass sharedRenderPass) {
	// -------------------------------------------------------------------------
	// Step 1: 创建描述符集布局
	// Binding 0: Uniform Buffer (CompositeUBO) - 片段阶段只读
	// Binding 1: Combined Image Sampler Array[16] (图层纹理) - 片段阶段采样
	// -------------------------------------------------------------------------
	VkDescriptorSetLayoutBinding uboBinding = {};
	uboBinding.binding = 0;
	uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	uboBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding texBinding = {};
	texBinding.binding = 1;
	texBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texBinding.descriptorCount = MAX_LAYERS;  // 数组大小固定为 MAX_LAYERS
	texBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	texBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding bindings[2] = { uboBinding, texBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = bindings;

	VK_CHECK(vkCreateDescriptorSetLayout(g_device, &layoutInfo, nullptr, &outState.descSetLayout));
	if (outState.descSetLayout == VK_NULL_HANDLE) {
		BE_LOG_ERROR("createCompositePipeline: failed to create descriptor set layout");
		return false;
	}

	// -------------------------------------------------------------------------
	// Step 2: 创建 Pipeline Layout（无 Push Constants）
	// -------------------------------------------------------------------------
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &outState.descSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VK_CHECK(vkCreatePipelineLayout(g_device, &pipelineLayoutInfo, nullptr, &outState.layout));
	if (outState.layout == VK_NULL_HANDLE) {
		BE_LOG_ERROR("createCompositePipeline: failed to create pipeline layout");
		vkDestroyDescriptorSetLayout(g_device, outState.descSetLayout, nullptr);
		outState.descSetLayout = VK_NULL_HANDLE;
		return false;
	}

	// -------------------------------------------------------------------------
	// Step 3: 加载着色器
	// -------------------------------------------------------------------------
	if (!loadShaderModule("shaders/composite.vert.spv", &g_composite.vertModule)) {
		BE_LOG_ERROR("createCompositePipeline: failed to load composite.vert.spv");
		vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
		outState.layout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(g_device, outState.descSetLayout, nullptr);
		outState.descSetLayout = VK_NULL_HANDLE;
		return false;
	}

	if (!loadShaderModule("shaders/composite.frag.spv", &g_composite.fragModule)) {
		BE_LOG_ERROR("createCompositePipeline: failed to load composite.frag.spv");
		vkDestroyShaderModule(g_device, g_composite.vertModule, nullptr);
		g_composite.vertModule = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
		outState.layout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(g_device, outState.descSetLayout, nullptr);
		outState.descSetLayout = VK_NULL_HANDLE;
		return false;
	}

	// -------------------------------------------------------------------------
	// Step 4: 创建 Render Pass（如果未提供 sharedRenderPass）
	// -------------------------------------------------------------------------
	if (sharedRenderPass == VK_NULL_HANDLE) {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = COMPOSITE_TARGET_FORMAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;     // 合成结果从头开始
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

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
			BE_LOG_ERROR("createCompositePipeline: failed to create render pass");
			vkDestroyShaderModule(g_device, g_composite.fragModule, nullptr);
			g_composite.fragModule = VK_NULL_HANDLE;
			vkDestroyShaderModule(g_device, g_composite.vertModule, nullptr);
			g_composite.vertModule = VK_NULL_HANDLE;
			vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
			outState.layout = VK_NULL_HANDLE;
			vkDestroyDescriptorSetLayout(g_device, outState.descSetLayout, nullptr);
			outState.descSetLayout = VK_NULL_HANDLE;
			return false;
		}
	} else {
		outState.renderPass = sharedRenderPass;
	}

	// -------------------------------------------------------------------------
	// Step 5: 使用 PipelineBuilder 创建图形管线
	// -------------------------------------------------------------------------
	PipelineBuilder builder;
	builder.setShaders(g_composite.vertModule, g_composite.fragModule);
	builder.setVertexInput(nullptr);                              // 无顶点输入，全屏三角形
	builder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);  // 3 个顶点组成全屏三角形
	builder.setPolygonMode(VK_POLYGON_MODE_FILL);
	builder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	builder.setMultisampling(VK_SAMPLE_COUNT_1_BIT);
	builder.setBlendPreset(BlendPreset::Replace());              // 合成直接覆盖，无混合
	builder.disableDepthTest();
	builder.setLayout(outState.layout);
	builder.setRenderPass(outState.renderPass, 0);

	if (!builder.build(g_device, &outState.pipeline)) {
		BE_LOG_ERROR("createCompositePipeline: failed to build graphics pipeline");
		if (sharedRenderPass == VK_NULL_HANDLE && outState.renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(g_device, outState.renderPass, nullptr);
		}
		outState.renderPass = VK_NULL_HANDLE;
		vkDestroyShaderModule(g_device, g_composite.fragModule, nullptr);
		g_composite.fragModule = VK_NULL_HANDLE;
		vkDestroyShaderModule(g_device, g_composite.vertModule, nullptr);
		g_composite.vertModule = VK_NULL_HANDLE;
		vkDestroyPipelineLayout(g_device, outState.layout, nullptr);
		outState.layout = VK_NULL_HANDLE;
		vkDestroyDescriptorSetLayout(g_device, outState.descSetLayout, nullptr);
		outState.descSetLayout = VK_NULL_HANDLE;
		return false;
	}

	// -------------------------------------------------------------------------
	// Step 6: 创建 UBO 缓冲区和采样器
	// -------------------------------------------------------------------------
	if (!createUBOBuffer()) {
		BE_LOG_ERROR("createCompositePipeline: failed to create UBO buffer");
		destroyCompositePipeline(outState);
		return false;
	}

	if (!createLayerSampler(&g_composite.layerSampler)) {
		BE_LOG_ERROR("createCompositePipeline: failed to create layer sampler");
		destroyCompositePipeline(outState);
		return false;
	}

	if (!createDescriptorPool(&g_composite.descriptorPool)) {
		BE_LOG_ERROR("createCompositePipeline: failed to create descriptor pool");
		destroyCompositePipeline(outState);
		return false;
	}

	// 分配描述符集
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = g_composite.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &outState.descSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(g_device, &allocInfo, &g_composite.descriptorSet));
	if (g_composite.descriptorSet == VK_NULL_HANDLE) {
		BE_LOG_ERROR("createCompositePipeline: failed to allocate descriptor set");
		destroyCompositePipeline(outState);
		return false;
	}

	// 初始化 UBO 描述符（绑定 0）
	VkDescriptorBufferInfo uboInfo = {};
	uboInfo.buffer = g_composite.uboBuffer.buffer;
	uboInfo.offset = 0;
	uboInfo.range = sizeof(CompositeUBO);

	VkWriteDescriptorSet uboWrite = {};
	uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboWrite.dstSet = g_composite.descriptorSet;
	uboWrite.dstBinding = 0;
	uboWrite.dstArrayElement = 0;
	uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboWrite.descriptorCount = 1;
	uboWrite.pBufferInfo = &uboInfo;

	vkUpdateDescriptorSets(g_device, 1, &uboWrite, 0, nullptr);

	return true;
}

// ============================================================================
// destroyCompositePipeline
// ============================================================================
void destroyCompositePipeline(CompositePipelineState& state) {
	if (g_composite.descriptorSet != VK_NULL_HANDLE && g_composite.descriptorPool != VK_NULL_HANDLE) {
		vkFreeDescriptorSets(g_device, g_composite.descriptorPool, 1, &g_composite.descriptorSet);
		g_composite.descriptorSet = VK_NULL_HANDLE;
	}

	if (g_composite.descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(g_device, g_composite.descriptorPool, nullptr);
		g_composite.descriptorPool = VK_NULL_HANDLE;
	}

	if (g_composite.layerSampler != VK_NULL_HANDLE) {
		vkDestroySampler(g_device, g_composite.layerSampler, nullptr);
		g_composite.layerSampler = VK_NULL_HANDLE;
	}

	destroyUBOBuffer();

	if (state.pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(g_device, state.pipeline, nullptr);
		state.pipeline = VK_NULL_HANDLE;
	}

	if (g_composite.fragModule != VK_NULL_HANDLE) {
		vkDestroyShaderModule(g_device, g_composite.fragModule, nullptr);
		g_composite.fragModule = VK_NULL_HANDLE;
	}

	if (g_composite.vertModule != VK_NULL_HANDLE) {
		vkDestroyShaderModule(g_device, g_composite.vertModule, nullptr);
		g_composite.vertModule = VK_NULL_HANDLE;
	}

	if (state.layout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(g_device, state.layout, nullptr);
		state.layout = VK_NULL_HANDLE;
	}

	if (state.descSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(g_device, state.descSetLayout, nullptr);
		state.descSetLayout = VK_NULL_HANDLE;
	}

	if (state.renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(g_device, state.renderPass, nullptr);
		state.renderPass = VK_NULL_HANDLE;
	}
}

// ============================================================================
// recordCompositeRender
// ============================================================================
uint32_t recordCompositeRender(
	VkCommandBuffer cmd,
	const CompositePipelineState& state,
	const std::vector<VkImageView>& layerViews,
	const std::vector<CompositeLayerParams>& layerParams,
	const Color& background,
	VkImageView outputView,
	uint32_t outputWidth,
	uint32_t outputHeight) {

	uint32_t layerCount = static_cast<uint32_t>(layerViews.size());

	if (layerCount == 0) {
		// 无图层时只渲染背景色
		BE_LOG_ERROR("recordCompositeRender: no layers to composite");
		return 0;
	}

	if (layerCount > MAX_LAYERS) {
		BE_LOG_ERROR("recordCompositeRender: layer count {}, exceeds MAX_LAYERS {}", layerCount, MAX_LAYERS);
		return 0;
	}

	if (layerViews.size() != layerParams.size()) {
		BE_LOG_ERROR("recordCompositeRender: layerViews.size() != layerParams.size()");
		return 0;
	}

	// -------------------------------------------------------------------------
	// Step 1: 填充并上传 UBO
	// -------------------------------------------------------------------------
	CompositeUBO uboData = {};
	uboData.layerCount = static_cast<int32_t>(layerCount);
	uboData.backgroundColor = background;

	for (uint32_t i = 0; i < layerCount; i++) {
		uboData.layerParams[i] = layerParams[i];
	}

	if (!updateUBO(uboData)) {
		BE_LOG_ERROR("recordCompositeRender: failed to update UBO");
		return 0;
	}

	// -------------------------------------------------------------------------
	// Step 2: 更新图层纹理描述符（绑定 1）
	// -------------------------------------------------------------------------
	if (!updateDescriptorSet(layerViews, layerCount)) {
		BE_LOG_ERROR("recordCompositeRender: failed to update descriptor set");
		return 0;
	}

	// -------------------------------------------------------------------------
	// Step 3: 创建临时 framebuffer
	// -------------------------------------------------------------------------
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	if (!createTempFramebuffer(outputView, outputWidth, outputHeight, state.renderPass, &framebuffer)) {
		BE_LOG_ERROR("recordCompositeRender: failed to create temp framebuffer");
		return 0;
	}

	// -------------------------------------------------------------------------
	// Step 4: 开始 Render Pass
	// -------------------------------------------------------------------------
	VkClearValue clearValue = {};
	clearValue.color.float32[0] = background.r;
	clearValue.color.float32[1] = background.g;
	clearValue.color.float32[2] = background.b;
	clearValue.color.float32[3] = background.a;

	VkRenderPassBeginInfo rpBegin = {};
	rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBegin.renderPass = state.renderPass;
	rpBegin.framebuffer = framebuffer;
	rpBegin.renderArea.offset = { 0, 0 };
	rpBegin.renderArea.extent = { outputWidth, outputHeight };
	rpBegin.clearValueCount = 1;
	rpBegin.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

	// -------------------------------------------------------------------------
	// Step 5: 绑定管线和描述符集
	// -------------------------------------------------------------------------
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		state.layout,
		0,
		1,
		&g_composite.descriptorSet,
		0, nullptr
	);

	// -------------------------------------------------------------------------
	// Step 6: 设置动态视口和裁剪区
	// -------------------------------------------------------------------------
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(outputWidth);
	viewport.height = static_cast<float>(outputHeight);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { outputWidth, outputHeight };
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	// -------------------------------------------------------------------------
	// Step 7: 绘制全屏三角形（3 个顶点，1 个实例）
	// -------------------------------------------------------------------------
	vkCmdDraw(cmd, 3, 1, 0, 0);

	// -------------------------------------------------------------------------
	// Step 8: 结束 Render Pass
	// -------------------------------------------------------------------------
	vkCmdEndRenderPass(cmd);

	// -------------------------------------------------------------------------
	// Step 9: 清理临时 framebuffer
	// -------------------------------------------------------------------------
	destroyTempFramebuffer(framebuffer);

	return layerCount;
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

static bool createLayerSampler(VkSampler* outSampler) {
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
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_LAYERS;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;

	VK_CHECK(vkCreateDescriptorPool(g_device, &poolInfo, nullptr, outPool));
	return *outPool != VK_NULL_HANDLE;
}

static bool createUBOBuffer() {
	return g_composite.uboBuffer.create(
		sizeof(CompositeUBO),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
}

static void destroyUBOBuffer() {
	g_composite.uboBuffer.destroy();
}

static bool updateUBO(const CompositeUBO& uboData) {
	if (g_composite.uboBuffer.mapped == nullptr) {
		BE_LOG_ERROR("updateUBO: UBO buffer not mapped");
		return false;
	}
	std::memcpy(g_composite.uboBuffer.mapped, &uboData, sizeof(CompositeUBO));
	return true;
}

static bool updateDescriptorSet(const std::vector<VkImageView>& layerViews, uint32_t layerCount) {
	// 更新 UBO（绑定 0）
	VkDescriptorBufferInfo uboInfo = {};
	uboInfo.buffer = g_composite.uboBuffer.buffer;
	uboInfo.offset = 0;
	uboInfo.range = sizeof(CompositeUBO);

	VkWriteDescriptorSet uboWrite = {};
	uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uboWrite.dstSet = g_composite.descriptorSet;
	uboWrite.dstBinding = 0;
	uboWrite.dstArrayElement = 0;
	uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboWrite.descriptorCount = 1;
	uboWrite.pBufferInfo = &uboInfo;

	// 更新图层纹理数组（绑定 1）
	// 所有 MAX_LAYERS 个槽位都必须有有效绑定，未使用的绑定 dummy
	VkDescriptorImageInfo imageInfos[MAX_LAYERS];
	for (uint32_t i = 0; i < MAX_LAYERS; i++) {
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		if (i < layerCount && layerViews[i] != VK_NULL_HANDLE) {
			imageInfos[i].imageView = layerViews[i];
			imageInfos[i].sampler = g_composite.layerSampler;
		} else {
			// 未使用的槽位：绑定到第一个有效纹理或创建 dummy
			// 为简化，这里复用第一个有效纹理，shader 中通过 layerCount 控制不采样
			imageInfos[i].imageView = layerCount > 0 ? layerViews[0] : VK_NULL_HANDLE;
			imageInfos[i].sampler = g_composite.layerSampler;
		}
	}

	VkWriteDescriptorSet texWrite = {};
	texWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	texWrite.dstSet = g_composite.descriptorSet;
	texWrite.dstBinding = 1;
	texWrite.dstArrayElement = 0;
	texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texWrite.descriptorCount = MAX_LAYERS;
	texWrite.pImageInfo = imageInfos;

	VkWriteDescriptorSet writes[2] = { uboWrite, texWrite };
	vkUpdateDescriptorSets(g_device, 2, writes, 0, nullptr);

	return true;
}

static bool createTempFramebuffer(VkImageView outputView, uint32_t width, uint32_t height,
                                   VkRenderPass renderPass, VkFramebuffer* outFramebuffer) {
	VkFramebufferCreateInfo fbInfo = {};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = 1;
	fbInfo.pAttachments = &outputView;
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
