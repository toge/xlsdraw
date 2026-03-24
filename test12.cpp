#include <array>
#include <iostream>
#include <string>
#include <variant>

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/workbook.hpp"

namespace {

auto make_gradient_shape(
  std::string name,
  std::string label,
  int32_t col,
  xlsdraw::drawing::GradientFill fill,
  int64_t width_emu,
  int64_t height_emu
) -> xlsdraw::drawing::Shape {
  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::RoundRect,
    std::move(name)
  );
  shape.from = {.col = col, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = col, .col_off = width_emu, .row = 1, .row_off = height_emu};
  shape.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
  shape.style.fill.gradient_fill = std::move(fill);
  shape.style.line.width_emu = 12700;
  shape.style.line.color = xlsdraw::drawing::Color{"FF5B9BD5"};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{
          .text = std::move(label),
          .color = xlsdraw::drawing::Color{"FF000000"},
          .font_size = 11.0,
        }},
        .alignment = "ctr",
      }
    },
  };
  return shape;
}

} // namespace

int main() {
  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{"test12.xlsx", "Gradients"};

  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const width_emu = converter.pixels_to_emu(180).value_or(0);
  auto const height_emu = converter.pixels_to_emu(72).value_or(0);

  auto const shapes = std::array{
    make_gradient_shape(
      "LinearGradient",
      "Linear",
      1,
      xlsdraw::drawing::GradientFill{
        .stops = {
          {.position = 0, .color = xlsdraw::drawing::Color{"FF5B9BD5"}},
          {.position = 100000, .color = xlsdraw::drawing::Color{"FFDDEBF7"}},
        },
        .shade = xlsdraw::drawing::LinearGradient{
          .angle = 5400000,
          .scaled = false,
        },
      },
      width_emu,
      height_emu
    ),
    make_gradient_shape(
      "ShapeGradient",
      "Path: shape",
      4,
      xlsdraw::drawing::GradientFill{
        .stops = {
          {.position = 0, .color = xlsdraw::drawing::Color{"FF70AD47"}},
          {.position = 100000, .color = xlsdraw::drawing::Color{"FFE2F0D9"}},
        },
        .shade = xlsdraw::drawing::PathGradient{
          .path = xlsdraw::drawing::PathGradientType::Shape,
          .fill_to_rect = xlsdraw::drawing::GradientFillToRect{
            .l = 15000,
            .t = 15000,
            .r = 15000,
            .b = 15000,
          },
        },
      },
      width_emu,
      height_emu
    ),
    make_gradient_shape(
      "CircleGradient",
      "Path: circle",
      7,
      xlsdraw::drawing::GradientFill{
        .stops = {
          {.position = 0, .color = xlsdraw::drawing::Color{"FFFFC000"}},
          {.position = 100000, .color = xlsdraw::drawing::Color{"FFFFF2CC"}},
        },
        .shade = xlsdraw::drawing::PathGradient{
          .path = xlsdraw::drawing::PathGradientType::Circle,
          .fill_to_rect = xlsdraw::drawing::GradientFillToRect{
            .l = 20000,
            .t = 10000,
            .r = 20000,
            .b = 10000,
          },
        },
      },
      width_emu,
      height_emu
    ),
    make_gradient_shape(
      "RectGradient",
      "Path: rect",
      10,
      xlsdraw::drawing::GradientFill{
        .stops = {
          {.position = 0, .color = xlsdraw::drawing::Color{"FFC55A11"}},
          {.position = 100000, .color = xlsdraw::drawing::Color{"FFFCE4D6"}},
        },
        .shade = xlsdraw::drawing::PathGradient{
          .path = xlsdraw::drawing::PathGradientType::Rect,
          .fill_to_rect = xlsdraw::drawing::GradientFillToRect{
            .l = 10000,
            .t = 25000,
            .r = 10000,
            .b = 25000,
          },
        },
      },
      width_emu,
      height_emu
    ),
  };

  for (auto const& shape : shapes) {
    auto const added = builder.add_shape(shape);
    if (!added) {
      std::cerr << added.error() << '\n';
      return 1;
    }
  }

  auto const saved = builder.save();
  if (!saved) {
    std::cerr << saved.error() << '\n';
    return 1;
  }

  std::cout << "Generated test12.xlsx with gradient-filled shapes.\n";
  return 0;
}
