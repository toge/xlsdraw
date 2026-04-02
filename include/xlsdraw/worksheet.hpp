#ifndef __XLSDRAW_WORKSHEET_HPP__
#define __XLSDRAW_WORKSHEET_HPP__

#include "units.hpp"
#include "drawing.hpp"

/**
 * @file worksheet.hpp
 * @brief ワークシート上の図形配置を補助するユーティリティを定義します。
 */

namespace xlsdraw::worksheet {

using namespace drawing;
using namespace units;

/**
 * @brief ワークシート用の簡易描画コンテキストです。
 *
 * 列・行インデックスとピクセル単位のオフセットを用いて、ワークシート上に
 * プリセット図形（現在は四角形のみ対応）を配置するための高レベル API を提供します。
 */
class WorksheetDrawingContext {
public:
  /**
   * @brief 描画コンテキストを初期化します。
   * @param[in] conv ピクセル値を EMU に変換するコンバータです。
   */
  WorksheetDrawingContext(EmuConverter const& conv) : converter_(conv) {}

  /**
   * @brief ピクセル指定で単純な矩形図形を追加します。
   * @param[in] col 開始列インデックス（0基点）です。
   * @param[in] row 開始行インデックス（0基点）です。
   * @param[in] width_px 図形の幅をピクセル単位で指定します。
   * @param[in] height_px 図形の高さをピクセル単位で指定します。
   * @param[in] text 図形内に表示するテキスト内容です（省略可）。
   * @return 成功時は自動的に割り当てられた図形 ID、失敗時はエラーコードを返します。
   * @note このメソッドは簡易配置用であり、現在は図形の右下座標を `(col, row)` からのピクセルオフセットで設定します。
   */
  auto add_simple_rect(
      int col, int row, int width_px, int height_px,
      std::string_view text = ""
  ) -> std::expected<uint32_t, DrawingManager::Error> {

    auto const emu_w = converter_.pixels_to_emu(width_px).value_or(0);
    auto const emu_h = converter_.pixels_to_emu(height_px).value_or(0);

    auto rect = make_preset_shape(PresetShape::Rect, "Rectangle");
    rect.from = {.col = col, .col_off = 0, .row = row, .row_off = 0};
    // 簡易的に同一セル内、またはオフセットでサイズを表現
    rect.to = {.col = col, .col_off = emu_w, .row = row, .row_off = emu_h};

    if (!text.empty()) {
      TextRun run{.text = std::string(text)};
      rect.text_body = TextBody{.paragraphs = {{.runs = {run}}}};
    }

    return manager_.add_shape(std::move(rect));
  }

  /**
   * @brief 現在登録されているすべての図形を SpreadsheetDrawing XML (xl/drawings/drawing*.xml) として出力します。
   * @return 生成された XML 文字列です。
   */
  auto final_xml() const -> std::string {
    return manager_.generate_xml();
  }

private:
  EmuConverter converter_; ///< EMU 変換器です。
  DrawingManager manager_; ///< 内部の描画管理インスタンスです。
};

} // namespace xlsdraw::worksheet

#endif /* __XLSDRAW_WORKSHEET_HPP__ */
