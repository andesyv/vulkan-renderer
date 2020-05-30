#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <algorithm>

namespace inexor::vulkan_renderer::world {
Indentation::Indentation(std::uint8_t start, std::uint8_t end) noexcept {}

bool Indentation::operator==(const Indentation &rhs) const {
    return this->m_start == rhs.m_start && this->m_end == rhs.m_end;
}

bool Indentation::operator!=(const Indentation &rhs) const {
    return !(*this == rhs);
}

void Indentation::set_start(std::uint8_t position) noexcept {
    this->m_start = std::clamp<std::uint8_t>(position, 0, Indentation::MAX);
    this->m_end = std::clamp<std::uint8_t>(this->m_end, this->m_start, Indentation::MAX);
}

void Indentation::set_end(std::uint8_t position) noexcept {
    this->m_end = std::clamp<std::uint8_t>(position, 0, Indentation::MAX);
    this->m_start = std::clamp<std::uint8_t>(this->m_start, 0, this->m_end);
}

std::uint8_t Indentation::start() const {
    return this->m_start;
}

std::uint8_t Indentation::end() const {
    return this->m_end;
}

std::uint8_t Indentation::end_rel() const {
    return Indentation::MAX - this->m_end;
}

std::uint8_t Indentation::offset() const noexcept {
    return this->m_end - this->m_start;
}

void Indentation::indent_start(std::int8_t steps) noexcept {
    this->set_start(this->m_start + steps);
}

void Indentation::indent_end(std::int8_t steps) noexcept {
    this->set_end(this->m_end - steps);
}
} // namespace inexor::vulkan_renderer::world
