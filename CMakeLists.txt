cmake_minimum_required(VERSION 3.25)
project(xlsdraw LANGUAGES CXX)

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
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

## add warning
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
endif()

if (MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

## optimization
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ox")
endif()

## other flag
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(libzip CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

enable_testing()
add_subdirectory(test)

include_directories(include)

# add_executable(example example.cpp)
# target_link_libraries(example PRIVATE
#     libzip::zip
#     pugixml::pugixml
#     fmt::fmt
# )
# target_compile_features(example PRIVATE ${STD_CPP})

foreach(idx RANGE 1 100)
    if(idx LESS 10)
        set(idx "0${idx}")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/test${idx}.cpp")
        add_executable(test${idx} test${idx}.cpp)
        target_link_libraries(test${idx} PRIVATE
            libzip::zip
            pugixml::pugixml
            fmt::fmt
        )
        target_compile_features(test${idx} PRIVATE ${STD_CPP})
    endif()
endforeach()
