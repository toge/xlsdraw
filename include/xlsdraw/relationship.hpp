#ifndef F7CD894A_794F_430B_B17C_A2568E274B49
#define F7CD894A_794F_430B_B17C_A2568E274B49

#include <string>
#include <vector>
#include <map>

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

} // namespace xlsdraw::resource

#endif /* F7CD894A_794F_430B_B17C_A2568E274B49 */
