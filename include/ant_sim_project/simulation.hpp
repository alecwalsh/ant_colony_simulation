#pragma once

#include "world.hpp"

#include <cstdint>
#include <cstddef>

namespace ant_sim {
class simulation {
    std::uint64_t tick_count = 0;
    bool is_running = true;

    world world;

  public:
    simulation(std::size_t rows, std::size_t columns);

    [[nodiscard]] bool running() const noexcept { return is_running; }

    void stop() noexcept { is_running = false; }

    void tick();
};
} // namespace ant_sim
