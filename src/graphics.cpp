#include "graphics.hpp"

#include <format>

namespace ant_sim::graphics {

// Returns the top left and bottom right tiles of the visible area
// This does not account for rotated views
static std::pair<point<>, point<>> get_visible_area(const sf::View& view,
                                                stdex::mdspan<tile, stdex::dextents<std::size_t, 2>> tiles,
                                                float tile_size) {
    auto [view_width, view_height] = view.getSize();
    auto [center_x, center_y] = view.getCenter();

    // Calculate the visible tiles
    // These values may be out of bounds, and need to be clamped before being used as indices
    auto left = std::floor((center_x - view_width / 2) / tile_size);
    auto right = std::ceil((center_x + view_width / 2) / tile_size);
    auto top = std::floor((center_y - view_height / 2) / tile_size);
    auto bottom = std::ceil((center_y + view_height / 2) / tile_size);

    auto world_width = static_cast<float>(tiles.extent(1));
    auto world_height = static_cast<float>(tiles.extent(0));

    // Clamp the coordinates to the world bounds
    auto left_clamped = static_cast<std::size_t>(std::clamp(left, 0.0f, world_width));
    auto right_clamped = static_cast<std::size_t>(std::clamp(right, 0.0f, world_width));
    auto top_clamped = static_cast<std::size_t>(std::clamp(top, 0.0f, world_height));
    auto bottom_clamped = static_cast<std::size_t>(std::clamp(bottom, 0.0f, world_height));

    return {{left_clamped, top_clamped}, {right_clamped, bottom_clamped}};
}

void world_drawable::draw_text(sf::RenderTarget& target, const sf::RenderStates& states, simulation& locked_sim) const {
    sf::Text text{font};

    auto [x, y] = sim->get_mouse_location();

    if(x < 0 || y < 0) {
        // Tile is out of bounds, and therefore has no associated information to display
        return;
    }

    auto tile_x = static_cast<std::size_t>(x / tile_size);
    auto tile_y = static_cast<std::size_t>(y / tile_size);

    auto tiles = locked_sim.get_tiles();

    if(tile_x >= tiles.extent(1) || tile_y >= tiles.extent(0)) {
        // Tile is out of bounds, and therefore has no associated information to display
        return;
    }

    const auto& tile = tiles[tile_y, tile_x];

    std::string tile_description;

    if(tile.has_nest) {
        tile_description = std::format("Nest {} with {} food", tile.nest_id, locked_sim.get_nests()[tile.nest_id].food_supply);
    } else if(tile.has_ant) {
        tile_description = std::format("Ant {} from nest {}", tile.ant_id, locked_sim.get_ants().at(tile.ant_id).nest_id);
    } else if(tile.food_supply > 0) {
        tile_description = std::format("Food supply: {}", tile.food_supply);
    } else {
        tile_description = std::format("Pheromones: {:.3f}", tile.pheromones.pheromone_strength[0][0]);
    }

    text.setString(std::format("{}, {}\n{}", tile_y, tile_x, tile_description));

    auto current_view = target.getView(); // Save the target's current view for restoring after rendering text

    auto [size_x, size_y] = target.getSize();

    auto text_view = sf::View{sf::FloatRect{{}, {static_cast<float>(size_x), static_cast<float>(size_y)}}};

    target.setView(text_view);

    auto [width, height] = text_view.getSize();

    text.setPosition({width - 375, height - 100});
    text.setCharacterSize(24 * 2);
    text.setFillColor(sf::Color::Blue);

    target.draw(text, states);

    target.setView(current_view); // Restore previous view
}

void world_drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto locked_sim = sim->lock();

    auto tiles = locked_sim->get_tiles();

    auto [top_left, bottom_right] = get_visible_area(target.getView(), tiles, tile_size);

    sf::RectangleShape rectangle{{tile_size - gap_size, tile_size - gap_size}};

    for(auto y = top_left.y; y < bottom_right.y; y++) {
        for(auto x = top_left.x; x < bottom_right.x; x++) {
            auto& tile = tiles[y, x];

            sf::Color color{};

            // Make sure pheromone values are up to date before drawing
            for(nest_id_t i = 0uz; i < tile::pheromone_type_count; i++) {
                simulation::update_pheromones(tile.pheromones, sim->get_tick_count(), i);
            }

            if(tile.has_nest) {
                color = {0, 0, 255};
            } else if(tile.has_ant) {
                color = {255, 255, 255};
            } else if(tile.food_supply > 0) {
                color = {0, static_cast<std::uint8_t>(tile.food_supply), 0};
            } else {
                auto red = tile.pheromones.pheromone_strength[visible_pheromone_nest_id][visible_pheromone_type];

                red = static_cast<pheromone_strength_t>(std::clamp(static_cast<float>(red) * 30.0f, 0.0f, 255.0f));

                color = {static_cast<std::uint8_t>(red), 0, 0};
            }

            rectangle.setPosition({static_cast<float>(x) * tile_size, static_cast<float>(y) * tile_size});
            rectangle.setFillColor(color);

            target.draw(rectangle, states);
        }
    }

    draw_text(target, states, *locked_sim);
}


void world_drawable::zoom_view(sf::View& view, bool zoom_in) noexcept {
    float multiplier = zoom_in ? 1 : -1;

    float new_zoom = current_zoom + zoom_increment * multiplier;

    // Zoom must be greater than 0
    if(new_zoom <= 0) return;

    auto zoom_ratio = current_zoom / new_zoom;

    current_zoom = new_zoom;

    auto size = view.getSize();

    size.x *= zoom_ratio;
    size.y *= zoom_ratio;

    view.setSize(size);
}

void world_drawable::pan_view(sf::View& view, point<float> amount) noexcept {
    auto center = view.getCenter();

    center.x += amount.x;
    center.y += amount.y;

    view.setCenter(center);
}

} // namespace ant_sim::graphics
