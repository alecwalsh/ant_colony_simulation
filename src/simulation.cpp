#include "simulation.hpp"

#include <atomic>
#include <thread>

#include <print>
#include <ranges>

namespace ant_sim {

std::minstd_rand get_rng(std::optional<std::uint64_t> seed) {
    std::uint32_t seed_parts[2];

    if(seed) {
        // std::seed_seq only uses the low 32 bits of each input, so break the seed into 2 32 bit values
        seed_parts[0] = *seed >> 32;
        seed_parts[1] = *seed & 0xFFFFFFFF;
    } else {
        std::random_device random_device;

        seed_parts[0] = random_device();
        seed_parts[1] = random_device();
    }

    std::seed_seq seed_seq{seed_parts[0], seed_parts[1]};

    std::println("Seed,{}", static_cast<std::uint64_t>(seed_parts[0]) << 32 | seed_parts[1]);

    return std::minstd_rand{seed_seq};
}

simulation::simulation(std::size_t rows, std::size_t columns, nest_id_t nest_count, ant_id_t ant_count_per_nest,
                       std::optional<std::uint64_t> seed)
    : rng{get_rng(seed)}, rows{rows}, columns{columns}, tiles(rows * columns) {
    if(nest_count > tile::max_nests) {
        auto error_string =
            std::format("Error: {} nests is greater than the maximum of {}", nest_count, tile::max_nests);
        throw std::runtime_error{error_string};
    }

    nests.reserve(nest_count);
    ants.reserve(ant_count_per_nest);

    generate(nest_count, ant_count_per_nest);
}

simulation::simulation(simulation_args_t args)
    : simulation{args.rows, args.columns, args.nest_count, args.ant_count_per_nest, args.seed} {
    hunger_increase_per_tick = args.hunger_increase_per_tick;
    hunger_to_die = args.hunger_to_die;
    food_taken = args.food_taken;
    food_resupply_rate = args.food_resupply_rate;
    max_food_supply = args.max_food_supply;
    food_per_new_ant = args.food_per_new_ant;
    food_hunger_ratio = args.food_hunger_ratio;
    falloff_rate = args.falloff_rate;
    increase_rate = args.increase_rate;
    type1_avoidance = args.type1_avoidance;
    type2_avoidance = args.type2_avoidance;
}

void simulation::queue_ant(nest_id_t nest_id) {
    // clang-format off
    new_ants.push_back({
        .nest_id = nest_id,
        .ant_id = next_id++,
        .caste = ant::caste::worker,
        .location = nests[nest_id].location
    });
    // clang-format on
}

void simulation::add_ant(ant new_ant) {
    auto [it, inserted] = ants.insert({new_ant.ant_id, new_ant});
    assert(inserted);

    nests[new_ant.nest_id].ant_count++;
}

void simulation::generate(nest_id_t nest_count, ant_id_t ant_count_per_nest) {
    auto tiles = get_tiles();

    std::uniform_int_distribution<std::size_t> location_dist_x{0, tiles.extent(1) - 1};
    std::uniform_int_distribution<std::size_t> location_dist_y{0, tiles.extent(0) - 1};

    // Randomly place the nests across the world
    for(nest_id_t i = 0; i < nest_count; i++) {
        auto& nest = nests.emplace_back(i);

        auto x = location_dist_x(rng);
        auto y = location_dist_y(rng);

        tiles[y, x].has_nest = true;
        tiles[y, x].nest_id = i;

        nest.location = {x, y};

        std::println("Nest,{},{},{}", i, y, x);
    }

    // Fill nests with ants
    for(auto& nest : nests) {
        for(auto i = 0uz; i < ant_count_per_nest; i++) {
            // Each nest has a single queen
            auto caste = i == 0 ? ant::caste::queen : ant::caste::worker;

            auto ant_id = static_cast<ant_id_t>(nest.nest_id * ant_count_per_nest + i);
            // clang-format off
            ants[ant_id] = {
                .nest_id = nest.nest_id,
                .ant_id = ant_id,
                .caste = caste,
                .location = nest.location,
                .state = ant::state::searching,
                .hunger = 0
            };
            // clang-format on

            nest.ant_count++;

            auto& tile = tiles[nest.location.y, nest.location.x];
            tile.has_ant = true;
            tile.ant_id = ant_id;
        }
    }

    next_id = ant_count_per_nest * nest_count;

    // Randomly place food across the world
    std::uniform_real_distribution<float> food_dist{};

    for(auto y = 0uz; y < tiles.extent(0); y++) {
        for(auto x = 0uz; x < tiles.extent(1); x++) {
            if(food_dist(rng) < food_chance) {
                food_sources.push_back({x, y});
                tiles[y, x].food_supply = 255;
                set_food_count(get_food_count() + tiles[y, x].food_supply);
            }
        }
    }
}

void simulation::update_pheromones(tile::pheromone_trails& pheromone_trails, tick_t current_tick, nest_id_t nest_id) {
    for(auto i = 0uz; i < tile::pheromone_type_count; i++) {
        auto& strength = pheromone_trails.pheromone_strength[nest_id][i];
        auto& last_updated = pheromone_trails.last_updated[nest_id][i];

        auto ticks_since_last_update = static_cast<float>(current_tick - last_updated);

        last_updated = current_tick;

        auto decrease = falloff_rate * ticks_since_last_update;

        if(falloff_rate * decrease > strength) {
            strength = 0;
        } else {
            strength -= static_cast<pheromone_strength_t>(decrease);
        }
    }
}

void simulation::tick() {
    if(paused()) return;

    std::println("Tick,{},{},{}", ants.size(), get_tick_count(), get_food_count());

    for(auto it = ants.begin(); it != ants.end();) {
        auto& ant = it->second;

        ant.tick(*this);

        if(ant.dead) {
            it = ants.erase(it);

            std::println("Death,{},{},{},{}", ant.ant_id, ant.nest_id, ant.location.x, ant.location.y);
        } else {
            ++it;
        }
    }

    for(auto& new_ant : new_ants) {
        std::println("Birth,{},{},{},{}", new_ant.ant_id, new_ant.nest_id, new_ant.location.x, new_ant.location.y);
        add_ant(new_ant);
    }

    new_ants.clear();

    for(auto [x, y] : food_sources) {
        auto old_food_supply = get_tiles()[y, x].food_supply;
        auto current_food_supply = get_tiles()[y, x].food_supply + food_resupply_rate;

        get_tiles()[y, x].food_supply = std::min(current_food_supply, max_food_supply);

        auto food_diff = get_tiles()[y, x].food_supply - old_food_supply;

        set_food_count(get_food_count() + food_diff);
    }

    ++std::atomic_ref{atomically_accessed.tick_count};

    if(get_state() == simulation_state::single_step) {
        pause(true);
    }
}

template <typename T>
// Atomically read a reference
// libc++ doesn't support atomic_ref<T> with const T yet
// This function works around it with a const_cast
// This is safe because no attempt is made to modify the value
T atomic_read(const T& t) noexcept {
    return std::atomic_ref{const_cast<T&>(t)};
}

simulation::simulation_state simulation::get_state() const noexcept { return atomic_read(atomically_accessed.state); }

void simulation::set_state(simulation_state new_state) noexcept {
    std::atomic_ref{atomically_accessed.state} = new_state;
}

bool simulation::stopped() const noexcept { return get_state() == simulation_state::stopped; }

void simulation::stop() noexcept { set_state(simulation_state::stopped); }

bool simulation::paused() const noexcept { return get_state() == simulation_state::paused; }

void simulation::pause(bool is_paused) noexcept {
    set_state(is_paused ? simulation_state::paused : simulation_state::running);
}

point<float> simulation::get_mouse_location() const noexcept { return atomic_read(atomically_accessed.mouse_location); }

void simulation::set_mouse_location(point<float> location) noexcept {
    std::atomic_ref{atomically_accessed.mouse_location} = location;
}

bool simulation::get_log_ant_movements() const noexcept { return atomic_read(atomically_accessed.log_ant_movements); }

void simulation::set_log_ant_movements(bool log_ant_movements) noexcept {
    std::atomic_ref{atomically_accessed.log_ant_movements} = log_ant_movements;
}

bool simulation::get_log_ant_state_changes() const noexcept {
    return atomic_read(atomically_accessed.log_ant_state_changes);
}

void simulation::set_log_ant_state_changes(bool log_ant_state_changes) noexcept {
    std::atomic_ref{atomically_accessed.log_ant_state_changes} = log_ant_state_changes;
}

tick_t simulation::get_tick_count() const noexcept { return atomic_read(atomically_accessed.tick_count); }

[[nodiscard]] float simulation::get_food_count() const noexcept {
    return atomic_read(atomically_accessed.food_count);
}

void simulation::set_food_count(float food_count) noexcept {
    std::atomic_ref{atomically_accessed.food_count} = food_count;
}

} // namespace ant_sim
