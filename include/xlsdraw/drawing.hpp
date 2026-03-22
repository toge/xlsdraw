#ifndef C888F150_6E10_4925_AE14_FCD9ECF976BA
#define C888F150_6E10_4925_AE14_FCD9ECF976BA

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "fmt/core.h"

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

}

#endif /* C888F150_6E10_4925_AE14_FCD9ECF976BA */
