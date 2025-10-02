#include "simulation.hpp"

#include <atomic>
#include <thread>

#include <print>

namespace ant_sim {

simulation::simulation(std::size_t rows, std::size_t columns) : sim_world{rows, columns, this} {}

void simulation::tick() {
    auto world = get_world();

    for(auto& ant : world->get_ants()) {
        ant.tick(*world);
    }

    ++std::atomic_ref{tick_count};

    if(get_tick_count() % 1'000 == 0) {
        std::println("tick(): {}", get_tick_count());
    }
    world.unlock();

    // TODO: adjust sleep time based on desired simulation framerate
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

[[nodiscard]] bool simulation::running() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<bool&>(is_running)};
}

[[nodiscard]] tick_t simulation::get_tick_count() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<tick_t&>(tick_count)};
}

void simulation::stop() noexcept { std::atomic_ref{is_running} = false; }

} // namespace ant_sim
