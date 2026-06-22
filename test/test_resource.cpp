#include "catch2/catch_all.hpp"

#include "xlsdraw/resource.hpp"

/**
 * @file test_resource.cpp
 * @brief RelationshipManager および ContentTypesManager の単体テストです。
 */

TEST_CASE("RelationshipManager assigns sequential rIds in insertion order") {
  auto mgr = xlsdraw::resource::RelationshipManager{};

  auto const id1 = mgr.add_relationship("type:a", "target/a");
  auto const id2 = mgr.add_relationship("type:b", "target/b");
  auto const id3 = mgr.add_relationship("type:c", "target/c");

  CHECK(id1 == "rId1");
  CHECK(id2 == "rId2");
  CHECK(id3 == "rId3");
}

TEST_CASE("RelationshipManager preserves insertion order for 10+ relationships") {
  // rId10 が rId2 の前にソートされないことを確認する（以前の std::map バグ）
  auto mgr = xlsdraw::resource::RelationshipManager{};

  for (auto const i : std::views::iota(0, 12)) {
    std::ignore = mgr.add_relationship(
      "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
      "worksheets/sheet" + std::to_string(i + 1) + ".xml"
    );
  }

  auto const xml = mgr.generate_xml();

  // rId1〜rId12 が昇順で現れることを確認する
  auto pos1  = xml.find("rId1\"");
  auto pos2  = xml.find("rId2\"");
  auto pos10 = xml.find("rId10\"");
  auto pos11 = xml.find("rId11\"");
  auto pos12 = xml.find("rId12\"");

  CHECK(pos1  != std::string::npos);
  CHECK(pos2  != std::string::npos);
  CHECK(pos10 != std::string::npos);
  CHECK(pos1  < pos2);
  CHECK(pos2  < pos10);   // 旧来の辞書順では rId10 < rId2 になっていた
  CHECK(pos10 < pos11);
  CHECK(pos11 < pos12);
}

TEST_CASE("RelationshipManager generate_xml produces well-formed Relationships XML") {
  auto mgr = xlsdraw::resource::RelationshipManager{};
  std::ignore = mgr.add_relationship(
    "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet",
    "worksheets/sheet1.xml"
  );

  auto const xml = mgr.generate_xml();

  CHECK(xml.find("<?xml version=\"1.0\"") != std::string::npos);
  CHECK(xml.find("<Relationships") != std::string::npos);
  CHECK(xml.find("</Relationships>") != std::string::npos);
  CHECK(xml.find("Id=\"rId1\"") != std::string::npos);
  CHECK(xml.find("Target=\"worksheets/sheet1.xml\"") != std::string::npos);
}

TEST_CASE("RelationshipManager generate_xml for empty manager produces valid XML") {
  auto mgr = xlsdraw::resource::RelationshipManager{};
  auto const xml = mgr.generate_xml();

  CHECK(xml.find("<Relationships") != std::string::npos);
  CHECK(xml.find("</Relationships>") != std::string::npos);
  // エントリが 0 件のとき <Relationship .../> が含まれてはならない
  CHECK(xml.find("<Relationship ") == std::string::npos);
}

TEST_CASE("ContentTypesManager generate_xml includes default and override entries") {
  auto mgr = xlsdraw::resource::ContentTypesManager{};
  // ContentTypesManager は ctor で rels/xml の 2 件を登録済み

  mgr.add_override(
    "xl/workbook.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"
  );
  mgr.add_override(
    "xl/worksheets/sheet1.xml",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"
  );

  auto const xml = mgr.generate_xml();

  CHECK(xml.find("<?xml") != std::string::npos);
  CHECK(xml.find("<Types") != std::string::npos);
  CHECK(xml.find("</Types>") != std::string::npos);

  // ctor で追加された Default エントリが含まれること
  CHECK(xml.find("Extension=\"rels\"") != std::string::npos);
  CHECK(xml.find("Extension=\"xml\"") != std::string::npos);

  // add_override で追加したエントリが含まれること
  CHECK(xml.find("PartName=\"/xl/workbook.xml\"") != std::string::npos);
  CHECK(xml.find("PartName=\"/xl/worksheets/sheet1.xml\"") != std::string::npos);
  CHECK(xml.find("spreadsheetml.sheet.main") != std::string::npos);
}

TEST_CASE("ContentTypesManager normalize_part_name prepends slash if missing") {
  auto mgr = xlsdraw::resource::ContentTypesManager{};

  // スラッシュなしで追加
  mgr.add_override("xl/drawing.xml", "application/vnd.openxmlformats-officedocument.drawing+xml");
  // スラッシュありで追加（二重にならないこと）
  mgr.add_override("/xl/other.xml", "application/xml");

  auto const xml = mgr.generate_xml();
  CHECK(xml.find("PartName=\"/xl/drawing.xml\"") != std::string::npos);
  CHECK(xml.find("PartName=\"/xl/other.xml\"") != std::string::npos);
  // 二重スラッシュが発生していないことを確認
  CHECK(xml.find("PartName=\"//xl/other.xml\"") == std::string::npos);
}
