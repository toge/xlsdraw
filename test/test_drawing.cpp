#include "catch2/catch_all.hpp"

#include "xlsdraw/drawing.hpp"

TEST_CASE("preset shape catalog exposes phase1 families") {
  auto const presets = xlsdraw::drawing::supported_preset_shapes();

  CHECK(presets.size() >= 70);
  CHECK(xlsdraw::drawing::preset_shape_prst(xlsdraw::drawing::PresetShape::Rect) == "rect");
  CHECK(xlsdraw::drawing::preset_shape_prst(xlsdraw::drawing::PresetShape::Chevron) == "chevron");
  CHECK(
    xlsdraw::drawing::preset_shape_prst(xlsdraw::drawing::PresetShape::FlowChartDecision)
      == "flowChartDecision"
  );
  CHECK(
    xlsdraw::drawing::preset_shape_family(xlsdraw::drawing::PresetShape::FlowChartDecision)
      == xlsdraw::drawing::ShapePresetFamily::Flowchart
  );
  CHECK(
    xlsdraw::drawing::preset_shape_family(xlsdraw::drawing::PresetShape::CloudCallout)
      == xlsdraw::drawing::ShapePresetFamily::Callout
  );
  CHECK(
    xlsdraw::drawing::preset_shape_render_kind(xlsdraw::drawing::PresetShape::StraightConnector1)
      == xlsdraw::drawing::ShapeRenderKind::Connector
  );
}

TEST_CASE("drawing generator emits custom text body") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto shape = xlsdraw::drawing::Shape{};
  shape.name = "Rect";
  shape.type = "rect";
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{.text = "Hello", .color = xlsdraw::drawing::Color{"FFFF0000"}, .font_size = 12.0}},
        .alignment = "ctr",
      }
    },
  };

  auto const added = manager.add_shape(std::move(shape));
  REQUIRE(added.has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("Hello") != std::string::npos);
  CHECK(xml.find("a:pPr algn=\"ctr\"") != std::string::npos);
  CHECK(xml.find("a:endParaRPr lang=\"ja-JP\"") == std::string::npos);
}

TEST_CASE("drawing generator emits preset geometry shapes") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto round_rect = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::RoundRect,
    "RoundRect"
  );
  round_rect.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  round_rect.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};

  auto chevron = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Chevron,
    "Chevron"
  );
  chevron.from = {.col = 3, .col_off = 0, .row = 1, .row_off = 0};
  chevron.to = {.col = 3, .col_off = 100, .row = 1, .row_off = 100};

  auto decision = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::FlowChartDecision,
    "Decision"
  );
  decision.from = {.col = 5, .col_off = 0, .row = 1, .row_off = 0};
  decision.to = {.col = 5, .col_off = 100, .row = 1, .row_off = 100};

  REQUIRE(manager.add_shape(std::move(round_rect)).has_value());
  REQUIRE(manager.add_shape(std::move(chevron)).has_value());
  REQUIRE(manager.add_shape(std::move(decision)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("prst=\"roundRect\"") != std::string::npos);
  CHECK(xml.find("prst=\"chevron\"") != std::string::npos);
  CHECK(xml.find("prst=\"flowChartDecision\"") != std::string::npos);
}

TEST_CASE("drawing generator emits callout and connector shapes") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto callout = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::CloudCallout,
    "Cloud"
  );
  callout.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  callout.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};

  auto connector = xlsdraw::drawing::make_connector_shape(
    xlsdraw::drawing::PresetShape::StraightConnector1,
    "Connector"
  );
  connector.from = {.col = 3, .col_off = 0, .row = 1, .row_off = 0};
  connector.to = {.col = 5, .col_off = 100, .row = 1, .row_off = 100};
  connector.style.line.width_emu = 12700;
  connector.style.line.color = xlsdraw::drawing::Color{"FF4472C4"};

  REQUIRE(manager.add_shape(std::move(callout)).has_value());
  REQUIRE(manager.add_shape(std::move(connector)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("prst=\"cloudCallout\"") != std::string::npos);
  CHECK(xml.find("<xdr:cxnSp>") != std::string::npos);
  CHECK(xml.find("prst=\"straightConnector1\"") != std::string::npos);
  CHECK(xml.find("<xdr:nvCxnSpPr>") != std::string::npos);
  CHECK(
    xml.find("</xdr:cxnSp><xdr:clientData/></xdr:twoCellAnchor>")
      != std::string::npos
  );
}
