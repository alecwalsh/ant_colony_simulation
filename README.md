# Ant colony simulation project

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

I have tested this on Linux and macOS.  It should work on Windows, as I've taken care to not write any platform-specific code, but I haven't actually tried yet.

## Dependencies

This project has two direct dependencies, not counting the C++ standard library.  

It uses [SFML](https://www.sfml-dev.org/), a cross-platform library that handles window creation, input handling, as well as providing a simple 2D graphics API.  

I also use [this](https://github.com/kokkos/mdspan) reference implementation of std::mdspan.  std::mdspan was added to the standard library in C++23, but is not yet widely available.  This library is a drop-in replacement.  

It should be possible to remove this dependency in the future, once libstdc++ supports std::mdspan.


## Usage

The executable will be located at src/ant_sim_project_main, in the directory the project was built in.
Running the executable should display a simple visualization of the simulation.

- Empty tiles are black.
- Nests are blue.
- Tiles with ants are white.
- Tiles containing food are green.
- Pheromone trails are red.  Brighter reds represent stronger pheromone trails. 

## Architecture Overview

The architecture is mostly as described in my submission for Milestone 1.  Here is a brief overview.

- The world is represented as a 2-dimensional grid of tiles.
- Ants and nests are stored in dynamic arrays(std::vector)
- Each tile contains pheromones.
- There are two types of pheromones per nest.
- Type 1 pheromones mark the path back to the nest.
- Type 2 pheromones mark the path from the nest to a food source.
- Ants will avoid one type of pheromone and seek out the other type.  Which is which depends on their current state.

There were no significant architectural changes from the proposal.

There are two things I feel are worth noting, however. 

1. The program uses two threads.  One thread handles user input and draws the graphics.  The other thread runs the simulation.  
The world state is protected with a mutex. 
2. The simulation seems to perform well.  While each tile has its own pheromone levels that decrease with time, they are lazily updated, so very large maps can be used with almost no CPU impact.  The only real limit is the amount of memory.
The time complexity of the simulation is O(n), with n being the number of ants. I have tested the simulation with hundreds of thousands of ants with no noticeable slowdown, although the ants begin to block each other's paths at that point, rendering such high numbers somewhat useless for data collections purposes.
