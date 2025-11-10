#include "EightWinds/Window.h"

namespace EWE {

	void ResizeCallbackWrapper(GLFWwindow* windowPtr, int width, int height) {
		Window* userPointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr));

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor()); //potentially not the primary monitor
		if (width > mode->width) {
			userPointer->screenDimensions.width = mode->width;
		}
		if (height > mode->height) {
			userPointer->screenDimensions.height = mode->height;
		}
		userPointer->screenDimensions.width = width;
		userPointer->screenDimensions.height = height;

		if(userPointer->ResizeCallback != nullptr){
			userPointer->ResizeCallback(windowPtr, width, height);
		}
	}

	Window::Window(Instance& instance, uint32_t width, uint32_t height, std::string_view name) : instance{instance}, screenDimensions{width, height} {

		glfwInit();
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (screenDimensions.width > mode->width) {
			screenDimensions.width = mode->width;
		}
		if (screenDimensions.height > mode->height) {
			screenDimensions.height = mode->height;
		}

		if ((screenDimensions.width == 0) || (screenDimensions.height == 0)) {
			if (mode != nullptr) {
				screenDimensions.width = mode->width;
				screenDimensions.height = mode->height;
			}
		}
		else {
			printf("failed to find primary monitor \n");
			screenDimensions.width = 1280;
			screenDimensions.height = 720;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//if (SettingsJSON::settingsData.windowMode == SettingsInfo::WT_windowed) {
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		//}
		//else {
		//	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		//}
		window = glfwCreateWindow(screenDimensions.width, screenDimensions.height, name.data(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, ResizeCallbackWrapper);


		EWE_VK(glfwCreateWindowSurface, instance, window, nullptr, &surface);
	}
}