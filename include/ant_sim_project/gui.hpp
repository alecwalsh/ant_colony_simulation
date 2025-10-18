#pragma once

#include "simulation.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

namespace ant_sim::gui {

// Only one instance of this class can exist at any one time
class gui {
    static inline bool imgui_initialized = false;

    sf::RenderWindow* window;
    simulation* sim;
public:
    gui(sf::RenderWindow& window, simulation& sim);

    void draw_gui(sf::Time delta_time);
    void render();

    ~gui();
};

}


