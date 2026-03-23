#include <iostream>

#include "xlsdraw/drawing.hpp"

int main() {
  using namespace xlsdraw::drawing;

  auto my_shape = Shape{
    .id = 1,
    .name = "Shape",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0},
    .to = {.col = 3, .col_off = 0, .row = 9, .row_off = 0}
  };

  auto run = TextRun{
    .text = "Hello, World!",
    .color = Color{"FFFF0000"}, // 赤色
    .font_size = 14.0
  };

  auto p = Paragraph{
    .runs = {run},
    .alignment = "ctr"
  };

  my_shape.text_body = TextBody {
    .paragraphs = {p}
  };
  my_shape.style.fill.solid_fill = Color{.argb = "FF4472C4"};

  auto gen = DrawingGenerator{};

  std::cout << gen.generate({my_shape}) << '\n';

  return 0;
}
