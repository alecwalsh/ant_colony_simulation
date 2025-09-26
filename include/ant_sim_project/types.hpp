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

using pheromone_strength_t = std::uint8_t;

// A 2-dimensional point
struct point {
    std::size_t x;
    std::size_t y;
};

}