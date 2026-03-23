#include <filesystem>
#include <string>
#include <vector>

#include "catch2/catch_all.hpp"
#include "zip.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/workbook.hpp"

TEST_CASE("single sheet drawing workbook builder saves drawing workbook") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Shapes"
  };

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "Rect1"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{.text = "Hello workbook", .color = xlsdraw::drawing::Color{"FFFF0000"}, .font_size = 12.0}},
        .alignment = "ctr",
      }
    },
  };

  auto const added = builder.add_shape(std::move(shape));
  REQUIRE(added.has_value());
  CHECK(added->id == 1);
  CHECK(added->name == "Rect1");
  CHECK(builder.shape_count() == 1);

  auto const saved = builder.save();
  REQUIRE(saved.has_value());
  REQUIRE(std::filesystem::exists(archive_path));

  int err = 0;
  auto* archive = zip_open(archive_path.string().c_str(), ZIP_RDONLY, &err);
  REQUIRE(archive != nullptr);

  zip_stat_t stat{};
  REQUIRE(zip_stat(archive, "xl/drawings/drawing1.xml", 0, &stat) == 0);

  auto* file = zip_fopen(archive, "xl/drawings/drawing1.xml", 0);
  REQUIRE(file != nullptr);

  auto xml = std::string(static_cast<std::size_t>(stat.size), '\0');
  REQUIRE(zip_fread(file, xml.data(), xml.size()) == static_cast<zip_int64_t>(xml.size()));
  REQUIRE(zip_fclose(file) == 0);
  REQUIRE(zip_close(archive) == 0);

  CHECK(xml.find("Hello workbook") != std::string::npos);
  CHECK(xml.find("Rect1 1") != std::string::npos);

  std::filesystem::remove(archive_path);
}
