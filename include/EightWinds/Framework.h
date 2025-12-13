#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include "EightWinds/ShaderFactory.h"
#include "EightWinds/Pipeline/PipelineSystem.h"
#include "EightWinds/Backend/Descriptor/LayoutCache.h"
//#include "EightWinds/Backend/ResourceHandler.h"


namespace EWE{
    //needs a better name

    //i think i'll treat this as the hub for the entire framework.
    //might even force everything to pass through here
    struct Framework{

        LogicalDevice& logicalDevice;

        [[nodiscard]] explicit Framework(LogicalDevice& logicalDevice) noexcept;
        //i dont really want to pierce this to get to the logicaldevice or whatever
        //i might even construct logicaldevice here
        //i think i'd want every 'factory' class here
        //i can do both descriptor and cmdbuf 'factory', which would handle pools underneath it

        ShaderFactory shaderFactory;
        PipelineSystem pipelineSystem;
        //handles command buffer pools underneath it
        //CommandBufferFactory cbFac;
        Backend::Descriptor::LayoutCache dslCache;

        bool graphicsLibraryEnabled = false;
        bool meshShadersEnabled = false;
        bool deviceFaultEnabled = false;

#if EWE_USING_EXCEPTIONS

        //it will be re-thrown
        void HandleVulkanException(EWEException& renderExcept);
#endif
    };

}