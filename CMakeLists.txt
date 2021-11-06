cmake_minimum_required(VERSION 3.14)
project(vwriter C CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/FFmpeg)

list(APPEND SOURCES
    src/main.cpp
    src/vwutils.h
    src/vwutils.cpp
)
#MACOSX_BUNDLE WIN32 

add_library(video_writer SHARED src/vwutils.h src/vwutils.cpp)

target_link_libraries(video_writer FFmpeg)

add_executable(vwriter ${SOURCES})

target_link_libraries(vwriter FFmpeg)