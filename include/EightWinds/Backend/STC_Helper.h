#pragma once

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Image.h"
#include "EightWinds/Buffer.h"

namespace EWE{
	namespace Command_Helper{
		void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait);
	}
}