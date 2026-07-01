# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  message(STATUS "vulkan sdk path - ${VULKAN_SDK_PATH}")
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
  set(Vulkan_FOUND TRUE)
else()
  message(STATUS "Vulkan path undefined in .env")
  find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    PATHS /usr/include /usr/local/include
    NO_CMAKE_FIND_ROOT_PATH
  )
  if (Vulkan_INCLUDE_DIR)
    set(Vulkan_FOUND TRUE)
  else()
    set(Vulkan_FOUND FALSE)
  endif()
  #message(STATUS "Found Vulkan package:  ${Vulkan_INCLUDE_DIRS}") #this line isnt working
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan headers at: ${Vulkan_INCLUDE_DIR}")
endif()



# volk
if (USING_VOLK)
  FetchContent_Declare(
      volk
      GIT_REPOSITORY https://github.com/zeux/volk.git
      GIT_TAG vulkan-sdk-1.4.350
  )
  set(VOLK_PULL_IN_VULKAN OFF CACHE BOOL "" FORCE) #headers
  FetchContent_MakeAvailable(volk)

  target_include_directories(volk PUBLIC ${Vulkan_INCLUDE_DIR})
endif()