cmake_minimum_required(VERSION 3.20)
project(league-paper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug) #realease or debug
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCES "src/*.cpp")
add_executable(league-paper ${SOURCES})

target_include_directories(league-paper PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(league-paper PRIVATE wininet)
target_link_options(league-paper PRIVATE
    -static-libgcc -static-libstdc++
)
