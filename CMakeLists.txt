cmake_minimum_required(VERSION 3.21)
project(umba-msvc-filters C CXX)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


set(PRJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(SRC_ROOT "${PRJ_ROOT}/src")
set(LIB_ROOT "${PRJ_ROOT}/_libs")

include(${PRJ_ROOT}/.cmake/umba.cmake)

include_directories(${LIB_ROOT})

add_subdirectory(${LIB_ROOT}/encoding)
add_subdirectory(${LIB_ROOT}/marty_cpp)
add_subdirectory(${LIB_ROOT}/marty_pugixml)
add_subdirectory(${LIB_ROOT}/pugixml)
add_subdirectory(${LIB_ROOT}/sfmt)
add_subdirectory(${LIB_ROOT}/umba)


set(UMBA_MSVC_FILTERS_ROOT "${SRC_ROOT}/umba-msvc-filters")

set(sources "${UMBA_MSVC_FILTERS_ROOT}/umba-msvc-filters.cpp")
set(headers 
    "${UMBA_MSVC_FILTERS_ROOT}/app_config.h"
    "${UMBA_MSVC_FILTERS_ROOT}/app_ver_config.h"
    "${UMBA_MSVC_FILTERS_ROOT}/arg_parser.h"
    "${UMBA_MSVC_FILTERS_ROOT}/log.h"
    "${UMBA_MSVC_FILTERS_ROOT}/msvc_project.h"
    "${UMBA_MSVC_FILTERS_ROOT}/msvc_sln.h"
    "${UMBA_MSVC_FILTERS_ROOT}/utils.h"
   )

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Sources" FILES "${sources}")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Headers" FILES "${headers}")


# kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)

add_executable(${PROJECT_NAME} ${sources} ${headers})
umba_add_target_options(${PROJECT_NAME}
                        #"UNICODE"
                        "CONSOLE"
                        #"WINDOWS"
                        #"BIGOBJ"
                        "UTF8"
                       )
target_link_libraries(${PROJECT_NAME} PRIVATE encoding::encoding)
target_link_libraries(${PROJECT_NAME} PRIVATE marty_cpp::marty_cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE marty_pugixml::marty_pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml::pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE sfmt::sfmt)
target_link_libraries(${PROJECT_NAME} PRIVATE umba::umba)

