cmake_minimum_required(VERSION 3.23)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectId ${ProjectId})
project(${ProjectId} LANGUAGES CXX)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

macro (ensure_dir_exist dir_name)
    if(NOT DEFINED ${dir_name})
        message(FATAL_ERROR "${dir_name} is not set")
    endif()
endmacro ()

ensure_dir_exist(GLAD)

add_executable(${ProjectId} main.cpp )
target_include_directories(${ProjectId} PRIVATE ${OPENGL_INCLUDE_DIRS} ${GLAD}/include ./)

set(WINDOWS_FILE $<$<PLATFORM_ID:Windows>:glad.lib>)
set(LINUX_FILE $<$<PLATFORM_ID:Linux>:libglad.a>)

target_link_directories(${ProjectId} PRIVATE ${GLAD}/lib)
target_link_libraries(${ProjectId} ${OPENGL_LIBRARIES} glfw glad glm::glm)

add_custom_target(run COMMAND $<TARGET_FILE:${ProjectId}>)

set_target_properties(${ProjectId} PROPERTIES
  PUBLIC_HEADER "")
  
install(TARGETS ${ProjectId})
