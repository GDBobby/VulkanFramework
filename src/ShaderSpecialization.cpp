#include "EightWinds/Backend/ShaderSpecialization.h"


namespace EWE{
	/*
    VkSpecInfo_RAII::VkSpecInfo_RAII(RuntimeArray<SpecializationEntry> const& entries) 
		: mapEntries(entries.size())
	{
		specInfo.dataSize = 0;
		for (uint8_t i = 0; i < mapEntries.size(); i++) {
			VkSpecializationMapEntry& mapEntry = mapEntries[i];
			mapEntry.constantID = entries[i].constantID;
			mapEntry.offset = specInfo.dataSize;
			mapEntry.size = entries[i].elementCount * sizeof(float);
			specInfo.dataSize += mapEntry.size;
		}

		specInfo.mapEntryCount = static_cast<uint32_t>(mapEntries.size());
		specInfo.pMapEntries = mapEntries.data();
		try {
			memPtr = reinterpret_cast<uint64_t>(malloc(specInfo.dataSize));
		}
		catch (const std::runtime_error& e) {
#if EWE_DEBUG_BOOL
			Log::Error("malloc error - %s\n", e.what());
#endif
			specInfo.mapEntryCount = 0;
			specInfo.pData = nullptr;
		}
		EWE_ASSERT(memPtr != 0);

		std::size_t offset = 0;
		for (uint8_t i = 0; i < mapEntries.size(); i++) {
			const std::size_t localSize = sizeof(float) * entries[i].elementCount;
			memcpy(reinterpret_cast<void*>(memPtr + offset), entries[i].value, localSize);
			offset += localSize;
		}
		specInfo.pData = reinterpret_cast<void*>(memPtr);

	}

	VkSpecInfo_RAII::VkSpecInfo_RAII(VkSpecInfo_RAII&& move) noexcept
		:  mapEntries{ std::move(move.mapEntries) },
		specInfo{ move.specInfo },
		memPtr{ move.memPtr }
	{
		move.memPtr = 0;
	}

	VkSpecInfo_RAII::VkSpecInfo_RAII(VkSpecInfo_RAII const& copy)
		: mapEntries{copy.mapEntries.begin(), copy.mapEntries.end() }, //this is a copy
		specInfo{ copy.specInfo }
		{
		memPtr = reinterpret_cast<uint64_t>(malloc(specInfo.dataSize));
		specInfo.mapEntryCount = static_cast<uint32_t>(mapEntries.size());
		specInfo.pMapEntries = mapEntries.data();
		specInfo.pData = reinterpret_cast<void*>(memPtr);
		if (memPtr == 0) {
			throw std::runtime_error("failed to allocate");
		}
		memcpy(reinterpret_cast<void*>(memPtr), copy.specInfo.pData, specInfo.dataSize);
	}

	VkSpecInfo_RAII::~VkSpecInfo_RAII() {
		if (memPtr != 0) {
			free(reinterpret_cast<void*>(memPtr));
		}
	}
	*/
}