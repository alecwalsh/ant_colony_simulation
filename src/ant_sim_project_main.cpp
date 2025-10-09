#include <ant_sim_project/simulation.hpp>
#include <ant_sim_project/graphics.hpp>

#include <thread>
#include <functional>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

int main() {
    ant_sim::simulation sim{100, 100};

    std::jthread simulation_thread{[](const std::stop_token& stop_token, ant_sim::simulation& sim) {
        while(!sim.stopped() && !stop_token.stop_requested()) {
            sim.tick();
        }
    }, std::ref(sim)};

    unsigned window_width = 800;
    unsigned window_height = 600;

    unsigned max_framerate = 60;

    auto view_width = static_cast<float>(window_width);
    auto view_height = static_cast<float>(window_height);

    sf::RenderWindow window{sf::VideoMode{{window_width, window_height}}, "Ant colony simulation"};

    window.setView(sf::View{sf::FloatRect{{}, {view_width, view_height}}});

    window.setFramerateLimit(max_framerate);

    ant_sim::graphics::world_drawable world_drawable{&sim};

    while(window.isOpen()) {
        window.clear(sf::Color::Black);

        while(const auto event = window.pollEvent()) {
            if(event->is<sf::Event::Closed>()) {
                window.close();
            } else if(const auto* resized = event->getIf<sf::Event::Resized>()) {
                auto [new_x, new_y] = resized->size;

                sf::Vector2f new_size = {static_cast<float>(new_x), static_cast<float>(new_y)};

                auto new_view = sf::View{sf::FloatRect{{}, new_size}};

                window.setView(new_view);
            } else if(const auto* key = event->getIf<sf::Event::KeyReleased>()) {
                if(key->code == sf::Keyboard::Key::Space) {
                    auto p = sim.paused();
                    sim.pause(!p);
                }
                if(key->code == sf::Keyboard::Key::Period) {
                    sim.set_state(ant_sim::simulation::simulation_state::single_step);
                }
            } else if(const auto* mouse_move = event->getIf<sf::Event::MouseMoved>()) {
                auto [x, y] = window.mapPixelToCoords(mouse_move->position);

                sim.set_mouse_location({x, y});
            }
        }

        window.draw(world_drawable);
        window.display();
    }
}
