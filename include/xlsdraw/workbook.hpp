#ifndef __XLSDRAW_WORKBOOK_HPP__
#define __XLSDRAW_WORKBOOK_HPP__

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "fmt/core.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"
#include "xlsdraw/resource.hpp"

/**
 * @file workbook.hpp
 * @brief 単一シートの XLSX ワークブックを生成する API を定義します。
 */

namespace xlsdraw::workbook {

namespace detail {

inline constexpr auto kWorkbookPath = "xl/workbook.xml";
inline constexpr auto kWorkbookRelsPath = "xl/_rels/workbook.xml.rels";
inline constexpr auto kWorksheetPath = "xl/worksheets/sheet1.xml";
inline constexpr auto kWorksheetRelsPath = "xl/worksheets/_rels/sheet1.xml.rels";
inline constexpr auto kDrawingPath = "xl/drawings/drawing1.xml";
inline constexpr auto kDrawingTarget = "../drawings/drawing1.xml";
inline constexpr auto kWorksheetTarget = "worksheets/sheet1.xml";
inline constexpr auto kOfficeDocumentTarget = "xl/workbook.xml";

/**
 * @brief 単一シート構成で必要な OpenXML 部品文字列を生成します。
 */
class SingleSheetOpenXmlParts {
public:
  /**
   * @brief Worksheet XML を生成します。
   * @param[in] drawing_rid 描画パートへの Relationship ID です。未指定なら `<drawing>` 要素は出力しません。
   * @return Worksheet パート XML を返します。
   */
  static auto worksheet_xml(std::optional<std::string_view> drawing_rid) {
    if (!drawing_rid) {
      return std::string{
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
          "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
          "<sheetData/>"
        "</worksheet>"
      };
    }

    return fmt::format(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
        "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
        "<sheetData/>"
        "<drawing r:id=\"{}\"/>"
      "</worksheet>",
      *drawing_rid
    );
  }

  /**
   * @brief Workbook XML を生成します。
   * @param[in] sheet_rel_id ワークブックからワークシートへの Relationship ID です。
   * @param[in] sheet_name シート名です。
   * @return Workbook パート XML を返します。
   */
  static auto workbook_xml(std::string_view sheet_rel_id, std::string_view sheet_name) {
    return fmt::format(
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
      "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
      "<sheets><sheet name=\"{}\" sheetId=\"1\" r:id=\"{}\"/></sheets>"
      "</workbook>",
      sheet_name,
      sheet_rel_id
    );
  }
};

} // namespace detail

/**
 * @brief 単一ワークシート・単一描画構成の XLSX ファイルを構築する高レベル API です。
 *
 * このクラスを使用することで、複雑な OpenXML パッケージ構造（Relationship, Content Types など）を
 * 意識することなく、図形を含んだ XLSX ファイルを出力できます。
 */
class SingleSheetDrawingWorkbookBuilder {
public:
  /**
   * @brief 追加した図形を後で参照・編集するためのハンドル情報です。
   */
  struct ShapeHandle {
    uint32_t id;      ///< 内部で自動採番された一意の図形 ID です。
    std::string name; ///< 図形に付けられた名前です。
  };

  /**
   * @brief ワークブックビルダを初期化します。
   * @param[in] output_path 生成する XLSX ファイルの保存パスです。
   * @param[in] sheet_name ワークブック内に作成するシートの名前（既定値: "Sheet1"）です。
   */
  explicit SingleSheetDrawingWorkbookBuilder(std::string output_path, std::string sheet_name = "Sheet1")
      : output_path_(std::move(output_path)),
        sheet_name_(std::move(sheet_name)) {}

  /**
   * @brief ワークシートに図形を追加します。
   * @param[in] shape 追加する図形オブジェクト（@ref drawing::Shape）です。
   * @return 成功時は図形ハンドル（@ref ShapeHandle）、失敗時はエラーメッセージを返します。
   */
  auto add_shape(drawing::Shape shape) -> std::expected<ShapeHandle, std::string> {
    auto const shape_name = shape.name;
    auto const add_result = drawing_mgr_.add_shape(std::move(shape));
    if (!add_result) {
      return std::unexpected(fmt::format(
        "failed to add shape '{}': {}",
        shape_name,
        drawing::drawing_manager_error_message(add_result.error())
      ));
    }

    return ShapeHandle{
      .id = *add_result,
      .name = shape_name,
    };
  }

  /**
   * @brief 現在登録されている図形の総数を取得します。
   * @return 図形数です。
   */
  [[nodiscard]]
  auto shape_count() const noexcept -> size_t {
    return drawing_mgr_.shape_count();
  }

  /**
   * @brief 登録済みの図形を ID 指定で取得し、プロパティを編集します。
   * @param[in] id 編集対象の図形 ID です。
   * @return 図形へのポインタを返します。指定した ID が存在しない場合は @c nullptr を返します。
   */
  auto get_shape_mut(uint32_t id) -> drawing::Shape* {
    return drawing_mgr_.get_shape_mut(id);
  }

  /**
   * @brief 登録されたすべてのデータ（シート、図形、パッケージ構造）を XLSX ファイルとして保存します。
   * @return 成功時は空の @c std::expected 、失敗時は詳細メッセージを返します。
   */
  auto save() -> std::expected<void, std::string> {
    if (sheet_name_.empty()) {
      return std::unexpected("sheet name must not be empty");
    }

    auto const current_shape_count = drawing_mgr_.shape_count();
    auto sheet_rel_manager = resource::RelationshipManager{};
    std::optional<std::string> drawing_rid;

    if (current_shape_count > 0) {
      drawing_rid = sheet_rel_manager.add_relationship(
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/drawing",
        detail::kDrawingTarget
      );
    }

    auto workbook_rel_manager = resource::RelationshipManager{};
    auto const sheet_rel_id = workbook_rel_manager.add_relationship(
      "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
      detail::kWorksheetTarget
    );

    auto package_rel_manager = resource::RelationshipManager{};
    package_rel_manager.add_relationship(
      "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument",
      detail::kOfficeDocumentTarget
    );

    auto content_types_mgr = resource::ContentTypesManager{};
    content_types_mgr.add_override(
      detail::kWorkbookPath,
      "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"
    );
    content_types_mgr.add_override(
      detail::kWorksheetPath,
      "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"
    );
    if (current_shape_count > 0) {
      content_types_mgr.add_override(
        detail::kDrawingPath,
        "application/vnd.openxmlformats-officedocument.drawing+xml"
      );
    }

    auto const worksheet_xml = detail::SingleSheetOpenXmlParts::worksheet_xml(
      drawing_rid ? std::optional<std::string_view>{*drawing_rid} : std::nullopt
    );
    auto const workbook_xml = detail::SingleSheetOpenXmlParts::workbook_xml(sheet_rel_id, sheet_name_);

    auto writer = io::ArchiveWriter(output_path_);
    auto const content_types_xml = content_types_mgr.generate_xml();
    auto const package_rels_xml = package_rel_manager.generate_xml();
    auto const workbook_rels_xml = workbook_rel_manager.generate_xml();

    if (auto r = write_required_file(writer, "[Content_Types].xml", content_types_xml); !r) return r;
    if (auto r = write_required_file(writer, "_rels/.rels", package_rels_xml); !r) return r;
    if (auto r = write_required_file(writer, detail::kWorkbookPath, workbook_xml); !r) return r;
    if (auto r = write_required_file(writer, detail::kWorkbookRelsPath, workbook_rels_xml); !r) return r;
    if (auto r = write_required_file(writer, detail::kWorksheetPath, worksheet_xml); !r) return r;

    if (current_shape_count > 0) {
      auto const sheet_rels_xml = sheet_rel_manager.generate_xml();
      auto const drawing_xml = drawing_mgr_.generate_xml();

      if (auto r = write_required_file(writer, detail::kWorksheetRelsPath, sheet_rels_xml); !r) return r;
      if (auto r = write_required_file(writer, detail::kDrawingPath, drawing_xml); !r) return r;
    }

    return {};
  }

private:
  /**
   * @brief 必須ファイルの書き込みを試行し、失敗時は詳細なエラーメッセージを返します。
   * @param[in,out] writer 使用するアーカイブライターです。
   * @param[in] internal_path 書き込み先の内部パスです。
   * @param[in] content 書き込むデータ内容です。
   * @return 成功時は空の expected です。
   */
  static auto write_required_file(io::ArchiveWriter& writer, std::string_view internal_path, std::string_view content)
      -> std::expected<void, std::string> {
    auto const result = writer.write_file_detailed(internal_path, content);
    if (!result) {
      return std::unexpected(fmt::format(
        "failed to write required XLSX entries: path='{}', message='{}'",
        result.error().internal_path,
        result.error().message
      ));
    }
    return {};
  }

  std::string output_path_;           ///< XLSX ファイルの出力先です。
  std::string sheet_name_;            ///< シート名です。
  drawing::DrawingManager drawing_mgr_{}; ///< 図形の管理と XML 生成を担当するインスタンスです。
};

} // namespace xlsdraw::workbook

#endif /* __XLSDRAW_WORKBOOK_HPP__ */
