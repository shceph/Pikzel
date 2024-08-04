# Pikzel

## Description
Pikzel is a drawing application, specialized for pixel art, built in C++.

## Table of Contents
- [Building](#building)
- [License](#license)

## Building
1. Clone the repository
   ```sh
   git clone --recurse-submodules https://github.com/shceph/Pikzel.git
   ```
   Make sure you have the submodules as well. If you forgot to use `--recurse-submodules` when cloning the repo, execute the following command to add them:
   
   ```sh
   git submodule update --init --recursive
   ```
2. Compile the project:
   Generate the build files using CMake based on the CMakeLists.txt in the project directory.
   I like to do this by creating a build directory and generating the build files in it. Execute the following commands in the project directory:

   ```sh
   mkdir build
   cd build
   cmake ..
   ```

   Please let me know if you encountered any issues with building.

## License
  The program is distributed under the MIT license.
