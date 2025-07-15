cmake_minimum_required(VERSION 3.20)

project(Retro3DRaycastingGame VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(${INCLUDE})

set(SOURCES
    ${SRC}/main.cpp
)

add_executable(Retro3DRaycastingGame ${SOURCES})
set_target_properties(Retro3DRaycastingGame PROPERTIES OUTPUT_NAME "Retro3DRaycastingGame")
set_target_properties(Retro3DRaycastingGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BIN})