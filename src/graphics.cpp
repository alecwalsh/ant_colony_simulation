#include "graphics.hpp"

namespace ant_sim::graphics {

// Returns the number of visible cells
// The return value is in units of cells
static std::pair<std::size_t, std::size_t>
get_visible_area(const sf::View& view, std::mdspan<world::cell, std::dextents<std::size_t, 2>> cells, float cell_size) {
    auto [view_width, view_height] = view.getSize();

    auto view_width_in_cells = static_cast<std::size_t>(view_width / cell_size);
    auto view_height_in_cells = static_cast<std::size_t>(view_height / cell_size);

    auto world_width = cells.extent(1);
    auto world_height = cells.extent(0);

    auto width = std::min(view_width_in_cells, world_width);
    auto height = std::min(view_height_in_cells, world_height);

    return std::pair{width, height};
}

void world_drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto world = sim->get_world();

    auto cells = world->get_cells();

    auto [visible_width, visible_height] = get_visible_area(target.getView(), cells, cell_size);

    sf::RectangleShape rectangle{{cell_size - 1, cell_size - 1}};

    for(auto y = 0uz; y < visible_height; y++) {
        for(auto x = 0uz; x < visible_width; x++) {
            const auto& cell = cells[y, x];

            auto brightness = static_cast<std::uint8_t>(cell.i / 10'000);

            sf::Color color{brightness, brightness, brightness};

            rectangle.setPosition({static_cast<float>(x) * cell_size, static_cast<float>(y) * cell_size});
            rectangle.setFillColor(color);

            target.draw(rectangle, states);
        }
    }
}
} // namespace ant_sim::graphics
