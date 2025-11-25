#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/LogicalDevice.h"

#include "EightWinds/ShaderFactory.h"
#include "EightWinds/Pipeline/PipelineSystem.h"
#include "EightWinds/Backend/Descriptors/LayoutCache.h"
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
        //CommandBufferFactory cbFac;//handles command buffer pools underneath it
        Backend::Descriptor::LayoutCache dslCache;
        //DescriptorFactory descFact;//this fact will need a ref to dslcache
        
        //this is going to be a copy. i need it internally for buffer alignment.
        VkPhysicalDeviceProperties properties; 

        //i could also use a few extensions internally
        //im considering using hooks (x86 assembly, code injection type hooks) to inline the branches for these extensions
        bool graphicsLibraryEnabled = false;
        bool meshShadersEnabled = false;
    };

}