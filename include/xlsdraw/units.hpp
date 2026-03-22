#ifndef B9146431_4BF1_429A_A985_8D2B11A9AC0F
#define B9146431_4BF1_429A_A985_8D2B11A9AC0F

#include <cstdint>
#include <expected>
#include <string>
#include <cstdint>

namespace xlsdraw::units {

enum class UnitError {
  InvalidDpi,
  NegativeValue
};

class EmuConverter {
public:
  // 標準的なDPI定義
  static auto constexpr DefaultDPI = 96.0;
  static auto constexpr EmuPerInch = 914400LL;

  explicit EmuConverter(double const dpi = DefaultDPI) {
    if (dpi <= 0) {
      dpi_ = DefaultDPI;
    } else {
      dpi_ = dpi;
    }
  }

  // ピクセルからEMUへの変換
  [[nodiscard]]
  auto pixels_to_emu(int const pixels) const -> std::expected<int64_t, UnitError> {
    if (pixels < 0) {
      return std::unexpected(UnitError::NegativeValue);
    }
    // 式: (pixels / DPI) * 914,400
    auto const emus = static_cast<int64_t>((static_cast<double>(pixels) / dpi_) * EmuPerInch);
    return emus;
  }

  // インチからEMUへの変換
  [[nodiscard]]
  auto inches_to_emu(double const inches) const -> std::expected<int64_t, UnitError> {
    if (inches < 0.0) {
      return std::unexpected(UnitError::NegativeValue);
    }
    return static_cast<int64_t>(inches * EmuPerInch);
  }

private:
  double dpi_;
};

}

#endif /* B9146431_4BF1_429A_A985_8D2B11A9AC0F */
