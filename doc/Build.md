# Build
Following this instruction, you will be guided to build RAMF and run a simple example. 

## System Requirement
### Platform 
Although RAMF supports both Linux and Windows system, it is recommended to use and develop RAMF on a **Linux** platform. If you want to choose Windows, please refer to the special notes on building RAMF on Windows.

### Dependency
RAMF depends on several third-party libraries:

1. Eigen3 (built-in)
2. OpenMP
3. Libtorch
4. Pybind11 (built-in, optional)
5. CUDA (optional)

**`Eigen3` is integrated as part of the project, so there is no need to download it manually.**

**You need to make sure `OpenMP` is available for the compiler.** 

**You need to install `Libtorch` manually, and specify the path to it when you build RAMF (see below for more detailed instruction on how to build RAMF)**. 
Please make sure you use the correct version of Libtorch that is capable to your environment (Linux/Windows, CUDA/cpu, CUDA version, c++-standard, debug/release etc.).

**`Pybind11` is integrated as part of the project, so there is no need to download it manually.**
Pybind11 is only used for exporting Environment module as a python module, so that you can use RAMF Environment in python and integrate it with agents written in python. For instance, you can use pytorch to write your own reinforcement learning agents and simulate active matters with RAMF Environment in python. 
However, pybind11 is optional and turned off by default. Currently, it is still an experimental function and API is not stable. **If you want to export python module, modified `config.cmake` and set `RAMF_BUILD_PYBIND` to true.**

**If you need to use CUDA version of Libtorch, you also need to install correct version of CUDA**

## Building on Linux
Before start, please make sure you have installed CMake with minimal version 3.16, and a c++ compiler that supports c++17 standard, as it is required by Libtorch. 

For Linux system, the best practice is to manage the project with Visual Studio Code. 
1. Install the c/c++ Extension Pack and CMake Tools extensions.
2. Open the root folder of RAMF.
3. Specify the path of libtorch in config.cmake. The path should direct to the subfolder of libtorch, e.g. `/Prefix/libtorch/share/cmake/Torch/` where it contains `TorchConfig.cmake`.
4. Configure `CMakeLists.txt`.
5. Specific the path to the config.cfg (`configpath`) in `RAMF_main.cpp`.
5. Build the project.

6. Prepare the config file.
7. Run RAMF.


## Building on Windows
Building RAMF on Windows is basically the same as on Linux. 
However, there are some restriction on the dependency.
1. Due to the limitation of Libtorch and CUDA on Windows, you must use msvc compiler to build RAMF. You may specify the compiler through CMake configuration.
2. Libtorch has two different version for Debug and Release build. Make sure you correctly set the path to each of it in `config.cmake`