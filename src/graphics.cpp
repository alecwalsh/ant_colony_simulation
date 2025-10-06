#include "graphics.hpp"

#include <format>

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

void world_drawable::draw_text(sf::RenderTarget& target, const sf::RenderStates& states, const world& world) const {
    sf::Text text{font};

    auto [x, y] = sim->get_mouse_location();

    auto tile_x = static_cast<std::size_t>(x / tile_size);
    auto tile_y = static_cast<std::size_t>(y / tile_size);

    auto tiles = world.get_tiles();

    if(tile_x >= tiles.extent(1) || tile_y >= tiles.extent(0)) {
        // Tile is out of bounds, and therefore has no associated information to display
        return;
    }

    const auto& tile = tiles[tile_y, tile_x];

    std::string tile_description;

    if(tile.has_nest) {
        tile_description = std::format("Nest {}", tile.nest_id);
    } else if(tile.has_ant) {
        tile_description = std::format("Ant {} from nest {}", tile.ant_id, world.get_ants()[tile.ant_id].nest_id);
    } else if(tile.food_supply > 0) {
        tile_description = std::format("Food supply: {}", tile.food_supply);
    } else {
        tile_description = std::format("Pheromones: {:.3f}", tile.pheromones.pheromone_strength[0][0]);
    }

    text.setString(std::format("{}, {}\n{}", tile_y, tile_x, tile_description));

    auto [width, height] = target.getView().getSize();

    text.setPosition({width - 325, height - 100});
    text.setCharacterSize(24 * 2);
    text.setFillColor(sf::Color::Blue);

    target.draw(text, states);
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

    draw_text(target, states, *world);
}
} // namespace ant_sim::graphics
