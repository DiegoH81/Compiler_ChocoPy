cmake_minimum_required(VERSION 3.20)

project(chocopy_compiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE CORE_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
)

list(REMOVE_ITEM CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

if(CORE_SOURCES)
    add_library(chocopy_core ${CORE_SOURCES})

    target_include_directories(chocopy_core PUBLIC
        include
    )

    target_compile_definitions(chocopy_core PUBLIC
        CHOCOPY_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    if(MSVC)
        target_compile_options(chocopy_core PRIVATE /W4)
    else()
        target_compile_options(chocopy_core PRIVATE
            -Wall -Wextra -Wpedantic
        )
    endif()
endif()

add_executable(chocopy
    src/main.cpp
)

target_include_directories(chocopy PRIVATE
    include
)

target_compile_definitions(chocopy PRIVATE
    CHOCOPY_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(TARGET chocopy_core)
    target_link_libraries(chocopy PRIVATE
        chocopy_core
    )
endif()