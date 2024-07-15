cmake_minimum_required(VERSION 3.20)

project(imgui)

file(GLOB IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.cpp
)

file(GLOB IMGUI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3_loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/misc/cpp/imgui_stdlib.h
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)

foreach(file ${IMGUI_HEADERS})
    # get_filename_component(filename ${file} NAME)
    file(COPY ${file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)
endforeach()

add_library(${PROJECT_NAME} ${IMGUI_SOURCES})

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
