#pragma once

#include "graphics.hpp"
#include "simulation.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

namespace ant_sim::gui {

// Only one instance of this class can exist at any one time
class gui {
    static inline bool imgui_initialized = false;

    sf::RenderWindow* window;
    simulation* sim;
    graphics::world_drawable* world_drawable;
public:
    gui(sf::RenderWindow& window, simulation& sim, graphics::world_drawable& world_drawable);

    void draw_gui(sf::Time delta_time);
    void render();

    ~gui();
};

}


