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

    // Returns a rows x columns std::mdspan referring to cells
    [[nodiscard]] auto get_cells(this auto&& self) noexcept {
        return std::mdspan{self.cells.data(), self.rows, self.columns};
    }
};
} // namespace ant_sim
