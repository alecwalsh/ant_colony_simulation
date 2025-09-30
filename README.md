# Ant colony simulation project

## Project Status

### What has been implemented
I have implemented most of the basic elements of the simulation.

- Simulated ants move around the world, searching for food.
- The ants leave behind pheromone trails as they move.
- Pheromones fade over time.
- Ants ignore pheromones from nests other than their own.
- The graphics show the different elements of the simulation with different colors, allowing the user to visualize what is happening.

### What needs improvement

While the pheromone based pathfinding has been implemented, it has several issues.

- Sometimes ants can get lost.  If they venture far from their nest, then the pheromone trail can fade, leaving them aimlessly wandering.  This can be improved by decreasing the rate at which pheromones fade.
- If pheromones fade too slowly, then large areas of the map become saturated with pheromones, which reduces their effectiveness as a pathfinding tool.
- When too many ants are present, they often get stuck following each other in circles.

I believe that the first two issues can be fixed by finding the right balance of pheromone strength increases and decreases.

The third one can likely be fixed by increasing the level of randomness in the ants' movements.

### What hasn't been implemented yet

Not everything I planned has been implemented yet.  Here are a list of features that still need to be implemented, in the order I plan to implement them.

- Starvation and reproduction.  As a result, population dynamics are not yet simulated.  I anticipate adding this to be relatively simple, now that I have the basics implemented.
- Seasonal variation in food supply.
- Data collection and visualization.  There is currently no easy way to collect data from the simulation.  This is going to be my priority over the next few weeks.
- Improved graphics.  I plan to allow the viewing larger sections of the world, as well as zooming and panning.  Currently, the only way to view more of the map is to resize the window.

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