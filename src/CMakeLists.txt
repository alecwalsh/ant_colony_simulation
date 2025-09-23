add_library(ant_sim_project
        simulation.cpp ../include/ant_sim_project/simulation.hpp
        world.cpp ../include/ant_sim_project/world.hpp
        ../include/ant_sim_project/mutex_guard.hpp
        graphics.cpp ../include/ant_sim_project/graphics.hpp
)

add_executable(ant_sim_project_main ant_sim_project_main.cpp)

find_package(SFML CONFIG REQUIRED COMPONENTS Window Graphics)

target_link_libraries(ant_sim_project PUBLIC SFML::Graphics)
target_link_libraries(ant_sim_project_main PRIVATE ant_sim_project SFML::Window SFML::Graphics)

target_compile_features(ant_sim_project PUBLIC c_std_23 cxx_std_23)

enable_warnings(ant_sim_project)
enable_warnings(ant_sim_project_main)

enable_lto(ant_sim_project)
enable_lto(ant_sim_project_main)

# Projects linking to this library need to explicitly specify the subfolder
# That isn't necessary within the project, though
target_include_directories(ant_sim_project PUBLIC "../include")
target_include_directories(ant_sim_project PRIVATE "../include/ant_sim_project")

configure_file(../include/ant_sim_project/version.hpp.in include/ant_sim_project/version.hpp)
target_include_directories(ant_sim_project PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

#Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(ant_sim_project PRIVATE DEBUG)
endif()
