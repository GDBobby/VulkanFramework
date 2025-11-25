#include "EightWinds/Backend/DSLCache.h"

namespace EWE{
    namespace Backend{
        LayoutCache::LayoutCache(VkDevice device) noexcept
        : device{device}
        {

        }

        LayoutCache::~LayoutCache(){
            for(auto& [binding, dsl] : cache){
                vkDestroyDescriptorSetLayout(device, dsl, nullptr);
            }
            cache.clear();
        }



        VkDescriptorSetLayout LayoutCache::Get(Descriptor::Bindings const& bindings) noexcept{
            auto it = cache.find(bindings);
            if (it != cache.end()) {
                return it->second;
            }

            VkDescriptorSetLayoutCreateInfo dslCreateInfo{};
            dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            dslCreateInfo.bindingCount = static_cast<uint32_t>(bindings.vkBindings.size());
            dslCreateInfo.pBindings = bindings.vkBindings.data();

            VkDescriptorSetLayout layout = VK_NULL_HANDLE;
            EWE_VK(vkCreateDescriptorSetLayout, device, &dslCreateInfo, nullptr, &layout);

            cache.emplace(bindings, layout);
            return layout;
        }
        void LayoutCache::Free(VkDescriptorSetLayout) noexcept{

        }
    } //namespace Backend
} //namespace EWE