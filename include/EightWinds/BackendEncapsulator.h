#pragma once

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/ShaderFactory.h"
#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/Pipeline/PipelineSystem.h"

namespace EWE{
    //needs a better name

    struct BackendEncapsulator{
        //i dont really want to pierce this to get to the logicaldevice or whatever
        //i might even construct logicaldevice here
        //i think i'd want every 'factory' class here
        //i can do both descriptor and cmdbuf 'factory', which would handle pools underneath it

        [[nodiscard]] explicit BackendEncapsulator(LogicalDevice& logicalDevice) noexcept;

        LogicalDevice& logicalDevice;
        ShaderFactory shaderFactory;
        PipelineSystem pipelineSystem;
        //CommandBufferFactory cbFac;//handles command buffer pools underneath it
        //DescriptorFactory descFact;//
    };

}