#include "graphics.hpp"

#include <print>

namespace ant_sim::graphics {
void world_drawable::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    auto world = sim->get_world();

    auto cells = world->get_cells();

    sf::RectangleShape rectangle{{100, 200}};

    target.draw(rectangle, states);

    std::println("world_drawable::draw");
}
} // namespace ant_sim::graphics
