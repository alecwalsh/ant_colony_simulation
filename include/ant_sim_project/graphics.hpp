#pragma once

#include <SFML/Graphics.hpp>

#include "simulation.hpp"

namespace ant_sim::graphics {
class world_drawable : public sf::Drawable {
    simulation* sim;

    float cell_size;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
    explicit world_drawable(simulation* sim, float cell_size = 20) noexcept : sim{sim}, cell_size{cell_size} {}
};
} // namespace ant_sim::graphics
