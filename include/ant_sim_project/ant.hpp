#pragma once

#include <optional>

#include "types.hpp"

namespace ant_sim {

struct tile;
class simulation;

class ant {
    std::optional<point<>> calculate_next_location(simulation& world);

    float calculate_tile_weight(const tile& tile, simulation& world) noexcept;

  public:
    nest_id_t nest_id;
    ant_id_t ant_id;

    enum class caste { queen, worker } caste;

    point<> location;

    enum class state { searching, returning } state;

    float hunger;
    food_supply_t food_in_inventory;

    void tick(simulation& sim);
    void move(simulation& sim, point<> new_location);
};

} // namespace ant_sim
