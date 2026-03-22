#include <iostream>

#include "xlsdraw/relationship.hpp"
#include "xlsdraw/drawing_generator.hpp"
#include "xlsdraw/archive_manager.hpp"

using namespace xlsdraw;

int main() {
  // 1. リレーションの準備
  auto sheet_rel_manager = resource::RelationshipManager{};
  auto const drawing_rel_id = sheet_rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
    "../drawings/drawing1.xml"
  );

  auto workbook_rel_manager = resource::RelationshipManager{};
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
  auto shape = drawing::Shape{
    .id = 1,
    .name = "Shape",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0},
    .to = {.col = 3, .col_off = 0, .row = 9, .row_off = 0}
  };
  shape.style.fill.solid_fill = drawing::Color{"FF4F81BD"};
  shape.style.line.color = drawing::Color{"FF1F497D"};
  shape.style.line.width_emu = 12700; // 約1pt

  auto const drawing_xml = draw_gen.generate({shape});

  // 3. Worksheet XMLの生成 (drawing参照を含む)
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

  auto const content_types_xml =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
    "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
    "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
    "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
    "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
    "<Override PartName=\"/xl/drawings/drawing1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawing+xml\"/>"
    "</Types>";

  // 4. ZIPへの書き込み
  io::ArchiveWriter writer("test03.xlsx");
  auto ok = true;
  ok &= writer.write_file("[Content_Types].xml", content_types_xml);
  ok &= writer.write_file("_rels/.rels", package_rel_manager.generate_xml());
  ok &= writer.write_file("xl/workbook.xml", workbook_xml);
  ok &= writer.write_file("xl/_rels/workbook.xml.rels", workbook_rel_manager.generate_xml());
  ok &= writer.write_file("xl/worksheets/sheet1.xml", sheet_xml);
  ok &= writer.write_file("xl/worksheets/_rels/sheet1.xml.rels", sheet_rel_manager.generate_xml());
  ok &= writer.write_file("xl/drawings/drawing1.xml", drawing_xml);

  if (!ok) {
    std::cerr << "failed to write required XLSX entries\n";
    return 1;
  }

  std::cout << "XLSX file generated with shapes.\n";

  return 0;
}
