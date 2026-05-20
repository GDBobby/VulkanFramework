#include "EightWinds/Command/ParamPointerChain.h"

#include "EightWinds/VulkanHash.h"

namespace EWE{
    bool ParamPointerChain::operator==(ParamPointerChain const& other) const{
        if(base != other.base){
            return false;
        }
        if(package_iter != other.package_iter){
            return false;
        }
        if(pointer_into.size() != other.pointer_into.size()){
            return false;
        }
        for(std::size_t i = 0; i < pointer_into.size(); i++){
            if(pointer_into[i] != other.pointer_into[i]){
                return false;
            }
        }
        return true;
    }
} //namespace EWE