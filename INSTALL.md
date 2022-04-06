# INSTALLING FAR HORIZONS

# Linux Systems

## Prerequisites

You will need `git` to clone the source files.

You will need a C compiler (`gcc`, for example) and `CMake` to compile the source files.

## Steps

### Clone the repository

    ~$ mkdir -p src
    ~$ cd src
    ~/src$ git clone https://github.com/Ramblurr/Far-Horizons.git

### Compiling

    ~$ cd ~/src/Far-Horizons/build
    ~/src/Far-Horizons/build$ cmake ..
      -- The C compiler identification is GNU 5.4.0
      -- Check for working C compiler: /usr/bin/cc
      -- Check for working C compiler: /usr/bin/cc -- works
      -- Detecting C compiler ABI info
      -- Detecting C compiler ABI info - done
      -- Detecting C compile features
      -- Detecting C compile features - done
      -- Looking for sqrt in m
      -- Looking for sqrt in m - found
      -- Configuring done
      -- Generating done
      -- Build files have been written to: ~/src/Far-Horizons/build
    ~/src/Far-Horizons/build$ cd ~/src/Far-Horizons/build
    ~/src/Far-Horizons$ cmake --build build

The executables will be created in the `~/src/Far-Horizons/build` directory.

# Windows

These instructions assume that you are using JetBrains' CLion IDE.

### Clone the repository

Use your `git` client to clone the repository `https://github.com/Ramblurr/Far-Horizons.git` to your computer.

### Compiling

Open the project in CLion.

Create a new CMake application using the `CMakeLists.txt` file in the root of the repository.

Compile.

## Problems?


## Running a Game
The Gamemaster should read the `README.md` file in the root of this repository.
It provides a brief description of the steps needed to start and run a game.
