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
    Indentation(const Indentation &rhs) = default;
    Indentation(std::uint8_t start, std::uint8_t end) noexcept;
    bool operator==(Indentation const &rhs) const;
    bool operator!=(Indentation const &rhs) const;

    /// Set absolute value of start.
    void set_start(std::uint8_t position) noexcept;
    /// Set absolute value of end.
    void set_end(std::uint8_t position) noexcept;

    /// Absolute value of start.
    [[nodiscard]] std::uint8_t start() const noexcept;
    /// Absolute value of end.
    [[nodiscard]] std::uint8_t end() const noexcept;
    /// Positive indent of end, relative from end's start.
    [[nodiscard]] std::uint8_t end_rel() const noexcept;
    /// Difference between start and end.
    [[nodiscard]] std::uint8_t offset() const noexcept;

    /// Positive steps into the direction of end.
    void indent_start(std::int8_t steps) noexcept;
    /// Positive steps into the direction of start.
    void indent_end(std::int8_t steps) noexcept;
};
} // namespace inexor::vulkan_renderer::world
