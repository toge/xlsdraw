// Catch2 v3 は Catch2::Catch2WithMain が提供する main エントリポイントを期待する
// (test/CMakeLists.txt でリンクされる)。このファイルは利便性ヘッダを取り込むだけで、
// 利用者が自身の .cpp ファイルでも `TEST_CASE` 等を記述できるようにする。
#include "catch2/catch_all.hpp"
