#include <array>
#include <iostream>
#include <string>

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/workbook.hpp"

namespace {

auto make_node_shape(
  std::string name,
  std::string label,
  int32_t col,
  int32_t row,
  int64_t width_emu,
  int64_t height_emu
) -> xlsdraw::drawing::Shape {
  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::RoundRect,
    std::move(name)
  );
  shape.from = {.col = col, .col_off = 0, .row = row, .row_off = 0};
  shape.to = {.col = col, .col_off = width_emu, .row = row, .row_off = height_emu};
  shape.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
  shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FFE2F0D9"};
  shape.style.line.width_emu = 12700;
  shape.style.line.color = xlsdraw::drawing::Color{"FF70AD47"};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{
          .text = std::move(label),
          .color = xlsdraw::drawing::Color{"FF000000"},
          .font_size = 11.0
        }},
        .alignment = "ctr",
      }
    },
  };
  return shape;
}

auto make_connector_shape(
  xlsdraw::drawing::PresetShape preset,
  std::string name,
  int32_t from_col,
  int32_t from_row,
  int32_t to_col,
  int32_t to_row,
  int64_t col_off_emu,
  int64_t row_off_emu,
  std::string color
) -> xlsdraw::drawing::Shape {
  auto connector = xlsdraw::drawing::make_connector_shape(preset, std::move(name));
  connector.from = {.col = from_col, .col_off = col_off_emu, .row = from_row, .row_off = row_off_emu};
  connector.to = {.col = to_col, .col_off = col_off_emu, .row = to_row, .row_off = row_off_emu};
  connector.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
  connector.style.line.width_emu = 19050;
  connector.style.line.color = xlsdraw::drawing::Color{std::move(color)};
  return connector;
}

} // namespace

int main() {
  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{"test11.xlsx", "Connectors"};

  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const node_width_emu = converter.pixels_to_emu(180).value_or(0);
  auto const node_height_emu = converter.pixels_to_emu(72).value_or(0);
  auto const center_x_emu = converter.pixels_to_emu(90).value_or(0);
  auto const center_y_emu = converter.pixels_to_emu(36).value_or(0);

  auto const nodes = std::array{
    make_node_shape("InputNode", "Input", 1, 1, node_width_emu, node_height_emu),
    make_node_shape("DecisionNode", "Decision", 4, 1, node_width_emu, node_height_emu),
    make_node_shape("ProcessNode", "Process", 7, 1, node_width_emu, node_height_emu),
    make_node_shape("ArchiveNode", "Archive", 4, 4, node_width_emu, node_height_emu),
  };

  for (auto const& node : nodes) {
    auto const added = builder.add_shape(node);
    if (!added) {
      std::cerr << added.error() << '\n';
      return 1;
    }
  }

  auto const connectors = std::array{
    make_connector_shape(
      xlsdraw::drawing::PresetShape::StraightConnector1,
      "StraightConnector",
      1, 1, 4, 1, center_x_emu, center_y_emu, "FF4472C4"
    ),
    make_connector_shape(
      xlsdraw::drawing::PresetShape::BentConnector2,
      "BentConnector2",
      4, 1, 4, 4, center_x_emu, center_y_emu, "FFED7D31"
    ),
    make_connector_shape(
      xlsdraw::drawing::PresetShape::BentConnector3,
      "BentConnector3",
      4, 1, 7, 4, center_x_emu, center_y_emu, "FF70AD47"
    ),
    make_connector_shape(
      xlsdraw::drawing::PresetShape::CurvedConnector3,
      "CurvedConnector3",
      1, 1, 7, 4, center_x_emu, center_y_emu, "FF7030A0"
    ),
  };

  for (auto const& connector : connectors) {
    auto const added = builder.add_shape(connector);
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

  std::cout << "Generated test11.xlsx with connector examples.\n";
  return 0;
}
