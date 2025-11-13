#include "ant_sim_project/gui.hpp"

#include <ant_sim_project/simulation.hpp>
#include <ant_sim_project/graphics.hpp>

#include <thread>
#include <functional>
#include <print>
#include <span>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

ant_sim::simulation_args_t parse_args(std::span<const char*> args) {
    assert(!args.empty());

    auto idx = 0uz;

    ant_sim::simulation_args_t result = {};

    if(idx >= args.size()) return result;
    if(args[idx] != std::string_view{"random"}) {
        result.seed = std::stoull(args[idx]);
    }
    idx++;

    if(idx >= args.size()) return result;
    result.rows = std::stoull(args[idx++]);

    if(idx >= args.size()) return result;
    result.columns = std::stoull(args[idx++]);

    if(idx >= args.size()) return result;
    result.nest_count = std::stoul(args[idx++]);

    if(idx >= args.size()) return result;
    result.ant_count_per_nest = std::stoul(args[idx++]);

    if(idx >= args.size()) return result;
    result.hunger_increase_per_tick = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.hunger_to_die = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.food_taken = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.food_resupply_rate = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.max_food_supply = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.food_per_new_ant = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.food_hunger_ratio = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.falloff_rate = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.increase_rate = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.type1_avoidance = std::stof(args[idx++]);

    if(idx >= args.size()) return result;
    result.type2_avoidance = std::stof(args[idx++]);

    return result;
}

// Exit after this many ticks have passed
ant_sim::tick_t max_ticks = 1200;

int main(int argc, char* argv[]) {
    for(int i = 0; i < argc; i++) {
        std::print("{} ", argv[i]);
    }
    std::println("");

    ant_sim::simulation_args_t args = {};

    if(argc > 1) {
        try {
            args = parse_args(std::span{const_cast<const char**>(argv) + 1, static_cast<std::size_t>(argc - 1)});
        } catch(...) {
            std::println("Error parsing arguments");
            return EXIT_FAILURE;
        }
    }

    ant_sim::simulation_mutex sim{args};

    std::jthread simulation_thread{[](const std::stop_token& stop_token, ant_sim::simulation_mutex& sim) {
        while(!sim.stopped() && !stop_token.stop_requested()) {
            auto locked_sim = sim.lock();

            if(locked_sim->get_tick_count() > max_ticks) {
                sim.stop();
                break;
            }

            locked_sim->tick();

            auto sleep_time = locked_sim->sleep_time;

            locked_sim.unlock();

            std::this_thread::sleep_for(sleep_time);
        }
    }, std::ref(sim)};

    // The default values for window width and height
    sf::Vector2u default_window_dimensions = {800, 600};

    unsigned max_framerate = 60;

    sf::RenderWindow window{sf::VideoMode{default_window_dimensions}, "Ant colony simulation"};

    auto view_width = static_cast<float>(default_window_dimensions.x);
    auto view_height = static_cast<float>(default_window_dimensions.y);

    window.setView(sf::View{sf::FloatRect{{}, {view_width, view_height}}});

    window.setFramerateLimit(max_framerate);

    ant_sim::graphics::world_drawable world_drawable{sim};

    ant_sim::gui::gui gui{window, sim, world_drawable};

    sf::Clock clock;

    while(window.isOpen() && !sim.stopped()) {
        while(const auto event = window.pollEvent()) {
            // Make sure the GUI knows about any inputs
            gui.process_event(*event);

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
                } else if(key->code == sf::Keyboard::Key::Period) {
                    sim.set_state(ant_sim::simulation::simulation_state::single_step);
                }
            } else if(const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                auto view = window.getView();

                float pan_amount = 25;

                if(key->code == sf::Keyboard::Key::Left) {
                    ant_sim::graphics::world_drawable::pan_view(view, {-pan_amount, 0});
                } else if(key->code == sf::Keyboard::Key::Right) {
                    ant_sim::graphics::world_drawable::pan_view(view, {pan_amount, 0});
                } else if(key->code == sf::Keyboard::Key::Up) {
                    ant_sim::graphics::world_drawable::pan_view(view, {0, -pan_amount});
                } else if(key->code == sf::Keyboard::Key::Down) {
                    ant_sim::graphics::world_drawable::pan_view(view, {0, pan_amount});
                } if(key->code == sf::Keyboard::Key::Equal) {
                    world_drawable.zoom_view(view, true);
                } else if(key->code == sf::Keyboard::Key::Hyphen) {
                    world_drawable.zoom_view(view, false);
                }

                window.setView(view);
            } else if(const auto* mouse_move = event->getIf<sf::Event::MouseMoved>()) {
                auto [x, y] = window.mapPixelToCoords(mouse_move->position);

                sim.set_mouse_location({x, y});
            }
        }

        // ImGui doesn't like it when we try to render after closing the window
        if(!window.isOpen()) break;

        window.clear();

        gui.draw_gui(clock.restart());

        window.draw(world_drawable);
        // Display the GUI and end the ImGui frame
        gui.render();

        window.display();
    }
}
