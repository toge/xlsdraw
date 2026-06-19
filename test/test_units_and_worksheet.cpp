#include "catch2/catch_all.hpp"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/worksheet.hpp"

TEST_CASE("EmuConverter converts pixels to EMU using the standard DPI") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};

  auto const zero = converter.pixels_to_emu(0);
  REQUIRE(zero.has_value());
  CHECK(*zero == 0);

  // 96 DPI: 1 インチ = 96 px = 914400 EMU なので、96 px は 914400 EMU に等しい。
  auto const inch = converter.pixels_to_emu(96);
  REQUIRE(inch.has_value());
  CHECK(*inch == xlsdraw::units::EmuConverter::EmuPerInch);

  auto const negative = converter.pixels_to_emu(-1);
  REQUIRE(!negative.has_value());
  CHECK(negative.error() == xlsdraw::units::UnitError::NegativeValue);
}

TEST_CASE("EmuConverter converts inches to EMU") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};

  auto const zero = converter.inches_to_emu(0.0);
  REQUIRE(zero.has_value());
  CHECK(*zero == 0);

  auto const one = converter.inches_to_emu(1.0);
  REQUIRE(one.has_value());
  CHECK(*one == xlsdraw::units::EmuConverter::EmuPerInch);

  auto const half = converter.inches_to_emu(0.5);
  REQUIRE(half.has_value());
  CHECK(*half == xlsdraw::units::EmuConverter::EmuPerInch / 2);
}

TEST_CASE("EmuConverter falls back to default DPI when given non-positive values") {
  // 0 または負の DPI で構築しても、コンバータは有効な EMU 値を生成しなければならない。
  auto const converter = xlsdraw::units::EmuConverter{0.0};
  auto const px = converter.pixels_to_emu(96);
  REQUIRE(px.has_value());
  CHECK(*px == xlsdraw::units::EmuConverter::EmuPerInch);
}

TEST_CASE("CellAnchorCalculator computes EMU offsets for a pixel delta") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const calculator = xlsdraw::drawing::CellAnchorCalculator{};

  auto const result = calculator.compute_offset(2, 2, 10, 20, converter);
  REQUIRE(result.has_value());
  CHECK(result->col == 2);
  CHECK(result->row == 2);
  // 10px = 10/96 * 914400 = 95250 EMU
  CHECK(result->col_offset_emu == 95250);
  // 20px = 20/96 * 914400 = 190500 EMU
  CHECK(result->row_offset_emu == 190500);
}

TEST_CASE("CellAnchorCalculator rejects negative pixel inputs") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const calculator = xlsdraw::drawing::CellAnchorCalculator{};

  auto const result = calculator.compute_offset(0, 0, -1, 0, converter);
  REQUIRE(!result.has_value());
  CHECK(result.error() == xlsdraw::units::UnitError::NegativeValue);
}

TEST_CASE("WorksheetDrawingContext assigns sequential ids to simple rectangles") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto ctx = xlsdraw::worksheet::WorksheetDrawingContext{converter};

  auto const first = ctx.add_simple_rect(1, 1, 200, 100, "First");
  auto const second = ctx.add_simple_rect(4, 2, 150, 150, "Second");

  REQUIRE(first.has_value());
  REQUIRE(second.has_value());
  CHECK(*first == 1);
  CHECK(*second == 2);
  CHECK(*second != *first);
}

TEST_CASE("WorksheetDrawingContext produces drawing XML for simple rectangles") {
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto ctx = xlsdraw::worksheet::WorksheetDrawingContext{converter};

  auto const first = ctx.add_simple_rect(1, 1, 200, 100, "Hello");
  auto const second = ctx.add_simple_rect(4, 2, 150, 150, "World");
  REQUIRE(first.has_value());
  REQUIRE(second.has_value());

  auto const xml = ctx.final_xml();
  CHECK(xml.find("Hello") != std::string::npos);
  CHECK(xml.find("World") != std::string::npos);
  CHECK(xml.find("prst=\"rect\"") != std::string::npos);
  CHECK(xml.find("<xdr:sp>") != std::string::npos);
}
