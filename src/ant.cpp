#include "ant.hpp"

#include "simulation.hpp"
#include "world.hpp"

#include <array>
#include <optional>
#include <ranges>
#include <cassert>

#include <print>

namespace ant_sim {

constexpr std::size_t max_neighbors = 8;

// Return the neighboring points as an array of std::optional<point>
// If the neighbor would have been out of bounds, that array element is std::nullopt
constexpr auto get_neighbors(world& world, point<> location) noexcept {
    auto [x, y] = location;

    std::array<std::optional<point<>>, max_neighbors> arr = {};

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
void ant::move(world& world, point<> new_location) {
    assert(caste != caste::queen);

    // Moving to the current location is a noop
    if(new_location == location) return;

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

    for(auto i = 0uz; i < pheromone_type_count; i++) {
        assert(new_tile.pheromones.last_updated[nest_id][i] == world.sim->get_tick_count());
    }
    // Apply pheromone trails
    current_tile.pheromones.pheromone_strength[nest_id][std::to_underlying(state)] += increase_rate;

    // Switch state to returning to nest
    // TODO: Collect food
    if(new_tile.food_supply != 0) {
        std::println("Ant {} switching state to returning", ant_id);
        state = state::returning;
    }

    // Ant has returned to its nest
    // Set state to searching
    // TODO: Deposit food in nest
    if(new_tile.has_nest && new_tile.nest_id == nest_id) {
        std::println("Ant {} switching state to searching", ant_id);
        state = state::searching;

        // Nests can hold multiple ants, so increment this nest's ant count
        auto& nest = nests[new_tile.nest_id];

        nest.ant_count++;
    }

    new_tile.has_ant = true;
    new_tile.ant_id = ant_id;

    location = new_location;
}


// Calculate the weight for a tile, from the perspective of current_ant
float ant::calculate_tile_weight(const tile& tile, world& world) noexcept {
    float multiplier = state == state::searching ? 1 : -1;

    // Tile has food
    // If searching, assign the highest possible weight to ensure that this tile is preferred
    // If returning, assign the lowest possible weight to ensure that this tile is avoided
    if(tile.food_supply != 0) {
        return std::numeric_limits<float>::infinity() * multiplier;
    }

    // Tile is the ant's nest
    // If returning, assign the highest possible weight to ensure that this tile is preferred
    // If searching, assign the lowest possible weight to ensure that this tile is avoided
    if(tile.has_nest && tile.nest_id == nest_id) {
        return -std::numeric_limits<float>::infinity() * multiplier;
    }

    float type1_strength = tile.pheromones.pheromone_strength[nest_id][0];
    float type2_strength = tile.pheromones.pheromone_strength[nest_id][1];

    // Apply some randomization to the pheromone strengths
    std::uniform_real_distribution dist1{add_random_range.first, add_random_range.second};
    std::uniform_real_distribution dist2{mul_random_range.first, mul_random_range.second};

    type1_strength += dist1(world.rand);
    type2_strength += dist1(world.rand);

    type1_strength *= dist2(world.rand);
    type2_strength *= dist2(world.rand);

    if(state == state::searching) {
        type1_strength *= -0.5f;
        // type2_strength *= 1;
    } else if(state == state::returning) {
        // type1_strength *= 1;
        type2_strength *= -0.5f;
    }

    return type1_strength + type2_strength;
}

// Returns the location this ant will move to, if such a location exists
std::optional<point<>> ant::calculate_next_location(world& world) {
    auto tiles = world.get_tiles();

    auto neighboring_points = get_neighbors(world, location);

    auto has_value = []<typename T>(const std::optional<T>& opt) { return opt.has_value(); };

    auto current_tick = world.sim->get_tick_count();

    struct result_t {
        point<> location;
        float weight;
    };

    std::optional<result_t> results[max_neighbors] = {};

    for(auto i = 0uz; i < std::size(results); i++) {
        auto& neighbor = neighboring_points[i];

        if(!neighbor) continue;

        auto& tile = tiles[neighbor->y, neighbor->x];

        // Ignore tiles that are already full
        if(tile.is_full()) continue;

        world::update_pheromones(tile.pheromones, current_tick, nest_id);

        float weight = calculate_tile_weight(tile, world);

        results[i] = {.location = *neighbor, .weight = weight};
    }

    // Exclude any out of bounds or full results
    auto possible_results = results | std::views::filter(has_value);

    // All possible locations are full, no movement is possible
    if(std::ranges::empty(possible_results)) return {};

    auto [new_location, weight] = **std::ranges::max_element(possible_results, {}, &result_t::weight);

    assert(!(tiles[new_location.y, new_location.x].is_full()));

    std::println("moving ant {} to {{{}, {}}} with weight {}", ant_id, new_location.x, new_location.y, weight);

    return new_location;
}

// Determines and executes the ant's next action
// When the ant is searching for food it will avoid tiles with type 1 pheromones and prefer tiles with type 2 pheromones
// It increases the strength of the type 1 pheromone on the tile it is leaving
// When the ant is returning to the nest it behaves the same, but with its pheromone preferences flipped
void ant::tick(world& world) {
    switch(caste) {
    case caste::queen:
        break;
    case caste::worker: {
        auto new_location = calculate_next_location(world).value_or(location);

        move(world, new_location);

        break;
    }
    default:
        std::unreachable();
    }
}

} // namespace ant_sim
