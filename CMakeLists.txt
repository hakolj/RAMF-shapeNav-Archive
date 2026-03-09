#
# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.16)
#set(CMAKE_CXX_COMPILER "msvc")
#set(CMAKE_C_COMPILER "msvc")
# set(CMAKE_CXX_COMPILER "g++-10")
# set(CMAKE_C_COMPILER "gcc-10")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_C_COMPILER "gcc")
project("RAMF")

include(config.cmake)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# set gprof (a built-in line profile tool of GNU)
if (RAMF_ENABLE_GPROF)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    endif()
endif()


# Eigen3
get_filename_component(EIGEN3_INCLUDE_DIRS ./lib/eigen-3.3.9 ABSOLUTE)
include_directories(${EIGEN3_INCLUDE_DIRS})
message("Eigen3 at ${EIGEN3_INCLUDE_DIRS}")


#openmp
find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
set(CMAKE_SHARE_LINKER_FLAGS "${CMAKE_SHARE_LINKER_FLAGS} ${OpenMP_SHARE_LINKER_FLAGS}") 
message("${OpenMP_CXX_FLAGS}")

# torchlib
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    # set(TorchPath /mnt/c/work/RAMFdev/3partylib/libtorch+cu121-linux/libtorch/share/cmake/Torch)
    set(TorchPath "${RAMF_TorchPath_linux}")
    message("${TorchPath}")
    

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(TorchPath "${RAMF_TorchPath_win_rel}")

    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    set(TorchPath "${RAMF_TorchPath_win_rel}")

    elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(TorchPath "${RAMF_TorchPath_dbg_rel}") 
    endif()
    
endif()

find_package(Torch REQUIRED
            NO_MODULE
            PATHS "${TorchPath}"
            NO_DEFAULT_PATH)
include_directories(${TORCH_INCLUDE_DIRS}) 

message("System name: ${CMAKE_SYSTEM_NAME}")
message("build type: ${CMAKE_BUILD_TYPE}")
message("TorchPath: ${TorchPath}")


# pybind11 only for release build
if (RAMF_BUILD_PYBIND)
    add_subdirectory(./lib/pybind11-2.10)
    pybind11_add_module(amfpy ./RAMF/pybind/amfpy.cpp)
endif()

# building source code


option(CHECK_INCLUDE_DIR "switch to show include directories for debugging cmakelists" ON)

# add_executable (RAMF "RAMF/testMain.cpp" )
add_executable (RAMF "RAMF/RAMF_main.cpp" )
# add_executable (RAMF RAMF.cpp RAMF.h)

include_directories(RAMF/include)
include_directories(RAMF/)



if (CHECK_INCLUDE_DIR)
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()
endif()

# if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
#     set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
#     set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
# endif()


## start compiling source


# add_library(PCH STATIC RAMF/include/pch.cpp )
# add_library(PTEST STATIC "RAMF/performanceTest.cpp" "RAMF/inferenceNet.cpp" )

# aux_source_directory(RAMF/utils src_utils) 
# add_library(Utils STATIC ${src_utils})
add_subdirectory(RAMF/utils Utils)

add_library(SManager STATIC "RAMF/TrainManager.cpp" )

# aux_source_directory(RAMF/Agent src_Agent) 
# add_library(Agent STATIC ${src_Agent} )
#add_library(ML STATIC "source/Machine Learning/QLearning.cpp" "source/Machine Learning/DDQL.cpp" "source/Machine Learning/Memory.cpp")
add_subdirectory(RAMF/Agent Agent)

# aux_source_directory(RAMF/Actor src_Actor) 
# add_library(Actor STATIC ${src_Actor} )
add_subdirectory(RAMF/Actor Actor)

# aux_source_directory(RAMF/ActiveMatter src_ActiveMatter) 
# add_library(ActiveMatter STATIC ${src_ActiveMatter} )
add_subdirectory(RAMF/ActiveMatter ActiveMatter)
# aux_source_directory(RAMF/Sensor src_Sensor) 
# add_library(Sensor STATIC ${src_Sensor} )
add_subdirectory(RAMF/Sensor Sensor)

# aux_source_directory(RAMF/Fluid src_Fluid) 
# add_library(Fluid STATIC ${src_Fluid} )
add_subdirectory(RAMF/Fluid Fluid)

# aux_source_directory(RAMF/Task src_Task) 
# add_library(Task STATIC ${src_Task} )
add_subdirectory(RAMF/Task Task)

aux_source_directory(RAMF/Env src_env)
add_library(Env STATIC ${src_env} )

# target_link_libraries(PCH ${TORCH_LIBRARIES})
target_link_libraries(Utils ${TORCH_LIBRARIES})
target_link_libraries(ActiveMatter Utils)
target_link_libraries(Sensor Utils)
target_link_libraries(Actor Utils)
target_link_libraries(Fluid Utils)
target_link_libraries(Task Utils)
target_link_libraries(SManager Utils)
target_link_libraries(Env ActiveMatter Sensor Actor Fluid Task SManager Utils)
# set_property(TARGET Env PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(Agent Utils)
# target_link_libraries(PTEST Agent Env)

target_link_libraries(RAMF Env Agent)
# target_link_libraries(RAMF PTEST)

set_target_properties(Utils PROPERTIES COMPILE_OPTIONS "${target_options}")

# allow pybind11 binder cpp to use other modules
if (BUILD_PYBIND)
target_link_libraries(amfpy PUBLIC Env)
endif()

if (MSVC)
    file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
    add_custom_command(TARGET RAMF
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TORCH_DLLS}
        $<TARGET_FILE_DIR:RAMF>)

 #   add_custom_command(TARGET RAMF
 #       POST_BUILD
  #      COMMAND ${CMAKE_COMMAND} -E copy_if_different
 #       E:/Intel/oneAPI/mkl/2021.3.0/lib/intel64/mkl_intel_ilp64.lib
  #      E:/Intel/oneAPI/mkl/2021.3.0/lib/intel64/mkl_core.lib
  #      E:/Intel/oneAPI/mkl/2021.3.0/lib/intel64/mkl_intel_thread.lib
  #      $<TARGET_FILE_DIR:RAMF>/..)
endif (MSVC)


# add_subdirectory(post RAMFPostGridSensor)

