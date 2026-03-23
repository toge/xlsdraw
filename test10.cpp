#include <array>
#include <iostream>
#include <string>

#include "fmt/core.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/workbook.hpp"

int main() {
  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{"test10.xlsx", "PresetShapes"};

  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const emu_w = converter.pixels_to_emu(180).value_or(0);
  auto const emu_h = converter.pixels_to_emu(80).value_or(0);

  auto const presets = std::array{
    xlsdraw::drawing::PresetShape::Rect,
    xlsdraw::drawing::PresetShape::RoundRect,
    xlsdraw::drawing::PresetShape::Ellipse,
    xlsdraw::drawing::PresetShape::Diamond,
    xlsdraw::drawing::PresetShape::Parallelogram,
    xlsdraw::drawing::PresetShape::RightArrow,
    xlsdraw::drawing::PresetShape::Chevron,
    xlsdraw::drawing::PresetShape::UturnArrow,
    xlsdraw::drawing::PresetShape::LeftRightArrow,
    xlsdraw::drawing::PresetShape::FlowChartProcess,
    xlsdraw::drawing::PresetShape::FlowChartDecision,
    xlsdraw::drawing::PresetShape::FlowChartInputOutput,
    xlsdraw::drawing::PresetShape::FlowChartDocument,
    xlsdraw::drawing::PresetShape::FlowChartConnector,
    xlsdraw::drawing::PresetShape::FlowChartDisplay,
  };

  for (auto i = std::size_t{0}; i < presets.size(); ++i) {
    auto const col_block = static_cast<int32_t>(i % 5);
    auto const row_block = static_cast<int32_t>(i / 5);
    auto const base_col = int32_t{1} + col_block * 2;
    auto const base_row = int32_t{1} + row_block * 3;

    auto shape = xlsdraw::drawing::make_preset_shape(
      presets[i],
      fmt::format("{}", xlsdraw::drawing::preset_shape_default_name(presets[i]))
    );
    shape.from = {.col = base_col, .col_off = 0, .row = base_row, .row_off = 0};
    shape.to = {.col = base_col, .col_off = emu_w, .row = base_row, .row_off = emu_h};
    shape.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
    shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FFDEEAF6"};
    shape.style.line.width_emu = 12700;
    shape.style.line.color = xlsdraw::drawing::Color{"FF4472C4"};
    shape.text_body = xlsdraw::drawing::TextBody{
      .paragraphs = {
        {
          .runs = {{
            .text = std::string(xlsdraw::drawing::preset_shape_default_name(presets[i])),
            .color = xlsdraw::drawing::Color{"FF000000"},
            .font_size = 11.0
          }},
          .alignment = "ctr",
        }
      },
    };

    auto const added = builder.add_shape(std::move(shape));
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

  std::cout << "Generated test10.xlsx with preset shape samples.\n";
  return 0;
}
