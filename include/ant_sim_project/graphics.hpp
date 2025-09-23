#pragma once

#include <SFML/Graphics.hpp>

#include "simulation.hpp"

namespace ant_sim::graphics {
class world_drawable : public sf::Drawable {
    simulation* sim;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
    explicit world_drawable(simulation* sim) noexcept : sim{sim} {}
};
} // namespace ant_sim::graphics
