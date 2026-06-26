#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Reflect/Enum.h"

#include <thread>
#if EWE_CALL_STACK_DEBUG
#include <iostream> //specifically so i dont have to manually print stacktrace
#endif

#ifdef _WIN32
#defien WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

namespace EWE{
    //i think im going to change this to print out some history of the render graph
    //that would mean castign a type erased pointer (void*), along with a enum val (VK_OBJECT_TYPE similar)
    //then including basically EVERYTHING in a cpp file
    //then interpreting the type, and printing a debug log

    void EWE_VK_RESULT(VkResult vkResult) {
        if (vkResult != VK_SUCCESS) {
#if EWE_DEBUG_BOOL
            if (vkResult != VK_ERROR_DEVICE_LOST) {
                //going to use the device lost extension here
                EWE_ASSERT(vkResult == VK_SUCCESS, Reflect::Enum::ToString(vkResult));

            //the callstack isnt relevant for device lost
#endif
#if EWE_CALL_STACK_DEBUG
                std::stacktrace error_trace = std::stacktrace::current(2);
                std::cout << "vk result : " << vkResult << std::endl << error_trace << std::endl << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
#endif
#if EWE_DEBUG_BOOL
            }
#endif
#if EWE_USING_EXCEPTIONS
            throw EWEException(vkResult);
#endif
        }
    }

    void NameCurrentThread(std::string_view name){
#ifdef _WIN32
        std::wstring wname(name.begin(), name.end());
        SetThreadDescription(GetCurrentThread(), wname.c_str());

#elif defined(__linux__)
        //linux has a 16 char limit for thread name?
        char buf[16];
        auto len = std::min(name.size(), (size_t)15);
        std::copy_n(name.begin(), len, buf);
        buf[len] = '\0';
        pthread_setname_np(pthread_self(), buf);
#endif
    }
} //namespace EWE