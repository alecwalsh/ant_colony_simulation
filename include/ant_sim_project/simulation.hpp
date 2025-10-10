#pragma once

#include <cstddef>
#include <mutex>

#include "world.hpp"
#include "types.hpp"
#include "mutex_guard.hpp"

namespace ant_sim {

constexpr std::size_t pheromone_type_count = tile::pheromone_type_count;

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

  private:
    // All members of this struct must always be accessed via std::atomic_ref, as multiple threads may access them.
    // Each member is independent of the others.  There is no need to pass the entire struct to std::atomic_ref.
    struct {
        tick_t tick_count = 0;
        simulation_state state = simulation_state::running;
        point<float> mouse_location = {0, 0};
    } atomically_accessed;

    // Must be accessed through get_world, not directly, even within this class
    world sim_world;

    mutable std::mutex mutex;

  public:
    [[nodiscard]] auto get_world(this auto&& self) noexcept { return mutex_guard{&self.sim_world, self.mutex}; }

    simulation(std::size_t rows, std::size_t columns);

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

    [[nodiscard]] tick_t get_tick_count() const noexcept;

    void tick();
};

} // namespace ant_sim
