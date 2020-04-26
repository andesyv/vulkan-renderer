//
// Created by moritz on 21.04.20.
//

#ifndef INEXOR_VULKAN_RENDERER_CUBE_H
#define INEXOR_VULKAN_RENDERER_CUBE_H


#include <optional>
#include <iostream>
#include <functional>

#include <boost/dynamic_bitset.hpp>
#include <boost/signals2.hpp>

#include <glm/vec3.hpp>

#include "BitStream.h"

using std::array;
using std::vector;
using std::optional;
using std::nullopt;
using std::shared_ptr;
using boost::dynamic_bitset;
using boost::signals2::signal;

// Values connected to corners of cubes are saved in the following order.
//
// | Order | X      | Y      | Z      |
// |-------|--------|--------|--------|
// | 1.    | lower  | lower  | lower  |
// | 2.    | lower  | lower  | higher |
// | 3.    | lower  | higher | lower  |
// | 4.    | lower  | higher | higher |
// | 5.    | higher | lower  | lower  |
// | 6.    | higher | lower  | higher |
// | 7.    | higher | higher | lower  |
// | 8.    | higher | higher | higher |

// Values connected to sides of cubes are saved in the following order.
//
// | Order | X      | Y      | Z      |
// |-------|--------|--------|--------|
// | 1.    | axis   | axis   | lower  |
// | 2.    | axis   | axis   | higher |
// | 3.    | axis   | lower  | axis   |
// | 4.    | axis   | higher | axis   |
// | 5.    | lower  | axis   | axis   |
// | 6.    | higher | axis   | axis   |

namespace inexor::world {

enum class CubeType {
    /**
     * The types a cube can have and its bit-representation.
     */

    /**
     * The cube has no surface and no vertices.
     */
    EMPTY = 0b00,

    /**
     * The cube is a "real" cube where each edge has the same length.
     */
    FULL = 0b01,

    /**
     * The cube has at least on edge which has been indented from at least one axis.
     * That means that the cube may or may not be a perfect cube with same-length edges.
     */
    INDENTED = 0b10,

    /**
     * The cube is divided in 8 octants (same-sized cubes).
     */
    OCTANTS = 0b11
};

class Indentation {
public:
    /**
     * Create an Indentation to assign to a cube corner.
     */
    Indentation();

    /**
     * Create an Indentation to assign to a cube corner.
     * @param x Indentation leven on the x-axis
     * @param y Indentation leven on the y-axis
     * @param z Indentation leven on the z-axis
     */
    explicit Indentation(uint8_t x, uint8_t y, uint8_t z);

    /**
     * Signal emitted when one of the indentation levels changes.
     * Argument: the indentation emitting the signal ("this").
     */
    signal<void (Indentation *)> on_change;

    /**
     * Set the indentations depth for the axis.
     * @param x Indentation leven on the x-axis.
     * @param y Indentation leven on the y-axis.
     * @param z Indentation leven on the z-axis.
     */
    void set(optional<uint8_t> x, optional<uint8_t> y, optional<uint8_t> z);

    /**
     * Set the indentation level for the x axis.
     * @param x Indentation level on the x axis.
     */
    void set_x(uint8_t x);

    /**
     * Set the indentation level for the y axis.
     * @param x Indentation level on the y axis.
     */
    void set_y(uint8_t y);

    /**
     * Set the indentation level for the z axis.
     * @param x Indentation level on the z axis.
     */
    void set_z(uint8_t z);

    /**
     * Parse an indentation from a bitstream.
     * @param stream The stream to extract the bitstream from.
     * @return The indentation on all three axes.
     */
    static Indentation parse(BitStream &stream);

    /**
     * Get the x-axis indentation level.
     * @return the x-axis indentation level.
     */
    [[nodiscard]] uint8_t x() const;

    /**
     * Get the y-axis indentation level.
     * @return the y-axis indentation level.
     */
    [[nodiscard]] uint8_t y() const;

    /**
     * Get the z-axis indentation level.
     * @return the z-axis indentation level.
     */
    [[nodiscard]] uint8_t z() const;

    /**
     * Get the indentation levels on all three axes as a glm::tvec3<uint8_t>.
     * @return all three axes as a glm::tvec3<uint8_t>
     */
    [[nodiscard]] glm::tvec3<uint8_t> vec() const;

    // TODO: Get bits from values.
    // [[nodiscard]] dynamic_bitset<> bits();

private:
    /**
     * Run on-change events.
     */
    void _change();

    /**
     * Parse one axis from a BitStream.
     * @param stream The BitStream to parse one axis from.
     * @return The indentation level on that axis.
     */
    static uint8_t _parse_one(BitStream &stream);

    /**
     * Indentation level on the x-axis.
     */
    uint8_t _x = 0;

    /**
     * Indentation level on the y-axis.
     */
    uint8_t _y = 0;

    /**
     * Indentation level on the z-axis.
     */
    uint8_t _z = 0;
};

class Cube {
    /**
     * A cube or octree representing the maps geometry.
     */

public:

    /**
     * Signal emitted when any of the geometry of this cube or its child cubes changes.
     * Argument: the cube which was originally changed ("this" or a child-cube).
     */
    signal<void (Cube *)> on_change;

    /**
     * The indentations of this cube if this cube is of CubeType::INDENTED.
     * Ordered as following:
     *  0. Corner with lower x-axis-value, lower y-value, lower z-value.
     *  1. Corner with lower x-axis-value, lower y-value, higher z-value.
     *  2. Corner with lower x-axis-value, higher y-value, lower z-value.
     *  3. Corner with lower x-axis-value, higher y-value, higher z-value.
     *  4. Corner with higher x-axis-value, lower y-value, lower z-value.
     *  5. Corner with higher x-axis-value, lower y-value, higher z-value.
     *  6. Corner with higher x-axis-value, higher y-value, lower z-value.
     *  7. Corner with higher x-axis-value, higher y-value, higher z-value.
     */
    optional<array<Indentation, 8>> indentations = nullopt;

    /**
     * The octants of this cube if this cube is of CubeType::OCTANTS.
     * Ordered as following:
     *  0. Octant with lower x-axis-value, lower y-value, lower z-value.
     *  1. Octant with lower x-axis-value, lower y-value, higher z-value.
     *  2. Octant with lower x-axis-value, higher y-value, lower z-value.
     *  3. Octant with lower x-axis-value, higher y-value, higher z-value.
     *  4. Octant with higher x-axis-value, lower y-value, lower z-value.
     *  5. Octant with higher x-axis-value, lower y-value, higher z-value.
     *  6. Octant with higher x-axis-value, higher y-value, lower z-value.
     *  7. Octant with higher x-axis-value, higher y-value, higher z-value.
     */
    optional<array<shared_ptr<Cube>, 8>> octants = nullopt;

    /**
     * Create a cube.
     * @param type The type of the cube. The cube needs further adjustment after construction if not of CubeType::FULL or CubeType::EMPTY.
     * @param size The maximum size of the cube.
     * @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
     * x, y, and z-axis).
     */
    Cube(CubeType type, float size, const glm::vec3 &position);

    /**
     * Create a CubeType::INDENTED cube.
     * @param indentations The indentations of the cube.
     * @param size The maximum size of the cube.
     * @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
     * x, y, and z-axis).
     */
    Cube(array<Indentation, 8> &indentations, float size, const glm::vec3 &position);

    /**
     * Create a CubeType::Octants cube.
     *
     * @param octants The octants (i.e. sub-cubes) of the cube.
     * @param size The maximum size of the cube.
     * @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
     * x, y, and z-axis).
     */
    Cube(array<shared_ptr<Cube>, 8> &octants, float size, const glm::vec3 &position);

    /**
     * Parse an octree from binary data.
     * @param data The data to parse the octree from.
     * @return Cube object representing the cubes / octrees from the data.
     */
    static Cube parse(vector<unsigned char> &data);

    /**
     * Parse an octree from a BitStream.
     * @param stream The BitStream to parse the octree from.
     * @return Cube object representing the cubes / octrees from the stream.
     */
    static Cube parse(BitStream &stream);

    /**
     * Parse an octree from a BitStream.
     * @param stream The BitStream to parse the octree from.
     * @param size The maximum size of the cube.
     * @param position The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
     * x, y, and z-axis).
     * @return Cube object representing the cubes / octrees from the stream.
     */
    static Cube parse(BitStream &stream, float size, const glm::vec3 &position);

    /**
     * Get the type of the cube.
     * @return type of the cube.
     */
    [[nodiscard]] CubeType type();

    /**
     * Get the number of leaves, this octree contains.
     * Leaves are cubes of CubeType::INDENTED or CubeTYPE::FULL.
     * @return Number of leaves, this octree contains.
     */
    [[nodiscard]] uint64_t leaves();

    // [[nodiscard]] dynamic_bitset<> bits(); // Bit representation of this cube
    // [[nodiscard]] vector<array<glm::vec3, 8>> vertices(); // All vertices this cube contains.
    // [[nodiscard]] vector<array<glm::vec3, 4>> sides(); // All sides this cube contains.
    /**
     * Get all polygons (triangles) of each cube of this octree.
     * @return A vector which contains the three vertices representing a triangle.
     */
    [[nodiscard]] vector<array<glm::vec3, 3>> polygons(); // All polygons this cube contains.

    /**
     * Invalidate the cache of this cube / octree (not its children).
     */
    void invalidate_cache();

private:

    /**
     * Insert all polygons into memory.
     * @param polygons Pointer to the memory where the polygons should be saved to.
     */
    void _all_polygons(array<glm::vec3, 3>* &polygons);

    /**
     * Run on-change events.
     */
    void _change();

    /**
     * Run on-change events.
     */
    void _change(Indentation *indentation);

    /**
     * Get all vertices of this cube (not its children).
     * @return vertices of this cube.
     */
    [[nodiscard]] array<glm::vec3, 8> _vertices();

    /**
     * Get the polygons of this cube as if it is a full cube.
     * @return polygons of this cube as if it is a full cube
     */
    array<array<glm::vec3, 3>, 12> _full_polygons();

    /**
     * Get the polygons of this cube (only when it is an indented cube).
     * @return polygons of this cube
     */
    array<array<glm::vec3, 3>, 12> _indented_polygons();

    /**
     * Cache of this cubes polygons. Not of its octants (i.e., empty of the cube is of type CubeType::OCTANTS).
     */
    array<array<glm::vec3, 3>, 12> _polygons_cache = {}; // Vertices of this cube (not its octants)

    /**
     * Whether this->_polygons_cache is valid and might be used.
     */
    bool _valid_cache = false;

    /**
     * Type of the cube.
     */
    CubeType _type = CubeType::EMPTY;

    /**
     * The position of the cube in the coordinate system (i.e., the vector from (0, 0, 0) to the bounds of the cube with the lowest values on
     * x, y, and z-axis).
     */
    glm::vec3 _position = {0.0f, 0.0f, 0.0f};

    /**
     * The maximum size of the cube (i.e. if the cube is not indented).
     */
    float _size = 32;
};
}

#endif // INEXOR_VULKAN_RENDERER_CUBE_H
