#include "gui.hpp"

#include <stdexcept>
#include <format>

#include <imgui.h>
#include <imgui-SFML.h>

namespace ant_sim::gui {

gui::gui(sf::RenderWindow& window, simulation& sim) : window{&window}, sim{&sim} {
    if(imgui_initialized) {
        throw std::runtime_error{"Dear ImGui has already been initialized"};
    }

    if(!ImGui::SFML::Init(window)) {
        throw std::runtime_error{"Failed to init Dear ImGui"};
    }

    imgui_initialized = true;
}

void gui::draw_gui(sf::Time delta_time) {
    // Begin a new ImGui frame
    ImGui::SFML::Update(*window, delta_time);

    ImGui::Begin("Simulation control");
    if(ImGui::Button("Change visible pheromones")) {
        // TODO: Implement
    }
    ImGui::End();

    ImGui::Begin("Simulation stats");
    ImGui::Text("%s", std::format("Tick: {}", sim->get_tick_count()).c_str());
    ImGui::End();
}

void gui::render() {
    ImGui::SFML::Render(*window);
}

gui::~gui() {
    ImGui::SFML::Shutdown(*window);

    imgui_initialized = false;
}

}