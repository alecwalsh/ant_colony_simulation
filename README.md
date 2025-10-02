# Ant colony simulation project

## Project Status

### What has been implemented

### What hasn't been implemented yet

## Installation Instructions

The project is written in C++.  It uses CMake as its build system, and vcpkg for managing dependencies.

To build this project, you will need a modern C++ toolchain, CMake, and Make.  Installing these tools is platform specific.  
On Linux you can install these with the system's package manager.  
On macOS, install the [Xcode command line tools](https://developer.apple.com/xcode/resources/), then install CMake with [Homebrew](https://brew.sh/)
On Windows, install Visual Studio.  Make sure to install Visual Studio's CMake support.

Clone vcpkg from GitHub and run the bootstrap-vcpkg script( bootstrap-vcpkg.sh for Linux and macOS, bootstrap-vcpkg.bat for Windows)

    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh

Next, clone this project and create a directory to hold the build.

    git clone https://github.com/alecwalsh/ant_colony_simulation.git
    mkdir ant_colony_simulation/build
    cd ant_colony_simulation/build

The project is now ready to build.

    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
    make

Replace /path/to/vcpkg with the path that vcpkg was cloned into.

I have tested this on Linux and macOS.  It should work on Windows, as I've taken care to not write any platform-specific code, but I haven't actually tried.

## Dependencies

This project has two direct dependencies, not counting the C++ standard library.  

It uses [SFML](https://www.sfml-dev.org/), a cross-platform library that handles window creation, input handling, as well as providing a simple 2D graphics API.  

I also use [this](https://github.com/kokkos/mdspan) reference implementation of std::mdspan.  std::mdspan was added to the standard library in C++23, but is not yet widely available.  This library is a drop-in replacement.  

It should be possible to remove this dependency in the future, once libstdc++ supports std::mdspan.


## Usage

The executable will be located at src/ant_sim_project_main, in the directory the project was built in.
Running the executable should display a simple visualization of the simulation.  Empty tiles are white, tiles with ants are black, and tiles containing food are green. 

## Architecture Overview
