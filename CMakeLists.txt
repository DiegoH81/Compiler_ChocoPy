cmake_minimum_required(VERSION 3.20)
project(chocopy_compiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE ALL_HEADERS CONFIGURE_DEPENDS "include/*.h" "include/*.hpp")

add_executable(chocopy ${ALL_SOURCES} ${ALL_HEADERS})
target_include_directories(chocopy PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(MSVC)
    target_compile_options(chocopy PRIVATE /W4)
else()
    target_compile_options(chocopy PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(
    TARGET chocopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/test"
            "$<TARGET_FILE_DIR:chocopy>/test"
    COMMENT "Copying tests"
)