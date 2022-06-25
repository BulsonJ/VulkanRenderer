find_package(Vulkan REQUIRED)

add_library(vk-bootstrap STATIC)

target_sources(vk-bootstrap PRIVATE 
    vk-bootstrap/src/VkBootstrap.h
    vk-bootstrap/src/VkBootstrap.cpp
    )

target_include_directories(vk-bootstrap PUBLIC vk-bootstrap/src)
target_link_libraries(vk-bootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

add_library(SDL2 INTERFACE)

target_include_directories(SDL2 INTERFACE SDL2/include)
target_link_directories(SDL2 INTERFACE SDL2/lib/x64)
target_link_libraries(SDL2 INTERFACE SDL2.lib SDL2main.lib)