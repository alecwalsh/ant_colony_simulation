#include "graphics.hpp"

namespace ant_sim::graphics {

// Returns the number of visible tiles
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
            auto& tile = tiles[y, x];

            sf::Color color{};

            // Make sure pheromone values are up to date before drawing
            for(nest_id_t i = 0uz; i < pheromone_type_count; i++) {
                world::update_pheromones(tile.pheromones, world->sim->get_tick_count(), i);
            }

            if(tile.has_nest) {
                color = {0, 0, 255};
            } else if(tile.has_ant) {
                color = {255, 255, 255};
            } else if(tile.food_supply > 0) {
                color = {0, 255, 0};
            } else {
                auto red = tile.pheromones.pheromone_strength[0][0];

                red = static_cast<pheromone_strength_t>(std::clamp(static_cast<float>(red) * 30.0f, 0.0f, 255.0f));

                color = {static_cast<std::uint8_t>(red), 0, 0};
            }

            rectangle.setPosition({static_cast<float>(x) * tile_size, static_cast<float>(y) * tile_size});
            rectangle.setFillColor(color);

            target.draw(rectangle, states);
        }
    }
}
} // namespace ant_sim::graphics
