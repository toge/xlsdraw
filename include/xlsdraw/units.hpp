#pragma once

#include <cstdint>
#include <expected>

/**
 * @file units.hpp
 * @brief EMU 単位への変換を扱うユーティリティを定義します。
 */

namespace xlsdraw::units {

/**
 * @brief 単位変換時のエラー種別です。
 */
enum class UnitError {
  InvalidDpi,   ///< DPI が不正です。
  NegativeValue ///< 負の値が指定されました。
};

/**
 * @brief ピクセルやインチを EMU (English Metric Unit) に変換するクラスです。
 *
 * OpenXML 規格では、図形のサイズや位置を EMU 単位で保持します。
 * 1 インチ = 914,400 EMU と定義されており、このクラスは実行時の DPI を考慮した
 * ピクセル単位からの変換を提供します。
 */
class EmuConverter {
public:
  /// @brief 既定の DPI 値 (96.0) です。
  static auto constexpr DefaultDPI = 96.0;
  /// @brief 1 インチあたりの EMU 数 (914,400) です。
  static auto constexpr EmuPerInch = 914400LL;

  /**
   * @brief 指定した DPI で変換器を初期化します。
   * @param[in] dpi 使用する DPI (Dots Per Inch) です。0 以下を指定した場合は @ref DefaultDPI を使用します。
   */
  explicit EmuConverter(double const dpi = DefaultDPI) {
    if (dpi <= 0) {
      dpi_ = DefaultDPI;
    } else {
      dpi_ = dpi;
    }
  }

  /**
   * @brief ピクセル値を EMU に変換します。
   * @param[in] pixels 変換対象のピクセル値です。
   * @return 成功時は EMU 値、失敗時は @ref UnitError::NegativeValue を返します。
   */
  [[nodiscard]]
  auto pixels_to_emu(int const pixels) const -> std::expected<int64_t, UnitError> {
    if (pixels < 0) {
      return std::unexpected(UnitError::NegativeValue);
    }
    auto const emus = static_cast<int64_t>((static_cast<double>(pixels) / dpi_) * EmuPerInch);
    return emus;
  }

  /**
   * @brief インチ値を EMU に変換します。
   * @param[in] inches 変換対象のインチ値です。
   * @return 成功時は EMU 値、失敗時は @ref UnitError::NegativeValue を返します。
   */
  [[nodiscard]]
  auto inches_to_emu(double const inches) const -> std::expected<int64_t, UnitError> {
    if (inches < 0.0) {
      return std::unexpected(UnitError::NegativeValue);
    }
    return static_cast<int64_t>(inches * EmuPerInch);
  }

private:
  double dpi_; ///< 使用する DPI 値です。
};

} // namespace xlsdraw::units
