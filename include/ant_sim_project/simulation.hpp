#pragma once

#include <cstddef>
#include <mutex>

#include "world.hpp"
#include "types.hpp"
#include "mutex_guard.hpp"

namespace ant_sim {

constexpr std::size_t pheromone_type_count = world::tile::pheromone_type_count;

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
    // tick_count must always be accessed using std::atomic_ref
    tick_t tick_count = 0;

    // is_running must always be accessed using std::atomic_ref
    bool is_running = true;

    // paused must always be accessed using std::atomic_ref
    bool is_paused = false;

    // Must be accessed through get_world, not directly, even within this class
    world sim_world;

    mutable std::mutex mutex;

  public:
    [[nodiscard]] auto get_world(this auto&& self) noexcept { return mutex_guard{&self.sim_world, self.mutex}; }

    simulation(std::size_t rows, std::size_t columns);

    [[nodiscard]] bool running() const noexcept;
    void stop() noexcept;

    [[nodiscard]] bool paused() const noexcept;
    void pause(bool is_paused = true) noexcept;

    [[nodiscard]] tick_t get_tick_count() const noexcept;

    void tick();
};

} // namespace ant_sim
