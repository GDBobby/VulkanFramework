#pragma once

#ifdef USING_NVIDIA_AFTERMATH

#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>
#include <GFSDK_Aftermath_GpuCrashDumpEditing.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdint>

namespace EWE{
	inline void GpuCrashDumpCallback(
		const void* pGpuCrashDump,
		uint32_t size,
		void* pUserData)
	{
		// Write GPU crash dump to disk
		std::ofstream file("gpu_crash_dump.nv-gpudmp", std::ios::binary);
		file.write(reinterpret_cast<const char*>(pGpuCrashDump), size);
	}

	inline void ShaderDebugInfoCallback(
		const void* pShaderDebugInfo,
		uint32_t size,
		void* pUserData
	) 
	{}

	inline void CrashDumpDescriptionCallback(
		PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addValue, 
		void* pUserData
	) {}

	inline void ResolveMarkerCallback(
		const void* pMarkerData,
		const uint32_t markerDataSize,
		void* pUserData,
		PFN_GFSDK_Aftermath_ResolveMarker resolveMarker
	) {}

	inline void InitializeAftermath() {
		GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
			GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks,
			GpuCrashDumpCallback,
			ShaderDebugInfoCallback,
			CrashDumpDescriptionCallback,
			ResolveMarkerCallback,
			nullptr
		);
	}
}
#endif