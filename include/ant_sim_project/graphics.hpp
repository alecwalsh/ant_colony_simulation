#pragma once

#include <SFML/Graphics.hpp>

#include "simulation.hpp"

namespace ant_sim::graphics {
class world_drawable : public sf::Drawable {
    simulation* sim;

    float tile_size;

    sf::Font font;

    void draw_text(sf::RenderTarget& target, const sf::RenderStates& states, const world& world) const;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
    explicit world_drawable(simulation* sim, float tile_size = 10,
                            const std::filesystem::path& font_path = "../../../data/ProggyTiny.ttf") noexcept
        : sim{sim}, tile_size{tile_size}, font{font_path} {}
};
} // namespace ant_sim::graphics
