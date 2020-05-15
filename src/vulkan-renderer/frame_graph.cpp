#include "inexor/vulkan-renderer/frame_graph.hpp"

#include <spdlog/spdlog.h>

#include <functional>

namespace inexor::vulkan_renderer {

void RenderStage::writes_to(const RenderResource &resource) {
    m_writes.push_back(&resource);
}

void RenderStage::reads_from(const RenderResource &resource) {
    m_reads.push_back(&resource);
}

void FrameGraph::compile(const RenderResource &target) {
    // TODO(): Many opportunities for optimisation

    // TODO(): Maybe store like this in the first place?
    std::unordered_map<const RenderResource *, std::vector<RenderStage *>> writers;
    for (auto &stage : m_stages) {
        for (const auto *resource : stage.m_writes) {
            writers[resource].push_back(&stage);
        }
    }

    // Post order depth first search
    // NOTE: Doesn't do any colouring, only works on acyclic graphs!
    // TODO(): Move away from recursive dfs algo
    std::function<void(RenderStage *)> dfs = [&](RenderStage *stage) {
        dfs(stage);
        m_stage_stack.push_back(stage);
    };

    // TODO(): Will there be more than one writer to the back buffer (maybe with blending)?
    for (auto *writer : writers[&target]) {
        m_stage_stack.push_back(writer);
    }

    // DFS starting from back buffer
    for (auto *stage : m_stage_stack) {
        dfs(stage);
    }

    // Reverse post order
    std::reverse(m_stage_stack.begin(), m_stage_stack.end());

#ifndef NDEBUG
    spdlog::debug("Frame graph stage order:");
    for (auto *stage : m_stage_stack) {
        spdlog::debug(stage->m_name);
    }
#endif

    // Physical resource allocation
    // TODO(): Resource aliasing
    for (const auto &resource : m_resources) {

    }

    // Render pass creation
    // Each render stage, after merging and reordering, maps to a vulkan render pass and pipeline
    for (const auto &stage : m_stages) {

    }
}

void test() {
    FrameGraph graph;

    auto &back_buffer = graph.add<TextureResource>();
    auto &texture = graph.add<TextureResource>();

    auto &stage = graph.add<GraphicsStage>("main");
    stage.writes_to(back_buffer);
    stage.reads_from(texture);

    graph.compile(back_buffer);
}

} // namespace inexor::vulkan_renderer
