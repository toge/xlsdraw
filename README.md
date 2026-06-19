# xlsdraw

XLSX ファイルを C++ で生成するための、ヘッダーオンリーの C++ ライブラリです。
描画(図形・テキストボックス・コネクタなど)を含むワークシートを、OpenXML 形式
で書き出すことに重点を置いています。

図の描画に特化しているため、セルの値や数式の編集はサポートしていません！

## 特徴

- **ヘッダーオンリー**: ライブラリ本体はヘッダーのみで構成され、リンクは
  既存の `libzip` / `pugixml` / `fmt` に委譲します。
- **モダン C++**: C++20 以上を要求します(ビルド時に C++23 / C++26 へ自動対応)。
- **単一シートワークブック API**: よく使われる「1 ワークブック = 1 シート +
  1 描画」という構成を、ビルダー1つで生成できます。
- **豊富なプリセット図形**: 矩形・楕円・基本図形・矢印・フローチャート・吹き出し
  など、Excel の表示カテゴリに沿ったプリセット図形をサポートします。
- **EMU 単位変換ヘルパー**: ピクセル/インチと OpenXML 標準の EMU
  (English Metric Units) を相互変換するユーティリティを提供します。
- **CMake / vcpkg 対応**: `find_package(xlsdraw)` で導入でき、依存関係は
  transitive に解決されます。

## 必要な環境

- C++20 対応のコンパイラ(GCC / Clang / MSVC)
- CMake 3.25 以上
- [vcpkg](https://github.com/microsoft/vcpkg) で導入される以下の依存ライブラリ
  - [libzip](https://libzip.org/)
  - [pugixml](https://pugixml.org/)
  - [fmt](https://github.com/fmtlib/fmt)
  - (テスト実行時のみ) [Catch2](https://github.com/catchorg/Catch2)

## ビルド

1. CMake の構成(ツールチェインに vcpkg を指定、`Release` ビルド)
2. `cmake --build build --parallel 4` でのコンパイル

ビルド成果物は `build/` ディレクトリ以下に出力されます。
Visual Studio 向けの `build_win64.sh` / MinGW 向けの `build_mingw.sh` も
用意しています。

## テスト

Catch2 v3 を使ったテストスイートが `test/` 以下にあります。

```sh
./test.sh
```

内部的には `build/` に移動して `ctest -V` を実行します。テストは JUnit 形式
(`-r junit`)で結果を報告します。

## 使い方(最小例)

`example/xlsdraw_example.cpp` と同等の内容を抜粋します。180×80 ピクセル
(96 DPI 想定)の角丸矩形を 1 つ配置した `xlsdraw_example.xlsx` を生成します。

```cpp
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

  // 180 x 80 px @ 96 DPI -> 約 1,714,500 x 762,000 EMU
  auto const converter = xlsdraw::units::EmuConverter{96.0};
  auto const width_emu  = converter.pixels_to_emu(180).value_or(0);
  auto const height_emu = converter.pixels_to_emu(80).value_or(0);

  auto shape = xlsdraw::drawing::make_preset_shape(
    xlsdraw::drawing::PresetShape::RoundRect,
    "Greeting"
  );
  shape.from = {.col = 1, .col_off = 0, .row = 1, .row_off = 0};
  shape.to   = {.col = 1, .col_off = width_emu,
                .row = 1, .row_off  = height_emu};
  shape.style.fill.solid_fill = xlsdraw::drawing::Color{"FFDEEAF6"};
  shape.style.line.width_emu  = 12700;
  shape.style.line.color      = xlsdraw::drawing::Color{"FF4472C4"};
  shape.text_body = xlsdraw::drawing::TextBody{
    .paragraphs = {{
      .runs = {{
        .text      = std::string{"Hello, xlsdraw!"},
        .color     = xlsdraw::drawing::Color{"FF1F497D"},
        .font_size = 14.0,
      }},
      .alignment = "ctr",
    }},
  };

  if (auto const added = builder.add_shape(std::move(shape)); !added) {
    std::cerr << "failed to add shape: " << added.error() << '\n';
    return 1;
  }
  if (auto const saved = builder.save(); !saved) {
    std::cerr << "failed to save xlsx: " << saved.error() << '\n';
    return 1;
  }

  std::cout << "Generated xlsdraw_example.xlsx with "
            << builder.shape_count() << " shape(s).\n";
  return 0;
}
```

## ヘッダー構成

| ヘッダー | 役割 |
| --- | --- |
| `xlsdraw/drawing.hpp` | `Marker` / `Shape` / `PresetShape` / `Color` などの図形モデルと、`SpreadsheetDrawing` XML の生成器。 |
| `xlsdraw/workbook.hpp` | 単一シート構成の XLSX ワークブックを生成する `SingleSheetDrawingWorkbookBuilder`。 |
| `xlsdraw/worksheet.hpp` | ワークシート XML 構築の補助。 |
| `xlsdraw/io.hpp` | libzip をラップした ZIP アーカイブ書き込みクラス `ArchiveWriter`。 |
| `xlsdraw/units.hpp` | EMU 単位とピクセル/インチの相互変換 (`EmuConverter`)。 |
| `xlsdraw/resource.hpp` | 埋め込みリソース(画像等)のためのハンドル。 |

## ライセンス

本プロジェクトは **MIT ライセンス** の下で公開されています。詳細については
リポジトリの `LICENSE` ファイルを参照してください。
