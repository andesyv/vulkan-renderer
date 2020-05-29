#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <utility>

namespace inexor::vulkan_renderer::world {
Cube::Cube(std::shared_ptr<Cube> parent, const Type type, const float size, const glm::vec3 &position)
    : m_parent(std::move(parent)), m_size(size), m_position(position) {
    this->set_type(type);
}

Cube::~Cube() {}

void Cube::remove_childs() {
    for (auto &child : this->m_childs) {
        child->remove_childs();
        child.reset();
    }
}

std::array<glm::vec3, 8> Cube::vertices() const noexcept {
    assert(this->m_type != Type::EMPTY);

    const glm::vec3 &pos = this->m_position;
    const glm::vec3 max = {this->m_position.x + this->m_size, this->m_position.y + this->m_size, this->m_position.z + this->m_size};

    if (this->m_type == Type::SOLID) {
        return {{{pos.x, pos.y, pos.z},
                 {pos.x, pos.y, max.z},
                 {pos.x, max.y, pos.z},
                 {pos.x, max.y, max.z},
                 {max.x, pos.y, pos.z},
                 {max.x, pos.y, max.z},
                 {max.x, max.y, pos.z},
                 {max.x, max.y, max.z}}};
    }
    if (this->m_type == Type::NORMAL) {
        const float step = this->m_size / Indentation::MAX;
        const std::array<Indentation, Cube::EDGES> &ind = this->m_indentations;

        return {{{pos.x + ind[0].start() * step, pos.y + ind[1].start() * step, pos.z + ind[2].start() * step},
                 {pos.x + ind[9].start() * step, pos.y + ind[4].start() * step, max.z - ind[2].end_rel() * step},
                 {pos.x + ind[3].start() * step, max.y - ind[1].end_rel() * step, pos.z + ind[11].start() * step},
                 {pos.x + ind[6].start() * step, max.y - ind[4].end_rel() * step, max.z - ind[11].end_rel() * step},
                 {max.x - ind[0].end_rel() * step, pos.y + ind[10].start() * step, pos.z + ind[5].start() * step},
                 {max.x - ind[9].end_rel() * step, pos.y + ind[7].start() * step, max.z - ind[5].end_rel() * step},
                 {max.x - ind[3].end_rel() * step, max.y - ind[10].end_rel() * step, pos.z + ind[8].start() * step},
                 {max.x - ind[6].end_rel() * step, max.y - ind[7].end_rel() * step, max.z - ind[8].end_rel() * step}}};
    }
    if (this->m_type == Type::OCTANT) {
        // TODO: optimized octant vertices
        return {};
    }
    return {};
}

std::weak_ptr<Cube> Cube::root() const noexcept {
    std::shared_ptr<Cube> parent = m_parent;
    while (!parent->is_root()) {
        parent = parent->m_parent;
    }
    return parent;
}

bool Cube::is_root() const noexcept {
    return &(*this->m_parent) == this;
}

std::size_t Cube::grid_level() const noexcept {
    std::size_t level = 0;
    std::shared_ptr<Cube> parent = m_parent;
    while (!parent->is_root()) {
        parent = parent->m_parent;
        level++;
    }
    return level;
}

void Cube::set_type(const Type new_type) {
    if (this->m_type == new_type) {
        return;
    }
    switch (new_type) {
    case Type::EMPTY:
        break;
    case Type::SOLID:
        break;
    case Type::NORMAL:
        this->m_indentations = {};
        break;
    case Type::OCTANT:
        const float half_size = this->m_size / 2;
        auto create_cube = [&](const glm::vec3 &offset) {
            return std::make_shared<Cube>(this->shared_from_this(), Type::SOLID, half_size, this->m_position + offset);
        };
        // about the order look into the octree documentation
        this->m_childs = {create_cube({0, 0, 0}),
                          create_cube({0, 0, half_size}),
                          create_cube({0, half_size, 0}),
                          create_cube({0, half_size, half_size}),
                          create_cube({half_size, 0, 0}),
                          create_cube({half_size, 0, half_size}),
                          create_cube({half_size, half_size, 0}),
                          create_cube({half_size, half_size, half_size})};
        break;
    }
    if (new_type != Type::OCTANT) {
        this->remove_childs();
    }
    this->m_type = new_type;
    // TODO: clean up if whole octant is empty, etc.
}

Cube::Type Cube::type() const noexcept {
    return this->m_type;
}

std::array<std::weak_ptr<Cube>, Cube::SUB_CUBES> Cube::childs() const {
    std::array<std::weak_ptr<Cube>, Cube::SUB_CUBES> weak_childs;
    if (this->m_type != Type::OCTANT) {
        return weak_childs;
    }
    std::transform(this->m_childs.begin(), this->m_childs.end(), weak_childs.begin(),
                   [](const std::shared_ptr<Cube> &cube) { return std::weak_ptr<Cube>(cube); });
    return weak_childs;
}

std::array<Indentation, 12> Cube::indentations() const noexcept {
    return this->m_indentations;
}

void Cube::indent(const std::uint8_t edge_id, const bool positive_direction, const std::uint8_t steps) {
    if (this->m_type != Type::NORMAL) {
        return;
    }
    assert(edge_id >= 0 && edge_id <= Indentation::MAX);
    if (positive_direction) {
        this->m_indentations[edge_id].indent_start(steps);
    } else {
        this->m_indentations[edge_id].indent_end(steps);
    }
}

void Cube::set_sub_cube_type(const std::uint8_t cube_id, const Type new_type) {
    if (this->m_type != Type::OCTANT) {
        return;
    }
    assert(cube_id >= 0 && cube_id <= Cube::SUB_CUBES);
    this->m_childs[cube_id]->set_type(new_type);
}

std::vector<std::array<glm::vec3, 3>> Cube::polygons() const {
    std::vector<std::array<glm::vec3, 3>> polygons;
    switch (this->m_type) {
    case Type::EMPTY:
        return polygons;
        break;
    case Type::SOLID:
        break;
    case Type::NORMAL:
        break;
    case Type::OCTANT:
        break;
    }
    return {};
}
} // namespace inexor::vulkan_renderer::world
