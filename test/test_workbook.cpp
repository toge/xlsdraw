#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "catch2/catch_all.hpp"
#include "zip.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"
#include "xlsdraw/workbook.hpp"

// ─── テストヘルパー ───────────────────────────────────────────────────────────

/**
 * @brief テスト用に ZIP アーカイブを読み取り専用でオープンします。
 * @param[in] path アーカイブのパスです。
 * @return 成功時は zip_t* ポインタ、失敗時は nullptr を返します。
 */
static auto open_zip_for_test(std::filesystem::path const& path) -> zip_t* {
  int err = 0;
  return zip_open(path.string().c_str(), ZIP_RDONLY, &err);
}

/**
 * @brief ZIP アーカイブ内の指定エントリを文字列として読み出します。
 * @param[in] archive オープン済みアーカイブです。
 * @param[in] entry_name 読み出すエントリ名です。
 * @return 成功時はエントリ内容、失敗時は std::nullopt を返します。
 */
static auto read_zip_entry(zip_t* archive, std::string_view entry_name) -> std::optional<std::string> {
  zip_stat_t stat{};
  if (zip_stat(archive, entry_name.data(), 0, &stat) != 0) {
    return std::nullopt;
  }
  auto* file = zip_fopen(archive, entry_name.data(), 0);
  if (!file) {
    return std::nullopt;
  }
  auto content = std::string(static_cast<std::size_t>(stat.size), '\0');
  if (zip_fread(file, content.data(), content.size()) != static_cast<zip_int64_t>(content.size())) {
    zip_fclose(file);
    return std::nullopt;
  }
  zip_fclose(file);
  return content;
}

// ─── テストケース ─────────────────────────────────────────────────────────────

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

  REQUIRE(builder.save().has_value());
  REQUIRE(std::filesystem::exists(archive_path));

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  auto const xml = read_zip_entry(archive, "xl/drawings/drawing1.xml");
  REQUIRE(xml.has_value());
  REQUIRE(zip_close(archive) == 0);

  CHECK(xml->find("Hello workbook") != std::string::npos);
  CHECK(xml->find("Rect1 1") != std::string::npos);

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

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  auto const xml = read_zip_entry(archive, "xl/drawings/drawing1.xml");
  REQUIRE(xml.has_value());
  REQUIRE(zip_close(archive) == 0);

  CHECK(xml->find("cloudCallout") != std::string::npos);
  CHECK(xml->find("<xdr:cxnSp>") != std::string::npos);
  CHECK(xml->find("bentConnector2") != std::string::npos);
  CHECK(xml->find("</xdr:cxnSp><xdr:clientData/></xdr:twoCellAnchor>") != std::string::npos);

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

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  auto const xml = read_zip_entry(archive, "xl/drawings/drawing1.xml");
  REQUIRE(xml.has_value());
  REQUIRE(zip_close(archive) == 0);

  CHECK(xml->find("<a:gradFill>") != std::string::npos);
  CHECK(xml->find("<a:lin ang=\"5400000\" scaled=\"0\"/>") != std::string::npos);
  CHECK(xml->find("<a:solidFill>") == std::string::npos);

  std::filesystem::remove(archive_path);
}

TEST_CASE("single sheet workbook builder omits drawing parts when no shapes are added") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_empty.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Empty"
  };

  CHECK(builder.shape_count() == 0);
  REQUIRE(builder.save().has_value());
  REQUIRE(std::filesystem::exists(archive_path));

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  // 描画パーツとそれを参照するシートの rels は書き込まれてはならない。
  zip_stat_t stat{};
  CHECK(zip_stat(archive, "xl/drawings/drawing1.xml", 0, &stat) != 0);
  CHECK(zip_stat(archive, "xl/worksheets/_rels/sheet1.xml.rels", 0, &stat) != 0);

  // ワークブックとワークシートは依然として存在しなければならない。
  CHECK(zip_stat(archive, "xl/workbook.xml", 0, &stat) == 0);
  CHECK(zip_stat(archive, "xl/worksheets/sheet1.xml", 0, &stat) == 0);

  REQUIRE(zip_close(archive) == 0);
  std::filesystem::remove(archive_path);
}

TEST_CASE("save() rejects empty sheet name") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_emptyname.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    ""  // 空のシート名
  };

  auto const result = builder.save();
  REQUIRE(!result.has_value());
  CHECK(result.error().find("sheet name") != std::string::npos);

  std::filesystem::remove(archive_path);
}

TEST_CASE("save() writes all 6 required XLSX parts when shapes exist") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_allparts.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Sheet1"
  };

  auto shape = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect, "R");
  shape.from = {.col = 0, .col_off = 0, .row = 0, .row_off = 0};
  shape.to   = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  REQUIRE(builder.add_shape(std::move(shape)).has_value());
  REQUIRE(builder.save().has_value());

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  // 6 パーツすべての存在を確認する
  auto const required_parts = std::vector<std::string_view>{
    "[Content_Types].xml",
    "_rels/.rels",
    "xl/workbook.xml",
    "xl/_rels/workbook.xml.rels",
    "xl/worksheets/sheet1.xml",
    "xl/worksheets/_rels/sheet1.xml.rels",
    "xl/drawings/drawing1.xml",
  };
  for (auto const& part : required_parts) {
    zip_stat_t stat{};
    INFO("part: " << part);
    CHECK(zip_stat(archive, part.data(), 0, &stat) == 0);
  }

  // [Content_Types].xml に各パートの MIME タイプが含まれることを確認する
  auto const ct = read_zip_entry(archive, "[Content_Types].xml");
  REQUIRE(ct.has_value());
  CHECK(ct->find("spreadsheetml.sheet.main") != std::string::npos);
  CHECK(ct->find("spreadsheetml.worksheet") != std::string::npos);
  CHECK(ct->find("drawing") != std::string::npos);

  REQUIRE(zip_close(archive) == 0);
  std::filesystem::remove(archive_path);
}

TEST_CASE("save() applies xml_escape to sheet name") {
  auto const archive_path = std::filesystem::current_path() / "test_workbook_escape.xlsx";
  std::filesystem::remove(archive_path);

  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    archive_path.string(),
    "Sheet<1>&\"2\""  // XML 特殊文字を含むシート名
  };
  REQUIRE(builder.save().has_value());

  auto* archive = open_zip_for_test(archive_path);
  REQUIRE(archive != nullptr);

  auto const wb_xml = read_zip_entry(archive, "xl/workbook.xml");
  REQUIRE(wb_xml.has_value());
  REQUIRE(zip_close(archive) == 0);

  // エスケープされた形式で埋め込まれていることを確認する
  CHECK(wb_xml->find("Sheet&lt;1&gt;&amp;&quot;2&quot;") != std::string::npos);
  // 生の特殊文字が埋め込まれていないことを確認する
  CHECK(wb_xml->find("Sheet<1>") == std::string::npos);

  std::filesystem::remove(archive_path);
}

TEST_CASE("ArchiveWriter is_open returns true after successful construction") {
  auto const path = std::filesystem::current_path() / "test_is_open.zip";
  std::filesystem::remove(path);

  {
    auto writer = xlsdraw::io::ArchiveWriter{path.string()};
    // libzip は ZIP_CREATE|ZIP_TRUNCATE では zip_open はほぼ常に成功する
    CHECK(writer.is_open());

    // close() を呼んだ後は is_open() が false を返す
    REQUIRE(writer.close().has_value());
    CHECK(!writer.is_open());

    // close 後の write_file_detailed はエラーを返す
    auto const r = writer.write_file_detailed("x.xml", "<x/>");
    CHECK(!r.has_value());
  }

  std::filesystem::remove(path);
}
