#pragma once

#include <cstddef>
#include <cstdint>

#include "types.hpp"

namespace ant_sim {

struct tile {
    static constexpr std::size_t pheromone_type_count = 2;

    static constexpr nest_id_t max_nests = 2;

    // Using struct of arrays instead of array of structs decreases sizeof(tile)
    struct pheromone_trails {
        tick_t last_updated[max_nests][pheromone_type_count];
        pheromone_strength_t pheromone_strength[max_nests][pheromone_type_count];
    };

    ant_id_t ant_id;   // Meaningless if has_ant is false
    nest_id_t nest_id; // Meaningless if has_nest is false

    bool has_ant;
    bool has_nest;

    food_supply_t food_supply;

    pheromone_trails pheromones;

    [[nodiscard]] bool is_full() const noexcept { return !has_nest && has_ant; }
};

}