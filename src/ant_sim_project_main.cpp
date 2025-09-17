#include "ant_sim_project/simulation.hpp"

#include <thread>

int main() {
    std::jthread simulation_thread{[](const std::stop_token& stop_token) {
        ant_sim::simulation sim{100, 100};

        while(sim.running() && !stop_token.stop_requested()) {
            sim.tick();
        }
    }};

    simulation_thread.join();
}
