#include <iostream>

#include "xlsdraw/units.hpp"
#include "xlsdraw/drawing.hpp"

int main() {
  using namespace xlsdraw::units;
  using namespace xlsdraw::drawing;

  // 1. 図形データの作成 (セル B2 から D10 まで)
  auto my_shape = Shape{
    .id = 1,
    .name = "Shape",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0},
    .to = {.col = 3, .col_off = 0, .row = 9, .row_off = 0}
  };

  auto const converter = EmuConverter{96.0};

  my_shape.style.fill.solid_fill = Color{"FF0000FF"}; // 青色
  my_shape.style.line.color = Color{"FFFF0000"}; // 赤色
  my_shape.style.line.width_emu = converter.pixels_to_emu(3).value_or(9625); // 3px相当

  // 2. ジェネレーターでXML文字列を生成
  auto const generator = DrawingGenerator{};
  auto const xml_output = generator.generate({my_shape});

  // 3. 結果の表示
  std::cout << xml_output << '\n';

  return 0;
}
