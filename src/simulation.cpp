#include "simulation.hpp"

#include <atomic>

#include <print>

namespace ant_sim {
simulation::simulation(std::size_t rows, std::size_t columns) : sim_world{rows, columns} {}

void simulation::tick() {
    auto world = get_world();

    auto cells = world->get_cells();

    for(auto y = 0uz; y < cells.extent(0); y++) {
        for(auto x = 0uz; x < cells.extent(1); x++) {
            auto& cell = cells[y, x];

            cell.i++;
        }
    }

    ++std::atomic_ref{tick_count};

    if(get_tick_count() % 1'000 == 0) {
        std::println("tick(): {}", get_tick_count());
    }
}

[[nodiscard]] bool simulation::running() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<bool&>(is_running)};
}

[[nodiscard]] std::uint64_t simulation::get_tick_count() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<std::uint64_t&>(tick_count)};
}

void simulation::stop() noexcept { std::atomic_ref{is_running} = false; }
} // namespace ant_sim
