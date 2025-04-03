cmake_minimum_required(VERSION 3.10)

# Set the project name
project(OpenPureCFR)

# Specify the C++ standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)

add_definitions (-O3 -Wall -Wpedantic)
add_definitions(-DNDEBUG -mtune=native -mavx2 -pthread -mfma -msse2 -ftree-vectorize -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -fopenmp-simd -funroll-loops -DUSE_AVX2)
add_definitions (-std=c++11 -O3 -lboost -march=native -Wall -DINFO -Wno-unused-variable -Wno-unused-function -Wno-reorder)
add_definitions (-Wno-write-strings -Wno-sign-compare -Wno-unused-result)

# Include directories
# include_directories(hand-isomorphism/src)
# include_directories(acpc_server_code)
# include_directories(omp)

# Add source files from the directories
file(GLOB HAND_ISOMORPHISM_SOURCES hand-isomorphism/src/*.c)
file(GLOB ACPC_SOURCES acpc_server_code/*.c)
file(GLOB CFR_SOURCES ./*.cpp)

add_library(C_Library OBJECT ${HAND_ISOMORPHISM_SOURCES} ${ACPC_SOURCES})
set_target_properties(C_Library PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Add the main source file
set(MAIN_SOURCE main/pure_cfr.cpp)

# Define the executable
add_executable(pure_cfr ${MAIN_SOURCE} ${CFR_SOURCES} $<TARGET_OBJECTS:C_Library>)

find_package(Threads REQUIRED)
target_link_libraries(pure_cfr PRIVATE Threads::Threads)
