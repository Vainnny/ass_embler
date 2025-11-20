cmake_minimum_required(VERSION 3.28)
project(MAPKART C)

set(CMAKE_C_STANDARD 11)

add_executable(map_colorizer
        main.c
        bmp_handler.c
        region_detector.c
        graph.c
        colorizer.c
        utils.c
)

message(STATUS "CMake configuration complete. Use 'make' to build the project.")