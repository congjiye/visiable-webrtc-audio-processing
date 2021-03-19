cmake_minimum_required(VERSION 3.10)
project(audio_processing)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src)
add_executable(audio_processing main.cpp)
target_link_libraries(${PROJECT_NAME} ap)
target_include_directories(${PROJECT_NAME} PRIVATE include)