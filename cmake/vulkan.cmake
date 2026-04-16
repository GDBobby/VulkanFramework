# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  message(STATUS "vulkan sdk path - ${VULKAN_SDK_PATH}")
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
  set(Vulkan_LIB_DIR "${VULKAN_SDK_PATH}/lib")
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