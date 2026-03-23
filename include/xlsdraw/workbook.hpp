#ifndef B1C5E7C3_95BF_4D51_9206_9A76DE0A8E5E
#define B1C5E7C3_95BF_4D51_9206_9A76DE0A8E5E

#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "fmt/core.h"

#include "xlsdraw/drawing.hpp"
#include "xlsdraw/io.hpp"
#include "xlsdraw/resource.hpp"

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

class SingleSheetOpenXmlParts {
public:
  static auto worksheet_xml(std::optional<std::string_view> drawing_rid) -> std::string {
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

  static auto workbook_xml(std::string_view sheet_rel_id, std::string_view sheet_name) -> std::string {
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

class SingleSheetDrawingWorkbookBuilder {
public:
  struct ShapeHandle {
    uint32_t id;
    std::string name;
  };

  explicit SingleSheetDrawingWorkbookBuilder(std::string output_path, std::string sheet_name = "Sheet1")
      : output_path_(std::move(output_path)),
        sheet_name_(std::move(sheet_name)) {}

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

  [[nodiscard]]
  auto shape_count() const noexcept -> size_t {
    return drawing_mgr_.shape_count();
  }

  auto get_shape_mut(uint32_t id) -> drawing::Shape* {
    return drawing_mgr_.get_shape_mut(id);
  }

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

  std::string output_path_;
  std::string sheet_name_;
  drawing::DrawingManager drawing_mgr_{};
};

} // namespace xlsdraw::workbook

#endif /* B1C5E7C3_95BF_4D51_9206_9A76DE0A8E5E */
