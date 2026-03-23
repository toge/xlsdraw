#include <filesystem>
#include <string>

#include "catch2/catch_all.hpp"
#include "zip.h"

#include "xlsdraw/io.hpp"

TEST_CASE("archive writer keeps previously written entry buffers stable") {
  auto const archive_path = std::filesystem::current_path() / "test_io_archive.zip";
  std::filesystem::remove(archive_path);

  {
    auto writer = xlsdraw::io::ArchiveWriter{archive_path.string()};

    for (auto i = 0; i < 64; ++i) {
      auto const path = "entry" + std::to_string(i) + ".txt";
      auto const content = "payload-" + std::to_string(i) + std::string(256, static_cast<char>('A' + (i % 26)));
      REQUIRE(writer.write_file_detailed(path, content).has_value());
    }
  }

  int err = 0;
  auto* archive = zip_open(archive_path.string().c_str(), ZIP_RDONLY, &err);
  REQUIRE(archive != nullptr);

  for (auto i = 0; i < 64; ++i) {
    auto const path = "entry" + std::to_string(i) + ".txt";
    zip_stat_t stat{};
    REQUIRE(zip_stat(archive, path.c_str(), 0, &stat) == 0);

    auto* file = zip_fopen(archive, path.c_str(), 0);
    REQUIRE(file != nullptr);

    auto content = std::string(static_cast<std::size_t>(stat.size), '\0');
    REQUIRE(zip_fread(file, content.data(), content.size()) == static_cast<zip_int64_t>(content.size()));
    REQUIRE(zip_fclose(file) == 0);

    CHECK(content.find("payload-" + std::to_string(i)) == 0);
  }

  REQUIRE(zip_close(archive) == 0);
  std::filesystem::remove(archive_path);
}
