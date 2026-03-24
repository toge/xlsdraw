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

TEST_CASE("single sheet workbook builder writes connector and callout XML") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_connector.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Connectors"
  };

  auto callout = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::CloudCallout,
    "Cloud"
  );
  callout.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  callout.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};

  auto connector = xlsdraw::drawing::make_connector_shape(
    xlsdraw::drawing::PresetShape::BentConnector2,
    "BentConnector"
  );
  connector.from = {.col = 3, .col_off = 0, .row = 1, .row_off = 0};
  connector.to = {.col = 5, .col_off = 100, .row = 1, .row_off = 100};
  connector.style.line.width_emu = 12700;
  connector.style.line.color = xlsdraw::drawing::Color{"FF1F497D"};

  REQUIRE(builder.add_shape(std::move(callout)).has_value());
  REQUIRE(builder.add_shape(std::move(connector)).has_value());
  REQUIRE(builder.save().has_value());

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

  CHECK(xml.find("cloudCallout") != std::string::npos);
  CHECK(xml.find("<xdr:cxnSp>") != std::string::npos);
  CHECK(xml.find("bentConnector2") != std::string::npos);
  CHECK(xml.find("</xdr:cxnSp><xdr:clientData/></xdr:twoCellAnchor>") != std::string::npos);

  std::filesystem::remove(archive_path);
}

TEST_CASE("single sheet workbook builder writes gradient fill XML") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_gradient.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Gradients"
  };

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "GradientRect"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
    .stops = {
      {.position = 0, .color = xlsdraw::drawing::Color{"FF0000FF"}},
      {.position = 100000, .color = xlsdraw::drawing::Color{"FFFFFFFF"}},
    },
    .shade = xlsdraw::drawing::LinearGradient{
      .angle = 5400000,
      .scaled = false,
    },
  };

  REQUIRE(builder.add_shape(std::move(shape)).has_value());
  REQUIRE(builder.save().has_value());

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

  CHECK(xml.find("<a:gradFill>") != std::string::npos);
  CHECK(xml.find("<a:lin ang=\"5400000\" scaled=\"0\"/>") != std::string::npos);
  CHECK(xml.find("<a:solidFill>") == std::string::npos);

  std::filesystem::remove(archive_path);
}
