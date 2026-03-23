#ifndef A4D77870_EC77_4561_8506_FA189AD52F4A
#define A4D77870_EC77_4561_8506_FA189AD52F4A

#include <span>
#include <expected>
#include <string>
#include <vector>
#include <string_view>
#include <initializer_list>
#include <utility>

#include "zip.h"

namespace xlsdraw::io {

struct WriteError {
  std::string internal_path;
  std::string message;
};

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
    return write_file_detailed(internal_path, content).has_value();
  }

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

  auto write_files(std::initializer_list<std::pair<std::string_view, std::string_view>> files) -> bool {
    return write_files_detailed(files).has_value();
  }

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
  zip_t* za_{nullptr};
  std::vector<std::string> owned_contents_{};
};

}  // namespace xlsdraw::io

#endif /* A4D77870_EC77_4561_8506_FA189AD52F4A */
