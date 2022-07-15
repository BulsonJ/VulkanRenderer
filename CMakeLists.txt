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
target_link_libraries(VulkanRenderer vk-bootstrap vma glm imgui)
target_link_libraries(VulkanRenderer Vulkan::Vulkan SDL2 Tracy::TracyClient)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

## find all the shader files under the shaders folder
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
    )

## iterate each shader
foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  ##execute glslang command to compile that specific shader
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(VulkanRenderer Shaders)