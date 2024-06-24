cmake_minimum_required(VERSION 3.21)
project(umba-msvc-filters C CXX)


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
add_subdirectory(${LIB_ROOT}/marty_pugixml)
add_subdirectory(${LIB_ROOT}/pugixml)
add_subdirectory(${LIB_ROOT}/sfmt)
add_subdirectory(${LIB_ROOT}/umba)


set(sources 
    ${SRC_ROOT}/umba-msvc-filters/umba-msvc-filters.cpp
   )

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE encoding::encoding)
target_link_libraries(${PROJECT_NAME} PRIVATE marty_cpp::marty_cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE marty_pugixml::marty_pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml::pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE sfmt::sfmt)
target_link_libraries(${PROJECT_NAME} PRIVATE umba::umba)

