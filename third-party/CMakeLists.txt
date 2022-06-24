find_package(Vulkan REQUIRED)

add_library(vk-bootstrap STATIC)

target_sources(vk-bootstrap PRIVATE 
    vk-bootstrap/src/VkBootstrap.h
    vk-bootstrap/src/VkBootstrap.cpp
    )

target_include_directories(vk-bootstrap PUBLIC vk-bootstrap/src)
target_link_libraries(vk-bootstrap PUBLIC Vulkan::Vulkan $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)