#ifndef C9E4F6D3_78FC_49D9_B32D_E3260CD6D599
#define C9E4F6D3_78FC_49D9_B32D_E3260CD6D599

#include <vector>
#include <string_view>

#include "fmt/core.h"

#include "drawing.hpp"

namespace xlsdraw::drawing {

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

} // namespace xlsdraw::drawing

#endif /* C9E4F6D3_78FC_49D9_B32D_E3260CD6D599 */
