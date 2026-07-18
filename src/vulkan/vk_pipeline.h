#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

// 预设颜色混合附件配置
namespace BlendPreset {
    // 标准 Alpha 混合：src * src_alpha + dst * (1 - src_alpha)
    inline VkPipelineColorBlendAttachmentState AlphaBlend() {
        VkPipelineColorBlendAttachmentState blend = {};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
        return blend;
    }

    // 加法混合：src + dst
    inline VkPipelineColorBlendAttachmentState Additive() {
        VkPipelineColorBlendAttachmentState blend = {};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
        return blend;
    }

    // 乘法混合：src * dst（注意：硬件混合不支持，这里只是关闭 blend，实际在 shader 中处理）
    // 或者使用 VK_BLEND_FACTOR_DST_COLOR
    inline VkPipelineColorBlendAttachmentState Multiply() {
        VkPipelineColorBlendAttachmentState blend = {};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
        return blend;
    }

    // 不混合（覆盖模式）
    inline VkPipelineColorBlendAttachmentState Replace() {
        VkPipelineColorBlendAttachmentState blend = {};
        blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend.blendEnable = VK_FALSE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
        return blend;
    }
}

// 管线创建器
// 使用 fluent API 链式调用配置图形管线各项状态
struct PipelineBuilder {
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    uint32_t subpass = 0;

    PipelineBuilder();

    // 设置顶点/片段着色器
    PipelineBuilder& setShaders(VkShaderModule vert, VkShaderModule frag);

    // 设置顶点输入布局。传 nullptr 表示无顶点输入（用于 SSBO/实例化渲染）
    PipelineBuilder& setVertexInput(const VkPipelineVertexInputStateCreateInfo* info);

    // 设置图元拓扑类型
    PipelineBuilder& setInputTopology(VkPrimitiveTopology topology);

    // 设置多边形填充模式
    PipelineBuilder& setPolygonMode(VkPolygonMode mode);

    // 设置剔除模式和正面朝向
    PipelineBuilder& setCullMode(VkCullModeFlags cull, VkFrontFace front);

    // 设置多重采样
    PipelineBuilder& setMultisampling(VkSampleCountFlagBits samples);

    // 设置颜色混合附件状态
    PipelineBuilder& setColorBlendAttachment(const VkPipelineColorBlendAttachmentState& blend);

    // 快捷设置：启用/禁用混合，使用预设
    PipelineBuilder& setBlendPreset(const VkPipelineColorBlendAttachmentState& preset);

    // 设置深度测试
    PipelineBuilder& setDepthTest(bool depthWrite, VkCompareOp compareOp);

    // 禁用深度测试
    PipelineBuilder& disableDepthTest();

    // 设置管线布局
    PipelineBuilder& setLayout(VkPipelineLayout pipelineLayout);

    // 设置渲染通道
    PipelineBuilder& setRenderPass(VkRenderPass rp, uint32_t sp);

    // 设置视口尺寸（影响 viewport 和 scissor 的默认值）
    PipelineBuilder& setViewportSize(float width, float height);

    // 构建管线。outPipeline 必须是非空有效指针
    bool build(VkDevice device, VkPipeline* outPipeline);
};