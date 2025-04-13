#https://github.com/meemknight/cmakeSetup
#Version 1.1.0

cmake_minimum_required(VERSION 3.16)

# Define USE_AVX512 option before project() to ensure it's recognized
option(USE_AVX512 "Use AVX512 instructions if available (may not work on all machines)" OFF)

# Instead of hardcoding the Vulkan SDK path
# set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.309.0")

# Use environment variable if available, otherwise fall back to a default location
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
    message(STATUS "Using Vulkan SDK from environment: ${VULKAN_SDK_PATH}")
else()
    # Try to find the latest version by checking common installation directories
    file(GLOB VULKAN_SDK_VERSIONS "C:/VulkanSDK/*" LIST_DIRECTORIES true)
    if(VULKAN_SDK_VERSIONS)
        list(SORT VULKAN_SDK_VERSIONS ORDER DESCENDING)
        list(GET VULKAN_SDK_VERSIONS 0 VULKAN_SDK_PATH)
        message(STATUS "Found Vulkan SDK automatically: ${VULKAN_SDK_PATH}")
    else()
        # Fall back to a default path if no installation found
        set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.309.0")
        message(WARNING "No Vulkan SDK found in environment or common locations, using default: ${VULKAN_SDK_PATH}")
    endif()
endif()

# The rest of your existing Vulkan setup
set(ENV{VULKAN_SDK} "${VULKAN_SDK_PATH}")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARY "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib")

# Add a clear message about AVX options - more detailed for debugging
message(STATUS "==== CPU Instruction Set Configuration ====")
message(STATUS "USE_AVX512 option is set to: ${USE_AVX512}")

# Set CMake policies to handle deprecation warnings
if(POLICY CMP0115)
  cmake_policy(SET CMP0115 NEW)  # Require explicit file extensions for sources
endif()

# Get system core count and use half for build
include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
  set(N 1)
endif()
math(EXPR N_HALF "${N} / 2")
if(N_HALF LESS 1)
  set(N_HALF 1)
endif()
set(CMAKE_BUILD_PARALLEL_LEVEL ${N_HALF} CACHE STRING "Set build parallelism to half available cores" FORCE)
message(STATUS "Building with ${N_HALF} parallel jobs (half of ${N} detected cores)")

set(CMAKE_CXX_STANDARD 17)
#! ! ! ! ! ! !
#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
#delete the out folder after changing if visual studio doesn recognize the change!


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#may give problems on linux
if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()



if(MSVC)
    # Add CPU optimization options for MSVC compiler
    if(USE_AVX512)
        message(STATUS "Building with AVX512 support - this requires compatible CPU hardware")
        add_compile_options(/arch:AVX512)
    else()
        message(STATUS "Building with AVX2 support for better compatibility")
        add_compile_options(/arch:AVX2)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Add CPU optimization options for GCC/Clang compilers
    if(USE_AVX512)
        message(STATUS "Building with AVX512 support - this requires compatible CPU hardware")
        set(AVX_FLAGS -mavx512f -mavx512vl -mavx512bw -mavx512dq)
    else()
        message(STATUS "Building with AVX2 support for better compatibility")
        set(AVX_FLAGS -mavx2 -mfma -mavx -msse4.2)
    endif()
endif()


project(gameengine)

# Only building with SDL3, removed GLFW settings
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Add the path to the thirdparty libraries
add_subdirectory(thirdparty/sdl3-3.2.10)		# SDL3

# Try to find Vulkan with our explicit paths first
find_package(Vulkan REQUIRED)

# If find_package failed, set up Vulkan manually
if(NOT Vulkan_FOUND)
    message(STATUS "Manual Vulkan configuration...")
    set(Vulkan_FOUND TRUE)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib")
endif()

message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)


if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="/resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=0) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=1) 

endif()


target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

endif()

# Handle Unicode flags properly based on compiler
if (MSVC)
    # Windows-style flags for MSVC
    add_compile_options(/UUNICODE /U_UNICODE)
else()
    # Unix-style flags for GCC/Clang
    add_compile_options(-DUNICODE=0 -D_UNICODE=0)
endif()

# Replace the WIN32 specific section with more specific MSVC check
if (MSVC)
    target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE /UUNICODE /U_UNICODE)
endif()

# Include directories for the project
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/headers/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${Vulkan_INCLUDE_DIRS}")

# Link with SDL3 and Vulkan
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE SDL3::SDL3 ${Vulkan_LIBRARIES})

# Apply the AVX flags directly to the target
target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE ${AVX_FLAGS})
