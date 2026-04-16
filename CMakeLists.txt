cmake_minimum_required(VERSION 3.22)

project(EWFramework)

include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

message(STATUS "Project source dir : ${PROJECT_SOURCE_DIR}")
message(STATUS "CMake current list dir : ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "~~~ CURRENT SOURCE DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "project name : ${PROJECT_NAME}")
message(STATUS "build location : ${CMAKE_CURRENT_BINARY_DIR}")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are disabled. Please create a build folder.")
endif()

set(CMAKE_CXX_STANDARD 26)
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

#message(STATUS "Found sources: ${SOURCES}")

include(FetchContent)
if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()

add_subdirectory(external/spirvcross)
add_subdirectory(external/vma)
include(cmake/glfw.cmake)
include(cmake/vulkan.cmake)

option(ENABLE_AFTERMATH "Enable NVIDIA NSight aftermath" OFF)
if (ENABLE_AFTERMATH)
include(cmake/aftermath.cmake)
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADER_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE
	$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall 
        -Wextra 
        -Wshadow -Werror=shadow
        -freflection
        -Wreorder -Werror=reorder
        -Wreturn-type -Werror=return-type
        -Wno-missing-field-initializers
        -Werror=uninitialized
        -Werror=delete-non-virtual-dtor
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 
        /we4456 /we4457 /we4458 /we4459
    >
)

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

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	
	${Vulkan_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include/
	${CMAKE_CURRENT_SOURCE_DIR}/external/include
	${CMAKE_CURRENT_SOURCE_DIR}/external
)

target_link_directories(${PROJECT_NAME} PUBLIC
	${Vulkan_LIB_DIR}
	$<$<CONFIG:Debug>:${AFTERMATH_LIB_DIR}>
)

message(STATUS "vulkan sdk path : ${VULKAN_SDK_PATH}")

target_precompile_headers(${PROJECT_NAME} PUBLIC 
    <utility>
    <vector>
    <meta>
    <array>
    <cstdint>
    <ranges>
    <vulkan/vulkan.h>
    <filesystem>
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
# target_precompile_headers(${PROJECT_NAME} PRIVATE include/EightWinds/stdafx.h)

