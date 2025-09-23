#include "ant_sim_project/simulation.hpp"

#include <thread>
#include <functional>

int main() {
    ant_sim::simulation sim{100, 100};

    std::jthread simulation_thread{[](const std::stop_token& stop_token, ant_sim::simulation& sim) {
        while(sim.running() && !stop_token.stop_requested()) {
            sim.tick();
        }
    }, std::ref(sim)};

    simulation_thread.join();
}
