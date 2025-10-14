
namespace EWE{
    #if CALL_TRACING
    void EWE_VK_RESULT(VkResult vkResult) {
    #if DEBUGGING_DEVICE_LOST                                                                                        
        if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
        else
    #endif
        if (vkResult != VK_SUCCESS) {
            printf("need to set up stack tracing here\n");
            /*
            printf("VK_ERROR : %s(%d) : %s - %d \n", sourceLocation.file_name(), sourceLocation.line(), sourceLocation.function_name(), vkResult);
            std::ofstream logFile{};
            logFile.open(GPU_LOG_FILE, std::ios::app);
            assert(logFile.is_open() && "Failed to open log file");
            logFile << "VK_ERROR : " << sourceLocation.file_name() << '(' << sourceLocation.line() << ") : " << sourceLocation.function_name() << " : VkResult(" << vkResult << ")\n";
            logFile << "current frame index - " << EWE::VK::Object->frameIndex << std::endl;
            */
    #if COMMAND_BUFFER_TRACING
            for (uint8_t i = 0; i < EWE::VK::Object->renderCommands.size(); i++) {
                while (EWE::VK::Object->renderCommands[i].usageTracking.size() > 0) {
                    for (auto& usage : EWE::VK::Object->renderCommands[i].usageTracking.front()) {
                        logFile << "cb(" << +i  << ")(" << EWE::VK::Object->renderCommands[i].usageTracking.size() << ") : " << usage.funcName << '\n';
                    }
                    EWE::VK::Object->renderCommands[i].usageTracking.pop();
                }
            }
    #endif
            //logFile.close();
            assert(vkResult == VK_SUCCESS && "VK_ERROR");
        }
    }
    #else
    void EWE_VK_RESULT(VkResult vkResult) {
    #if DEBUGGING_DEVICE_LOST                                                                                        
        if (vkResult == VK_ERROR_DEVICE_LOST) { EWE::VKDEBUG::OnDeviceLost(); }
    #endif
    }
    #endif
}