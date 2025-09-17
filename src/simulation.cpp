#include "simulation.hpp"

#include <print>

namespace ant_sim {
simulation::simulation(std::size_t rows, std::size_t columns) : world{rows, columns} {}

void simulation::tick() {
    tick_count++;

    std::println("tick(): {}", tick_count);

    if(tick_count >= 1'000'000) {
        stop();
    }
}
} // namespace ant_sim
