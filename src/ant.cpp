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

void ant::move(world& world, point new_location) {
    auto tiles = world.get_tiles();

    auto& current_tile = tiles[location.y, location.x];
    auto& new_tile = tiles[new_location.y, new_location.x];

    assert(!new_tile.has_ant); // Can't have multiple ants per tile

    current_tile.has_ant = false;

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
