find_package(Catch2 REQUIRED CONFIG)

file(GLOB test_src CONFIGURE_DEPENDS test_*.cpp)

add_executable(all_test main.cpp ${test_src})

target_link_libraries(
    all_test
    PRIVATE
        Catch2::Catch2WithMain
        xlsdraw::xlsdraw
)
target_compile_features(all_test PRIVATE cxx_std_23)

add_test(NAME all_test COMMAND all_test -r junit)
