include_guard(GLOBAL)

# "UNICODE" "CONSOLE" "WINDOWS" "BIGOBJ" "UTF8"
# "SRCUTF8"
function(umba_add_target_options TARGET)

    # https://jeremimucha.com/2021/02/cmake-functions-and-macros/

    #math(EXPR indices "${ARGC} - 1")
    #foreach(index RANGE ${indices})

    math(EXPR maxArgN "${ARGC} - 1")
    foreach(index RANGE 1 ${maxArgN} 1)

        #message("  ARGV${index}: ${ARGV${index}}")

        if(${ARGV${index}} STREQUAL "UNICODE")
            if(WIN32)

                # Common for all
                target_compile_definitions(${TARGET} PRIVATE "UNICODE" "_UNICODE")

                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add UNICODE options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-municode")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add UNICODE options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    #message(NOTICE "Add UNICODE options for MSVC")
                endif()
            endif() # UNICODE

        elseif(${ARGV${index}} STREQUAL "SRCUTF8" OR ${ARGV${index}} STREQUAL "UTF8SRC")
            if(WIN32)
                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add SRCUTF8 options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-finput-charset=UTF-8")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add SRCUTF8 options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    # /utf-8, that sets both /source-charset:utf-8 and /execution-charset:utf-8.
                    target_compile_options(${TARGET} PRIVATE "/source-charset:utf-8")
                endif()
            endif() # CONSOLE

        elseif(${ARGV${index}} STREQUAL "UTF8")
            if(WIN32)
                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add UTF8 options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-finput-charset=UTF-8")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add UTF8 options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    target_compile_options(${TARGET} PRIVATE "/utf-8")
                endif()
            endif() # CONSOLE

        elseif(${ARGV${index}} STREQUAL "CONSOLE")
            if(WIN32)

                # Common for all
                target_compile_definitions(${TARGET} PRIVATE "CONSOLE" "_CONSOLE")

                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add CONSOLE options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-mconsole")
                    target_link_options(${TARGET} PRIVATE "-Wl,--subsystem,console")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add CONSOLE options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:CONSOLE")
                endif()
            endif() # CONSOLE

        elseif(${ARGV${index}} STREQUAL "WINDOWS")
            if(WIN32)

                # Common for all
                target_compile_definitions(${TARGET} PRIVATE "WINDOWS" "_WINDOWS")

                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add WINDOWS options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-mwindows")
                    target_link_options(${TARGET} PRIVATE "-Wl,--subsystem,windows")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add WINDOWS options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    target_link_options(${TARGET} PRIVATE "/SUBSYSTEM:WINDOWS")
                endif()
            endif() # CONSOLE

        elseif(${ARGV${index}} STREQUAL "BIGOBJ")
            if(WIN32)
                if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                    message(NOTICE "Add BIGOBJ options for Clang")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
                    target_compile_options(${TARGET} PRIVATE "-Wa,-mbig-obj")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
                    message(NOTICE "Add BIGOBJ options for Intel")
                elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                    target_compile_options(${TARGET} PRIVATE "/bigobj")
                endif()
            endif() # BIGOBJ

        endif()

    endforeach()

endfunction()


