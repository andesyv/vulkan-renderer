﻿#pragma once

#include <chrono>
#include <optional>

namespace inexor::vulkan_renderer {

class InexorFPSCounter {
private:
    std::size_t frames = 0;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_time;

    float fps_update_interval = 1.0f;

public:
    InexorFPSCounter() = default;

    ~InexorFPSCounter() = default;

    std::optional<uint32_t> update();
};

} // namespace inexor::vulkan_renderer
