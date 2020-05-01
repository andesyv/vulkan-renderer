#include "inexor/vulkan-renderer/imgui/imgui_overlay.hpp"

namespace inexor::vulkan_renderer {

VkResult UIOverlay::init(VkDevice &device, std::shared_ptr<VulkanTextureManager> texture_manager, std::shared_ptr<MeshBufferManager> mesh_buffer_manager,
                         std::shared_ptr<VulkanShaderManager> shader_manager) {
    assert(device);
    assert(texture_manager);
    assert(mesh_buffer_manager);
    assert(shader_manager);

    this->device = device;
    this->texture_manager = texture_manager;
    this->mesh_buffer_manager = mesh_buffer_manager;
    this->shader_manager = shader_manager;

    imgui_overlay_initialised = true;

    return VK_SUCCESS;
}

UIOverlay::UIOverlay() {
    // Init ImGui
    ImGui::CreateContext();

    // Color scheme
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

    // Dimensions
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = scale;
}

UIOverlay::~UIOverlay() {}

/** Prepare all vulkan resources required to render the UI overlay */
void UIOverlay::prepareResources() {
    ImGuiIO &io = ImGui::GetIO();

    // Create font texture
    unsigned char *fontData;
    int texWidth, texHeight;

    io.Fonts->AddFontFromFileTTF("./../data/Roboto-Medium.ttf", 16.0f);
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

    imgui_texture = std::make_shared<Texture>();

    imgui_texture->name = "imgui_overlay";
    imgui_texture->format = VK_FORMAT_R8G8B8A8_UNORM;
    imgui_texture->width = texWidth;
    imgui_texture->height = texHeight;
    imgui_texture->mip_levels = 1;

    result = texture_manager->create_texture_from_memory("imgui_overlay", &fontData, uploadSize, imgui_texture);
    vulkan_error_check(result);

    // Descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes = {vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)};

    VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
    result = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool);
    vulkan_error_check(result);

    // Descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
        vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);

    result = vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout);
    vulkan_error_check(result);

    // Descriptor set
    VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);

    result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
    vulkan_error_check(result);

    VkDescriptorImageInfo fontDescriptor =
        vks::initializers::descriptorImageInfo(imgui_texture->sampler, imgui_texture->image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
        vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)};

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

/** Prepare a separate pipeline for the UI overlay rendering decoupled from the main application */
void UIOverlay::preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass) {
    // Pipeline layout
    // Push constants for UI rendering parameters
    VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

    // Setup graphics pipeline for UI rendering
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
        vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    // Enable blending
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

    VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(rasterizationSamples);

    std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaders.size());
    pipelineCreateInfo.pStages = shaders.data();
    pipelineCreateInfo.subpass = subpass;

    // Vertex bindings an attributes based on ImGui vertex definition
    std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
        vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    };
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
        vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),  // Location 0: Position
        vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),   // Location 1: UV
        vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)), // Location 0: Color
    };
    VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

    pipelineCreateInfo.pVertexInputState = &vertexInputState;

    result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline);
    vulkan_error_check(result);
}

/** Update vertex and index buffer containing the imGui elements when required */
bool UIOverlay::update() {
    ImDrawData *imDrawData = ImGui::GetDrawData();
    bool updateCmdBuffers = false;

    if (!imDrawData) {
        return false;
    };

    // Note: Alignment is done inside buffer creation
    VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    // Update buffers only if vertex or index count has been changed compared to current buffer size
    if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
        return false;
    }

    if (!vertex_buffer) {
        result = mesh_buffer_manager->create_vertex_buffer("imgui_vertex_buffer", vertexBufferSize, vertex_buffer);
        vulkan_error_check(result);
    }

    if (!index_buffer) {
        result = mesh_buffer_manager->create_index_buffer("imgui_index_buffer", indexBufferSize, index_buffer);
        vulkan_error_check(result);
    }

    // Vertex buffer
    if ((vertex_buffer->vertex_buffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
        // TODO: Do we have to unmap memory before destroying it?
        // TOOD: Destroy vertex buffer!
        // result = mesh_buffer_manager->destroy_vertex_buffer(vertex_buffer);
        // vulkan_error_check(result);

        result = mesh_buffer_manager->create_vertex_buffer("imgui_vertex_buffer", vertexBufferSize, vertex_buffer);
        vulkan_error_check(result);

        vertexCount = imDrawData->TotalVtxCount;
        updateCmdBuffers = true;
    }

    // Index buffer
    VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
    if ((index_buffer->index_buffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
        // TODO: Do we have to unmap memory before destroying it?
        // TOOD: Destroy index buffer!
        // result = mesh_buffer_manager->destroy_index_buffer(index_buffer);
        // vulkan_error_check(result);

        result = mesh_buffer_manager->create_index_buffer("imgui_index_buffer", indexBufferSize, index_buffer);
        vulkan_error_check(result);

        indexCount = imDrawData->TotalIdxCount;
        updateCmdBuffers = true;
    }

    // Upload data
    ImDrawVert *vtxDst = (ImDrawVert *)vertex_buffer->vertex_buffer.allocation_info.pMappedData;
    ImDrawIdx *idxDst = (ImDrawIdx *)index_buffer->index_buffer.allocation_info.pMappedData;

    for (int n = 0; n < imDrawData->CmdListsCount; n++) {
        const ImDrawList *cmd_list = imDrawData->CmdLists[n];
        memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        vtxDst += cmd_list->VtxBuffer.Size;
        idxDst += cmd_list->IdxBuffer.Size;
    }

    // TODO: Flush to make writes visible to GPU?

    return updateCmdBuffers;
}

void UIOverlay::draw(const VkCommandBuffer commandBuffer) {
    ImDrawData *imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;

    if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
        return;
    }

    ImGuiIO &io = ImGui::GetIO();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.0f);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertex_buffer->vertex_buffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, index_buffer->index_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++) {
        const ImDrawList *cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];
            VkRect2D scissorRect;
            scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
            scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
            scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
            vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

void UIOverlay::resize(uint32_t width, uint32_t height) {
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)(width), (float)(height));
}

void UIOverlay::freeResources() {
    ImGui::DestroyContext();
    // Other resources will be shut down by manager classes!
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
}

bool UIOverlay::header(const char *caption) {
    return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}

bool UIOverlay::checkBox(const char *caption, bool *value) {
    bool res = ImGui::Checkbox(caption, value);
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::checkBox(const char *caption, int32_t *value) {
    bool val = (*value == 1);
    bool res = ImGui::Checkbox(caption, &val);
    *value = val;
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::inputFloat(const char *caption, float *value, float step, uint32_t precision) {
    bool res = ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::sliderFloat(const char *caption, float *value, float min, float max) {
    bool res = ImGui::SliderFloat(caption, value, min, max);
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::sliderInt(const char *caption, int32_t *value, int32_t min, int32_t max) {
    bool res = ImGui::SliderInt(caption, value, min, max);
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::comboBox(const char *caption, int32_t *itemindex, std::vector<std::string> items) {
    if (items.empty()) {
        return false;
    }
    std::vector<const char *> charitems;
    charitems.reserve(items.size());
    for (size_t i = 0; i < items.size(); i++) {
        charitems.push_back(items[i].c_str());
    }
    uint32_t itemCount = static_cast<uint32_t>(charitems.size());
    bool res = ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
    if (res) {
        updated = true;
    };
    return res;
}

bool UIOverlay::button(const char *caption) {
    bool res = ImGui::Button(caption);
    if (res) {
        updated = true;
    };
    return res;
}

void UIOverlay::text(const char *formatstr, ...) {
    va_list args;
    va_start(args, formatstr);
    ImGui::TextV(formatstr, args);
    va_end(args);
}
} // namespace inexor::vulkan_renderer
