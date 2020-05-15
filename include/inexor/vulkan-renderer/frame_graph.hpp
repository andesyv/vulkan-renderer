#pragma once

// TODO(): Consider creating a forward declaration header for wrappers
#include "inexor/vulkan-renderer/shader.hpp"
#include "inexor/vulkan-renderer/texture.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// TODO(): Compute stage
// TODO(): Support buffers of arbitrary data as well as textures

namespace inexor::vulkan_renderer {

class RenderResource {};

// TODO(): Support textures with dimensions not equal to back buffer size
class TextureResource : public RenderResource {

public:
    TextureResource() {}
};

/// @brief A single render stage in the frame graph.
/// @note Not to be confused with a vulkan render pass!
class RenderStage {
    friend class FrameGraph;

private:
    const std::string m_name;
    std::vector<const RenderResource *> m_writes;
    std::vector<const RenderResource *> m_reads;

    VkPipeline m_pipeline;
    std::vector<Shader> m_shaders;

public:
    explicit RenderStage(std::string name) : m_name(std::move(name)) {}

    void writes_to(const RenderResource &resource);
    void reads_from(const RenderResource &resource);
};

class GraphicsStage : public RenderStage {
private:
    VkRenderPass m_render_pass;

public:
    explicit GraphicsStage(const std::string &name) : RenderStage(name) {}
};

class FrameGraph {
    template <typename Base, typename T, typename... Args>
    using IsDerivedFromAndConstructible =
        std::enable_if_t<std::bool_constant<!std::is_same_v<Base, T> && std::is_base_of_v<Base, T> && std::is_constructible_v<T, Args...>>::value, int>;

private:
    std::vector<RenderStage> m_stages;
    std::vector<RenderResource> m_resources;

    // Stage execution order
    std::vector<RenderStage *> m_stage_stack;

    // Physical resource map
    std::unordered_map<RenderResource *, Texture> m_resource_map;

public:
    template <typename T, typename... Args, IsDerivedFromAndConstructible<RenderStage, T, Args...> = 0>
    T &add(Args &&... args) {
        return static_cast<T &>(m_stages.emplace_back(std::forward<Args>(args)...));
    }

    template <typename T, typename... Args, IsDerivedFromAndConstructible<RenderResource, T, Args...> = 0>
    T &add(Args &&... args) {
        return static_cast<T &>(m_resources.emplace_back(std::forward<Args>(args)...));
    }

    void compile(const RenderResource &target);
};

} // namespace inexor::vulkan_renderer
