cmake_minimum_required(VERSION 3.21)
project(umba-msvc-filters C CXX)

# Modules - https://cmake.org/cmake/help/book/mastering-cmake/chapter/Modules.html
# Finding Packages - https://cmake.org/cmake/help/book/mastering-cmake/chapter/Finding%20Packages.html
# How to structure your project - https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html

# CMake 3 Tools - https://github.com/CLIUtils/cmake
# Modern CMake (заменяет CMake 3 Tools) - https://github.com/CLIUtils/modern_cmake
# The Ultimate Guide to Modern CMake - https://rix0r.nl/blog/2015/08/13/cmake-guide/

# Введение в CMake - https://habr.com/ru/articles/155467/
# Руководство по CMake для разработчиков C++ библиотек - https://habr.com/ru/articles/683204/
#   cpp-lib-template - https://github.com/pananton/cpp-lib-template

# cmake-init - The missing CMake project initializer - https://github.com/friendlyanon/cmake-init


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



set(PRJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(SRC_ROOT "${PRJ_ROOT}/src")
set(LIB_ROOT "${PRJ_ROOT}/_libs")

include_directories(${LIB_ROOT})

add_subdirectory(${LIB_ROOT}/encoding)
add_subdirectory(${LIB_ROOT}/marty_cpp)
add_subdirectory(${LIB_ROOT}/sfmt)
add_subdirectory(${LIB_ROOT}/umba)


set(sources 
    ${SRC_ROOT}/umba-msvc-filters/umba-msvc-filters.cpp
   )

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE encoding::encoding)
target_link_libraries(${PROJECT_NAME} PRIVATE marty_cpp::marty_cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE sfmt::sfmt)
target_link_libraries(${PROJECT_NAME} PRIVATE umba::umba)
