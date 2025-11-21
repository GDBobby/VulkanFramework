#include "EightWinds/Framework.h"

namespace EWE{
    
    Framework::Framework(LogicalDevice& logicalDevice) noexcept
    : logicalDevice{logicalDevice},
        shaderFactory{logicalDevice},
        dslCache{logicalDevice.device}
    {

    }
}