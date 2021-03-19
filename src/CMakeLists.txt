cmake_minimum_required(VERSION 3.10)
project(ap)

aux_source_directory(. AP_SRC)
add_library(${PROJECT_NAME} ${AP_SRC})
target_link_libraries(${PROJECT_NAME} m)
target_include_directories(${PROJECT_NAME} PRIVATE ../include)