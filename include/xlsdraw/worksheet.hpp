#ifndef __XLSDRAW_WORKSHEET_HPP__
#define __XLSDRAW_WORKSHEET_HPP__

#include "units.hpp"
#include "drawing.hpp"

namespace xlsdraw::worksheet {

using namespace drawing;
using namespace units;

class WorksheetDrawingContext {
public:
  WorksheetDrawingContext(EmuConverter const& conv) : converter_(conv) {}

  /**
   * @brief ピクセル座標指定で矩形を追加するヘルパー
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

  auto final_xml() const -> std::string {
    return manager_.generate_xml();
  }

private:
  EmuConverter converter_;
  DrawingManager manager_;
};

} // namespace xlsdraw::worksheet

#endif /* __XLSDRAW_WORKSHEET_HPP__ */
