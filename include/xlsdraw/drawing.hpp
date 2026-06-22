#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "fmt/format.h"

#include "xlsdraw/units.hpp"
#include "xlsdraw/xml_escape.hpp"

/**
 * @file drawing.hpp
 * @brief SpreadsheetDrawing XML を構築する図形モデルと生成器を定義します。
 */

namespace xlsdraw::drawing {

/**
 * @brief 2 セルアンカー上の始点または終点を表します。
 */
struct Marker {
  int32_t col;      ///< 列インデックスです。
  int64_t col_off;  ///< 列方向オフセット (EMU) です。
  int32_t row;      ///< 行インデックスです。
  int64_t row_off;  ///< 行方向オフセット (EMU) です。

  /**
   * @brief OpenXML のマーカー要素に変換します。
   * @param[in] tag 出力する要素名です。例: `xdr:from` / `xdr:to`。
   * @return 指定タグ名で包んだ XML を返します。
   */
  [[nodiscard]]
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

/**
 * @brief 図形のアンカー挙動（セル移動・サイズ変更への追従）です。
 */
enum class AnchorType {
  MoveAndSize,  ///< セルの移動とサイズ変更に合わせて図形も移動・変形します。
  MoveButNoSize, ///< セルの移動には追従しますが、図形のサイズは固定されます。
  NoMoveNoSize   ///< セルの移動やサイズ変更に関わらず、図形の絶対位置とサイズを維持します。
};

/**
 * @brief 描画要素のレンダリング種別です。
 *
 * OpenXML では通常の図形 (`xdr:sp`) とコネクタ (`xdr:cxnSp`) で要素名や
 * 属性の構成が異なるため、この種別で区別します。
 */
enum class ShapeRenderKind {
  Shape,     ///< 一般的な図形（Rectangle, Ellipse など）です。
  Connector  ///< 図形間を結ぶコネクタ（直線、カギ線など）です。
};

/**
 * @brief プリセット図形の表示上のカテゴリ（Excel のメニュー構成に準拠）です。
 */
enum class ShapePresetFamily {
  Basic,          ///< 基本図形（四角形、円、三角形など）です。
  BlockArrow,     ///< ブロック矢印です。
  Flowchart,      ///< フローチャート記号です。
  Callout,        ///< 吹き出しです。
  Connector,      ///< コネクタ（線）です。
  StarAndBanner,  ///< 星とリボンです。
  Equation        ///< 数式図形です。
};

/**
 * @brief サポートしている OpenXML プリセット図形です。
 */
enum class PresetShape {
  // 基本図形
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
  Pentagon,
  Heptagon,
  Decagon,
  Dodecagon,
  Plus,
  Can,
  Cube,
  Bevel,
  Donut,
  NoSmoking,
  Pie,
  Chord,
  Teardrop,
  Frame,
  HalfFrame,
  Corner,
  DiagStripe,
  Heart,
  LightningBolt,
  SmileyFace,
  Sun,
  Moon,
  Cloud,
  Arc,
  BlockArc,
  // 数式図形
  MathPlus,
  MathMinus,
  MathMultiply,
  MathDivide,
  MathEqual,
  MathNotEqual,
  // ブロック矢印
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
  CircularArrow,
  // 吹き出し
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
  // 星とリボン
  Star4,
  Star5,
  Star8,
  Star10,
  Star12,
  Star16,
  Star24,
  Star32,
  IrregularSeal1,
  IrregularSeal2,
  Ribbon,
  Ribbon2,
  VerticalScroll,
  HorizontalScroll,
  Wave,
  DoubleWave,
  // フローチャート
  FlowChartProcess,
  FlowChartDecision,
  FlowChartInputOutput,
  FlowChartPredefinedProcess,
  FlowChartInternalStorage,
  FlowChartDocument,
  FlowChartMultidocument,
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
  FlowChartAlternateProcess,
  // 調整可能四角形
  Snip1Rect,
  Round1Rect,
  Snip2SameRect,
  Round2SameRect,
  Snip2DiagRect,
  Round2DiagRect,
  SnipRoundRect,
  // コネクタ
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

/**
 * @brief プリセット図形のメタデータです。
 */
struct PresetShapeInfo {
  PresetShape preset;           ///< 列挙値です。
  std::string_view prst;        ///< OpenXML のプリセット名です。
  std::string_view default_name; ///< 既定の表示名です。
  ShapePresetFamily family;     ///< 図形カテゴリです。
  ShapeRenderKind render_kind;  ///< 描画種別です。
};

/**
 * @brief サポート対象のプリセット図形一覧です。
 *
 * 基本図形、数式図形、ブロック矢印、吹き出し、星とリボン、フローチャート、調整可能四角形、コネクタを収録します。
 */
inline auto constexpr kSupportedPresetShapes = std::array{
  // 基本図形
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
  PresetShapeInfo{PresetShape::Pentagon, "pentagon", "Pentagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Heptagon, "heptagon", "Heptagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Decagon, "decagon", "Decagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Dodecagon, "dodecagon", "Dodecagon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Plus, "plus", "Plus", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Can, "can", "Can", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Cube, "cube", "Cube", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Bevel, "bevel", "Bevel", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Donut, "donut", "Donut", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::NoSmoking, "noSmoking", "No Smoking", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Pie, "pie", "Pie", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Chord, "chord", "Chord", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Teardrop, "teardrop", "Teardrop", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Frame, "frame", "Frame", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::HalfFrame, "halfFrame", "Half Frame", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Corner, "corner", "Corner", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::DiagStripe, "diagStripe", "Diagonal Stripe", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Heart, "heart", "Heart", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::LightningBolt, "lightningBolt", "Lightning Bolt", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::SmileyFace, "smileyFace", "Smiley Face", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Sun, "sun", "Sun", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Moon, "moon", "Moon", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Cloud, "cloud", "Cloud", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Arc, "arc", "Arc", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::BlockArc, "blockArc", "Block Arc", ShapePresetFamily::Basic, ShapeRenderKind::Shape},

  // 数式図形
  PresetShapeInfo{PresetShape::MathPlus, "mathPlus", "Math Plus", ShapePresetFamily::Equation, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::MathMinus, "mathMinus", "Math Minus", ShapePresetFamily::Equation, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::MathMultiply, "mathMultiply", "Math Multiply", ShapePresetFamily::Equation, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::MathDivide, "mathDivide", "Math Divide", ShapePresetFamily::Equation, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::MathEqual, "mathEqual", "Math Equal", ShapePresetFamily::Equation, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::MathNotEqual, "mathNotEqual", "Math Not Equal", ShapePresetFamily::Equation, ShapeRenderKind::Shape},

  // ブロック矢印
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
  PresetShapeInfo{PresetShape::CircularArrow, "circularArrow", "Circular Arrow", ShapePresetFamily::BlockArrow, ShapeRenderKind::Shape},

  // 吹き出し
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

  // 星とリボン
  PresetShapeInfo{PresetShape::Star4, "star4", "4-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star5, "star5", "5-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star8, "star8", "8-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star10, "star10", "10-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star12, "star12", "12-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star16, "star16", "16-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star24, "star24", "24-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Star32, "star32", "32-Point Star", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::IrregularSeal1, "irregularSeal1", "Explosion 1", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::IrregularSeal2, "irregularSeal2", "Explosion 2", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Ribbon, "ribbon", "Ribbon", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Ribbon2, "ribbon2", "Ribbon 2", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::VerticalScroll, "verticalScroll", "Vertical Scroll", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::HorizontalScroll, "horizontalScroll", "Horizontal Scroll", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Wave, "wave", "Wave", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::DoubleWave, "doubleWave", "Double Wave", ShapePresetFamily::StarAndBanner, ShapeRenderKind::Shape},

  // フローチャート
  PresetShapeInfo{PresetShape::FlowChartProcess, "flowChartProcess", "Process", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDecision, "flowChartDecision", "Decision", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartInputOutput, "flowChartInputOutput", "Input Output", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartPredefinedProcess, "flowChartPredefinedProcess", "Predefined Process", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartInternalStorage, "flowChartInternalStorage", "Internal Storage", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartDocument, "flowChartDocument", "Document", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::FlowChartMultidocument, "flowChartMultidocument", "Multidocument", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},
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
  PresetShapeInfo{PresetShape::FlowChartAlternateProcess, "flowChartAlternateProcess", "Alternate Process", ShapePresetFamily::Flowchart, ShapeRenderKind::Shape},

  // 調整可能四角形
  PresetShapeInfo{PresetShape::Snip1Rect, "snip1Rect", "Snip Single Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Round1Rect, "round1Rect", "Round Single Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Snip2SameRect, "snip2SameRect", "Snip Same Side Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Round2SameRect, "round2SameRect", "Round Same Side Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Snip2DiagRect, "snip2DiagRect", "Snip Diagonal Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::Round2DiagRect, "round2DiagRect", "Round Diagonal Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},
  PresetShapeInfo{PresetShape::SnipRoundRect, "snipRoundRect", "Snip and Round Single Corner Rectangle", ShapePresetFamily::Basic, ShapeRenderKind::Shape},

  // コネクタ
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

/**
 * @brief サポート対象のプリセット図形一覧を返します。
 * @return @ref kSupportedPresetShapes への span です。
 */
[[nodiscard]]
inline auto supported_preset_shapes() noexcept -> std::span<PresetShapeInfo const> {
  return kSupportedPresetShapes;
}

// kSupportedPresetShapes の順序と PresetShape 列挙値の順序が一致することを静的検証する。
// これにより O(1) のインデックス直接参照の正当性を保証します。
static_assert(kSupportedPresetShapes[0].preset  == PresetShape::Rect,
              "kSupportedPresetShapes[0] must be Rect");
static_assert(kSupportedPresetShapes[35].preset == PresetShape::MathPlus,
              "kSupportedPresetShapes[35] must be MathPlus");
static_assert(kSupportedPresetShapes[132].preset == PresetShape::CurvedConnector5,
              "kSupportedPresetShapes[132] must be CurvedConnector5");
static_assert(kSupportedPresetShapes.size() == 133,
              "kSupportedPresetShapes must have exactly 133 entries");

/**
 * @brief プリセット図形のメタデータを返します。
 *
 * 列挙値を配列インデックスとして直接参照する O(1) 実装です。
 * @param[in] preset 取得対象のプリセット図形です。
 * @return 対応するメタデータです。
 */
[[nodiscard]]
inline auto preset_shape_info(PresetShape preset) noexcept -> PresetShapeInfo const& {
  auto const idx = static_cast<std::size_t>(preset);
  if (idx >= kSupportedPresetShapes.size()) {
    return kSupportedPresetShapes.front();
  }
  return kSupportedPresetShapes[idx];
}

/**
 * @brief プリセット図形の OpenXML 名を返します。
 * @param[in] preset プリセット図形です。
 * @return `a:prstGeom` に指定する名前です。
 */
[[nodiscard]]
inline auto preset_shape_prst(PresetShape preset) noexcept -> std::string_view {
  return preset_shape_info(preset).prst;
}

/**
 * @brief プリセット図形の既定表示名を返します。
 * @param[in] preset プリセット図形です。
 * @return 図形名として使う既定値です。
 */
[[nodiscard]]
inline auto preset_shape_default_name(PresetShape preset) noexcept -> std::string_view {
  return preset_shape_info(preset).default_name;
}

/**
 * @brief プリセット図形のカテゴリを返します。
 * @param[in] preset プリセット図形です。
 * @return 図形カテゴリを返します。
 */
[[nodiscard]]
inline auto preset_shape_family(PresetShape preset) noexcept -> ShapePresetFamily {
  return preset_shape_info(preset).family;
}

/**
 * @brief プリセット図形の描画種別を返します。
 * @param[in] preset プリセット図形です。
 * @return 通常図形かコネクタかを返します。
 */
[[nodiscard]]
inline auto preset_shape_render_kind(PresetShape preset) noexcept -> ShapeRenderKind {
  return preset_shape_info(preset).render_kind;
}


/**
 * @brief ARGB 形式の色情報を保持する構造体です。
 */
struct Color {
  /**
   * @brief 8 桁の ARGB 16 進数文字列（例: "FFFF0000" は不透明な赤）です。
   *
   * 既定値は "FF000000"（不透明な黒）です。
   */
  std::string argb{"FF000000"};

  /**
   * @brief 色情報を OpenXML の `a:srgbClr` 要素 XML に変換します。
   * @return 生成された XML 文字列です。
   */
  [[nodiscard]]
  auto generate_xml() const -> std::string {
    return fmt::format("<a:srgbClr val=\"{}\"/>", argb);
  }
};

/**
 * @brief 図形の輪郭線（アウトライン）のスタイルを定義します。
 */
struct LineProperties {
  std::optional<int64_t> width_emu;  ///< 線の太さを EMU 単位で指定します。未指定時はシステムの既定値が使われます。
  std::optional<Color> color;        ///< 線の色を指定します。
  std::string cap_type{"flat"}; ///< 線の端の形状（flat, round, sq）を指定します。

  /**
   * @brief 線のスタイルを OpenXML の `a:ln` 要素 XML に変換します。
   * @return 生成された XML 文字列です。
   */
  [[nodiscard]]
  auto generate_xml() const -> std::string {
    auto xml = std::string{"<a:ln"};
    if (width_emu) {
      xml += fmt::format(" w=\"{}\"", *width_emu);
    }
    xml += fmt::format(" cap=\"{}\">", cap_type);
    if (color) {
      xml += color->generate_xml();
    }
    xml += "</a:ln>";
    return xml;
  }
};

/**
 * @brief 図形の塗りつぶしスタイルを定義します。
 */
struct FillProperties {
  std::optional<Color> solid_fill; ///< 単色で塗りつぶす場合の色指定です。

  /**
   * @brief グラデーションの各変化点（ストップ）を定義します。
   */
  struct GradientStop {
    /**
     * @brief グラデーション全体における位置 (0-100000) です。
     *
     * 0 は開始点、100000 は終了点を表します。
     */
    int32_t position;
    Color color;      ///< この地点での色を指定します。

    /**
     * @brief 停止点情報を OpenXML の `a:gs` 要素 XML に変換します。
     * @return 生成された XML 文字列です。
     */
    [[nodiscard]]
    auto generate_xml() const -> std::string {
      auto const color_xml = color.generate_xml();
      return fmt::format("<a:gs pos=\"{}\">{}</a:gs>", position, color_xml);
    }
  };

  /**
   * @brief 線形グラデーションの設定を保持します。
   */
  struct LinearGradient {
    /**
     * @brief グラデーションの角度を 1/60000 度単位で指定します。
     *
     * 0 は右から左、5400000 (90度) は上から下へのグラデーションになります。
     */
    int32_t angle{0};
    bool scaled{false}; ///< グラデーション領域を塗りつぶし領域に合わせてスケーリングするかどうかを指定します。

    /**
     * @brief 線形グラデーション設定を OpenXML の `a:lin` 要素 XML に変換します。
     * @return 生成された XML 文字列です。
     */
    [[nodiscard]]
    auto generate_xml() const -> std::string {
      return fmt::format("<a:lin ang=\"{}\" scaled=\"{}\"/>", angle, scaled ? 1 : 0);
    }
  };

  /**
   * @brief パスグラデーション（円状など）の適用範囲を制限するための矩形領域を定義します。
   */
  struct GradientFillToRect {
    std::optional<int32_t> l; ///< 左端からのオフセットです。
    std::optional<int32_t> t; ///< 上端からのオフセットです。
    std::optional<int32_t> r; ///< 右端からのオフセットです。
    std::optional<int32_t> b; ///< 下端からのオフセットです。

    /**
     * @brief 適用範囲矩形を OpenXML の `a:fillToRect` 要素 XML に変換します。
     * @return 生成された XML 文字列です。
     */
    [[nodiscard]]
    auto generate_xml() const -> std::string {
      auto xml = std::string{"<a:fillToRect"};
      if (l) {
        xml += fmt::format(" l=\"{}\"", *l);
      }
      if (t) {
        xml += fmt::format(" t=\"{}\"", *t);
      }
      if (r) {
        xml += fmt::format(" r=\"{}\"", *r);
      }
      if (b) {
        xml += fmt::format(" b=\"{}\"", *b);
      }
      xml += "/>";
      return xml;
    }
  };

  /**
   * @brief パスグラデーションの形状パターンです。
   */
  enum class PathGradientType {
    Shape,  ///< 図形の形状に沿ったグラデーションです。
    Circle, ///< 円形のグラデーションです。
    Rect    ///< 矩形のグラデーションです。
  };

  /**
   * @brief パスグラデーション（円状、放射状など）の設定を保持します。
   */
  struct PathGradient {
    PathGradientType path{PathGradientType::Shape}; ///< グラデーションのパス種別を指定します。
    std::optional<GradientFillToRect> fill_to_rect; ///< グラデーションを適用する矩形範囲を制限します。

    /**
     * @brief パスグラデーションの設定を OpenXML の `a:path` 要素 XML に変換します。
     * @return 生成された XML 文字列です。
     */
    [[nodiscard]]
    auto generate_xml() const -> std::string {
      auto const path_name = [this]() -> std::string_view {
        switch (path) {
          case PathGradientType::Shape:
            return "shape";
          case PathGradientType::Circle:
            return "circle";
          case PathGradientType::Rect:
            return "rect";
        }
        return "shape";
      }();

      auto xml = fmt::format("<a:path path=\"{}\">", path_name);
      if (fill_to_rect) {
        xml += fill_to_rect->generate_xml();
      }
      xml += "</a:path>";
      return xml;
    }
  };

  /**
   * @brief グラデーション塗りつぶしの全体設定を保持します。
   */
  struct GradientFill {
    std::vector<GradientStop> stops; ///< グラデーションの変化点リストです。最低 2 つ必要です。
    std::variant<LinearGradient, PathGradient> shade; ///< グラデーションの種類（線形またはパス）です。

    /**
     * @brief グラデーション設定全体を OpenXML の `a:gradFill` 要素 XML に変換します。
     * @return 生成された XML 文字列です。停止点が不足している場合は空文字列を返します。
     */
    [[nodiscard]]
    auto generate_xml() const -> std::string {
      if (stops.size() < 2) {
        return {};
      }

      auto xml = std::string{"<a:gradFill><a:gsLst>"};
      for (auto const& stop : stops) {
        xml += stop.generate_xml();
      }
      xml += "</a:gsLst>";
      xml += std::visit([](auto const& value) { return value.generate_xml(); }, shade);
      xml += "</a:gradFill>";
      return xml;
    }
  };

  std::optional<GradientFill> gradient_fill; ///< グラデーション塗りつぶしを適用する場合の設定です。

  /**
   * @brief 塗りつぶし XML を生成します。
   * @return `a:solidFill` / `a:gradFill` / `a:noFill` のいずれかを返します。
   */
  [[nodiscard]]
  auto generate_xml() const -> std::string {
    if (gradient_fill) {
      auto const gradient_xml = gradient_fill->generate_xml();
      if (!gradient_xml.empty()) {
        return gradient_xml;
      }
      return "<a:noFill/>";
    }
    if (solid_fill) {
      return fmt::format("<a:solidFill>{}</a:solidFill>", solid_fill->generate_xml());
    }
    return "<a:noFill/>"; // デフォルトは塗りつぶしなし
  }
};

/// @brief @ref FillProperties::GradientStop の公開別名です。
using GradientStop = FillProperties::GradientStop;
/// @brief @ref FillProperties::LinearGradient の公開別名です。
using LinearGradient = FillProperties::LinearGradient;
/// @brief @ref FillProperties::GradientFillToRect の公開別名です。
using GradientFillToRect = FillProperties::GradientFillToRect;
/// @brief @ref FillProperties::PathGradient の公開別名です。
using PathGradient = FillProperties::PathGradient;
/// @brief @ref FillProperties::PathGradientType の公開別名です。
using PathGradientType = FillProperties::PathGradientType;
/// @brief @ref FillProperties::GradientFill の公開別名です。
using GradientFill = FillProperties::GradientFill;

/**
 * @brief 1 つのテキストランを表します。
 */
struct TextRun {
  std::string text;           ///< テキスト内容です。
  std::optional<Color> color; ///< 文字色です。
  double font_size{11.0};     ///< フォントサイズ (pt) です。

  /**
   * @brief テキストラン XML を生成します。
   * @return `a:r` 要素 XML を返します。
   */
  [[nodiscard]]
  auto generate_xml() const {
    auto const sz = static_cast<int>(font_size * 100);

    auto rPr = fmt::format("<a:rPr lang=\"ja-JP\" sz=\"{}\">", sz);
    if (color) {
      rPr += color->generate_xml();
    }
    rPr += "</a:rPr>";

    // ユーザ入力テキストを XML エスケープして埋め込む
    return fmt::format("<a:r>{0}<a:t>{1}</a:t></a:r>", rPr, detail::xml_escape(text));
  }
};

/**
 * @brief 段落を表します。
 */
struct Paragraph {
  std::vector<TextRun> runs;   ///< 含まれるテキストランです。
  std::string alignment{"ctr"}; ///< 段落配置です。

  /**
   * @brief 段落 XML を生成します。
   * @return `a:p` 要素 XML を返します。
   */
  [[nodiscard]]
  auto generate_xml() const {
    auto xml = fmt::format("<a:p><a:pPr algn=\"{}\"/>", alignment);
    for (auto const& run : runs) {
      xml += run.generate_xml();
    }
    xml += "</a:p>";
    return xml;
  }
};

/**
 * @brief 図形内のテキスト全体（xdr:txBody）の構成を保持します。
 */
struct TextBody {
  std::vector<Paragraph> paragraphs; ///< テキストに含まれる段落のリストです。
  bool vertical_centered{true};      ///< テキストを垂直方向に中央揃えにするかどうかを指定します。

  /**
   * @brief テキストボディ全体を OpenXML の `xdr:txBody` 要素 XML に変換します。
   * @return 生成された XML 文字列です。
   */
  [[nodiscard]]
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

/**
 * @brief 図形の外観（塗りつぶし、線）のスタイルをまとめた構造体です。
 */
struct ShapeStyle {
  FillProperties fill; ///< 塗りつぶしのスタイル設定です。
  LineProperties line; ///< 枠線のスタイル設定です。
};

/**
 * @brief 描画対象となる単一の図形（Shape/Connector）を表す中心的な構造体です。
 */
struct Shape {
  uint32_t id;                              ///< 図形を一意識別するための ID です。
  std::string name;                         ///< 図形の表示名（Excel のオブジェクト名）です。
  std::string type;                         ///< OpenXML のプリセット図形名（例: "rect"）です。
  std::optional<PresetShape> preset_shape;  ///< @ref PresetShape 列挙値による指定です。
  std::optional<ShapeRenderKind> render_kind; ///< 明示的に描画種別（Shape または Connector）を指定します。
  Marker from;                              ///< 図形の開始（左上）位置です。
  Marker to;                                ///< 図形の終了（右下）位置です。
  AnchorType anchor{AnchorType::MoveButNoSize}; ///< セルの移動やサイズ変更に対する図形の追従挙動を指定します。
  ShapeStyle style;                         ///< 塗りつぶしや線などの詳細なスタイル設定です。
  std::optional<TextBody> text_body;        ///< 段落や書式を含む詳細なリッチテキスト設定です。
};

/**
 * @brief 指定したプリセット図形に基づき、初期化された @ref Shape オブジェクトを作成します。
 * @param[in] preset 使用するプリセット図形の種類です。
 * @param[in] name 図形に付ける名前です。空の場合はプリセットの既定名が使用されます。
 * @return 初期化済みの図形オブジェクトです。
 */
[[nodiscard]]
inline auto make_preset_shape(PresetShape preset, std::string name = {}) -> Shape {
  auto shape = Shape{};
  shape.preset_shape = preset;
  shape.render_kind = preset_shape_render_kind(preset);
  shape.type = std::string(preset_shape_prst(preset));
  shape.name = name.empty() ? std::string(preset_shape_default_name(preset)) : std::move(name);
  return shape;
}

/**
 * @brief 指定したプリセット図形に基づき、コネクタとして初期化された @ref Shape オブジェクトを作成します。
 * @param[in] preset 使用するコネクタ用のプリセット図形の種類です。
 * @param[in] name コネクタに付ける名前です。
 * @return コネクタ設定が施された図形オブジェクトです。
 */
[[nodiscard]]
inline auto make_connector_shape(PresetShape preset, std::string name = {}) -> Shape {
  auto shape = make_preset_shape(preset, std::move(name));
  shape.render_kind = ShapeRenderKind::Connector;
  return shape;
}

/**
 * @brief 図形が使用する OpenXML のプリセット名文字列を取得します。
 * @param[in] shape 対象の図形オブジェクトです。
 * @return `a:prstGeom` の `prst` 属性に指定すべき文字列です。
 */
[[nodiscard]]
inline auto shape_geometry_prst(Shape const& shape) noexcept -> std::string_view {
  if (shape.preset_shape) {
    return preset_shape_prst(*shape.preset_shape);
  }
  return shape.type;
}

/**
 * @brief 図形の描画種別を取得します。
 * @param[in] shape 対象図形です。
 * @return 明示設定、プリセット推定、既定値の順で描画種別を返します。
 */
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

/**
 * @brief セル基準オフセットを表します。
 */
struct CellOffset {
  int32_t col;           ///< 列インデックスです。
  int32_t row;           ///< 行インデックスです。
  int64_t col_offset_emu; ///< 列方向オフセット (EMU) です。
  int64_t row_offset_emu; ///< 行方向オフセット (EMU) です。
};

/**
 * @brief ピクセルオフセットをセル基準オフセットへ変換します。
 */
class CellAnchorCalculator {
public:
  /// @brief 利用する EMU コンバータ型です。
  using EmuConverter = units::EmuConverter;

  /**
   * @brief セル位置とピクセルオフセットから EMU オフセットを計算します。
   * @param[in] col 基準列です。
   * @param[in] row 基準行です。
   * @param[in] offsetX_px X 方向オフセット (px) です。
   * @param[in] offsetY_px Y 方向オフセット (px) です。
   * @param[in] converter ピクセル→EMU 変換器です。
   * @return 成功時は @ref CellOffset 、失敗時は @ref units::UnitError を返します。
   */
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

/**
 * @brief @ref AnchorType を OpenXML の `editAs` 属性値に変換します。
 * @param[in] anchor 変換対象のアンカー種別です。
 * @return `editAs` 属性に指定する文字列を返します。
 */
[[nodiscard]]
inline auto anchor_type_to_edit_as(AnchorType anchor) noexcept -> std::string_view {
  switch (anchor) {
    case AnchorType::MoveAndSize:   return "twoCell";
    case AnchorType::MoveButNoSize: return "oneCell";
    case AnchorType::NoMoveNoSize:  return "absolute";
  }
  return "oneCell";
}

/**
 * @brief 図形配列から SpreadsheetDrawing XML を生成します。
 */
class DrawingGenerator {
public:
  /**
   * @brief 登録されたすべての図形を走査し、1 つの XML 文字列を生成します。
   * @param[in] shapes 生成対象の図形リストです。
   * @return `xl/drawings/drawing*.xml` として保存可能な XML 文字列です。
   */
  [[nodiscard]]
  auto generate(std::vector<Shape> const& shapes) const {
    auto xml = std::string{header_};
    // 1 図形あたり約 600 バイトを見積もって事前確保し O(N²) コピーを回避する
    xml.reserve(header_.size() + shapes.size() * 600 + 16);
    for (auto const& shape : shapes) {
      xml += generate_anchor(shape);
    }
    xml += "</xdr:wsDr>";
    return xml;
  }

private:
  /**
   * @brief 空のテキストボディ向けに最小限の XML を生成します。
   * @return `xdr:txBody` 要素 XML 文字列です。
   */
  static auto default_tx_body_xml() -> std::string_view {
    return "<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p/></xdr:txBody>";
  }

  /**
   * @brief 図形の共通スタイル（参照形式）XML を生成します。
   *
   * 不変な定数文字列なので static constexpr で保持しヒープ確保をゼロにします。
   * @return `xdr:style` 要素 XML 文字列です。
   */
  static auto generate_style_xml() noexcept -> std::string_view {
    return kStyleXml_;
  }

  /**
   * @brief 図形の形状と塗りの設定（xdr:spPr）XML を生成します。
   * @param[in] shape 対象の図形オブジェクトです。
   * @return `xdr:spPr` 要素 XML 文字列です。
   */
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

  /**
   * @brief 通常図形のアンカー（配置情報）XML を生成します。
   * @param[in] shape 対象の図形オブジェクトです。
   * @return `xdr:twoCellAnchor` 要素 XML 文字列です。
   */
  auto generate_shape_anchor(Shape const& shape) const -> std::string {
    auto const tx_body_xml = shape.text_body
          ? shape.text_body->generate_xml()
          : default_tx_body_xml();

    return fmt::format(
      "<xdr:twoCellAnchor editAs=\"{0}\">"
        "{1}{2}" // from, to
        "<xdr:sp>"
          "<xdr:nvSpPr>"
            "<xdr:cNvPr id=\"{3}\" name=\"{4} {3}\"/>"
            "<xdr:cNvSpPr/>"
          "</xdr:nvSpPr>"
          "{5}" // spPr (スタイル適用)
          "{6}" // style
          "{7}" // txBody
        "</xdr:sp>"
        "<xdr:clientData/>"
      "</xdr:twoCellAnchor>",
      anchor_type_to_edit_as(shape.anchor),
      shape.from.to_xml("xdr:from"),
      shape.to.to_xml("xdr:to"),
      shape.id,
      detail::xml_escape(shape.name),
      generate_spPr(shape),
      generate_style_xml(),
      tx_body_xml
    );
  }

  /**
   * @brief コネクタ図形のアンカー（配置情報）XML を生成します。
   * @param[in] shape 対象の図形オブジェクトです。
   * @return `xdr:twoCellAnchor` 要素 XML 文字列です。
   */
  auto generate_connector_anchor(Shape const& shape) const -> std::string {
    auto const tx_body_xml = shape.text_body
          ? shape.text_body->generate_xml()
          : std::string{};

    return fmt::format(
      "<xdr:twoCellAnchor editAs=\"{0}\">"
        "{1}{2}" // from, to
        "<xdr:cxnSp>"
          "<xdr:nvCxnSpPr>"
            "<xdr:cNvPr id=\"{3}\" name=\"{4} {3}\"/>"
            "<xdr:cNvCxnSpPr/>"
          "</xdr:nvCxnSpPr>"
          "{5}" // spPr
          "{6}" // style
          "{7}" // optional txBody
        "</xdr:cxnSp>"
        "<xdr:clientData/>"
      "</xdr:twoCellAnchor>",
      anchor_type_to_edit_as(shape.anchor),
      shape.from.to_xml("xdr:from"),
      shape.to.to_xml("xdr:to"),
      shape.id,
      detail::xml_escape(shape.name),
      generate_spPr(shape),
      generate_style_xml(),
      tx_body_xml
    );
  }

  /**
   * @brief 図形のレンダリング種別に応じて適切なアンカー XML を生成します。
   * @param[in] shape 対象の図形オブジェクトです。
   * @return 生成された XML 文字列です。
   */
  auto generate_anchor(Shape const& shape) const -> std::string {
    if (shape_render_kind(shape) == ShapeRenderKind::Connector) {
      return generate_connector_anchor(shape);
    }
    return generate_shape_anchor(shape);
  }

  static constexpr std::string_view header_ =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<xdr:wsDr xmlns:xdr=\"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing\" "
    "xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\">";

  static constexpr std::string_view kStyleXml_ =
    "<xdr:style>"
      "<a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
      "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"
      "<a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
      "<a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef>"
    "</xdr:style>";
};


/**
 * @brief 図形の登録、ID の自動採番、および全体の XML 生成を管理する中心的なクラスです。
 */
class DrawingManager {
public:
  /**
   * @brief 図形追加時に発生しうるエラーの定義です。
   */
  enum class Error : std::uint8_t {
    InvalidPosition, ///< 終点（to）の座標が始点（from）より左または上にある場合に発生します。
    InvalidGeometry, ///< 図形のジオメトリ名（プリセット名）が空の場合に発生します。
  };

  /**
   * @brief 描画マネージャを初期化します。
   * @param[in] start_id 自動採番を開始する図形 ID の初期値です。
   */
  explicit
  DrawingManager(uint32_t start_id = 1)
  : next_id_(start_id)
  {}

  /**
   * @brief 新しい図形を登録し、一意な ID を自動的に割り当てます。
   * @param[in] shape 登録する図形オブジェクトです。入力された Shape の id フィールドは上書きされます。
   * @return 成功時は割り当てられた図形 ID、失敗時はエラー種別 (@ref Error) を返します。
   */
  [[nodiscard]]
  auto add_shape(Shape shape) -> std::expected<uint32_t, Error> {
    // 検証を ID 採番より先に行い、失敗時に ID を消費しない
    if (shape_geometry_prst(shape).empty()) {
      return std::unexpected(Error::InvalidGeometry);
    }

    if (shape.to.col < shape.from.col || shape.to.row < shape.from.row) {
      return std::unexpected(Error::InvalidPosition);
    }

    auto const id = next_id_++;
    shape.id = id;
    shapes_.push_back(std::move(shape));
    return id;
  }

  /**
   * @brief 管理しているすべての図形から、XLSX 保存用の SpreadsheetDrawing XML を一括生成します。
   * @return 生成された XML 文字列です。
   */
  [[nodiscard]]
  auto generate_xml() const {
    DrawingGenerator generator;
    return generator.generate(shapes_);
  }

  /**
   * @brief 現在登録されている図形の総数を返します。
   * @return 図形の数です。
   */
  [[nodiscard]]
  auto shape_count() const noexcept {
    return shapes_.size();
  }

  /**
   * @brief 指定した ID を持つ図形を取得し、直接編集可能なポインタを返します。
   * @param[in] id 検索対象の図形 ID です。
   * @return 図形が見つかった場合はそのポインタ、見つからない場合は @c nullptr を返します。
   * @warning std::vector の再確保が発生するとポインタが無効になります。編集後は図形を追加しないでください。
   */
  [[nodiscard]]
  auto get_shape_mut(uint32_t id) -> Shape* {
    auto it = std::find_if(shapes_.begin(), shapes_.end(),
                           [id](auto const& s) { return s.id == id; });
    return (it != shapes_.end()) ? &(*it) : nullptr;
  }

private:
  uint32_t next_id_;           ///< 次に割り当てる図形 ID のカウンタです。
  std::vector<Shape> shapes_;  ///< 管理対象の図形リストを保持します。
};

/**
 * @brief @ref DrawingManager::Error のエラー種別を人間が読みやすいメッセージに変換します。
 * @param[in] error 対象のエラー種別です。
 * @return エラー内容を説明する文字列（英語）です。
 */
[[nodiscard]]
inline auto drawing_manager_error_message(DrawingManager::Error error) noexcept -> std::string_view {
  switch (error) {
    case DrawingManager::Error::InvalidPosition:
      return "invalid shape position";
    case DrawingManager::Error::InvalidGeometry:
      return "shape geometry is not set";
  }
  return "unknown drawing manager error";
}

} // namespace xlsdraw::drawing
