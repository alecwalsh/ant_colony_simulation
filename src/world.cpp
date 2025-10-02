#include "world.hpp"

#include <stdexcept>
#include <format>
#include <print>

namespace ant_sim {

void world::generate(nest_id_t nest_count, ant_id_t ant_count) {
    auto tiles = get_tiles();

    std::uniform_int_distribution<std::size_t> location_dist{0, 100};

    // Randomly place the nests across the world
    for(nest_id_t i = 0; i < nest_count; i++) {
        auto& nest = nests.emplace_back(i);

        auto x = location_dist(rand);
        auto y = location_dist(rand);

        tiles[y, x].has_nest = true;
        tiles[y, x].nest_id = i;

        nest.location = {x, y};

        std::println("Nest {} placed at {{{}, {}}}", i, y, x);
    }

    // Randomly place ants around the world
    for(auto& nest : nests) {
        for(auto i = 0uz; i < ant_count; i++) {
            // clang-format off
            ants.push_back({
                .nest_id = nest.nest_id,
                .ant_id = static_cast<ant_id_t>(nest.nest_id * ant_count + i),
                .caste = ant::caste::queen,
                .location = nest.location
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
world::world(std::size_t rows, std::size_t columns, nest_id_t nest_count, ant_id_t ant_count)
    : rows{rows}, columns{columns}, tiles(rows * columns), rand{std::random_device{}()} {

    if(nest_count > tile::max_nests) {
        auto error_string = std::format("Error: {} nests is greater than the maximum of {}", nest_count, tile::max_nests);
        throw std::runtime_error{error_string};
    }

    nests.reserve(nest_count);
    ants.reserve(ant_count);

    generate(nest_count, ant_count);
}

} // namespace ant_sim
