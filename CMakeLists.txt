cmake_minimum_required(VERSION 3.0)
include(ExternalProject)

project(BasicJ2k)

message(STATUS "Generating BasicJ2K project")

set(BASICJ2K_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BASICJ2K_ROOT_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BASICJ2K_ROOT_DIR}/bin")

# OpenJpeg
set(OPENJPEG_INSTALL_DIR ${BasicJ2k_SOURCE_DIR}/ext/openjpeg/build/install)
ExternalProject_Add(OpenJpeg
  SOURCE_DIR ${BasicJ2k_SOURCE_DIR}/ext/openjpeg
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=OPENJPEG_INSTALL_DIR)
set(OPENJPEG_INCLUDE_DIR ${OPENJPEG_INSTALL_DIR}/include/openjpeg-2.1/)
set(OPENJPEG_LIBRARY_DIR ${OPENJPEG_INSTALL_DIR}/lib/)

add_subdirectory(src)
set(CMAKE_BUILD_TYPE Debug)
