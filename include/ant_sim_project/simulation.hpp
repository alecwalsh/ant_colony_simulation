#pragma once

#include <cstdint>
#include <cstddef>
#include <mutex>

#include "world.hpp"
#include "mutex_guard.hpp"

namespace ant_sim {

class simulation {
    // is_running must always be accessed using std::atomic_ref
    std::uint64_t tick_count = 0;

    // is_running must always be accessed using std::atomic_ref
    bool is_running = true;

    // Must be accessed through get_world, not directly, even within this class
    world sim_world;

    mutable std::mutex mutex;

  public:
    [[nodiscard]] mutex_guard<world> get_world() noexcept { return {&sim_world, mutex}; }
    [[nodiscard]] mutex_guard<const world> get_world() const noexcept { return {&sim_world, mutex}; }

    simulation(std::size_t rows, std::size_t columns);

    [[nodiscard]] bool running() const noexcept;
    void stop() noexcept;

    [[nodiscard]] std::uint64_t get_tick_count() const noexcept;

    void tick();
};
} // namespace ant_sim
