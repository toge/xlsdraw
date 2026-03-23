#ifndef C888F150_6E10_4925_AE14_FCD9ECF976BA
#define C888F150_6E10_4925_AE14_FCD9ECF976BA

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "fmt/core.h"

#include "xlsdraw/units.hpp"

namespace xlsdraw::drawing {

struct Marker {
  int32_t col;
  int64_t col_off;
  int32_t row;
  int64_t row_off;

  auto to_xml(std::string_view tag) const {
    return fmt::format(
      "<{0}>"
        "<xdr:col>{1}</xdr:col>"
        "<xdr:colOff>{2}</xdr:colOff>"
        "<xdr:row>{3}</xdr:row>"
        "<xdr:rowOff>{4}</xdr:rowOff>"
      "</{0}>",
      tag, col, col_off, row, row_off
    );
  }
};

enum class AnchorType { MoveAndSize, MoveButNoSize, NoMoveNoSize };

enum class ShapeRenderKind { Shape, Connector };

enum class ShapePresetFamily { Basic, BlockArrow, Flowchart, Callout, Connector };

enum class PresetShape {
  Rect,
  RoundRect,
  Ellipse,
  Triangle,
  RtTriangle,
  Diamond,
  Parallelogram,
  Trapezoid,
  Hexagon,
  Octagon,
  Plus,
  Can,
  Cube,
  Bevel,
  Donut,
  NoSmoking,
  LeftArrow,
  RightArrow,
  UpArrow,
  DownArrow,
  LeftRightArrow,
  UpDownArrow,
  QuadArrow,
  LeftRightUpArrow,
  BentArrow,
  UturnArrow,
  LeftUpArrow,
  BentUpArrow,
  StripedRightArrow,
  NotchedRightArrow,
  HomePlate,
  Chevron,
  Callout1,
  Callout2,
  Callout3,
  AccentCallout1,
  AccentCallout2,
  AccentCallout3,
  BorderCallout1,
  BorderCallout2,
  BorderCallout3,
  AccentBorderCallout1,
  AccentBorderCallout2,
  AccentBorderCallout3,
  CloudCallout,
  WedgeRectCallout,
  WedgeRoundRectCallout,
  WedgeEllipseCallout,
  FlowChartProcess,
  FlowChartDecision,
  FlowChartInputOutput,
  FlowChartPredefinedProcess,
  FlowChartInternalStorage,
  FlowChartDocument,
  FlowChartTerminator,
  FlowChartPreparation,
  FlowChartManualInput,
  FlowChartManualOperation,
  FlowChartConnector,
  FlowChartOffpageConnector,
  FlowChartPunchedCard,
  FlowChartPunchedTape,
  FlowChartSummingJunction,
  FlowChartOr,
  FlowChartCollate,
  FlowChartSort,
  FlowChartExtract,
  FlowChartMerge,
  FlowChartStoredData,
  FlowChartDelay,
  FlowChartMagneticDisk,
  FlowChartDirectAccessStorage,
  FlowChartDisplay,
  StraightConnector1,
  BentConnector2,
  BentConnector3,
  BentConnector4,
  BentConnector5,
  CurvedConnector2,
  CurvedConnector3,
  CurvedConnector4,
  CurvedConnector5,
};

struct PresetShapeInfo {
  PresetShape preset;
  std::string_view prst;
  std::string_view default_name;
  ShapePresetFamily family;
  ShapeRenderKind render_kind;
};

// Phase 1 では、Excel で利用頻度の高い標準オートシェイプを
// 基本図形・ブロック矢印・フローチャートの3 family で公開する。
inline constexpr auto kSupportedPresetShapes = std::array{
  PresetShapeInfo{PresetShape::Rect, "rect", "Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::RoundRect, "roundRect", "Rounded Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Ellipse, "ellipse", "Ellipse", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Triangle, "triangle", "Triangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::RtTriangle, "rtTriangle", "Right Triangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Diamond, "diamond", "Diamond", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Parallelogram, "parallelogram", "Parallelogram", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Trapezoid, "trapezoid", "Trapezoid", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Hexagon, "hexagon", "Hexagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Octagon, "octagon", "Octagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Plus, "plus", "Plus", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Can, "can", "Can", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Cube, "cube", "Cube", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Bevel, "bevel", "Bevel", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Donut, "donut", "Donut", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::NoSmoking, "noSmoking", "No Smoking", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::LeftArrow, "leftArrow", "Left Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::RightArrow, "rightArrow", "Right Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::UpArrow, "upArrow", "Up Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::DownArrow, "downArrow", "Down Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::LeftRightArrow, "leftRightArrow", "Left Right Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::UpDownArrow, "upDownArrow", "Up Down Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::QuadArrow, "quadArrow", "Quad Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::LeftRightUpArrow, "leftRightUpArrow", "Left Right Up Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BentArrow, "bentArrow", "Bent Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::UturnArrow, "uturnArrow", "U-Turn Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::LeftUpArrow, "leftUpArrow", "Left Up Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BentUpArrow, "bentUpArrow", "Bent Up Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::StripedRightArrow, "stripedRightArrow", "Striped Right Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::NotchedRightArrow, "notchedRightArrow", "Notched Right Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::HomePlate, "homePlate", "Home Plate", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Chevron, "chevron", "Chevron", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Callout1, "callout1", "Callout 1", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Callout2, "callout2", "Callout 2", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Callout3, "callout3", "Callout 3", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentCallout1, "accentCallout1", "Accent Callout 1", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentCallout2, "accentCallout2", "Accent Callout 2", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentCallout3, "accentCallout3", "Accent Callout 3", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BorderCallout1, "borderCallout1", "Border Callout 1", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BorderCallout2, "borderCallout2", "Border Callout 2", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BorderCallout3, "borderCallout3", "Border Callout 3", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentBorderCallout1, "accentBorderCallout1", "Accent Border Callout 1", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentBorderCallout2, "accentBorderCallout2", "Accent Border Callout 2", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::AccentBorderCallout3, "accentBorderCallout3", "Accent Border Callout 3", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::CloudCallout, "cloudCallout", "Cloud Callout", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::WedgeRectCallout, "wedgeRectCallout", "Wedge Rect Callout", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::WedgeRoundRectCallout, "wedgeRoundRectCallout", "Wedge Round Rect Callout", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::WedgeEllipseCallout, "wedgeEllipseCallout", "Wedge Ellipse Callout", ShapePresetFamily::Callout, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartProcess, "flowChartProcess", "Process", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDecision, "flowChartDecision", "Decision", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartInputOutput, "flowChartInputOutput", "Input Output", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartPredefinedProcess, "flowChartPredefinedProcess", "Predefined Process", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartInternalStorage, "flowChartInternalStorage", "Internal Storage", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDocument, "flowChartDocument", "Document", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartTerminator, "flowChartTerminator", "Terminator", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartPreparation, "flowChartPreparation", "Preparation", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartManualInput, "flowChartManualInput", "Manual Input", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartManualOperation, "flowChartManualOperation", "Manual Operation", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartConnector, "flowChartConnector", "Connector", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartOffpageConnector, "flowChartOffpageConnector", "Offpage Connector", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartPunchedCard, "flowChartPunchedCard", "Punched Card", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartPunchedTape, "flowChartPunchedTape", "Punched Tape", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartSummingJunction, "flowChartSummingJunction", "Summing Junction", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartOr, "flowChartOr", "Or", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartCollate, "flowChartCollate", "Collate", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartSort, "flowChartSort", "Sort", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartExtract, "flowChartExtract", "Extract", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartMerge, "flowChartMerge", "Merge", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartStoredData, "flowChartStoredData", "Stored Data", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDelay, "flowChartDelay", "Delay", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartMagneticDisk, "flowChartMagneticDisk", "Magnetic Disk", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDirectAccessStorage, "flowChartDirectAccessStorage", "Direct Access Storage", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDisplay, "flowChartDisplay", "Display", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::StraightConnector1, "straightConnector1", "Straight Connector", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::BentConnector2, "bentConnector2", "Bent Connector 2", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::BentConnector3, "bentConnector3", "Bent Connector 3", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::BentConnector4, "bentConnector4", "Bent Connector 4", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::BentConnector5, "bentConnector5", "Bent Connector 5", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::CurvedConnector2, "curvedConnector2", "Curved Connector 2", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::CurvedConnector3, "curvedConnector3", "Curved Connector 3", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::CurvedConnector4, "curvedConnector4", "Curved Connector 4", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
  PresetShapeInfo{PresetShape::CurvedConnector5, "curvedConnector5", "Curved Connector 5", ShapePresetFamily::Connector, ShapeRenderKind::Connector},
};

[[nodiscard]]
inline auto supported_preset_shapes() noexcept -> std::span<PresetShapeInfo const> {
  return kSupportedPresetShapes;
}

[[nodiscard]]
inline auto preset_shape_info(PresetShape preset) noexcept -> PresetShapeInfo const& {
  auto const it = std::find_if(
    kSupportedPresetShapes.begin(),
    kSupportedPresetShapes.end(),
    [preset](PresetShapeInfo const& info) { return info.preset == preset; }
  );
  return (it != kSupportedPresetShapes.end()) ? *it : kSupportedPresetShapes.front();
}

[[nodiscard]]
inline auto preset_shape_prst(PresetShape preset) noexcept -> std::string_view {
  return preset_shape_info(preset).prst;
}

[[nodiscard]]
inline auto preset_shape_default_name(PresetShape preset) noexcept -> std::string_view {
  return preset_shape_info(preset).default_name;
}

[[nodiscard]]
inline auto preset_shape_family(PresetShape preset) noexcept -> ShapePresetFamily {
  return preset_shape_info(preset).family;
}

[[nodiscard]]
inline auto preset_shape_render_kind(PresetShape preset) noexcept -> ShapeRenderKind {
  return preset_shape_info(preset).render_kind;
}

struct Image {
  uint32_t id;
  std::string name;
  std::string rId;
  Marker from, to;
};

struct Color {
  std::string argb{"FF000000"}; // デフォルト黒

  auto to_xml() const -> std::string {
    // a:srgbClr タグを使用
    return fmt::format("<a:srgbClr val=\"{}\"/>", argb);
  }
};

struct LineProperties {
  std::optional<int64_t> width_emu; // EMU単位の太さ
  std::optional<Color> color;
  std::string_view cap_type{"flat"}; // flat, rnd, sq

  auto generate_xml() const -> std::string {
    auto xml = std::string{"<a:ln"};
    if (width_emu) {
      xml += fmt::format(" w=\"{}\"", *width_emu);
    }
    xml += ">";
    if (color) {
      xml += color->to_xml();
    }
    xml += "</a:ln>";
    return xml;
  }
};

struct FillProperties {
  std::optional<Color> solid_fill;
  // 将来的には gradient_fill などを追加可能

  auto generate_xml() const -> std::string {
    if (solid_fill) {
      return fmt::format("<a:solidFill>{}</a:solidFill>", solid_fill->to_xml());
    }
    return "<a:noFill/>"; // デフォルトは塗りつぶしなし
  }
};

struct TextRun {
  std::string text;
  std::optional<Color> color;
  double font_size{11.0}; // ポイント単位

  auto generate_xml() const {
    // 1ポイント = 100 ユニット (OpenXML仕様)
    auto const sz = static_cast<int>(font_size * 100);

    auto rPr = fmt::format("<a:rPr lang=\"ja-JP\" sz=\"{}\">", sz);
    if (color) {
      rPr += color->to_xml();
    }
    rPr += "</a:rPr>";

    return fmt::format("<a:r>{0}<a:t>{1}</a:t></a:r>", rPr, text);
  }
};

struct Paragraph {
  std::vector<TextRun> runs;
  std::string_view alignment{"ctr"}; // l, ctr, r

  auto generate_xml() const {
    auto xml = fmt::format("<a:p><a:pPr algn=\"{}\"/>", alignment);
    for (auto const& run : runs) {
      xml += run.generate_xml();
    }
    xml += "</a:p>";
    return xml;
  }
};

struct TextBody {
  std::vector<Paragraph> paragraphs;
  bool vertical_centered{true};

  auto generate_xml() const {
    auto const anchor = vertical_centered ? "ctr" : "t";
    auto xml = fmt::format("<xdr:txBody><a:bodyPr anchor=\"{}\" rtlCol=\"0\"><a:spAutoFit/></a:bodyPr><a:lstStyle/>", anchor);
    for (auto const& p : paragraphs) {
      xml += p.generate_xml();
    }
    xml += "</xdr:txBody>";
    return xml;
  }
};

struct ShapeStyle {
  FillProperties fill;
  LineProperties line;
};

struct Shape {
  uint32_t id;
  std::string name;
  std::string type;
  std::optional<PresetShape> preset_shape;
  std::optional<ShapeRenderKind> render_kind;
  Marker from, to;
  std::string color_argb{"FF4472C4"};
  std::string text;
  AnchorType anchor{AnchorType::MoveButNoSize};
  ShapeStyle style;
  std::optional<TextBody> text_body; // 追加
};

[[nodiscard]]
inline auto make_preset_shape(PresetShape preset, std::string name = {}) -> Shape {
  auto shape = Shape{};
  shape.preset_shape = preset;
  shape.render_kind = preset_shape_render_kind(preset);
  shape.type = std::string(preset_shape_prst(preset));
  shape.name = name.empty() ? std::string(preset_shape_default_name(preset)) : std::move(name);
  return shape;
}

[[nodiscard]]
inline auto make_connector_shape(PresetShape preset, std::string name = {}) -> Shape {
  auto shape = make_preset_shape(preset, std::move(name));
  shape.render_kind = ShapeRenderKind::Connector;
  return shape;
}

[[nodiscard]]
inline auto shape_geometry_prst(Shape const& shape) noexcept -> std::string_view {
  if (shape.preset_shape) {
    return preset_shape_prst(*shape.preset_shape);
  }
  return shape.type;
}

[[nodiscard]]
inline auto shape_render_kind(Shape const& shape) noexcept -> ShapeRenderKind {
  if (shape.render_kind) {
    return *shape.render_kind;
  }
  if (shape.preset_shape) {
    return preset_shape_render_kind(*shape.preset_shape);
  }
  return ShapeRenderKind::Shape;
}

struct CellOffset {
  int32_t col;
  int32_t row;
  int64_t col_offset_emu;
  int64_t row_offset_emu;
};

class CellAnchorCalculator {
public:
  using EmuConverter = units::EmuConverter;

  // 列幅(ピクセル)と行高さ(ピクセル)のリストを保持している想定
  // 実際には Worksheet クラスから参照する
  auto compute_offset(
    int const col, int const row,
    int const offsetX_px, int const offsetY_px,
    EmuConverter const& converter
  ) const -> std::expected<CellOffset, units::UnitError> {
    auto const x_emu = converter.pixels_to_emu(offsetX_px);
    auto const y_emu = converter.pixels_to_emu(offsetY_px);

    if (!x_emu || !y_emu) {
      return std::unexpected(units::UnitError::NegativeValue);
    }

    return CellOffset{
      .col = col,
      .row = row,
      .col_offset_emu = *x_emu,
      .row_offset_emu = *y_emu
    };
  }
};


using namespace std::string_view_literals;

class DrawingGenerator {
public:
  // 全体のXMLを組み立てる
  auto generate(std::vector<Shape> const& shapes) const {
    auto xml = std::string{header_};

    for (auto const& shape : shapes) {
      xml += generate_anchor(shape);
    }

    xml += "</xdr:wsDr>";
    return xml;
  }

private:
  static auto default_tx_body_xml() -> std::string {
    return "<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p/></xdr:txBody>";
  }

  auto generate_style_xml() const -> std::string {
    return std::string{
      "<xdr:style>"
        "<a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
        "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"
        "<a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
        "<a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef>"
      "</xdr:style>"
    };
  }

  auto generate_spPr(Shape const& shape) const {
    return fmt::format(
      "<xdr:spPr>"
        "<a:xfrm>"
          "<a:off x=\"0\" y=\"0\"/><a:ext cx=\"0\" cy=\"0\"/>"
        "</a:xfrm>"
        "<a:prstGeom prst=\"{0}\"><a:avLst/></a:prstGeom>"
        "{1}" // Fill
        "{2}" // Line
      "</xdr:spPr>",
      shape_geometry_prst(shape),
      shape.style.fill.generate_xml(),
      shape.style.line.generate_xml()
    );
  }

  auto generate_shape_anchor(Shape const& shape) const -> std::string {
    auto const tx_body_xml = shape.text_body
          ? shape.text_body->generate_xml()
          : default_tx_body_xml();

    return fmt::format(
      "<xdr:twoCellAnchor editAs=\"oneCell\">"
        "{0}{1}" // from, to
        "<xdr:sp>"
          "<xdr:nvSpPr>"
            "<xdr:cNvPr id=\"{2}\" name=\"{3} {2}\"/>"
            "<xdr:cNvSpPr/>"
          "</xdr:nvSpPr>"
          "{4}" // spPr (スタイル適用)
          "{5}" // style
          "{6}" // txBody
        "</xdr:sp>"
        "<xdr:clientData/>"
      "</xdr:twoCellAnchor>",
      shape.from.to_xml("xdr:from"),
      shape.to.to_xml("xdr:to"),
      shape.id,
      shape.name,
      generate_spPr(shape),
      generate_style_xml(),
      tx_body_xml
    );
  }

  auto generate_connector_anchor(Shape const& shape) const -> std::string {
    auto const tx_body_xml = shape.text_body
          ? shape.text_body->generate_xml()
          : std::string{};

    return fmt::format(
      "<xdr:twoCellAnchor editAs=\"oneCell\">"
        "{0}{1}" // from, to
        "<xdr:cxnSp>"
          "<xdr:nvCxnSpPr>"
            "<xdr:cNvPr id=\"{2}\" name=\"{3} {2}\"/>"
            "<xdr:cNvCxnSpPr/>"
          "</xdr:nvCxnSpPr>"
          "{4}" // spPr
          "{5}" // style
          "{6}" // optional txBody
        "</xdr:cxnSp>"
        "<xdr:clientData/>"
      "</xdr:twoCellAnchor>",
      shape.from.to_xml("xdr:from"),
      shape.to.to_xml("xdr:to"),
      shape.id,
      shape.name,
      generate_spPr(shape),
      generate_style_xml(),
      tx_body_xml
    );
  }

  auto generate_anchor(Shape const& shape) const -> std::string {
    if (shape_render_kind(shape) == ShapeRenderKind::Connector) {
      return generate_connector_anchor(shape);
    }
    return generate_shape_anchor(shape);
  }

  // XMLヘッダー（名前空間定義）
  static constexpr auto header_ =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" "
    "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">"sv;
};


class DrawingManager {
public:
  // 図形追加時のエラー定義
  enum class Error { InvalidPosition, InvalidGeometry, DuplicateId };

  explicit
  DrawingManager(uint32_t start_id = 1)
  : next_id_(start_id)
  {}

  /**
   * @brief 図形を管理リストに追加し、自動的にIDを割り振る
   * @param shape 追加したい図形オブジェクト（IDは自動上書きされる）
   * @return 割り振られたID、またはエラー
   */
  auto add_shape(Shape shape) -> std::expected<uint32_t, Error> {
    // IDの自動採番
    shape.id = next_id_++;

    if (shape_geometry_prst(shape).empty()) {
      return std::unexpected(Error::InvalidGeometry);
    }

    // 簡易的なバリデーション: 終了セルが開始セルより前でないか
    if (shape.to.col < shape.from.col || shape.to.row < shape.from.row) {
      return std::unexpected(Error::InvalidPosition);
    }

    shapes_.push_back(std::move(shape));
    return shape.id;
  }

  /**
   * @brief 現在管理している全図形のXMLを生成する
   */
  [[nodiscard]]
  auto generate_xml() const -> std::string {
    DrawingGenerator generator;
    return generator.generate(shapes_);
  }

  /**
   * @brief 管理している図形の数を取得
   */
  [[nodiscard]]
  auto shape_count() const noexcept -> size_t {
    return shapes_.size();
  }

  /**
   * @brief 特定のIDを持つ図形を検索して編集用参照を返す
   */
  auto get_shape_mut(uint32_t id) -> Shape* {
    auto it = std::find_if(shapes_.begin(), shapes_.end(),
                           [id](auto const& s) { return s.id == id; });
    return (it != shapes_.end()) ? &(*it) : nullptr;
  }

private:
  uint32_t next_id_;
  std::vector<Shape> shapes_;
};

[[nodiscard]]
inline auto drawing_manager_error_message(DrawingManager::Error error) noexcept -> std::string_view {
  switch (error) {
    case DrawingManager::Error::InvalidPosition:
      return "invalid shape position";
    case DrawingManager::Error::InvalidGeometry:
      return "shape geometry is not set";
    case DrawingManager::Error::DuplicateId:
      return "duplicate shape id";
  }
  return "unknown drawing manager error";
}


}

#endif /* C888F150_6E10_4925_AE14_FCD9ECF976BA */
