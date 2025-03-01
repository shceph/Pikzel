cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(glew-cmake_BUILD_SHARED FALSE)
set(ONLY_LIBS ON) # for glew

# file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
# file(COPY ${CMAKE_SOURCE_DIR}/shader DESTINATION "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

file(GLOB PIKZEL_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB GLA_SOURCES ${CMAKE_SOURCE_DIR}/src/gla/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Compiler can't find this file when building stb's image-resize,
# so I'm copying it to the dir of the source file that makes the problem
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/vendor/stb/stb)
file(COPY ${CMAKE_SOURCE_DIR}/vendor/stb/stb/stb_image_resize2.h
     DESTINATION ${CMAKE_BINARY_DIR}/vendor/stb/stb)

project(pikzel)

add_executable(${PROJECT_NAME} ${PIKZEL_SOURCES} ${GLA_SOURCES})

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}/$<CONFIGURATION>/assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/shader
	${CMAKE_BINARY_DIR}/$<CONFIGURATION>/shader
	COMMENT "Copying assets and shaders to build directory"
)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glew)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/imgui)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glew/include
    PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glm
    PUBLIC ${CMAKE_SOURCE_DIR}/vendor/stb/stb
    PUBLIC ${CMAKE_BINARY_DIR}/vendor/stb/include
    PUBLIC ${CMAKE_BINARY_DIR}/vendor/imgui/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE libglew_static
    PRIVATE glfw
    PRIVATE image
    PRIVATE image-write
    PRIVATE image-resize
    PRIVATE perlin
    PRIVATE glm::glm-header-only
    PRIVATE imgui
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE GL GLU)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
