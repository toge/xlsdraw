#include <iostream>

#include "xlsdraw/worksheet.hpp"
#include "xlsdraw/units.hpp"

int main() {
  using namespace xlsdraw::worksheet;
  using namespace xlsdraw::units;

  auto const conv = xlsdraw::units::EmuConverter{96.0}; // 96 DPI
  auto ctx = xlsdraw::worksheet::WorksheetDrawingContext{conv};

  auto const id1 = ctx.add_simple_rect(1, 1, 200, 100, "First Shape");
  auto const id2 = ctx.add_simple_rect(4, 2, 150, 150, "Second Shape");

  if (id1 and id2) {
    std::cout << "Successfully added shapes with IDs: " << *id1 << ", " << *id2 << '\n';
  }

  std::cout << "Generated XML:\n" << ctx.final_xml() << '\n';

  return 0;
}
