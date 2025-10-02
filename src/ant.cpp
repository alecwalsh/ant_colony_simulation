#include "ant.hpp"

#include "world.hpp"

#include <array>
#include <optional>
#include <cassert>

namespace ant_sim {

// Return the neighboring points as an array of std::optional<point>
// If the neighbor would have been out of bounds, that array element is std::nullopt
constexpr auto get_neighbors(world& world, point location) noexcept {
    auto [x, y] = location;

    std::array<std::optional<point>, 8> arr = {};

    auto max_x = world.get_tiles().extent(1) - 1;
    auto max_y = world.get_tiles().extent(0) - 1;

    if(x != 0) {
        if(y != 0) {
            arr[0] = point{x - 1, y - 1};
        }

        arr[1] = point{x - 1, y};

        if(y != max_y) {
            arr[2] = point{x - 1, y + 1};
        }
    }

    if(x != max_x) {
        if(y != 0) {
            arr[3] = point{x + 1, y - 1};
        }

        arr[4] = point{x + 1, y};

        if(y != max_y) {
            arr[5] = point{x + 1, y + 1};
        }
    }

    if(y != 0) {
        arr[6] = point{x, y - 1};
    }

    if(y != max_y) {
        arr[7] = point{x, y + 1};
    }

    return arr;
}

// Move the ant to a new location
// This updates the ant's location field, the has_ant field in the starting tile and destination tile
// and updates the strength of the pheromone trails
void ant::move(world& world, point new_location) {
    auto tiles = world.get_tiles();

    auto& current_tile = tiles[location.y, location.x];
    auto& new_tile = tiles[new_location.y, new_location.x];

    assert(!new_tile.is_full()); // Can't have multiple ants per tile unless the tile is a nest

    auto nests = world.get_nests();

    if(current_tile.has_nest) {
        // Nests can hold multiple ants
        auto& nest = nests[current_tile.nest_id];

        assert(nest.ant_count != 0);
        if(--nest.ant_count == 0) {
            current_tile.has_ant = false;
        }
    } else {
        current_tile.has_ant = false;
    }

    if(new_tile.has_nest) {
        // Nests can hold multiple ants, so increment this nest's ant count
        auto& nest = nests[new_tile.nest_id];

        nest.ant_count++;
    }

    new_tile.has_ant = true;
    new_tile.ant_id = ant_id;

    location = new_location;
}

void ant::tick(world& world) {
    auto tiles = world.get_tiles();

    auto neighbors = get_neighbors(world, location);

    for(auto& neighbor : neighbors) {
        if(!neighbor) continue;

        auto [x, y] = *neighbor;

        if(!tiles[y, x].has_ant && y > location.y) {
            move(world, *neighbor);
            return;
        }
    }
}

} // namespace ant_sim
