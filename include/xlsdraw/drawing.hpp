#ifndef C888F150_6E10_4925_AE14_FCD9ECF976BA
#define C888F150_6E10_4925_AE14_FCD9ECF976BA

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

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
  Marker from, to;
  std::string color_argb{"FF4472C4"};
  std::string text;
  AnchorType anchor{AnchorType::MoveButNoSize};
  ShapeStyle style;
  std::optional<TextBody> text_body; // 追加
};

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
      shape.type,
      shape.style.fill.generate_xml(),
      shape.style.line.generate_xml()
    );
  }

  auto generate_anchor(Shape const& shape) const -> std::string {
    auto const tx_body_xml = shape.text_body
          ? shape.text_body->generate_xml()
          : "<xdr:txBody><a:bodyPr/><a:lstStyle/><a:p/></xdr:txBody>";

    return fmt::format(
      "<xdr:twoCellAnchor editAs=\"oneCell\">"
        "{0}{1}" // from, to
        "<xdr:sp>"
          "<xdr:nvSpPr>"
            "<xdr:cNvPr id=\"{2}\" name=\"{3} {2}\"/>"
            "<xdr:cNvSpPr/>"
          "</xdr:nvSpPr>"
          "{4}" // spPr (スタイル適用)
          "<xdr:style>" // 文字列などの簡易スタイル
            "<a:lnRef idx=\"2\"><a:schemeClr val=\"accent1\"/></a:lnRef>"
            "<a:fillRef idx=\"1\"><a:schemeClr val=\"accent1\"/></a:fillRef>"
            "<a:effectRef idx=\"0\"><a:schemeClr val=\"accent1\"/></a:effectRef>"
            "<a:fontRef idx=\"minor\"><a:schemeClr val=\"lt1\"/></a:fontRef>"
          "</xdr:style>"
          "<xdr:txBody>" // テキストボックスが必要な場合
             "<a:bodyPr/><a:lstStyle/><a:p><a:endParaRPr lang=\"ja-JP\"/></a:p>"
          "</xdr:txBody>"
        "</xdr:sp>"
        "<xdr:clientData/>"
      "</xdr:twoCellAnchor>",
      shape.from.to_xml("xdr:from"),
      shape.to.to_xml("xdr:to"),
      shape.id,
      shape.name,
      generate_spPr(shape),
      tx_body_xml
    );
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
  enum class Error { InvalidPosition, DuplicateId };

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


}

#endif /* C888F150_6E10_4925_AE14_FCD9ECF976BA */
