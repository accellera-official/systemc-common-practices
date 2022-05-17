cmake_minimum_required(VERSION 3.14)
project(MasterIDExtension VERSION 1.0 LANGUAGES CXX C)


add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
   ${PROJECT_NAME} INTERFACE
   $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
