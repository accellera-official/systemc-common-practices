cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(reporting VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to build all targets.")

set(GITHUB "https://github.com/" CACHE STRING "github base url")

include(FetchContent)
include(CTest)

FetchContent_Declare(
  cpm-cmake
  GIT_REPOSITORY ${GITHUB}cpm-cmake/CPM.cmake.git
  GIT_SHALLOW True
  GIT_TAG v0.31.1
)

FetchContent_MakeAvailable(cpm-cmake)
include(${cpm-cmake_SOURCE_DIR}/cmake/CPM.cmake)

cpmaddpackage("${GITHUB}TheLartians/PackageProject.cmake.git@1.4.1")

if(DEFINED ENV{SYSTEMC_HOME})
  cpmaddpackage(
    NAME SystemCLanguage
    SOURCE_DIR ENV{SYSTEMC_HOME}
  )
else ()
  cpmaddpackage(
    NAME SystemCLanguage
    GIT_REPOSITORY  ${GITHUB}accellera-official/systemc.git
    GIT_SHALLOW True
    GIT_TAG main
  )
endif()

cpmaddpackage(
  NAME
  RapidJSON
  GIT_REPOSITORY
  https://github.com/Tencent/rapidjson
  GIT_TAG
  e0f68a435610e70ab5af44fc6a90523d69b210b3
  GIT_SHALLOW
  FALSE
  OPTIONS
  "RAPIDJSON_BUILD_TESTS OFF"
  "RAPIDJSON_BUILD_DOC OFF"
  "RAPIDJSON_BUILD_EXAMPLES OFF")

  # nasty hack till dady takes the t-bird away.
  set(RAPIDJSON_INCLUDE_DIRS "${RapidJSON_SOURCE_DIR}/include")

cpmaddpackage(
  NAME SystemCCCI
  GIT_REPOSITORY  ${GITHUB}accellera-official/cci.git
  GIT_SHALLOW True
  GIT_TAG main
  OPTIONS "SYSTEMCCCI_BUILD_TESTS OFF"
)

set(WITH_FMT "true" CACHE STRING "Include FMT library")
if (WITH_FMT)
cpmaddpackage(
  NAME fmt
  GIT_REPOSITORY  ${GITHUB}fmtlib/fmt.git
  GIT_SHALLOW True
  GIT_TAG 9.1.0
  OPTIONS FMT_INSTALL "" ON
)
endif()

FetchContent_Declare(
    spdlog_git
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG        "v1.9.2"
    GIT_SHALLOW    ON
)
FetchContent_Populate(spdlog_git)
FetchContent_GetProperties(
  spdlog_git
  SOURCE_DIR spdlog_git_SRC_DIR
  POPULATED spdlog_git_FOUND
)

add_library(${PROJECT_NAME} src/report.cpp)

target_include_directories(
  ${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(TARGET fmt)
  target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${spdlog_git_SRC_DIR}/include)
if(TARGET SystemC::cci)
  target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::cci)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC SystemC::systemc)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(BUILD_TESTING AND ("${PROJECT_NAME}" STREQUAL "${CMAKE_PROJECT_NAME}"))
   enable_testing()
   add_subdirectory(tests)
endif()

add_library("scp::report::lib${PROJECT_NAME}" ALIAS ${PROJECT_NAME})
packageproject(
   NAME "${PROJECT_NAME}"
   VERSION ${PROJECT_VERSION}
   NAMESPACE scp
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

