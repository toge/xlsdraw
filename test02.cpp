#include <iostream>

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/drawing_generator.hpp"

int main() {
  using namespace xlsdraw::drawing;

  // 1. 図形データの作成 (セル B2 から D10 まで)
  auto const my_shape = Shape{
    .id = 1,
    .name = "Shape",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0},
    .to = {.col = 3, .col_off = 0, .row = 9, .row_off = 0}
  };

  // 2. ジェネレーターでXML文字列を生成
  auto const generator = DrawingGenerator{};
  auto const xml_output = generator.generate({my_shape});

  // 3. 結果の表示
  std::cout << xml_output << '\n';

  return 0;
}
