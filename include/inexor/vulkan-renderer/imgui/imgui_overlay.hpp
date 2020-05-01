#pragma once

#include <assert.h>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>
#include <vulkan/vulkan.h>

// TODO: Refactor those initialiser functions!
#include "vks.hpp"

#include "inexor/vulkan-renderer/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/mesh_buffer_manager.hpp"
#include "inexor/vulkan-renderer/shader_manager.hpp"
#include "inexor/vulkan-renderer/texture_manager.hpp"

#include <cassert>
#include <memory>

namespace inexor::vulkan_renderer {
class UIOverlay {
    VkDevice device;
    VkResult result;

    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    uint32_t subpass = 0;

    std::shared_ptr<MeshBuffer> vertex_buffer;
    std::shared_ptr<MeshBuffer> index_buffer;

    int32_t vertexCount = 0;
    int32_t indexCount = 0;

    std::vector<VkPipelineShaderStageCreateInfo> shaders;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;

    bool visible = true;

    bool imgui_overlay_initialised = false;

    std::shared_ptr<VulkanTextureManager> texture_manager;
    std::shared_ptr<MeshBufferManager> mesh_buffer_manager;
    std::shared_ptr<VulkanShaderManager> shader_manager;

    std::shared_ptr<Texture> imgui_texture;

    std::shared_ptr<Shader> imgui_vertex_shader;
    std::shared_ptr<Shader> imgui_fragment_shader;

public:
    // TODO: Sort public/private!
    float scale = 1.0f;
    bool updated = false;

    UIOverlay();
    ~UIOverlay();

    VkResult init(VkDevice &device, std::shared_ptr<VulkanTextureManager> texture_manager, std::shared_ptr<MeshBufferManager> mesh_buffer_manager,
                  std::shared_ptr<VulkanShaderManager> shader_manager);

    /// @brief Prepare a separate pipeline for the UI overlay rendering decoupled from the main application.
    VkResult preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

    /// @brief Prepare all vulkan resources required to render the UI overlay.
    VkResult prepareResources();

    /// @brief Update vertex and index buffer containing the imGui elements when required.
    bool update();
    VkResult draw(const VkCommandBuffer commandBuffer);
    VkResult resize(uint32_t width, uint32_t height);

    VkResult freeResources();

    bool header(const char *caption);
    bool checkBox(const char *caption, bool *value);
    bool checkBox(const char *caption, int32_t *value);
    bool inputFloat(const char *caption, float *value, float step, uint32_t precision);
    bool sliderFloat(const char *caption, float *value, float min, float max);
    bool sliderInt(const char *caption, int32_t *value, int32_t min, int32_t max);
    bool comboBox(const char *caption, int32_t *itemindex, std::vector<std::string> items);
    bool button(const char *caption);
    void text(const char *formatstr, ...);
};
} // namespace inexor::vulkan_renderer
