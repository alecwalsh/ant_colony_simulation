cmake_minimum_required(VERSION 3.25.0)

project(ant_sim_project VERSION 0.0.0.1 LANGUAGES CXX)

include("cmake/functions.cmake")

add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND "test" IN_LIST VCPKG_MANIFEST_FEATURES)
    add_subdirectory(tests)
endif()
