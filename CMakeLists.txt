cmake_minimum_required(VERSION 3.25)
project(
    xlsdraw
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "Header-only C++ library for building XLSX with drawings"
    HOMEPAGE_URL "https://github.com/toge/xlsdraw"
)

## ビルドオプション（副作用より前に宣言する）
option(BUILD_TEST "Build xlsdraw tests" ON)
option(BUILD_EXAMPLE "Build xlsdraw usage examples" ON)

## 警告フラグ（ビルドタイプを問わず適用）
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

## 最適化フラグ（ビルドタイプ別に制御する）
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox")
endif()

## other flag
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Header-only library target (xlsdraw::xlsdraw)
# ---------------------------------------------------------------------------
add_library(xlsdraw INTERFACE)
add_library(xlsdraw::xlsdraw ALIAS xlsdraw)

target_include_directories(
    xlsdraw
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(xlsdraw INTERFACE cxx_std_23)

# xlsdraw uses libzip, pugixml, and fmt. Declare them as transitive
# dependencies so consumers only need to find_package(xlsdraw).
find_package(libzip CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(
    xlsdraw
    INTERFACE
        libzip::zip
        pugixml::pugixml
        fmt::fmt
)

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(
    TARGETS xlsdraw
    EXPORT xlsdrawTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
)

# Export the library target so consumers can use it through find_package.
install(
    EXPORT xlsdrawTargets
    FILE xlsdrawTargets.cmake
    NAMESPACE xlsdraw::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xlsdraw
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/xlsdrawConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/xlsdrawConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xlsdraw
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/xlsdrawConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/xlsdrawConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/xlsdrawConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/xlsdraw
)

# ---------------------------------------------------------------------------
# Tests and a minimal usage sample.
# ---------------------------------------------------------------------------
if(BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

if(BUILD_EXAMPLE)
    add_executable(xlsdraw_example example/xlsdraw_example.cpp)
    target_link_libraries(xlsdraw_example PRIVATE xlsdraw::xlsdraw)
    target_compile_features(xlsdraw_example PRIVATE cxx_std_23)
endif()
