#pragma once

#include <string>
#include <string_view>

/**
 * @file xml_escape.hpp
 * @brief XML 特殊文字のエスケープユーティリティを定義します。
 */

namespace xlsdraw::detail {

/**
 * @brief XML 特殊文字をエンティティ参照に変換します。
 *
 * `<` → `&lt;` / `>` → `&gt;` / `&` → `&amp;` /
 * `"` → `&quot;` / `'` → `&apos;` の変換を行います。
 * ユーザ入力を XML 属性値や要素テキストに埋め込む前に必ず呼び出してください。
 *
 * @param[in] input エスケープ対象の文字列です。
 * @return エスケープ後の文字列です。
 */
[[nodiscard]]
inline auto xml_escape(std::string_view input) -> std::string {
  auto result = std::string{};
  result.reserve(input.size());
  for (auto const ch : input) {
    switch (ch) {
      case '<':  result += "&lt;";   break;
      case '>':  result += "&gt;";   break;
      case '&':  result += "&amp;";  break;
      case '"':  result += "&quot;"; break;
      case '\'': result += "&apos;"; break;
      default:   result += ch;       break;
    }
  }
  return result;
}

} // namespace xlsdraw::detail
