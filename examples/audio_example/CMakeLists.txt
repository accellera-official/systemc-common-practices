cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(audioexample VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to build all targets.")

set(GITHUB "https://github.com/" CACHE STRING "github base url")

include(FetchContent)
include(CTest)

# download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.2/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

cpmaddpackage("${GITHUB}TheLartians/PackageProject.cmake.git@1.4.1")

cpmaddpackage(
  NAME SystemCLanguage
  GIT_REPOSITORY ${GITHUB}accellera-official/systemc.git
  GIT_SHALLOW True
  GIT_TAG 3.0.1
)

set(SYSTEMC_HOME ${SystemCLanguage_SOURCE_DIR})
include_directories(${SystemCLanguage_SOURCE_DIR}/src)
include_directories(${SystemCAMS_SOURCE_DIR}/src)
cpmaddpackage(
    NAME SystemCAMS
    GIT_REPOSITORY ${GITHUB}/Risto97/systemc-ams.git
    GIT_SHALLOW True
    GIT_TAG master
)
message("including ${SystemCAMS_SOURCE_DIR}/src")
target_include_directories(systemc-ams PRIVATE ${SystemCAMS_SOURCE_DIR}/src)


cpmaddpackage(
    NAME SystemCCCI
    GIT_REPOSITORY ${GITHUB}accellera-official/cci.git
    GIT_SHALLOW True
    GIT_TAG main
)

cpmaddpackage(
    NAME SCP
    GIT_REPOSITORY https://github.com/accellera-official/systemc-common-practices.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

cpmaddpackage(
    NAME MidiParser
    GIT_REPOSITORY https://github.com/MStefan99/Midi-Parser.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

file(DOWNLOAD
    https://www.midiworld.com/midis/other/bach/bwv972.mid
    ${CMAKE_CURRENT_BINARY_DIR}/bwv972.mid
    SHOW_PROGRESS
)

include_directories(${PORTAUDIO_INCLUDE_DIRS})
include_directories(include)
add_executable(${PROJECT_NAME}
    src/main.cc
)

find_package(PkgConfig REQUIRED) 
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET GLOBAL portaudio-2.0)
#include_directories(${PORTAUDIO_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::systemc SystemC::cci systemc-ams scp::reporting PkgConfig::PORTAUDIO libParser)
