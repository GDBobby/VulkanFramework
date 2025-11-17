#include "EightWinds/BackendEncapsulator.h"

namespace EWE{
    BackendEncapsulator::BackendEncapsulator(LogicalDevice& logicalDevice) noexcept
    : logicalDevice{logicalDevice},
    shaderFactory{logicalDevice},
    pipelineSystem{pipelineSystem}
    {
        
    }
}