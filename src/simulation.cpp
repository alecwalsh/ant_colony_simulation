#include "simulation.hpp"

#include <atomic>
#include <thread>

#include <print>

namespace ant_sim {

simulation::simulation(std::size_t rows, std::size_t columns) : sim_world{rows, columns, this} {}

void simulation::tick() {
    if(paused()) return;

    auto world = get_world();

    for(auto& ant : world->get_ants()) {
        ant.tick(*world);
    }

    ++std::atomic_ref{atomically_accessed.tick_count};

    if(get_tick_count() % 1'000 == 0) {
        std::println("tick(): {}", get_tick_count());
    }
    world.unlock();

    // TODO: adjust sleep time based on desired simulation framerate
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    if(get_state() == simulation_state::single_step) {
        pause(true);
    }
}

simulation::simulation_state simulation::get_state() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<simulation_state&>(atomically_accessed.state)};
}

void simulation::set_state(simulation_state new_state) noexcept {
    std::atomic_ref{atomically_accessed.state} = new_state;
}

bool simulation::stopped() const noexcept { return get_state() == simulation_state::stopped; }

void simulation::stop() noexcept { set_state(simulation_state::stopped); }

bool simulation::paused() const noexcept { return get_state() == simulation_state::paused; }

void simulation::pause(bool is_paused) noexcept {
    set_state(is_paused ? simulation_state::paused : simulation_state::running);
}

point<> simulation::get_mouse_location() const noexcept {
    return std::atomic_ref{const_cast<point<>&>(atomically_accessed.mouse_location)};
}

void simulation::set_mouse_location(point<> location) noexcept {
    std::atomic_ref{atomically_accessed.mouse_location} = location;
}

tick_t simulation::get_tick_count() const noexcept {
    // libc++ doesn't yet support atomic_ref<T> with const T, so work around it with const_cast
    return std::atomic_ref{const_cast<tick_t&>(atomically_accessed.tick_count)};
}

} // namespace ant_sim
