#include "catch2/catch_all.hpp"

#include "xlsdraw/drawing.hpp"

TEST_CASE("preset shape catalog exposes phase1 families") {
  auto const presets = xlsdraw::drawing::supported_preset_shapes();

  CHECK(presets.size() == 133);
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

TEST_CASE("drawing generator emits linear gradient fills ahead of solid fills") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "GradientRect"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FF445566"};
  shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
    .stops = {
      {.position = 0, .color = xlsdraw::drawing::Color{"FF112233"}},
      {.position = 100000, .color = xlsdraw::drawing::Color{"FFABCDEF"}},
    },
    .shade = xlsdraw::drawing::LinearGradient{
      .angle = 5400000,
      .scaled = false,
    },
  };

  REQUIRE(manager.add_shape(std::move(shape)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("<a:gradFill>") != std::string::npos);
  CHECK(xml.find("<a:gs pos=\"0\">") != std::string::npos);
  CHECK(xml.find("<a:gs pos=\"100000\">") != std::string::npos);
  CHECK(xml.find("<a:srgbClr val=\"FF112233\"/>") != std::string::npos);
  CHECK(xml.find("<a:srgbClr val=\"FFABCDEF\"/>") != std::string::npos);
  CHECK(xml.find("<a:lin ang=\"5400000\" scaled=\"0\"/>") != std::string::npos);
  CHECK(xml.find("<a:solidFill>") == std::string::npos);
}

TEST_CASE("drawing generator emits path gradient fills for all supported path kinds") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto make_shape = [](std::string name, int col, xlsdraw::drawing::PathGradientType path_type) {
    auto shape = xlsdraw::drawing::make_preset_shape(
      xlsdraw::drawing::PresetShape::Rect,
      std::move(name)
    );
    shape.from = {.col = col, .col_off = 0, .row = 1, .row_off = 0};
    shape.to = {.col = col, .col_off = 100, .row = 1, .row_off = 100};
    shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
      .stops = {
        {.position = 0, .color = xlsdraw::drawing::Color{"FF0000FF"}},
        {.position = 100000, .color = xlsdraw::drawing::Color{"FFFFFFFF"}},
      },
      .shade = xlsdraw::drawing::PathGradient{
        .path = path_type,
        .fill_to_rect = xlsdraw::drawing::GradientFillToRect{
          .l = 10000,
          .t = 20000,
          .r = 30000,
          .b = 40000,
        },
      },
    };
    return shape;
  };

  REQUIRE(manager.add_shape(make_shape("ShapePath", 1, xlsdraw::drawing::PathGradientType::Shape)).has_value());
  REQUIRE(manager.add_shape(make_shape("CirclePath", 3, xlsdraw::drawing::PathGradientType::Circle)).has_value());
  REQUIRE(manager.add_shape(make_shape("RectPath", 5, xlsdraw::drawing::PathGradientType::Rect)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("<a:path path=\"shape\">") != std::string::npos);
  CHECK(xml.find("<a:path path=\"circle\">") != std::string::npos);
  CHECK(xml.find("<a:path path=\"rect\">") != std::string::npos);
  CHECK(xml.find("<a:fillToRect l=\"10000\" t=\"20000\" r=\"30000\" b=\"40000\"/>") != std::string::npos);
  CHECK(xml.find("<a:srgbClr val=\"FF0000FF\"/>") != std::string::npos);
  CHECK(xml.find("<a:srgbClr val=\"FFFFFFFF\"/>") != std::string::npos);
}

TEST_CASE("drawing generator omits unset fillToRect attributes") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "PartialFillToRect"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
    .stops = {
      {.position = 0, .color = xlsdraw::drawing::Color{"FF0000FF"}},
      {.position = 100000, .color = xlsdraw::drawing::Color{"FFFFFFFF"}},
    },
    .shade = xlsdraw::drawing::PathGradient{
      .path = xlsdraw::drawing::PathGradientType::Shape,
      .fill_to_rect = xlsdraw::drawing::GradientFillToRect{
        .l = 10000,
        .t = std::nullopt,
        .r = std::nullopt,
        .b = 40000,
      },
    },
  };

  REQUIRE(manager.add_shape(std::move(shape)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("<a:fillToRect l=\"10000\" b=\"40000\"/>") != std::string::npos);
  CHECK(xml.find(" t=\"0\"") == std::string::npos);
  CHECK(xml.find(" r=\"0\"") == std::string::npos);
}

TEST_CASE("drawing generator skips incomplete gradient fills with fewer than two stops") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "IncompleteGradient"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
    .stops = {
      {.position = 0, .color = xlsdraw::drawing::Color{"FF112233"}},
    },
    .shade = xlsdraw::drawing::LinearGradient{
      .angle = 0,
      .scaled = false,
    },
  };

  REQUIRE(manager.add_shape(std::move(shape)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("<a:gradFill>") == std::string::npos);
  CHECK(xml.find("<a:noFill/>") != std::string::npos);
}

TEST_CASE("drawing generator keeps gradient precedence even when the gradient is incomplete") {
  auto manager = xlsdraw::drawing::DrawingManager{};

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::Rect,
    "IncompleteGradientWithSolid"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = 100, .row = 1, .row_off = 100};
  shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FF445566"};
  shape.style.fill.gradient_fill = xlsdraw::drawing::GradientFill{
    .stops = {
      {.position = 0, .color = xlsdraw::drawing::Color{"FF112233"}},
    },
    .shade = xlsdraw::drawing::LinearGradient{
      .angle = 0,
      .scaled = false,
    },
  };

  REQUIRE(manager.add_shape(std::move(shape)).has_value());

  auto const xml = manager.generate_xml();
  CHECK(xml.find("<a:gradFill>") == std::string::npos);
  CHECK(xml.find("<a:solidFill>") == std::string::npos);
  CHECK(xml.find("<a:noFill/>") != std::string::npos);
}

TEST_CASE("new preset shapes are correctly mapped") {
  using namespace xlsdraw::drawing;

  CHECK(preset_shape_prst(PresetShape::Pentagon) == "pentagon");
  CHECK(preset_shape_prst(PresetShape::Heart) == "heart");
  CHECK(preset_shape_prst(PresetShape::MathPlus) == "mathPlus");
  CHECK(preset_shape_prst(PresetShape::Star5) == "star5");
  CHECK(preset_shape_prst(PresetShape::Ribbon) == "ribbon");
  CHECK(preset_shape_prst(PresetShape::Snip1Rect) == "snip1Rect");

  CHECK(
    preset_shape_family(PresetShape::Star5)
      == ShapePresetFamily::StarAndBanner
  );
  CHECK(
    preset_shape_family(PresetShape::MathPlus)
      == ShapePresetFamily::Equation
  );
}

TEST_CASE("anchor type maps to correct editAs attribute") {
  using namespace xlsdraw::drawing;
  auto const make = [](AnchorType a) {
    auto s = make_preset_shape(PresetShape::Rect, "R");
    s.from = {.col = 0, .col_off = 0, .row = 0, .row_off = 0};
    s.to   = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
    s.anchor = a;
    DrawingManager mgr;
    std::ignore = mgr.add_shape(std::move(s));
    return mgr.generate_xml();
  };

  CHECK(make(AnchorType::MoveAndSize).find("editAs=\"twoCell\"") != std::string::npos);
  CHECK(make(AnchorType::MoveButNoSize).find("editAs=\"oneCell\"") != std::string::npos);
  CHECK(make(AnchorType::NoMoveNoSize).find("editAs=\"absolute\"") != std::string::npos);
}

TEST_CASE("DrawingManager rejects shape with empty geometry") {
  auto mgr = xlsdraw::drawing::DrawingManager{};
  auto s = xlsdraw::drawing::Shape{};
  s.type = "";  // 空のジオメトリ
  s.from = {.col = 0, .col_off = 0, .row = 0, .row_off = 0};
  s.to   = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};

  auto result = mgr.add_shape(std::move(s));
  REQUIRE(!result.has_value());
  CHECK(result.error() == xlsdraw::drawing::DrawingManager::Error::InvalidGeometry);
  // 失敗時に ID が進んでいないことを確認する
  CHECK(mgr.shape_count() == 0);
}

TEST_CASE("DrawingManager rejects shape with invalid position") {
  auto mgr = xlsdraw::drawing::DrawingManager{};
  auto s = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect, "R");
  // to.col < from.col → InvalidPosition
  s.from = {.col = 5, .col_off = 0, .row = 1, .row_off = 0};
  s.to   = {.col = 3, .col_off = 0, .row = 5, .row_off = 0};

  auto result = mgr.add_shape(std::move(s));
  REQUIRE(!result.has_value());
  CHECK(result.error() == xlsdraw::drawing::DrawingManager::Error::InvalidPosition);
  CHECK(mgr.shape_count() == 0);
}

TEST_CASE("DrawingManager next_id does not advance on failed add") {
  auto mgr = xlsdraw::drawing::DrawingManager{};

  // 失敗させる
  auto bad = xlsdraw::drawing::Shape{};
  bad.type = "";
  bad.from = {0, 0, 0, 0};
  bad.to   = {1, 0, 1, 0};
  REQUIRE(!mgr.add_shape(std::move(bad)).has_value());

  // 成功した図形の ID が 1 から始まることを確認する
  auto good = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect);
  good.from = {0, 0, 0, 0};
  good.to   = {1, 0, 1, 0};
  auto r = mgr.add_shape(std::move(good));
  REQUIRE(r.has_value());
  CHECK(*r == 1);
}

TEST_CASE("DrawingManager get_shape_mut returns pointer on hit and nullptr on miss") {
  auto mgr = xlsdraw::drawing::DrawingManager{};
  auto s = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect, "R1");
  s.from = {0, 0, 0, 0};
  s.to   = {1, 0, 1, 0};
  auto r = mgr.add_shape(std::move(s));
  REQUIRE(r.has_value());

  auto* hit = mgr.get_shape_mut(*r);
  REQUIRE(hit != nullptr);
  CHECK(hit->name == "R1");

  auto* miss = mgr.get_shape_mut(9999);
  CHECK(miss == nullptr);
}

TEST_CASE("xml_escape is applied to TextRun text in generated XML") {
  auto mgr = xlsdraw::drawing::DrawingManager{};
  auto s = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect, "Escape");
  s.from = {0, 0, 0, 0};
  s.to   = {1, 0, 1, 0};
  s.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {{
      .runs = {{.text = "<hello & \"world\">", .color = std::nullopt}}
    }}
  };
  REQUIRE(mgr.add_shape(std::move(s)).has_value());

  auto const xml = mgr.generate_xml();
  CHECK(xml.find("&lt;hello &amp; &quot;world&quot;&gt;") != std::string::npos);
  CHECK(xml.find("<hello & \"world\">") == std::string::npos);
}

TEST_CASE("xml_escape is applied to shape name in generated XML") {
  auto mgr = xlsdraw::drawing::DrawingManager{};
  auto s = xlsdraw::drawing::make_preset_shape(xlsdraw::drawing::PresetShape::Rect, "A&B<C>");
  s.from = {0, 0, 0, 0};
  s.to   = {1, 0, 1, 0};
  REQUIRE(mgr.add_shape(std::move(s)).has_value());

  auto const xml = mgr.generate_xml();
  CHECK(xml.find("name=\"A&amp;B&lt;C&gt;") != std::string::npos);
  CHECK(xml.find("name=\"A&B<C>") == std::string::npos);
}
