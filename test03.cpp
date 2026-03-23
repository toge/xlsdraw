#include <iostream>

#include "xlsdraw/resource.hpp"
#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"

using namespace xlsdraw;

int main() {
  // このサンプルは「1シート + 1図形」の最小構成 XLSX を組み立てる。
  // 目的は、Drawing パーツと各 .rels のつながりを明示すること。

  // 1. リレーションの準備
  auto sheet_rel_manager = resource::RelationshipManager{};
  // worksheet -> drawing の参照 ID。sheet1.xml 内の <drawing r:id="..."> と対になる。
  auto const drawing_rel_id = sheet_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
    "../drawings/drawing1.xml"
  );

  auto workbook_rel_manager = resource::RelationshipManager{};
  // workbook -> worksheet の参照 ID。workbook.xml 内の <sheet r:id="..."> に埋め込む。
  auto const sheet_rel_id = workbook_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
    "worksheets/sheet1.xml"
  );

  auto package_rel_manager = resource::RelationshipManager{};
  package_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
    "xl/workbook.xml"
  );

  // 2. Drawing XMLの生成 (前回のコードを使用)
  drawing::DrawingGenerator draw_gen;
  // 図形そのものは drawing1.xml 側に置き、sheet 側は r:id 参照のみを持つ。
  auto shape = drawing::Shape{
    .id = 1,
    .name = "Shape",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0},
    .to = {.col = 3, .col_off = 0, .row = 9, .row_off = 0},
    .color_argb = "FF4472C4",
    .text = "",
    .anchor = drawing::AnchorType::MoveButNoSize,
    .style = {},
    .text_body = std::nullopt,
  };
  shape.style.fill.solid_fill = drawing::Color{"FF4F81BD"};
  shape.style.line.color = drawing::Color{"FF1F497D"};
  shape.style.line.width_emu = 12700; // 約1pt

  auto const drawing_xml = draw_gen.generate({shape});

  // 3. Worksheet XMLの生成 (drawing参照を含む)
  // ここで drawing_rel_id を使うことで、sheet1.xml.rels の対象パーツへ解決される。
  auto const sheet_xml = fmt::format(
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
    "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
    "<sheetData/>"
    "<drawing r:id=\"{}\"/>"
    "</worksheet>",
    drawing_rel_id
  );

  auto const workbook_xml = fmt::format(
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
    "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
    "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"{}\"/></sheets>"
    "</workbook>",
    sheet_rel_id
  );

  auto content_types_mgr = resource::ContentTypesManager{};
  // Drawing パーツを含むため、Content Types の override も忘れずに追加する。
  content_types_mgr.add_override(
    "xl/workbook.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"
  );
  content_types_mgr.add_override(
    "xl/worksheets/sheet1.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"
  );
  content_types_mgr.add_override(
    "xl/drawings/drawing1.xml",
    "application/vnd.openxmlformats-officedocument.drawing+xml"
  );

  // 4. ZIPへの書き込み
  // OpenXML は ZIP 内の内部パス名で解決されるため、パスの綴りは厳密に一致させる。
  io::ArchiveWriter writer("test03.xlsx");
  auto const write_result = writer.write_files_detailed({
    {"[Content_Types].xml", content_types_mgr.generate_xml()},
    {"_rels/.rels", package_rel_manager.generate_xml()},
    {"xl/workbook.xml", workbook_xml},
    {"xl/_rels/workbook.xml.rels", workbook_rel_manager.generate_xml()},
    {"xl/worksheets/sheet1.xml", sheet_xml},
    {"xl/worksheets/_rels/sheet1.xml.rels", sheet_rel_manager.generate_xml()},
    {"xl/drawings/drawing1.xml", drawing_xml},
  });

  if (!write_result) {
    std::cerr << "failed to write required XLSX entries: path='"
              << write_result.error().internal_path
              << "', message='"
              << write_result.error().message
              << "'\n";
    return 1;
  }

  std::cout << "XLSX file generated with shapes.\n";

  return 0;
}
