# set the path to libtorch on linux
# set(RAMF_TorchPath_linux "/mnt/c/work/RAMFdev/3partylib/libtorch+cu121-linux/libtorch/share/cmake/Torch")
# set(RAMF_TorchPath_linux "/mnt/disk3/lbc/RAMF/libtorch-cxx11-abi-shared-with-deps-2.1.1+cpu/libtorch/share/cmake/Torch")
set(RAMF_TorchPath_linux "/mnt/d/work/RAMF/3partylib/libtorch_linux/share/cmake/Torch")

# set the path to libtorch-release on windows
set(RAMF_TorchPath_win_rel "C:/Studies/RAMF/libtorch-release/libtorch/share/cmake/Torch")

# set the path to libtorch-debug on windows
set(RAMF_TorchPath_win_dbg "C:/Studies/RAMF/libtorch-win-shared-with-deps-debug-2.1.0+cpu/libtorch/share/cmake/Torch")

# set true if you want to export a python library for environment
set(RAMF_BUILD_PYBIND false)

# set true to use gprof to profile. This affects the performance. Only valid for Release build.
set(RAMF_ENABLE_GPROF false)