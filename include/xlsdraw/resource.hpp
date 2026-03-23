#ifndef __XLSDRAW_RESOURCE_HPP__
#define __XLSDRAW_RESOURCE_HPP__

#include <string>
#include <map>
#include <string_view>

#include "fmt/core.h"

namespace xlsdraw::resource {

struct Relationship {
  std::string id;
  std::string type;
  std::string target;
};

class RelationshipManager {
public:
  auto add_relationship(std::string_view type, std::string_view target) {
    auto const next_id = fmt::format("rId{}", rels_.size() + 1);
    rels_[next_id] = Relationship{next_id, std::string(type), std::string(target)};
    return next_id;
  }

  auto generate_xml() const {
    auto xml = std::string{"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
                           "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"};
    for (auto const& [id, rel] : rels_) {
      xml += fmt::format("<Relationship Id=\"{}\" Type=\"{}\" Target=\"{}\"/>",
                         rel.id, rel.type, rel.target);
    }
    xml += "</Relationships>";
    return xml;
  }

private:
  std::map<std::string, Relationship> rels_;
};

class ContentTypesManager {
public:
  ContentTypesManager() {
    add_default("rels", "application/vnd.openxmlformats-package.relationships+xml");
    add_default("xml", "application/xml");
  }

  auto add_default(std::string_view extension, std::string_view content_type) -> void {
    defaults_[std::string(extension)] = std::string(content_type);
  }

  auto add_override(std::string_view part_name, std::string_view content_type) -> void {
    overrides_[normalize_part_name(part_name)] = std::string(content_type);
  }

  [[nodiscard]]
  auto generate_xml() const -> std::string {
    auto xml = std::string{
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
    };

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
  static auto normalize_part_name(std::string_view part_name) -> std::string {
    if (part_name.empty() || part_name.front() == '/') {
      return std::string(part_name);
    }
    return fmt::format("/{}", part_name);
  }

  std::map<std::string, std::string> defaults_;
  std::map<std::string, std::string> overrides_;
};

} // namespace xlsdraw::resource

#endif /* __XLSDRAW_RESOURCE_HPP__ */
