find_package(GTest CONFIG REQUIRED)

add_executable(ant_sim_project_test ant_sim_project_test.cpp)

target_link_libraries(ant_sim_project_test PRIVATE ant_sim_project GTest::gtest_main)

enable_warnings(ant_sim_project_test)
enable_lto(ant_sim_project_test)

include(CTest)
include(GoogleTest)
gtest_discover_tests(ant_sim_project_test)
