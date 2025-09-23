#include <ant_sim_project/simulation.hpp>
#include <ant_sim_project/graphics.hpp>

#include <thread>
#include <functional>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main() {
    ant_sim::simulation sim{100, 100};

    std::jthread simulation_thread{[](const std::stop_token& stop_token, ant_sim::simulation& sim) {
        while(sim.running() && !stop_token.stop_requested()) {
            sim.tick();
        }
    }, std::ref(sim)};

    unsigned window_width = 800;
    unsigned window_height = 600;
    unsigned max_framerate = 60;

    sf::RenderWindow window{sf::VideoMode{{window_width, window_height}}, "Ant colony simulation"};

    window.setFramerateLimit(max_framerate);

    ant_sim::graphics::world_drawable world_drawable{&sim};

    while(window.isOpen()) {
        while(const auto event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.draw(world_drawable);
        window.display();
    }
}
