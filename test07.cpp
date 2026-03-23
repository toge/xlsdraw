#include <iostream>

#include "fmt/core.h"

#include "xlsdraw/units.hpp"
#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"
#include "xlsdraw/resource.hpp"

int main() {
  auto const converter = xlsdraw::units::EmuConverter{96.0}; // 標準DPI
  auto const emu_w = converter.pixels_to_emu(200).value_or(0);
  auto const emu_h = converter.pixels_to_emu(100).value_or(0);

  auto my_shape = xlsdraw::drawing::Shape{
    .id = 0,
    .name = "MyBlueRect",
    .type = "rect",
    .from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0}, // B2
    .to = {.col = 1, .col_off = emu_w, .row = 1, .row_off = emu_h}, // 同一セル内でサイズを表現
    .color_argb = "FF4472C4",
    .text = "",
    .anchor = xlsdraw::drawing::AnchorType::MoveButNoSize,
    .style = {
      .fill = {
        .solid_fill = xlsdraw::drawing::Color{"FF4F81BD"},
      },
      .line = {
        .width_emu = 12700, // 約1pt
        .color = xlsdraw::drawing::Color{"FF1F497D"},
      },
    },
    .text_body = xlsdraw::drawing::TextBody{
      .paragraphs = {
        {
          .runs = {{.text = "Hello C++26", .color = xlsdraw::drawing::Color{"FFFF0000"}, .font_size = 14.0,}},
          .alignment = "ctr",
        } // 赤文字
      }
    },
  };

  auto drawing_mgr = xlsdraw::drawing::DrawingManager{};
  drawing_mgr.add_shape(std::move(my_shape));

  auto rel_manager = xlsdraw::resource::RelationshipManager{};
  auto const drawing_rid = rel_manager.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
    "../drawings/drawing1.xml"
  );

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

  auto const worksheet_xml = fmt::format(
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
      "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
      "<sheetData/>"
      "<drawing r:id=\"{}\"/>"
    "</worksheet>", drawing_rid
  );

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
  content_types_mgr.add_override(
    "xl/drawings/drawing1.xml",
    "application/vnd.openxmlformats-officedocument.drawing+xml"
  );

  auto writer = xlsdraw::io::ArchiveWriter("test07.xlsx");
  auto const write_result = writer.write_files_detailed({
    {"[Content_Types].xml", content_types_mgr.generate_xml()},
    {"_rels/.rels", package_rel_manager.generate_xml()},
    {"xl/workbook.xml", workbook_xml},
    {"xl/_rels/workbook.xml.rels", workbook_rel_manager.generate_xml()},
    {"xl/worksheets/sheet1.xml", worksheet_xml},
    {"xl/worksheets/_rels/sheet1.xml.rels", rel_manager.generate_xml()},
    {"xl/drawings/drawing1.xml", drawing_mgr.generate_xml()},
  });

  if (!write_result) {
    std::cerr << "failed to write required XLSX entries: path='"
              << write_result.error().internal_path
              << "', message='"
              << write_result.error().message
              << "'\n";
    return 1;
  }

  return 0;
}
