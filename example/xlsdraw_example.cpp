// xlsdraw ヘッダオンリーライブラリの最小使用例。
//
// この例では、1つのスタイル付き矩形を含む単一ワークシートを持つ
// XLSX ファイルの構築方法を示す。同じ API がテストスイート内部でも
// 使用されているが、インストールされたライブラリの利用者がコンパイル
// して実行できる自己完結型のスモークテストとしてここに置かれている。

#include <iostream>
#include <string>

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/units.hpp"
#include "xlsdraw/workbook.hpp"

int main() {
  auto builder = xlsdraw::workbook::SingleSheetDrawingWorkbookBuilder{
    "xlsdraw_example.xlsx",
    "Example"
  };

  // 180 x 80 px @ 96 DPI は約 1714500 x 762000 EMU に変換される。
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const width_emu = converter.pixels_to_emu(180).value_or(0);
  auto const height_emu = converter.pixels_to_emu(80).value_or(0);

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::RoundRect,
    "Greeting"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to = {.col = 1, .col_off = width_emu, .row = 1, .row_off = height_emu};
  shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FFDEEAF6"};
  shape.style.line.width_emu = 12700;
  shape.style.line.color = xlsdraw::drawing::Color{"FF4472C4"};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {
      {
        .runs = {{
          .text = std::string{"Hello, xlsdraw!"},
          .color = xlsdraw::drawing::Color{"FF1F497D"},
          .font_size = 14.0,
        }},
        .alignment = "ctr",
      }
    },
  };

  auto const added = builder.add_shape(std::move(shape));
  if (!added) {
    std::cerr << "failed to add shape: " << added.error() << '\n';
    return 1;
  }

  auto const saved = builder.save();
  if (!saved) {
    std::cerr << "failed to save xlsx: " << saved.error() << '\n';
    return 1;
  }

  std::cout << "Generated xlsdraw_example.xlsx with " << builder.shape_count() << " shape(s).\n";
  return 0;
}
