#pragma once

#include "inexor/vulkan-renderer/world/indentation.hpp"

#include <glm/vec3.hpp>

#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <vector>

namespace inexor::vulkan_renderer::world {
using Polygon = std::array<glm::vec3, 3>;

class Cube : public std::enable_shared_from_this<Cube> {
public:
    static constexpr std::size_t SUB_CUBES = 8;
    static constexpr std::size_t EDGES = 12;
    enum class Type { EMPTY = 0b00, SOLID = 0b01, NORMAL = 0b10, OCTANT = 0b11 };

private:
    Type m_type = Type::SOLID;
    float m_size = 32;
    glm::vec3 m_position = glm::vec3{0.0F, 0.0F, 0.0F};

    /// Root cube points to itself.
    std::shared_ptr<Cube> m_parent = this->shared_from_this();

    std::array<Indentation, Cube::EDGES> m_indentations = {};
    std::array<std::shared_ptr<Cube>, Cube::SUB_CUBES> m_childs = {};

    void remove_childs();

    [[nodiscard]] std::weak_ptr<Cube> root() const noexcept;

    [[nodiscard]] std::array<glm::vec3, 8> vertices() const noexcept;

public:
    Cube() = default;
    /// Not implemented.
    Cube(const Cube &) = delete;
    /// Not implemented.
    Cube &operator=(Cube const &) = delete;
    Cube(std::shared_ptr<Cube> parent, const Type type, const float size, const glm::vec3 &position);
    ~Cube();

    [[nodiscard]] bool is_root() const noexcept;
    [[nodiscard]] std::size_t grid_level() const noexcept;

    void set_type(const Type new_type);
    [[nodiscard]] Type type() const noexcept;

    [[nodiscard]] std::array<Indentation, Cube::EDGES> indentations() const noexcept;
    [[nodiscard]] std::array<std::weak_ptr<Cube>, Cube::SUB_CUBES> childs() const;

    void indent(const std::uint8_t edge_id, const bool positive_direction, const std::uint8_t steps);
    /// Short for childs(), weak_ptr checking and set_type().
    void set_sub_cube_type(const std::uint8_t cube_id, const Type new_type);

    /// Not implemented.
    [[deprecated("not implemented")]] [[nodiscard]] std::array<std::weak_ptr<Cube>, 25> get_neighbours() const;
    /// Not implemented.
    [[deprecated("not implemented")]] [[nodiscard]] std::weak_ptr<Cube> get_neighbour(const glm::vec3 &position) const;

    /// Get the cube you were looking at it's geometry.
    [[deprecated("not implemented")]] [[nodiscard]] std::weak_ptr<Cube> look_at_geometry(const glm::vec3 &position, const glm::vec3 &direction) const;
    /// Gets the smallest cube, where the point is inside the geometry.
    [[deprecated("not implemented")]] [[nodiscard]] std::weak_ptr<Cube> point_in_geometry(const glm::vec3 &position) const;
    /// Get the smallest cube, where the point is inside the cubes bounding box.
    [[deprecated("not implemented")]] [[nodiscard]] std::weak_ptr<Cube> point_in_cube(const glm::vec3 &position) const;

    [[nodiscard]] std::vector<std::array<glm::vec3, 3>> polygons() const;
};
} // namespace inexor::vulkan_renderer::world
