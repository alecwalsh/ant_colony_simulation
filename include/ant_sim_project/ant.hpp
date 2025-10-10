#pragma once

#include "types.hpp"

namespace ant_sim {

class world;

class ant {
public:
    nest_id_t nest_id;
    ant_id_t ant_id;

    enum class caste {
        queen,
        worker
    } caste;

    point<> location;

    enum class state {
        searching,
        returning
    } state;

    void tick(world& world);
    void move(world& world, point<> new_location);
};

}