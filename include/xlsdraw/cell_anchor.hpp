#ifndef E8DB1A6B_06F2_43FF_8F89_F7941F2D721F
#define E8DB1A6B_06F2_43FF_8F89_F7941F2D721F

#include <vector>
#include <numeric>
#include <ranges>

#include "units.hpp"

namespace xlsdraw::drawing {

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

} // namespace xlsdraw::drawing

#endif /* E8DB1A6B_06F2_43FF_8F89_F7941F2D721F */
