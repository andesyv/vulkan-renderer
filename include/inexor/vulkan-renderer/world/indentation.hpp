#pragma once

#include <cstdint>

namespace inexor::vulkan_renderer::world {

class Indentation {
public:
    static constexpr std::uint8_t MAX = 8;

private:
    std::uint8_t m_start = 0;
    std::uint8_t m_end = Indentation::MAX;

public:
    Indentation() = default;
    Indentation(std::uint8_t start, std::uint8_t end) noexcept;

    void set_start(std::uint8_t position) noexcept;
    void set_end(std::uint8_t position) noexcept;

    [[nodiscard]] std::uint8_t start() const noexcept;
    [[nodiscard]] std::uint8_t end() const noexcept;
    [[nodiscard]] std::uint8_t end_rel() const noexcept;
    [[nodiscard]] std::uint8_t offset() const noexcept;

    /// Positive steps into the direction of the end.
    void indent_start(std::int8_t steps) noexcept;
    /// Positive steps into the direction of the start.
    void indent_end(std::int8_t steps) noexcept;
};
} // namespace inexor::vulkan_renderer::world
