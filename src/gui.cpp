#include "gui.hpp"

#include <stdexcept>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

namespace ant_sim::gui {

gui::gui(sf::RenderWindow& window, simulation_mutex& sim, graphics::world_drawable& world_drawable)
    : window{&window}, sim{&sim}, world_drawable{&world_drawable} {
    if(imgui_initialized) {
        throw std::runtime_error{"Dear ImGui has already been initialized"};
    }

    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error{"Failed to init Dear ImGui"};
    }

    imgui_initialized = true;
}

void gui::process_event(const sf::Event& event) const {
    ImGui::SFML::ProcessEvent(*window, event);
}

void gui::draw_gui(sf::Time delta_time) const {
    // Begin a new ImGui frame
    ImGui::SFML::Update(*window, delta_time);

    ImGui::Begin("Simulation control");

    if(ImGui::Button("Toggle visible pheromone type")) {
        world_drawable->visible_pheromone_type = world_drawable->visible_pheromone_type == 0 ? 1 : 0;
    }

    ImGui::Text("Currently displaying pheromones from nest %u", world_drawable->visible_pheromone_nest_id);

    auto locked_sim = sim->lock();

    // Get speed in updates per second, then allow setting it with a slider
    float speed = 1 / std::chrono::duration<float>{locked_sim->sleep_time}.count();
    ImGui::SliderFloat("Updates per second", &speed, 0.5f, 100);
    locked_sim->sleep_time = std::chrono::duration<float>{1 / speed};

    auto nest_count = static_cast<nest_id_t>(locked_sim->get_nests().size());

    int i = world_drawable->visible_pheromone_nest_id;
    ImGui::SliderInt("Visible pheromone nest", &i, 0, nest_count - 1);
    world_drawable->visible_pheromone_nest_id = static_cast<nest_id_t>(i);

    ImGui::End();

    ImGui::Begin("Simulation stats");
    ImGui::Text("%s", std::format("Tick: {}", sim->get_tick_count()).c_str());
    ImGui::Text("%s", std::format("Ant count: {}", locked_sim->get_ants().size()).c_str());
    ImGui::End();
}

void gui::render() const { ImGui::SFML::Render(*window); }

gui::~gui() {
    ImGui::SFML::Shutdown(*window);

    imgui_initialized = false;
}

} // namespace ant_sim::gui
