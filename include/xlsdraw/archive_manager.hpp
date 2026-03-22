#ifndef A4D77870_EC77_4561_8506_FA189AD52F4A
#define A4D77870_EC77_4561_8506_FA189AD52F4A

#include <span>
#include <expected>
#include <string>
#include <vector>
#include <string_view>

#include "zip.h"

namespace xlsdraw::io {

class ArchiveWriter {
public:
  explicit ArchiveWriter(std::string_view path) {
    int err = 0;
    za_ = zip_open(path.data(), ZIP_CREATE | ZIP_TRUNCATE, &err);
  }

  ~ArchiveWriter() {
    if (za_) {
      zip_close(za_);
    }
  }

  auto write_file(std::string_view internal_path, std::string_view content) -> bool {
    if (!za_) {
      return false;
    }

    // zip_source_buffer はデータをコピーしないため、ArchiveWriter 側で寿命を保持する。
    owned_contents_.emplace_back(content);
    auto const& owned = owned_contents_.back();

    auto const source = zip_source_buffer(za_, owned.data(), owned.size(), 0);
    if (!source) {
      owned_contents_.pop_back();
      return false;
    }

    if (zip_file_add(za_, internal_path.data(), source, ZIP_FL_OVERWRITE) < 0) {
      zip_source_free(source);
      return false;
    }
    return true;
  }

private:
  zip_t* za_{nullptr};
  std::vector<std::string> owned_contents_{};
};

}  // namespace xlsdraw::io

#endif /* A4D77870_EC77_4561_8506_FA189AD52F4A */
