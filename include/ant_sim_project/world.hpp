#pragma once

#include <cstddef>
#include <vector>
#include <mdspan>

namespace ant_sim {
class world {
  public:
    struct tile {
        int i;
    };

  private:
    std::size_t rows;
    std::size_t columns;

    std::vector<tile> tiles;

  public:
    world(std::size_t rows, std::size_t columns) : rows{rows}, columns{columns} {
        tiles = std::vector<tile>(rows * columns);
    }

    // Returns a rows x columns std::mdspan referring to tiles
    [[nodiscard]] auto get_tiles(this auto&& self) noexcept {
        return std::mdspan{self.tiles.data(), self.rows, self.columns};
    }
};
} // namespace ant_sim
