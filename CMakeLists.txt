cmake_minimum_required(VERSION 3.25)
project(
    xlsdraw
    LANGUAGES CXX
    VERSION 0.1.0
    DESCRIPTION "Header-only C++ library for building XLSX with drawings"
    HOMEPAGE_URL "https://github.com/toge/xlsdraw"
)

## ビルドオプション（副作用より前に宣言する）
option(XLSDRAW_BUILD_TESTS "Build xlsdraw tests" ON)
option(XLSDRAW_BUILD_EXAMPLES "Build xlsdraw usage examples" ON)
option(XLSDRAW_NATIVE_ARCH "Enable -march=native optimization (不可搬、配布ライブラリでは使用しないこと)" OFF)

## add stdc++ compiler flag
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++26" COMPILER_SUPPORTS_CXX26)
CHECK_CXX_COMPILER_FLAG("-std=c++23" COMPILER_SUPPORTS_CXX23)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX26)
    set(STD_CPP cxx_std_26)
elseif(COMPILER_SUPPORTS_CXX23)
    set(STD_CPP cxx_std_23)
elseif(COMPILER_SUPPORTS_CXX20)
    set(STD_CPP cxx_std_20)
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

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
    if(XLSDRAW_NATIVE_ARCH)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()
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
target_compile_features(xlsdraw INTERFACE ${STD_CPP})

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
if(XLSDRAW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(XLSDRAW_BUILD_EXAMPLES)
    add_executable(xlsdraw_example example/xlsdraw_example.cpp)
    target_link_libraries(xlsdraw_example PRIVATE xlsdraw::xlsdraw)
    target_compile_features(xlsdraw_example PRIVATE ${STD_CPP})
endif()
