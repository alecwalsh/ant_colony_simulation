#pragma once

#include <cstddef>
#include <mutex>
#include <atomic>

#include "world.hpp"
#include "types.hpp"
#include "mutex_guard.hpp"

namespace ant_sim {

// TODO: move these constants somewhere else
// TODO: Allow adjusting these values at runtime
// TODO: Support separate values for each pheromone type

// How much a pheromone's strength decreases each tick
constexpr float falloff_rate = 0.025f;

// How much a pheromone's strength increases when left by an ant
constexpr pheromone_strength_t increase_rate = 3;

// A value in this range is added to pheromone strength when calculating weights
constexpr std::pair add_random_range = {-0.1f, 0.1f};
// Pheromone strength is multiplied by a value in this range when calculating weights
constexpr std::pair mul_random_range = {0.5f, 1.5f};

class simulation {
  public:
    enum class simulation_state : std::uint8_t {
        running,     // The simulation is running
        stopped,     // The simulation has permanently stopped
        single_step, // The simulation is running, but will be automatically paused after the next tick
        paused       // The simulation is paused
    };

    float hunger_increase_per_tick = 1.0f;
    std::uint8_t food_taken = 5; // The amount of food ants take when they encounter a food source

    std::minstd_rand rng;

  private:
    // All members of this struct must always be accessed via std::atomic_ref, as multiple threads may access them.
    // Each member is independent of the others.  There is no need to pass the entire struct to std::atomic_ref.
    struct {
        tick_t tick_count = 0;
        simulation_state state = simulation_state::running;

        using point_t = point<float>;
        // Ensure mouse_location is sufficiently aligned to use with std::atomic_ref
        alignas(std::atomic_ref<point_t>::required_alignment) point_t mouse_location = {0, 0};

        bool log_ant_movements = false;
        bool log_ant_state_changes = false;
    } atomically_accessed;

    // Must be accessed through get_world, not directly, even within this class
    world sim_world;

    mutable std::mutex mutex;

  public:
    [[nodiscard]] auto get_world(this auto&& self) noexcept { return mutex_guard{&self.sim_world, self.mutex}; }

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

    void tick();
};

} // namespace ant_sim
