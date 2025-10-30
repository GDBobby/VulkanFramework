#pragma once

#include "EightWinds/VulkanHeader.h"

namespace EWE{
    namespace DebugNaming {

		void SetObjectName(void* object, VkObjectType objectType, const char* name);
#define SetObjectNameRC(object, objectType, name) SetObjectName(reinterpret_cast<void*>(object), objectType, name)
	} //namespace DebugNaming
}