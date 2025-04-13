cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)
endif()

project(stb_truetype)

add_library(stb_truetype STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/stb_truetype.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

