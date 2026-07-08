if (DEFINED VULKAN_SDK_PATH AND NOT "${VULKAN_SDK_PATH}" STREQUAL "")
  message(STATUS "Using explicitly defined Vulkan SDK path: ${VULKAN_SDK_PATH}")
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/include")
  set(Vulkan_FOUND TRUE)
elseif (WIN32)
  if (DEFINED ENV{VULKAN_SDK} AND NOT "$ENV{VULKAN_SDK}" STREQUAL "")
    message(STATUS "Automatically detected Windows Vulkan SDK at: $ENV{VULKAN_SDK}")
    set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
    set(Vulkan_FOUND TRUE)
  else()
    message(STATUS "VULKAN_SDK env var not found. Scanning C:/VulkanSDK/")
    find_path(Vulkan_INCLUDE_DIR
      NAMES vulkan/vulkan.h
      PATHS "C:/VulkanSDK/*"
    )
    if (Vulkan_INCLUDE_DIR)
      set(Vulkan_FOUND TRUE)
    endif()
  endif()
#linux
else()
  message(STATUS "Searching for linux Vulkan headers")
  find_path(Vulkan_INCLUDE_DIR
    NAMES vulkan/vulkan.h
    PATHS
    /usr/include
    /usr/local/include
    /opt/vulkan/*
    NO_CMAKE_FIND_ROOT_PATH
  )
  if (Vulkan_INCLUDE_DIR)
    set(Vulkan_FOUND TRUE)
  endif()
endif()

if (NOT Vulkan_FOUND)
  message(STATUS "No Vulkan SDK found on this system. Fetching Vulkan-Headers")
  FetchContent_Declare(
    VulkanHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.4.350
  )
  FetchContent_MakeAvailable(VulkanHeaders)
  set(Vulkan_INCLUDE_DIR "${vulkanheaders_SOURCE_DIR}/include")
  set(Vulkan_FOUND TRUE)
  message(STATUS "Vulkan headers fetched : ${Vulkan_INCLUDE_DIR}")
else()
  message(STATUS "Vulkan headers at : ${Vulkan_INCLUDE_DIR}")
endif()

# volk
FetchContent_Declare(
      volk
GIT_REPOSITORY https://github.com/zeux/volk.git
GIT_TAG vulkan-sdk-1.4.350
  )
set(VOLK_PULL_IN_VULKAN OFF CACHE BOOL "" FORCE) #headers
FetchContent_MakeAvailable(volk)
target_include_directories(volk PUBLIC ${Vulkan_INCLUDE_DIR})