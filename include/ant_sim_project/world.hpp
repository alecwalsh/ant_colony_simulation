#pragma once

#include <cstddef>
#include <vector>
#include <mdspan>

namespace ant_sim {
class world {
  public:
    struct cell {
        int i;
    };

  private:
    std::size_t rows;
    std::size_t columns;

    std::vector<cell> cells;

  public:
    world(std::size_t rows, std::size_t columns) : rows{rows}, columns{columns} {
        cells = std::vector<cell>(rows * columns);
    }

    [[nodiscard]] std::mdspan<cell, std::dextents<std::size_t, 2>> get_cells() noexcept {
        return std::mdspan{cells.data(), rows, columns};
    }
    [[nodiscard]] std::mdspan<const cell, std::dextents<std::size_t, 2>> get_cells() const noexcept {
        return std::mdspan{cells.data(), rows, columns};
    }
};
} // namespace ant_sim
