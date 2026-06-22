#pragma once

#include <deque>
#include <expected>
#include <string>
#include <vector>
#include <string_view>
#include <initializer_list>
#include <utility>

#include "zip.h"

/**
 * @file io.hpp
 * @brief ZIP アーカイブへの書き込みを扱う I/O ヘルパーを定義します。
 */

namespace xlsdraw::io {

/**
 * @brief アーカイブ書き込み失敗時の詳細情報です。
 */
struct WriteError {
  std::string internal_path; ///< 書き込み対象のアーカイブ内部パスです。
  std::string message;       ///< 失敗内容を表すメッセージです。
};

/**
 * @brief XLSX (ZIP) アーカイブの生成を行うライブラリ libzip のラッパークラスです。
 *
 * 各パートの XML 文字列を ZIP アーカイブ内の指定したパスへ書き込む機能を提供します。
 * メモリ上のデータを書き込む際に、一時的にバッファの寿命を管理します。
 */
class ArchiveWriter {
public:
  /**
   * @brief 出力アーカイブを作成、または初期化します。
   * @param[in] path 作成する XLSX ファイルの保存先パスです。既に存在する場合は上書きされます。
   * @note 構築後に @ref is_open() で初期化成功を確認してください。
   */
  explicit ArchiveWriter(std::string_view path) {
    int err = 0;
    za_ = zip_open(path.data(), ZIP_CREATE | ZIP_TRUNCATE, &err);
  }

  /// @brief コピー構築は禁止します（二重 zip_close を防ぎます）。
  ArchiveWriter(ArchiveWriter const&) = delete;
  /// @brief コピー代入は禁止します。
  ArchiveWriter& operator=(ArchiveWriter const&) = delete;

  /**
   * @brief ムーブ構築します。元のインスタンスはクローズ済み状態になります。
   */
  ArchiveWriter(ArchiveWriter&& other) noexcept : za_(other.za_), owned_contents_(std::move(other.owned_contents_)) {
    other.za_ = nullptr;
  }

  /**
   * @brief ムーブ代入します。元のインスタンスはクローズ済み状態になります。
   */
  ArchiveWriter& operator=(ArchiveWriter&& other) noexcept {
    if (this != &other) {
      if (za_) {
        zip_close(za_);
      }
      za_ = other.za_;
      owned_contents_ = std::move(other.owned_contents_);
      other.za_ = nullptr;
    }
    return *this;
  }

  /**
   * @brief デストラクタで ZIP アーカイブをクローズし、変更を確定（保存）します。
   * @note エラー検出が必要な場合は @ref close() を明示的に呼び出してください。
   */
  ~ArchiveWriter() {
    if (za_) {
      zip_close(za_);
      za_ = nullptr;
    }
  }

  /**
   * @brief アーカイブが正常にオープンされているかを確認します。
   * @return オープン済みなら @c true を返します。
   */
  [[nodiscard]]
  auto is_open() const noexcept -> bool {
    return za_ != nullptr;
  }

  /**
   * @brief アーカイブを明示的にクローズし、書き込み結果を確認します。
   *
   * `zip_close` の戻り値を検査するため、 @ref save() 等の処理終端で
   * このメソッドを呼び出してエラーを確実に検出してください。
   * @return 成功時は空の @c std::expected 、失敗時は @ref WriteError を返します。
   */
  [[nodiscard]]
  auto close() -> std::expected<void, WriteError> {
    if (!za_) {
      return {};
    }
    auto const result = zip_close(za_);
    za_ = nullptr;
    if (result != 0) {
      return std::unexpected(WriteError{
        .internal_path = {},
        .message = "zip_close failed"
      });
    }
    return {};
  }

  /**
   * @brief メモリ上のデータを 1 つのファイルとしてアーカイブ内に書き込みます。
   * @param[in] internal_path ZIP アーカイブ内部での保存パス（例: "xl/workbook.xml"）です。
   * @param[in] content 書き込むファイルの内容を表す文字列データです。
   * @return 書き込みに成功した場合は @c true 、何らかの理由で失敗した場合は @c false を返します。
   */
  [[nodiscard]]
  auto write_file(std::string_view internal_path, std::string_view content) -> bool {
    return write_file_detailed(internal_path, content).has_value();
  }

  /**
   * @brief メモリ上のデータを詳細なエラー情報と共にアーカイブ内に書き込みます。
   * @param[in] internal_path ZIP アーカイブ内部での保存パスです。
   * @param[in] content 書き込むファイルの内容です。
   * @return 成功時は void を含む @c std::expected 、失敗時は @ref WriteError インスタンスを返します。
   */
  [[nodiscard]]
  auto write_file_detailed(std::string_view internal_path, std::string_view content)
    -> std::expected<void, WriteError> {
    if (!za_) {
      return std::unexpected(WriteError{
        .internal_path = std::string(internal_path),
        .message = "archive is not open"
      });
    }

    // zip_source_buffer はデータをコピーしないため、ArchiveWriter 側で寿命を保持する。
    owned_contents_.emplace_back(content);
    auto const& owned = owned_contents_.back();

    auto const source = zip_source_buffer(za_, owned.data(), owned.size(), 0);
    if (!source) {
      owned_contents_.pop_back();
      return std::unexpected(WriteError{
        .internal_path = std::string(internal_path),
        .message = std::string(zip_strerror(za_))
      });
    }

    if (zip_file_add(za_, internal_path.data(), source, ZIP_FL_OVERWRITE) < 0) {
      zip_source_free(source);
      return std::unexpected(WriteError{
        .internal_path = std::string(internal_path),
        .message = std::string(zip_strerror(za_))
      });
    }
    return {};
  }

  /**
   * @brief initializer_list を用いて一度に複数のファイルをアーカイブに書き込みます。
   * @param[in] files 書き込む内部パスと内容のペアのリストです。
   * @return すべてのファイルが正常に書き込まれた場合は @c true 、1つでも失敗した場合は @c false を返します。
   */
  [[nodiscard]]
  auto write_files(std::initializer_list<std::pair<std::string_view, std::string_view>> files) -> bool {
    return write_files_detailed(files).has_value();
  }

  /**
   * @brief 複数ファイルの書き込みを詳細なエラー情報と共に行います。
   * @param[in] files 書き込む内部パスと内容のペアのリストです。
   * @return 成功時は空の @c std::expected 、失敗時は最初のエラー詳細を返します。
   */
  [[nodiscard]]
  auto write_files_detailed(std::initializer_list<std::pair<std::string_view, std::string_view>> files)
    -> std::expected<void, WriteError> {
    for (auto const& [internal_path, content] : files) {
      auto const result = write_file_detailed(internal_path, content);
      if (!result) {
        return std::unexpected(result.error());
      }
    }
    return {};
  }

private:
  zip_t* za_{nullptr};                       ///< libzip のアーカイブハンドルです。
  std::deque<std::string> owned_contents_{}; ///< 書き込み待ちデータのコピーを保持するキューです。
};

} // namespace xlsdraw::io
