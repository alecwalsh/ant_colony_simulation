#include "graphics.hpp"

namespace ant_sim::graphics {

// Returns the number of visible tiles
// The return value is in units of tiles
static std::pair<std::size_t, std::size_t>
get_visible_area(const sf::View& view, stdex::mdspan<world::tile, stdex::dextents<std::size_t, 2>> tiles, float tile_size) {
    auto [view_width, view_height] = view.getSize();

    auto view_width_in_tiles = static_cast<std::size_t>(view_width / tile_size);
    auto view_height_in_tiles = static_cast<std::size_t>(view_height / tile_size);

    auto world_width = tiles.extent(1);
    auto world_height = tiles.extent(0);

    auto width = std::min(view_width_in_tiles, world_width);
    auto height = std::min(view_height_in_tiles, world_height);

    return std::pair{width, height};
}

void world_drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto world = sim->get_world();

    auto tiles = world->get_tiles();

    auto [visible_width, visible_height] = get_visible_area(target.getView(), tiles, tile_size);

    sf::RectangleShape rectangle{{tile_size - 1, tile_size - 1}};

    for(auto y = 0uz; y < visible_height; y++) {
        for(auto x = 0uz; x < visible_width; x++) {
            const auto& tile = tiles[y, x];

            auto brightness = static_cast<std::uint8_t>(tile.i / 10'000);

            sf::Color color{brightness, brightness, brightness};

            rectangle.setPosition({static_cast<float>(x) * tile_size, static_cast<float>(y) * tile_size});
            rectangle.setFillColor(color);

            target.draw(rectangle, states);
        }
    }
}
} // namespace ant_sim::graphics
