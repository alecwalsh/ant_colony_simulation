#pragma once

#include "types.hpp"

namespace ant_sim {

struct nest {
    nest_id_t nest_id;

    std::size_t ant_count;

    point location;
};

}