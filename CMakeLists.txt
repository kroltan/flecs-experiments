cmake_minimum_required(VERSION 3.25)
project(chillworld)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(flecs)
add_subdirectory(raylib)

add_executable(${PROJECT_NAME} main.cpp
        sdf.cpp
        sdf.h
        vectors.h
        draw.h
        draw.cpp
        util.h
        sdf_fragments.h
        animate.cpp
        animate.h
        transform.h
        transform.cpp
)
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)
target_link_libraries(${PROJECT_NAME} raylib)

add_custom_target(
        ${PROJECT_NAME}_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_assets)