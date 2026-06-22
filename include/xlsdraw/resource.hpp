#pragma once

#include <string>
#include <map>
#include <vector>
#include <string_view>

#include "fmt/format.h"

/**
 * @file resource.hpp
 * @brief OpenXML パッケージの relationship / content-types リソースを生成します。
 */

namespace xlsdraw::resource {

/**
 * @brief OpenXML Relationship エントリを表します。
 */
struct Relationship {
  std::string id;     ///< Relationship ID です。
  std::string type;   ///< Relationship の型 URI です。
  std::string target; ///< 参照先パートの相対パスです。
};

/**
 * @brief Relationship (.rels) パートを構築するクラスです。
 *
 * 各パート（Workbook, Worksheet など）間の依存関係を管理し、
 * `_rels` 内の `.rels` XML を生成します。
 */
class RelationshipManager {
public:
  /**
   * @brief 新しい依存関係 (Relationship) を追加します。
   * @param[in] type Relationship の型 URI（Schema URL）です。
   * @param[in] target 参照先パートへの相対パスです。
   * @return 自動生成された Relationship ID (例: "rId1") を返します。
   */
  [[nodiscard]]
  auto add_relationship(std::string_view type, std::string_view target) {
    auto const next_id = fmt::format("rId{}", rels_.size() + 1);
    rels_.push_back(Relationship{next_id, std::string(type), std::string(target)});
    return next_id;
  }

  /**
   * @brief 現在登録されているすべての依存関係を Relationships XML として生成します。
   * @return 生成された XML 文字列です。
   */
  [[nodiscard]]
  auto generate_xml() const {
    static constexpr std::string_view kHeader =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
      "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    auto xml = std::string{kHeader};
    // 1 エントリあたり約 200 バイトを見積もって事前確保する
    xml.reserve(kHeader.size() + rels_.size() * 200 + 20);
    for (auto const& rel : rels_) {
      xml += fmt::format("<Relationship Id=\"{}\" Type=\"{}\" Target=\"{}\"/>",
                         rel.id, rel.type, rel.target);
    }
    xml += "</Relationships>";
    return xml;
  }

private:
  std::vector<Relationship> rels_; ///< 挿入順を保持する Relationship リストです。
};

/**
 * @brief `[Content_Types].xml` パートを構築するクラスです。
 *
 * パッケージ内の各ファイルの拡張子に対する既定のコンテンツタイプや、
 * 特定のパスに対するコンテンツタイプの上書きを管理します。
 */
class ContentTypesManager {
public:
  /**
   * @brief コンテンツタイプ管理クラスを初期化し、標準的な XML 拡張子を登録します。
   */
  ContentTypesManager() {
    add_default("rels", "application/vnd.openxmlformats-package.relationships+xml");
    add_default("xml", "application/xml");
  }

  /**
   * @brief 拡張子に対する既定のコンテンツタイプを追加します。
   * @param[in] extension 対象の拡張子（例: "xml", "rels"）です。
   * @param[in] content_type MIME タイプ文字列です。
   */
  auto add_default(std::string_view extension, std::string_view content_type) -> void {
    defaults_[std::string(extension)] = std::string(content_type);
  }

  /**
   * @brief 特定のパートパスに対するコンテンツタイプの上書き設定を追加します。
   * @param[in] part_name パッケージ内の絶対パス（例: "/xl/workbook.xml"）です。先頭の `/` は自動補完されます。
   * @param[in] content_type MIME タイプ文字列です。
   */
  auto add_override(std::string_view part_name, std::string_view content_type) -> void {
    overrides_[normalize_part_name(part_name)] = std::string(content_type);
  }

  /**
   * @brief Content Types パートの XML を生成します。
   * @return 生成された `[Content_Types].xml` の内容です。
   */
  [[nodiscard]]
  auto generate_xml() const -> std::string {
    static constexpr std::string_view kHeader =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">";
    auto xml = std::string{kHeader};
    // 1 エントリあたり約 150 バイトを見積もって事前確保する
    xml.reserve(kHeader.size() + (defaults_.size() + overrides_.size()) * 150 + 10);

    for (auto const& [ext, ct] : defaults_) {
      xml += fmt::format("<Default Extension=\"{}\" ContentType=\"{}\"/>", ext, ct);
    }

    for (auto const& [part_name, ct] : overrides_) {
      xml += fmt::format("<Override PartName=\"{}\" ContentType=\"{}\"/>", part_name, ct);
    }

    xml += "</Types>";
    return xml;
  }

private:
  /**
   * @brief パート名を OpenXML の命名規則（先頭に `/` を付与）に合わせて正規化します。
   * @param[in] part_name 正規化前のパート名です。
   * @return 正規化後のパート名です。
   */
  static auto normalize_part_name(std::string_view part_name) -> std::string {
    if (part_name.empty() || part_name.front() == '/') {
      return std::string(part_name);
    }
    return fmt::format("/{}", part_name);
  }

  std::map<std::string, std::string> defaults_;  ///< 拡張子ごとの既定設定です。
  std::map<std::string, std::string> overrides_; ///< パスごとの上書き設定です。
};

} // namespace xlsdraw::resource
