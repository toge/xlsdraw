#include <iostream>

#include "xlsdraw/units.hpp"
#include "xlsdraw/cell_anchor.hpp"
#include "xlsdraw/drawing.hpp"

int main() {
  using namespace xlsdraw::units;
  using namespace xlsdraw::drawing;

  auto const converter = EmuConverter{96.0}; // 標準DPI
  auto const calculator = CellAnchorCalculator{};

  // 例: セル C3 (index 2, 2) の左上から 10px, 20px ずれた位置を計算
  auto const result = calculator.compute_offset(2, 2, 10, 20, converter);

  if (result) {
    auto const& pos = *result;
    std::cout << "Cell: [" << pos.col << ", " << pos.row << "]\n"
              << "X Offset (EMU): " << pos.col_offset_emu << "\n"
              << "Y Offset (EMU): " << pos.row_offset_emu << "\n";
  } else {
    std::cerr << "Error calculating offsets.\n";
  }

  return 0;
}
