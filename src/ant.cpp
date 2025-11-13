#include "ant.hpp"

#include "simulation.hpp"

#include <array>
#include <optional>
#include <ranges>
#include <cassert>

#include <print>

namespace ant_sim {

constexpr std::size_t max_neighbors = 8;

// Return the neighboring points as an array of std::optional<point>
// If the neighbor would have been out of bounds, that array element is std::nullopt
constexpr auto get_neighbors(simulation& sim, point<> location) noexcept {
    auto [x, y] = location;

    std::array<std::optional<point<>>, max_neighbors> arr = {};

    auto max_x = sim.get_tiles().extent(1) - 1;
    auto max_y = sim.get_tiles().extent(0) - 1;

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
void ant::move(simulation& sim, point<> new_location) {
    assert(caste != caste::queen); // Queens should stay at their nest

    // Moving to the current location is a noop
    if(new_location == location) return;

    auto tiles = sim.get_tiles();

    auto& current_tile = tiles[location.y, location.x];
    auto& new_tile = tiles[new_location.y, new_location.x];

    assert(!new_tile.is_full()); // Can't have multiple ants per tile unless the tile is a nest

    auto nests = sim.get_nests();

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

    for(auto i = 0uz; i < tile::pheromone_type_count; i++) {
        assert(new_tile.pheromones.last_updated[nest_id][i] == sim.get_tick_count());
    }
    // Apply pheromone trails
    current_tile.pheromones.pheromone_strength[nest_id][std::to_underlying(state)] += sim.increase_rate;

    // Add some food to the inventory, then set state to returning to nest
    if(new_tile.food_supply != 0) {
        // Ensure that we don't take more food than the tile contains
        auto food_taken = std::min(sim.food_taken, new_tile.food_supply);

        // The maximum amount of food this ant's inventory has room for
        food_supply_t max_food_taken = std::numeric_limits<food_supply_t>::max() - food_in_inventory;

        // Ensure that we don't take more food than this ant has room for
        food_taken = std::min(food_taken, max_food_taken);

        new_tile.food_supply -= food_taken;
        food_in_inventory += food_taken;

        sim.set_food_count(sim.get_food_count() - food_taken);

        state = state::returning;

        if(sim.get_log_ant_state_changes()) {
            std::println("StateChange,Returning,{},{},{},{},{}", ant_id, nest_id, location.x, location.y, food_taken);
        }
    }

    // Ant has returned to its nest
    // Deposit food in the nest, then set state to searching
    if(new_tile.has_nest && new_tile.nest_id == nest_id) {
        auto& nest = nests[nest_id];

        // The maximum amount of food this nest's inventory has room for
        food_supply_t max_food_deposited = std::numeric_limits<food_supply_t>::max() - nest.food_supply;

        // Ensure that we don't take more food than this ant's inventory has room for
        auto food_deposited = std::min(food_in_inventory, max_food_deposited);

        nest.food_supply += food_deposited;
        food_in_inventory -= food_deposited;

        // Nests can hold multiple ants, so increment this nest's ant count
        nest.ant_count++;

        state = state::searching;

        if(sim.get_log_ant_state_changes()) {
            std::println("StateChange,Searching,{},{},{},{},{}", ant_id, nest_id, location.x, location.y, food_deposited);
        }
    }

    new_tile.has_ant = true;
    new_tile.ant_id = ant_id;

    location = new_location;
}

// Calculate the weight for a tile, from the perspective of current_ant
float ant::calculate_tile_weight(const tile& tile, simulation& sim) const noexcept {
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
    type1_strength += sim.add_dist(sim.rng);
    type2_strength += sim.add_dist(sim.rng);

    type1_strength *= sim.multiply_dist(sim.rng);
    type2_strength *= sim.multiply_dist(sim.rng);

    if(state == state::searching) {
        type1_strength *= -sim.type1_avoidance;
    } else if(state == state::returning) {
        type2_strength *= -sim.type2_avoidance;
    }

    return type1_strength + type2_strength;
}

// Returns the location this ant will move to, if such a location exists
std::optional<point<>> ant::calculate_next_location(simulation& sim) {
    auto tiles = sim.get_tiles();

    auto neighboring_points = get_neighbors(sim, location);

    auto has_value = []<typename T>(const std::optional<T>& opt) { return opt.has_value(); };

    auto current_tick = sim.get_tick_count();

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

        sim.update_pheromones(tile.pheromones, current_tick, nest_id);

        float weight = calculate_tile_weight(tile, sim);

        results[i] = {.location = *neighbor, .weight = weight};
    }

    // Exclude any out of bounds or full results
    auto possible_results = results | std::views::filter(has_value);

    // All possible locations are full, no movement is possible
    if(std::ranges::empty(possible_results)) return {};

    auto [new_location, weight] = **std::ranges::max_element(possible_results, {}, &result_t::weight);

    assert(!(tiles[new_location.y, new_location.x].is_full()));

    if(sim.get_log_ant_movements()) {
        std::println("Move,{},{},{},{}", ant_id, new_location.x, new_location.y, weight);
    }

    return new_location;
}

// Determines and executes the ant's next action
// When the ant is searching for food it will avoid tiles with type 1 pheromones and prefer tiles with type 2 pheromones
// It increases the strength of the type 1 pheromone on the tile it is leaving
// When the ant is returning to the nest it behaves the same, but with its pheromone preferences flipped
void ant::tick(simulation& sim) {
    switch(caste) {
    case caste::queen: {
        auto& nest = sim.get_nests()[nest_id];

        // Add a new ant at the cost of food
        if(nest.food_supply >= sim.food_per_new_ant) {
            sim.queue_ant(nest_id);

            nest.food_supply -= sim.food_per_new_ant;
        }

        break;
    }
    case caste::worker: {
        hunger += sim.hunger_increase_per_tick;

        if(food_in_inventory != 0) {
            auto food_needed = static_cast<food_supply_t>(hunger * sim.food_hunger_ratio);
            auto food_eaten = std::min(food_in_inventory, food_needed);

            food_in_inventory -= food_eaten;
            hunger -= static_cast<float>(food_eaten) / sim.food_hunger_ratio;
        }

        if(hunger >= sim.hunger_to_die) {
            // Mark ant as dead
            // It will be removed once this method returns
            assert((sim.get_tiles()[location.y, location.x].has_ant));

            dead = true;
            sim.get_tiles()[location.y, location.x].has_ant = false;

            sim.increment_deaths();

            return;
        }

        auto new_location = calculate_next_location(sim).value_or(location);

        move(sim, new_location);

        break;
    }
    default:
        std::unreachable();
    }
}

} // namespace ant_sim
