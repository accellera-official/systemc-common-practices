cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(initiator_id VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to build all targets.")

set(GITHUB "https://github.com/" CACHE STRING "github base url")

include(FetchContent)
include(CTest)

file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.5/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

cpmaddpackage("${GITHUB}TheLartians/PackageProject.cmake.git@1.4.1")

cpmaddpackage(
   NAME SystemCLanguage
   GIT_REPOSITORY  ${GITHUB}accellera-official/systemc.git
   GIT_SHALLOW True
   GIT_TAG main
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
   ${PROJECT_NAME} INTERFACE
   $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(BUILD_TESTING AND ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}"))
   enable_testing()
   add_subdirectory(tests)
endif()

add_library("scp::tlm_extensions::lib${PROJECT_NAME}" ALIAS ${PROJECT_NAME})
packageproject(
   NAME "${PROJECT_NAME}"
   VERSION ${PROJECT_VERSION}
   NAMESPACE scp::tlm_extensions
   BINARY_DIR ${PROJECT_BINARY_DIR}
   INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
   INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   VERSION_HEADER "${VERSION_HEADER_LOCATION}"
   COMPATIBILITY SameMajorVersion
)
install(
   TARGETS ${PROJECT_NAME}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   COMPONENT "${PROJECT_NAME}_Runtime"
   NAMELINK_COMPONENT "${PROJECT_NAME}_Development"
)

