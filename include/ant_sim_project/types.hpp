#pragma once

#include <cstddef>
#include <cstdint>

namespace ant_sim {

// The type used to store the current tick count
using tick_t = std::uint32_t;

// The type used to represent nest ids
using nest_id_t = std::uint8_t;

// The type used to represent ant ids
using ant_id_t = std::uint32_t;

using pheromone_strength_t = float;

// This type is used to store the amount of food contained in a tile or in an ant's inventory
using food_supply_t = float;

// A 2-dimensional point
template<typename T = std::size_t>
struct point {
    T x;
    T y;

    bool operator==(const point&) const = default;
};

}