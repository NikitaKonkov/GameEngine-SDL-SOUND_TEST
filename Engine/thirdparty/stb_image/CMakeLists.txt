cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)
endif()

project(stb_image)

add_library(stb_image STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



