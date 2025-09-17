#pragma once

#include <cstddef>
#include <memory>
#include <mdspan>

namespace ant_sim {
class world {
    struct cell {
        int i;
    };

    std::size_t rows;
    std::size_t columns;

    std::unique_ptr<cell[]> cells;

  public:
    world(std::size_t rows, std::size_t columns) : rows{rows}, columns{columns} {
        cells = std::make_unique<cell[]>(rows * columns);
    }

    [[nodiscard]] std::mdspan<cell, std::dextents<std::size_t, 2>> get_cells() noexcept {
        return std::mdspan{cells.get(), rows, columns};
    }
    [[nodiscard]] std::mdspan<const cell, std::dextents<std::size_t, 2>> get_cells() const noexcept {
        return std::mdspan{cells.get(), rows, columns};
    }
};
} // namespace ant_sim
