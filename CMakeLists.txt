cmake_minimum_required(VERSION 3.18.4)

set(GCC_LOCATION "/tools/gcc/debian/11/v11.2.0/")
set(CMAKE_C_COMPILER "/tools/gcc/debian/11/v11.2.0/bin/gcc")
set(CMAKE_CXX_COMPILER "/tools/gcc/debian/11/v11.2.0/bin/g++")

set(CONAN_DISABLE_COMPILER_CHECK "on")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(process CXX)

include(conan.cmake)
conan_cmake_run(
  CONANFILE conanfile.txt
  BASIC_SETUP CMAKE_TARGETS UPDATE
  PROFILE ${CMAKE_SOURCE_DIR}/conan_profile
  BUILD missing)

add_link_options("-Wl,-rpath,${GCC_LOCATION}/lib64")

add_executable(ps src/main.cpp)

#find_package(spdlog)
#find_package(fmt)
