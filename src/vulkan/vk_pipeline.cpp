#define NOMINMAX
#include "base/log.h"
#include "vk_pipeline.h"
#include "vk_utils.h"

// ============================================================================
// PipelineBuilder
// ============================================================================

PipelineBuilder::PipelineBuilder() {
    // Initialize all Vulkan structs to zero with their sType set
    // All pointers default to nullptr, all flags to 0, all booleans to VK_FALSE

    // -------------------------------------------------------------------------
    // Vertex Input State
    // -------------------------------------------------------------------------
    vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    // -------------------------------------------------------------------------
    // Input Assembly State
    // -------------------------------------------------------------------------
    inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // -------------------------------------------------------------------------
    // Viewport
    // -------------------------------------------------------------------------
    viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 0.0f;
    viewport.height = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // -------------------------------------------------------------------------
    // Scissor
    // -------------------------------------------------------------------------
    scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { 0, 0 };

    // -------------------------------------------------------------------------
    // Viewport State
    // -------------------------------------------------------------------------
    viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // -------------------------------------------------------------------------
    // Rasterization State
    // -------------------------------------------------------------------------
    rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // -------------------------------------------------------------------------
    // Multisample State
    // -------------------------------------------------------------------------
    multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // -------------------------------------------------------------------------
    // Color Blend Attachment State
    // -------------------------------------------------------------------------
    colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // -------------------------------------------------------------------------
    // Color Blend State
    // -------------------------------------------------------------------------
    colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // -------------------------------------------------------------------------
    // Depth Stencil State
    // -------------------------------------------------------------------------
    depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
    depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencil.front.passOp = VK_STENCIL_OP_KEEP;
    depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencil.front.compareMask = 0;
    depthStencil.front.writeMask = 0;
    depthStencil.front.reference = 0;
    depthStencil.back = {};
    depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
    depthStencil.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencil.back.compareMask = 0;
    depthStencil.back.writeMask = 0;
    depthStencil.back.reference = 0;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    // -------------------------------------------------------------------------
    // Other
    // -------------------------------------------------------------------------
    layout = VK_NULL_HANDLE;
    renderPass = VK_NULL_HANDLE;
    subpass = 0;
}

PipelineBuilder& PipelineBuilder::setShaders(VkShaderModule vert, VkShaderModule frag) {
    shaderStages.clear();

    // Vertex shader stage
    VkPipelineShaderStageCreateInfo vertStage = {};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vert;
    vertStage.pName = "main";
    vertStage.pSpecializationInfo = nullptr;
    shaderStages.push_back(vertStage);

    // Fragment shader stage
    VkPipelineShaderStageCreateInfo fragStage = {};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = frag;
    fragStage.pName = "main";
    fragStage.pSpecializationInfo = nullptr;
    shaderStages.push_back(fragStage);

    return *this;
}

PipelineBuilder& PipelineBuilder::setVertexInput(const VkPipelineVertexInputStateCreateInfo* info) {
    if (info != nullptr) {
        vertexInputInfo = *info;
    } else {
        // Reset to empty vertex input (no vertex attributes).
        // Used for full-screen quads or SSBO-based instanced rendering
        // where vertex data is generated in the vertex shader from gl_VertexIndex.
        vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }
    return *this;
}

PipelineBuilder& PipelineBuilder::setInputTopology(VkPrimitiveTopology topology) {
    inputAssembly.topology = topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    return *this;
}

PipelineBuilder& PipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    rasterizer.polygonMode = mode;
    return *this;
}

PipelineBuilder& PipelineBuilder::setCullMode(VkCullModeFlags cull, VkFrontFace front) {
    rasterizer.cullMode = cull;
    rasterizer.frontFace = front;
    return *this;
}

PipelineBuilder& PipelineBuilder::setMultisampling(VkSampleCountFlagBits samples) {
    multisampling.rasterizationSamples = samples;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 1.0f;
    return *this;
}

PipelineBuilder& PipelineBuilder::setColorBlendAttachment(const VkPipelineColorBlendAttachmentState& blend) {
    colorBlendAttachment = blend;
    return *this;
}

PipelineBuilder& PipelineBuilder::setBlendPreset(const VkPipelineColorBlendAttachmentState& preset) {
    colorBlendAttachment = preset;
    return *this;
}

PipelineBuilder& PipelineBuilder::setDepthTest(bool depthWrite, VkCompareOp compareOp) {
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = compareOp;
    return *this;
}

PipelineBuilder& PipelineBuilder::disableDepthTest() {
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    return *this;
}

PipelineBuilder& PipelineBuilder::setLayout(VkPipelineLayout pipelineLayout) {
    layout = pipelineLayout;
    return *this;
}

PipelineBuilder& PipelineBuilder::setRenderPass(VkRenderPass rp, uint32_t sp) {
    renderPass = rp;
    subpass = sp;
    return *this;
}

PipelineBuilder& PipelineBuilder::setViewportSize(float width, float height) {
    viewport.width = width;
    viewport.height = height;
    scissor.extent.width = static_cast<uint32_t>(width);
    scissor.extent.height = static_cast<uint32_t>(height);
    return *this;
}

bool PipelineBuilder::build(VkDevice device, VkPipeline* outPipeline) {
    if (outPipeline == nullptr) {
        BE_LOG_ERROR("PipelineBuilder::build: outPipeline is null");
        return false;
    }

    if (shaderStages.empty()) {
        BE_LOG_ERROR("PipelineBuilder::build: no shader stages set");
        return false;
    }

    if (layout == VK_NULL_HANDLE) {
        BE_LOG_ERROR("PipelineBuilder::build: pipeline layout not set");
        return false;
    }

    if (renderPass == VK_NULL_HANDLE) {
        BE_LOG_ERROR("PipelineBuilder::build: render pass not set");
        return false;
    }

    // -------------------------------------------------------------------------
    // Dynamic States: viewport and scissor are dynamic to allow runtime changes
    // without recreating the pipeline.
    // -------------------------------------------------------------------------
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    VkDynamicState dynamicStates[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // -------------------------------------------------------------------------
    // Graphics Pipeline Create Info
    // -------------------------------------------------------------------------
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(
        device,
        VK_NULL_HANDLE,  // pipelineCache
        1,               // createInfoCount
        &pipelineInfo,
        nullptr,         // pAllocator
        outPipeline
    );

    if (result != VK_SUCCESS) {
        BE_LOG_ERROR("PipelineBuilder::build: vkCreateGraphicsPipelines failed with result={}", static_cast<int>(result));
        return false;
    }

    return true;
}