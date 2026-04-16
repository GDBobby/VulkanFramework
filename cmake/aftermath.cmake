
if(ENABLE_AFTERMATH)
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

    message(WARNING "Attempted to enable Nsight Aftermath but failed to find NVIDIA_AFTERMATH_PATH. Check .env.cmake")
endif()