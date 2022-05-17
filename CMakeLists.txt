cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(scp VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to build all targets.")

include(FetchContent)
include(CTest)

FetchContent_Declare(
  cpm-cmake
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake.git
  GIT_SHALLOW True
  GIT_TAG v0.31.1)
FetchContent_MakeAvailable(cpm-cmake)
include(${cpm-cmake_SOURCE_DIR}/cmake/CPM.cmake)
cpmaddpackage("gh:TheLartians/PackageProject.cmake@1.4.1")

cpmaddpackage(
  NAME SystemCLanguage
  GIT_REPOSITORY  https://github.com/accellera-official/systemc.git
  GIT_SHALLOW True
  GIT_TAG 2.3.3
)

add_subdirectory(${PROJECT_SOURCE_DIR}/MasterIDExtension)
add_subdirectory(${PROJECT_SOURCE_DIR}/PathTraceExtension)

enable_testing()
add_subdirectory(tests)
