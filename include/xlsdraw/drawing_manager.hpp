#ifndef D13720EA_CBAC_48A2_9409_7F14280AE39D
#define D13720EA_CBAC_48A2_9409_7F14280AE39D

#include <variant>

#include "drawing.hpp"
#include "drawing_generator.hpp"

#include <vector>
#include <memory>
#include <string>
#include <expected>
#include <algorithm>

namespace xlsdraw::drawing {

class DrawingManager {
public:
  // 図形追加時のエラー定義
  enum class Error { InvalidPosition, DuplicateId };

  explicit DrawingManager(uint32_t start_id = 1) : next_id_(start_id) {}

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

} // namespace xlsdraw::drawing

#endif /* D13720EA_CBAC_48A2_9409_7F14280AE39D */
