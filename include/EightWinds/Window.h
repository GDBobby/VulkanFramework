#pragma once
#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Instance.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <functional>

template<typename T>
struct Dimensions {
	T width;
	T height;
};

namespace EWE {
	struct Window {
		[[nodiscard]] Window(Instance& instance, uint32_t width, uint32_t height, std::string_view name);
		
		Dimensions<uint32_t> screenDimensions;
		GLFWwindow* window;
		VkSurfaceKHR surface;
		Instance& instance;

		std::function<void(GLFWwindow* windowPtr, int width, int height)> ResizeCallback = nullptr;
	};
}