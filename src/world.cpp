#include "world.hpp"

#include "simulation.hpp"

#include <stdexcept>
#include <format>
#include <print>

namespace ant_sim {
void world::generate(nest_id_t nest_count, ant_id_t ant_count) {
    auto tiles = get_tiles();

    std::uniform_int_distribution<std::size_t> location_dist_x{0, tiles.extent(1) - 1};
    std::uniform_int_distribution<std::size_t> location_dist_y{0, tiles.extent(0) - 1};

    // Randomly place the nests across the world
    for(nest_id_t i = 0; i < nest_count; i++) {
        auto& nest = nests.emplace_back(i);

        auto x = location_dist_x(rand);
        auto y = location_dist_y(rand);

        tiles[y, x].has_nest = true;
        tiles[y, x].nest_id = i;

        nest.location = {x, y};

        std::println("Nest {} placed at {{{}, {}}}", i, y, x);
    }

    // Fill nests with ants
    for(auto& nest : nests) {
        for(auto i = 0uz; i < ant_count; i++) {
            // Each nest has a single queen
            auto caste = i == 0 ? ant::caste::queen : ant::caste::worker;

            // clang-format off
            ants.push_back({
                .nest_id = nest.nest_id,
                .ant_id = static_cast<ant_id_t>(nest.nest_id * ant_count + i),
                .caste = caste,
                .location = nest.location,
                .state = ant::state::searching,
                .hunger = 0
            });
            // clang-format on

            nest.ant_count++;

            auto& tile = tiles[nest.location.y, nest.location.x];
            tile.has_ant = true;
            tile.ant_id = ants.back().ant_id;
        }
    }

    // Randomly place food across the world
    std::uniform_real_distribution<float> food_dist{};

    for(auto y = 0uz; y < tiles.extent(0); y++) {
        for(auto x = 0uz; x < tiles.extent(1); x++) {
            if(food_dist(rand) < 0.01f) {
                tiles[y, x].food_supply = 255;
            }
        }
    }
}

// TODO: allow specifying RNG seed
world::world(std::size_t rows, std::size_t columns, simulation* sim, nest_id_t nest_count, ant_id_t ant_count)
: rows{rows}, columns{columns}, tiles(rows * columns), rand{std::random_device{}()}, sim{sim} {
    if(nest_count > tile::max_nests) {
        auto error_string = std::format("Error: {} nests is greater than the maximum of {}", nest_count, tile::max_nests);
        throw std::runtime_error{error_string};
    }

    nests.reserve(nest_count);
    ants.reserve(ant_count);

    generate(nest_count, ant_count);
}

void world::update_pheromones(tile::pheromone_trails& pheromone_trails, tick_t current_tick, nest_id_t nest_id) {
    for(auto i = 0uz; i < pheromone_type_count; i++) {
        auto& strength = pheromone_trails.pheromone_strength[nest_id][i];
        auto& last_updated = pheromone_trails.last_updated[nest_id][i];

        auto ticks_since_last_update = static_cast<float>(current_tick - last_updated);

        last_updated = current_tick;

        auto decrease = falloff_rate * ticks_since_last_update;

        if(decrease > 0.1f) {
            std::println("decrease: {}", decrease);
        }

        if(falloff_rate * decrease > strength) {
            strength = 0;
        } else {
            strength -= static_cast<pheromone_strength_t>(decrease);
        }
    }
};

} // namespace ant_sim
