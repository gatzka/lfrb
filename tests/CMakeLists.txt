cmake_minimum_required(VERSION 3.9)
project(lfrb_unit_test C)

find_package(Git QUIET REQUIRED)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


enable_testing()

add_library(fff INTERFACE)
target_include_directories(fff INTERFACE fff)

add_library(unity
    Unity/src/unity.c
)
set_target_properties(unity PROPERTIES
    COMPILE_OPTIONS "$<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-Wno-strict-overflow>"
    C_CLANG_TIDY ""
)
target_include_directories(unity PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Unity/src/)

add_executable(test_lfrb test_lfrb.c ../src/lfrb.c)

get_property(includes TARGET lfrb::lfrb PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(targets DIRECTORY "${CMAKE_CURRENT_LIST_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${targets})
    get_target_property(target_type ${tgt} TYPE)
    if (target_type STREQUAL "EXECUTABLE")
        target_link_libraries(${tgt} fff unity)
        set_target_properties(${tgt} PROPERTIES
            C_STANDARD 11
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF
            C_CLANG_TIDY ""
        )
        target_include_directories(${tgt} PUBLIC ${includes})
        add_test(NAME lfrb.common.${tgt} COMMAND ${tgt})
    endif()

	if(CIO_ENABLE_LTO AND NOT ${target_type} STREQUAL "INTERFACE_LIBRARY")
        set_property(TARGET ${tgt} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endforeach()



