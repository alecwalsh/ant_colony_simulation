#include "simulation.hpp"

#include <atomic>
#include <thread>

#include <print>

namespace ant_sim {

std::minstd_rand get_rng(std::optional<std::uint64_t> seed) {
    std::uint32_t seed_parts[2];

    if(seed) {
        // std::seed_seq only uses the low 32 bits of each input, so break the seed into 2 32 bit values
        seed_parts[0] = *seed >> 32;
        seed_parts[1] = *seed & 0xFFFFFFFF;
    } else {
        std::random_device random_device;

        seed_parts[0] = random_device();
        seed_parts[1] = random_device();
    }

    std::seed_seq seed_seq{seed_parts[0], seed_parts[1]};

    std::println("Using seed {}", static_cast<std::uint64_t>(seed_parts[0]) << 32 | seed_parts[1]);

    return std::minstd_rand{seed_seq};
}

simulation::simulation(std::size_t rows, std::size_t columns, nest_id_t nest_count, ant_id_t ant_count_per_nest,
                       std::optional<std::uint64_t> seed)
    : rng{get_rng(seed)}, sim_world{rows, columns, this, nest_count, ant_count_per_nest} {
}

void simulation::tick() {
    if(paused()) return;

    auto world = get_world();

    for(auto& [ant_id, ant] : world->get_ants()) {
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

template <typename T>
// Atomically read a reference
// libc++ doesn't support atomic_ref<T> with const T yet
// This function works around it with a const_cast
// This is safe because no attempt is made to modify the value
T atomic_read(const T& t) noexcept {
    return std::atomic_ref{const_cast<T&>(t)};
}

simulation::simulation_state simulation::get_state() const noexcept { return atomic_read(atomically_accessed.state); }

void simulation::set_state(simulation_state new_state) noexcept {
    std::atomic_ref{atomically_accessed.state} = new_state;
}

bool simulation::stopped() const noexcept { return get_state() == simulation_state::stopped; }

void simulation::stop() noexcept { set_state(simulation_state::stopped); }

bool simulation::paused() const noexcept { return get_state() == simulation_state::paused; }

void simulation::pause(bool is_paused) noexcept {
    set_state(is_paused ? simulation_state::paused : simulation_state::running);
}

point<float> simulation::get_mouse_location() const noexcept { return atomic_read(atomically_accessed.mouse_location); }

void simulation::set_mouse_location(point<float> location) noexcept {
    std::atomic_ref{atomically_accessed.mouse_location} = location;
}

bool simulation::get_log_ant_movements() const noexcept { return atomic_read(atomically_accessed.log_ant_movements); }

void simulation::set_log_ant_movements(bool log_ant_movements) noexcept {
    std::atomic_ref{atomically_accessed.log_ant_movements} = log_ant_movements;
}

bool simulation::get_log_ant_state_changes() const noexcept {
    return atomic_read(atomically_accessed.log_ant_state_changes);
}

void simulation::set_log_ant_state_changes(bool log_ant_state_changes) noexcept {
    std::atomic_ref{atomically_accessed.log_ant_state_changes} = log_ant_state_changes;
}

tick_t simulation::get_tick_count() const noexcept { return atomic_read(atomically_accessed.tick_count); }

} // namespace ant_sim
