cmake_minimum_required(VERSION 3.22)

set(CMAKE_CONFIGURATION_TYPES "Debug;DebugASan;Release;RelWithDebInfo;RelWithDebInfoASan" CACHE STRING "" FORCE)


project(EWGraphics)

include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

message(STATUS "Project source dir : ${PROJECT_SOURCE_DIR}")
message(STATUS "CMake current list dir : ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "~~~ CURRENT SOURCE DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "project name : ${PROJECT_NAME}")
message(STATUS "build location : ${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(is_project_root ON)
else()
	set(is_project_root OFF)
endif()
message(STATUS "is ${PROJECT_NAME} project root? ${is_project_root}")

Message(STATUS "Cmake Generator : ${CMAKE_GENERATOR}")

set(OUTPUT_BASE_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_BASE_DIR}/${CMAKE_BUILD_TYPE}")

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)

message(STATUS "Found sources: ${SOURCES}")

include(FetchContent)
if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()

add_subdirectory(external/spirvcross)
add_subdirectory(external/vma)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
set(GLFW_VULKAN_STATIC ON CACHE BOOL "")

add_subdirectory(external/glfw)

option(ENABLE_AFTERMATH "Enable NVIDIA NSight aftermath" ON)
if (DEFINED NVIDIA_AFTERMATH_PATH AND ENABLE_AFTERMATH)
    message(STATUS "Including NVIDIA Nsight Aftermath - ${NVIDIA_AFTERMATH_PATH}")

    add_library(nvidia_aftermath INTERFACE)

    target_include_directories(nvidia_aftermath INTERFACE
        "${NVIDIA_AFTERMATH_PATH}/include"
    )

    target_link_libraries(nvidia_aftermath INTERFACE
        "${NVIDIA_AFTERMATH_PATH}/lib/x64/GFSDK_Aftermath_Lib.x64.lib"
    )

    target_compile_definitions(nvidia_aftermath INTERFACE
        USING_NVIDIA_AFTERMATH=1
    )

    # Set the global big bool
    set(GLOBAL_USING_AFTERMATH ON CACHE INTERNAL "Whether Aftermath is actually enabled")
else()
    if(ENABLE_AFTERMATH)
        message(WARNING "Attempted to enable Nsight Aftermath but failed to find NVIDIA_AFTERMATH_PATH. Check .env.cmake")
    endif()
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADER_FILES})



option(USING_ASAN "address sanitizer" OFF)
if(USING_ASAN)
	if(MSVC)
        #target_compile_options(${PROJECT_NAME} PUBLIC /fsanitize=address)
        #target_link_options(${PROJECT_NAME} PUBLIC /fsanitize=address)
	else()
        #target_compile_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
        #target_link_options(${PROJECT_NAME} PUBLIC -fsanitize=address)
    endif()
endif()

# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  message(STATUS "vulkan sdk path - ${VULKAN_SDK_PATH}")
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIB_DIR "${VULKAN_SDK_PATH}/lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
  if(UNIX)
  	set(Vulkan_LIBRARY libvulkan.so)
  else()
	set(Vulkan_LIBRARY vulkan-1)
  endif()#include "EightWinds/VulkanHeader.h"
else()
  message(STATUS "Vulkan path undefined in .env")
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan package:  ${Vulkan_INCLUDE_DIRS}") #this line isnt working
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARY}")
endif()

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	
	${Vulkan_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/external/GLFW/include/
	${CMAKE_CURRENT_SOURCE_DIR}/external/include
	${CMAKE_CURRENT_SOURCE_DIR}/external
	${LinearAlgebra_SOURCE_DIR}
)

target_link_directories(${PROJECT_NAME} PUBLIC
	${Vulkan_LIB_DIR}
	$<$<CONFIG:Debug>:${AFTERMATH_LIB_DIR}>
	$<$<CONFIG:Debug>:${LinearAlgebra_SOURCE_DIR}/Debug>
	$<$<CONFIG:Release>:${LinearAlgebra_SOURCE_DIR}/Release>
)

message(STATUS "linking libs")


target_link_libraries(${PROJECT_NAME} PUBLIC
	spirv-cross-core
	spirv-cross-reflect
	glfw
	${Vulkan_LIBRARY}
)
if(GLOBAL_USING_AFTERMATH)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${NVIDIA_AFTERMATH_PATH}/include"
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC nvidia_aftermath)
endif()

# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADER_FILES})
# target_precompile_headers(${PROJECT_NAME} PRIVATE include/EWGraphics/stdafx.h)

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
    #include <stacktrace>
    int main() {
        auto st = std::stacktrace::current();
        return st.size();
    }
" HAVE_STD_STACKTRACE)

if (HAVE_STD_STACKTRACE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_STD_STACKTRACE)
endif()


