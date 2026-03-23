#include <charconv>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>

#include "fmt/core.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"
#include "xlsdraw/resource.hpp"
#include "xlsdraw/units.hpp"

namespace {

auto parse_shape_count(int argc, char* argv[]) -> std::expected<std::size_t, std::string> {
  // 引数未指定時はデモしやすい件数をデフォルト値として使う。
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
              << "usage: ./test08 [shape_count]\n";
    return 1;
  }

  auto const shape_count = *parsed_count;
  // 本サンプルは shape_count を変えても壊れない XLSX 構成を確認する目的。

  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const emu_w = converter.pixels_to_emu(180).value_or(0);
  auto const emu_h = converter.pixels_to_emu(80).value_or(0);

  auto drawing_mgr = xlsdraw::drawing::DrawingManager{};

  // 図形は 5 列グリッドで配置し、件数が増えても座標が単調に伸びるようにする。
  for (auto i = std::size_t{0}; i < shape_count; ++i) {
    auto const col_block = static_cast<int32_t>(i % 5);
    auto const row_block = static_cast<int32_t>(i / 5);

    auto const base_col = int32_t{1} + col_block * 2;
    auto const base_row = int32_t{1} + row_block * 3;

    auto shape = xlsdraw::drawing::Shape{};
    shape.id = 0;
    shape.name = fmt::format("Shape{}", i + 1);
    shape.type = "rect";
    shape.from = {.col = base_col, .col_off = 0, .row = base_row, .row_off = 0};
    shape.to = {.col = base_col, .col_off = emu_w, .row = base_row, .row_off = emu_h};
    shape.color_argb = "FF4472C4";
    shape.text = "";
    shape.anchor = xlsdraw::drawing::AnchorType::MoveButNoSize;
    shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FF4F81BD"};
    shape.style.line.width_emu = 12700;
    shape.style.line.color = xlsdraw::drawing::Color{"FF1F497D"};

    shape.text_body = xlsdraw::drawing::TextBody{
      .paragraphs = {
        {
          .runs = {{.text = fmt::format("Hello #{}", i + 1), .color = xlsdraw::drawing::Color{"FFFF0000"}, .font_size = 12.0}},
          .alignment = "ctr",
        }
      },
    };

    auto const add_result = drawing_mgr.add_shape(std::move(shape));
    if (!add_result) {
      std::cerr << "failed to add shape index=" << i << "\n";
      return 1;
    }
  }

  auto rel_manager = xlsdraw::resource::RelationshipManager{};
  auto drawing_rid = std::string{};
  // 図形が 0 件のときは drawing パーツ自体を作らない。
  // これにより「参照だけ存在する壊れた構成」を避ける。
  if (shape_count > 0) {
    drawing_rid = rel_manager.add_relationship(
      "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
      "../drawings/drawing1.xml"
    );
  }

  auto workbook_rel_manager = xlsdraw::resource::RelationshipManager{};
  auto const sheet_rel_id = workbook_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
    "worksheets/sheet1.xml"
  );

  auto package_rel_manager = xlsdraw::resource::RelationshipManager{};
  package_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
    "xl/workbook.xml"
  );

  auto const worksheet_xml = [drawing_rid]() {
    // drawing_rid が空なら worksheet 側にも <drawing> を出力しない。
    if (drawing_rid.empty()) {
      return std::string{
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
          "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
          "<sheetData/>"
        "</worksheet>"
      };
    }

    return fmt::format(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheetData/>"
        "<drawing r:id=\"{}\"/>"
      "</worksheet>",
      drawing_rid
    );
  }();

  auto const workbook_xml = fmt::format(
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
    "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
    "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"{}\"/></sheets>"
    "</workbook>",
    sheet_rel_id
  );

  auto content_types_mgr = xlsdraw::resource::ContentTypesManager{};
  content_types_mgr.add_override(
    "xl/workbook.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"
  );
  content_types_mgr.add_override(
    "xl/worksheets/sheet1.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"
  );
  if (shape_count > 0) {
    // drawing パーツを追加したときだけ ContentType override を宣言する。
    content_types_mgr.add_override(
      "xl/drawings/drawing1.xml",
      "application/vnd.openxmlformats-officedocument.drawing+xml"
    );
  }

  auto const content_types_xml = content_types_mgr.generate_xml();
  auto const package_rels_xml = package_rel_manager.generate_xml();
  auto const workbook_rels_xml = workbook_rel_manager.generate_xml();

  auto writer = xlsdraw::io::ArchiveWriter("test08.xlsx");

  // エラー出力の形式を統一する小さなヘルパー。
  auto const write_or_report = [&writer](std::string_view internal_path, std::string_view content) {
    auto const result = writer.write_file_detailed(internal_path, content);
    if (!result) {
      std::cerr << "failed to write required XLSX entries: path='"
                << result.error().internal_path
                << "', message='"
                << result.error().message
                << "'\n";
      return false;
    }
    return true;
  };

  if (!write_or_report("[Content_Types].xml", content_types_xml)) return 1;
  if (!write_or_report("_rels/.rels", package_rels_xml)) return 1;
  if (!write_or_report("xl/workbook.xml", workbook_xml)) return 1;
  if (!write_or_report("xl/_rels/workbook.xml.rels", workbook_rels_xml)) return 1;
  if (!write_or_report("xl/worksheets/sheet1.xml", worksheet_xml)) return 1;

  if (shape_count > 0) {
    // 図形ありの場合のみ、sheet rels と drawing 本体を書き込む。
    auto const sheet_rels_xml = rel_manager.generate_xml();
    auto const drawing_xml = drawing_mgr.generate_xml();

    if (!write_or_report("xl/worksheets/_rels/sheet1.xml.rels", sheet_rels_xml)) return 1;
    if (!write_or_report("xl/drawings/drawing1.xml", drawing_xml)) return 1;
  }

  return 0;
}
