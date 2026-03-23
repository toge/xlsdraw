#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "fmt/core.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/workbook.hpp"

namespace {

auto parse_shape_count(int argc, char* argv[]) -> std::expected<std::size_t, std::string> {
  if (argc < 2) {
    return std::size_t{10};
  }

  auto count = std::size_t{};
  auto const input = std::string_view{argv[1]};
  auto const result = std::from_chars(input.data(), input.data() + input.size(), count);

  if (result.ec != std::errc{} || result.ptr != input.data() + input.size()) {
    return std::unexpected(fmt::format("invalid shape count: '{}'", input));
  }

  return count;
}

} // namespace

int main(int argc, char* argv[]) {
  auto const parsed_count = parse_shape_count(argc, argv);
  if (!parsed_count) {
    std::cerr << parsed_count.error() << "\n"
              << "usage: ./test09 [shape_count]\n";
    return 1;
  }

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{"test09.xlsx"};

  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const emu_w = converter.pixels_to_emu(180).value_or(0);
  auto const emu_h = converter.pixels_to_emu(80).value_or(0);

  auto rect_template = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "Rect"
  );
  rect_template.id = 0;
  rect_template.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  rect_template.to = {.col = 1, .col_off = emu_w, .row = 1, .row_off = emu_h};
  rect_template.color_argb = "FF4472C4";
  rect_template.text = "";
  rect_template.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
  rect_template.style.fill.solid_fill = xlsdraw::drawing::Color{"FF4F81BD"};
  rect_template.style.line.width_emu = 12700;
  rect_template.style.line.color = xlsdraw::drawing::Color{"FF1F497D"};

  rect_template.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{.text = "Hello from template", .color = xlsdraw::drawing::Color{"FFFF0000"}, .font_size = 12.0}},
        .alignment = "ctr",
      }
    },
  };

  auto shape_handles = std::vector<xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder::ShapeHandle>{};

  for (auto i = std::size_t{0}; i < *parsed_count; ++i) {
    auto const col_block = static_cast<int32_t>(i % 5);
    auto const row_block = static_cast<int32_t>(i / 5);

    auto const base_col = int32_t{1} + col_block * 2;
    auto const base_row = int32_t{1} + row_block * 3;

    auto shape = rect_template;
    shape.id = 0;
    shape.name = fmt::format("{}{}", rect_template.name, i + 1);
    shape.from = {.col = base_col, .col_off = 0, .row = base_row, .row_off = 0};
    shape.to = {.col = base_col, .col_off = emu_w, .row = base_row, .row_off = emu_h};

    auto const populate_result = builder.add_shape(std::move(shape));
    if (!populate_result) {
      std::cerr << populate_result.error() << "\n";
      return 1;
    }

    // save() 前でも、追加済み図形の id/name を後から参照できるように保持する。
    shape_handles.push_back(*populate_result);
  }

  auto const save_result = builder.save();
  if (!save_result) {
    std::cerr << save_result.error() << "\n";
    return 1;
  }

  return 0;
}
