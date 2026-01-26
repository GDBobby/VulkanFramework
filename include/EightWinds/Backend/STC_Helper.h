#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/Queue.h"

namespace EWE{
	namespace Command_Helper{
		void Transition(LogicalDevice& logicalDevice, Queue& queue, VkDependencyInfo const& dependencyInfo, bool wait);
	}
}