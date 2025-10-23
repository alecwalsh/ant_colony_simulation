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

void gui::process_event(const sf::Event& event) {
    ImGui::SFML::ProcessEvent(*window, event);
}

void gui::draw_gui(sf::Time delta_time) {
    // Begin a new ImGui frame
    ImGui::SFML::Update(*window, delta_time);

    ImGui::Begin("Simulation visualization control");

    if(ImGui::Button("Toggle visible pheromone type")) {
        world_drawable->visible_pheromone_type = world_drawable->visible_pheromone_type == 0 ? 1 : 0;
    }

    ImGui::Text("Currently displaying pheromones from nest %u", world_drawable->visible_pheromone_nest_id);

    // TODO: Allow retrieving number of nests without locking sim
    auto nest_count = static_cast<nest_id_t>(sim->lock()->world.get_nests().size());

    int i = world_drawable->visible_pheromone_nest_id;
    ImGui::SliderInt("Change visible pheromone nest", &i, 0, nest_count - 1);
    world_drawable->visible_pheromone_nest_id = static_cast<nest_id_t>(i);

    ImGui::End();

    ImGui::Begin("Simulation stats");
    ImGui::Text("%s", std::format("Tick: {}", sim->get_tick_count()).c_str());
    ImGui::End();
}

void gui::render() { ImGui::SFML::Render(*window); }

gui::~gui() {
    ImGui::SFML::Shutdown(*window);

    imgui_initialized = false;
}

} // namespace ant_sim::gui
