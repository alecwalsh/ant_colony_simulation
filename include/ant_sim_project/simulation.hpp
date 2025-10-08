#pragma once

#include <cstddef>
#include <mutex>
#include <atomic>
#include <random>
#include <unordered_map>

#include "tile.hpp"
#include "ant.hpp"
#include "nest.hpp"
#include "types.hpp"
#include "mutex_guard.hpp"

#include <experimental/mdspan>

namespace ant_sim {

namespace stdex = std::experimental;

// TODO: Support separate values for each pheromone type

class simulation {
  public:
    enum class simulation_state : std::uint8_t {
        running,     // The simulation is running
        stopped,     // The simulation has permanently stopped
        single_step, // The simulation is running, but will be automatically paused after the next tick
        paused       // The simulation is paused
    };

    float hunger_increase_per_tick = 1.0f;
    float hunger_to_die = 100.0f;
    food_supply_t food_taken = 5;         // The amount of food ants take when they encounter a food source
    food_supply_t food_per_new_ant = 150; // Food needed for a queen to produce a new ant
    float food_hunger_ratio = 1.0f;

    // How much a pheromone's strength decreases each tick
    float falloff_rate = 0.025f;

    // How much a pheromone's strength increases when left by an ant
    pheromone_strength_t increase_rate = 3 * 4;

    // These variables control how strongly ants avoid backtracking
    float type1_avoidance = 1.0f; // How strongly ants avoid type 1 pheromones when searching for food
    float type2_avoidance = 1.0f; // How strongly ants avoid type 2 pheromones when returning to their nest with food

    // A value from this distribution is added to pheromone strength when calculating weights
    std::uniform_real_distribution<float> add_dist{-0.1f, 0.1f};
    // Pheromone strength is multiplied by a value from this distribution when calculating weights
    std::uniform_real_distribution<float> multiply_dist{0.5f, 1.5f};

    std::minstd_rand rng;

    std::chrono::duration<float, std::milli> sleep_time{100};

  private:
    std::size_t rows;
    std::size_t columns;

    std::vector<tile> tiles;

    std::unordered_map<ant_id_t, ant> ants;
    std::vector<nest> nests;

    // All members of this struct must always be accessed via std::atomic_ref, as multiple threads may access them.
    // Each member is independent of the others.  There is no need to pass the entire struct to std::atomic_ref.
    struct {
        tick_t tick_count = 0;
        simulation_state state = simulation_state::running;

        using point_t = point<float>;
        // Ensure mouse_location is sufficiently aligned to use with std::atomic_ref
        alignas(std::atomic_ref<point_t>::required_alignment) point_t mouse_location = {0, 0};

        std::uint64_t food_count = 0;

        bool log_ant_movements = false;
        bool log_ant_state_changes = true;
    } atomically_accessed;

    // Holds new ants that have not yet been added to the simulation
    std::vector<ant> new_ants;

    // The lowest unused id
    ant_id_t next_id = 0;

  public:
    simulation(std::size_t rows, std::size_t columns, nest_id_t nest_count, ant_id_t ant_count_per_nest,
               std::optional<std::uint64_t> seed = {});

    [[nodiscard]] simulation_state get_state() const noexcept;
    void set_state(simulation_state new_state) noexcept;

    // Checks if state == simulation_state::stopped
    [[nodiscard]] bool stopped() const noexcept;
    // Sets state to simulation_state::stopped
    void stop() noexcept;

    // Checks if state == simulation_state::paused
    [[nodiscard]] bool paused() const noexcept;
    // Sets state to simulation_state::paused or simulation_state::running, depending on the argument's value
    void pause(bool is_paused = true) noexcept;

    point<float> get_mouse_location() const noexcept;
    void set_mouse_location(point<float> location) noexcept;

    bool get_log_ant_movements() const noexcept;
    void set_log_ant_movements(bool log_ant_movements) noexcept;

    bool get_log_ant_state_changes() const noexcept;
    void set_log_ant_state_changes(bool log_ant_state_changes) noexcept;

    [[nodiscard]] tick_t get_tick_count() const noexcept;

    [[nodiscard]] std::uint64_t get_food_count() const noexcept;
    void set_food_count(std::uint64_t food_count) noexcept;

    // Returns a rows x columns std::mdspan referring to tiles
    [[nodiscard]] auto get_tiles(this auto&& self) noexcept {
        return stdex::mdspan{self.tiles.data(), self.rows, self.columns};
    }

    // Returns a reference to ants
    [[nodiscard]] auto& get_ants(this auto&& self) noexcept { return self.ants; }

    // Returns a std::span referring to nests
    [[nodiscard]] auto get_nests(this auto&& self) noexcept { return std::span{self.nests}; }

    // Updates the strength of the pheromone trails to account for fading over time
    void update_pheromones(tile::pheromone_trails& pheromone_trails, tick_t current_tick, nest_id_t nest_id);

    void generate(nest_id_t nest_count, ant_id_t ant_count);

    // Queues the addition of a new worker ant to the nest with id nest_id
    // This is done because inserting into a std::unordered_map while iterating is difficult
    void queue_ant(nest_id_t nest_id);

    // Adds a new worker ant to the nest with id new_ant.nest_id
    void add_ant(ant new_ant);

    void tick();
};

// Wraps a mutex_with_data<simulation>
// Provides a lock method that returns a mutex_guard
// Also provides methods to bypass the mutex when accessing fields that can be accessed via std::atomic_ref
class simulation_mutex {
    mutex_with_data<simulation> sim;

  public:
    explicit simulation_mutex(auto&&... args) : sim(std::in_place, std::forward<decltype(args)>(args)...) {}

    [[nodiscard]] auto lock(this auto&& self) { return self.sim.lock(); }

    // These methods simply call the methods on the protected simulation, without locking
    // This is safe because they all access the members using std::atomic_ref

    [[nodiscard]] simulation::simulation_state get_state() const noexcept { return sim.get_unsafe().get_state(); }
    void set_state(simulation::simulation_state new_state) noexcept { sim.get_unsafe().set_state(new_state); }

    // Checks if state == simulation_state::stopped
    [[nodiscard]] bool stopped() const noexcept { return sim.get_unsafe().stopped(); }
    // Sets state to simulation_state::stopped
    void stop() noexcept { sim.get_unsafe().stop(); }

    // Checks if state == simulation_state::paused
    [[nodiscard]] bool paused() const noexcept { return sim.get_unsafe().paused(); }
    // Sets state to simulation_state::paused or simulation_state::running, depending on the argument's value
    void pause(bool is_paused = true) noexcept { sim.get_unsafe().pause(is_paused); }

    point<float> get_mouse_location() const noexcept { return sim.get_unsafe().get_mouse_location(); }
    void set_mouse_location(point<float> location) noexcept { sim.get_unsafe().set_mouse_location(location); }

    bool get_log_ant_movements() const noexcept { return sim.get_unsafe().get_log_ant_movements(); }
    void set_log_ant_movements(bool log_ant_movements) noexcept {
        sim.get_unsafe().set_log_ant_movements(log_ant_movements);
    }

    bool get_log_ant_state_changes() const noexcept { return sim.get_unsafe().get_log_ant_state_changes(); }
    void set_log_ant_state_changes(bool log_ant_state_changes) noexcept {
        sim.get_unsafe().set_log_ant_state_changes(log_ant_state_changes);
    }

    [[nodiscard]] tick_t get_tick_count() const noexcept { return sim.get_unsafe().get_tick_count(); }
};

} // namespace ant_sim
