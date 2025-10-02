#pragma once

#include "ant.hpp"
#include "nest.hpp"
#include "types.hpp"

#include <cstddef>
#include <random>
#include <vector>
#include <span>

#include <experimental/mdspan>

namespace ant_sim {

namespace stdex = std::experimental;

class simulation;

class world {
  public:
    struct tile {
        // TODO: Remove this definition, merge with the one in simulation.hpp
        static constexpr std::size_t pheromone_type_count = 2;

        static constexpr nest_id_t max_nests = 2;

        // Using struct of arrays instead of array of structs decreases the size of tile
        struct pheromone_trails {
            tick_t last_updated[max_nests][pheromone_type_count];
            pheromone_strength_t pheromone_strength[max_nests][pheromone_type_count];
        };

        ant_id_t ant_id; // Meaningless if has_ant is false
        nest_id_t nest_id; // Meaningless if has_nest is false

        bool has_ant;
        bool has_nest;

        std::uint8_t food_supply;

        pheromone_trails pheromones;

        [[nodiscard]] bool is_full() const noexcept { return !has_nest && has_ant; }
    };

  private:
    std::size_t rows;
    std::size_t columns;

    std::vector<tile> tiles;

    std::vector<ant> ants;
    std::vector<nest> nests;

    void generate(nest_id_t nest_count, ant_id_t ant_count);

  public:
    std::default_random_engine rand;

    // TODO: Merge simulation and world classes
    simulation* sim;

    world(std::size_t rows, std::size_t columns, simulation* sim, nest_id_t nest_count = 1, ant_id_t ant_count = 1);

    // Returns a rows x columns std::mdspan referring to tiles
    [[nodiscard]] auto get_tiles(this auto&& self) noexcept {
        return stdex::mdspan{self.tiles.data(), self.rows, self.columns};
    }

    // Returns a std::span referring to ants
    [[nodiscard]] auto get_ants(this auto&& self) noexcept { return std::span{self.ants}; }

    // Returns a std::span referring to nests
    [[nodiscard]] auto get_nests(this auto&& self) noexcept { return std::span{self.nests}; }

    // Updates the strength of the pheromone trails to account for fading over time
    static void update_pheromones(tile::pheromone_trails& pheromone_trails, tick_t current_tick, nest_id_t nest_id);
};

} // namespace ant_sim
