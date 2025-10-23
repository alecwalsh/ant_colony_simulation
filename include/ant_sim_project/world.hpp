#pragma once

#include "ant.hpp"
#include "nest.hpp"
#include "tile.hpp"
#include "types.hpp"

#include <cstddef>
#include <random>
#include <vector>
#include <unordered_map>
#include <span>

#include <experimental/mdspan>

namespace ant_sim {

namespace stdex = std::experimental;

class simulation;

class world {
    std::size_t rows;
    std::size_t columns;

    std::vector<tile> tiles;

    std::unordered_map<ant_id_t, ant> ants;
    std::vector<nest> nests;

  public:
    // TODO: Merge simulation and world classes
    simulation* sim;

    world(std::size_t rows, std::size_t columns, simulation* sim, nest_id_t nest_count, ant_id_t ant_count_per_nest);

    // Returns a rows x columns std::mdspan referring to tiles
    [[nodiscard]] auto get_tiles(this auto&& self) noexcept {
        return stdex::mdspan{self.tiles.data(), self.rows, self.columns};
    }

    // Returns a reference to ants
    [[nodiscard]] auto& get_ants(this auto&& self) noexcept { return self.ants; }

    // Returns a std::span referring to nests
    [[nodiscard]] auto get_nests(this auto&& self) noexcept { return std::span{self.nests}; }

    // Updates the strength of the pheromone trails to account for fading over time
    static void update_pheromones(tile::pheromone_trails& pheromone_trails, tick_t current_tick, nest_id_t nest_id);

    void generate(nest_id_t nest_count, ant_id_t ant_count);
};

} // namespace ant_sim
