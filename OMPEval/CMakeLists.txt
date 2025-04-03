cmake_minimum_required(VERSION 3.10)

# Set the project name
project(OMPEval)

#OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "no OpenMP support")
endif()

# Specify the C++ standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)

add_definitions (-O3 -Wall -Wpedantic -pthread -msse4.2)
add_definitions(-DNDEBUG -mtune=native -mavx2 -pthread -mfma -msse2 -ftree-vectorize -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -fopenmp-simd -funroll-loops -DUSE_AVX2)
add_definitions (-std=c++11 -O3 -lboost -march=native -Wall -DINFO -Wno-unused-variable -Wno-unused-function -Wno-reorder)
add_definitions (-Wno-write-strings -Wno-sign-compare -Wno-unused-result)

# Include directories
include_directories(hand-isomorphism/src)
include_directories(omp)

# Add source files from the directories
file(GLOB_RECURSE HAND_ISOMORPHISM_SOURCES hand-isomorphism/src/*.c)
file(GLOB_RECURSE OMP_SOURCES omp/*.cpp)

add_library(C_Library OBJECT ${HAND_ISOMORPHISM_SOURCES})
set_target_properties(C_Library PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Add the main source file
set(MAIN_SOURCE cal_equity.cpp)

# Define the executable
add_executable(cal_equity ${MAIN_SOURCE} ${OMP_SOURCES} $<TARGET_OBJECTS:C_Library>)
add_executable(test_time test_time.cpp ${OMP_SOURCES} $<TARGET_OBJECTS:C_Library>)
