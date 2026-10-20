#include "EightWinds/Window.h"

namespace EWE {

    void ResizeCallbackWrapper(GLFWwindow* windowPtr, int width, int height) {
        Window* userPointer = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr));

        int nextWidth = width;
        int nextHeight = height;
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (mode != nullptr) {
            if (nextWidth > mode->width) {
                nextWidth = mode->width;
            }
            if (nextHeight > mode->height) {
                nextHeight = mode->height;
            }
        }
        if (nextWidth < 0) {
            nextWidth = 0;
        }
        if (nextHeight < 0) {
            nextHeight = 0;
        }
        userPointer->screenDimensions.width = static_cast<uint32_t>(nextWidth);
        userPointer->screenDimensions.height = static_cast<uint32_t>(nextHeight);

        if (userPointer->ResizeCallback != nullptr) {
            userPointer->ResizeCallback(windowPtr, nextWidth, nextHeight);
        }
    }

    Window::Window(Instance& _instance, uint32_t width, uint32_t height, std::string_view name, bool decorated)
        : instance{_instance},
          screenDimensions{width, height} {
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
            else {
                Log::Error("failed to find primary monitor \n");
                screenDimensions.width = 1280;
                screenDimensions.height = 720;
            }
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
        window = glfwCreateWindow(screenDimensions.width, screenDimensions.height, name.data(), nullptr, nullptr);
        if (!decorated) {
            glfwSetWindowPos(window, 0, 0);
        }
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, ResizeCallbackWrapper);


        EWE_VK(glfwCreateWindowSurface, instance, window, nullptr, &surface);
    }

    void Window::Resize(uint32_t width, uint32_t height, bool decorated) {
        glfwSetWindowAttrib(window, GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
        if (width > 0 && height > 0) {
            glfwSetWindowSize(window, static_cast<int>(width), static_cast<int>(height));
        }
        if (!decorated) {
            glfwSetWindowPos(window, 0, 0);
        }
        screenDimensions.width = width;
        screenDimensions.height = height;
    }

    void Window::SetResizeable(bool _resizeable) {
        this->resizeable = _resizeable;
        glfwSetWindowAttrib(window, GLFW_RESIZABLE, resizeable);
    }
}