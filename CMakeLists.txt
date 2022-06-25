cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(VulkanRenderer VERSION 1.0)

set(CMAKE_AUTOGEN_VERBOSE 1)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
find_package(Vulkan REQUIRED)

file(GLOB SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB HEADER_FILES CONFIGURE_DEPENDS "include/*.h")
add_executable(VulkanRenderer ${SRC_FILES} ${HEADER_FILES})

target_include_directories(VulkanRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(third-party)
target_link_libraries(VulkanRenderer vk-bootstrap vma glm)
target_link_libraries(VulkanRenderer Vulkan::Vulkan SDL2 Tracy::TracyClient)