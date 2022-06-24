cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(hello VERSION 1.0)

file(GLOB SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(hello ${SRC_FILES})
target_include_directories(hello PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)