#pragma once

#include <SFML/Graphics.hpp>

#include "simulation.hpp"

namespace ant_sim::graphics {
class world_drawable : public sf::Drawable {
    simulation* sim;

    float tile_size; // The tile size in pixels, prior to zoom being applied
    float gap_size = 1; // Size of the gap between tiles, prior to zoom being applied

    float current_zoom = 1;

    // The amount to zoom in or out by
    float zoom_increment = 0.1f;

    sf::Font font;

    void draw_text(sf::RenderTarget& target, const sf::RenderStates& states, const world& world) const;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

  public:
    std::size_t visible_pheromone_type = 0;
    nest_id_t visible_pheromone_nest_id = 0;

    explicit world_drawable(simulation* sim, float tile_size = 10,
                            const std::filesystem::path& font_path = "../../../data/ProggyTiny.ttf") noexcept
        : sim{sim}, tile_size{tile_size}, font{font_path} {}

    // Increase or decrease current_zoom, and update the view's size to account for the change
    // Zoom is increased relative to a zoom of 1: zooming in by 20% twice is equivalent to a single 40% zoom, not 44%
    void zoom_view(sf::View& view, bool zoom_in) noexcept;

    static void pan_view(sf::View& view, point<float> amount) noexcept;
};
} // namespace ant_sim::graphics
