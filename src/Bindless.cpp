#include "EightWinds/Backend/Descriptor/Bindless.h"

#include "EightWinds/LogicalDevice.h"

namespace EWE{
    namespace Backend {

        void BindlessDescriptor::CreateLayout(){
            bindings = std::array<VkDescriptorSetLayoutBinding, static_cast<size_t>(DescriptorType::COUNT)>{
                VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = UINT16_MAX,
                    .stageFlags = VK_SHADER_STAGE_ALL
                },
                VkDescriptorSetLayoutBinding{
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = UINT16_MAX,
                    .stageFlags = VK_SHADER_STAGE_ALL
                },
                VkDescriptorSetLayoutBinding{
                    .binding = 2,
                    .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = UINT16_MAX,
                    .stageFlags = VK_SHADER_STAGE_ALL
                },
                VkDescriptorSetLayoutBinding{
                    .binding = 3,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    .descriptorCount = UINT16_MAX,
                    .stageFlags = VK_SHADER_STAGE_ALL
                }
            };
            std::array<VkDescriptorBindingFlags, static_cast<size_t>(DescriptorType::COUNT)> bindingFlags;
            bindingFlags.fill(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            bindingFlags.back() |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

            VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
                .pNext = nullptr,
                .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
                .pBindingFlags = bindingFlags.data()
            }; 
            VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &flagsInfo,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data()
            };
            EWE_VK(vkCreateDescriptorSetLayout, logicalDevice, &layoutInfo, nullptr, &layout);
        }

        void BindlessDescriptor::CreatePool(){

            std::array<VkDescriptorPoolSize, static_cast<uint64_t>(DescriptorType::COUNT)> poolSizes = {
                VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .descriptorCount = max_images_per_type
                },
                VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = max_images_per_type
                },
                VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .descriptorCount = max_images_per_type
                },
                VkDescriptorPoolSize{
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                    .descriptorCount = max_images_per_type
                }
            };

            VkDescriptorPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .maxSets = 1,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data()
            };

            EWE_VK(vkCreateDescriptorPool, logicalDevice, &poolInfo, nullptr, &pool);
        }
        void BindlessDescriptor::CreateDescriptor(){
            const uint32_t variableCount = max_images_per_type;

            VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
                .descriptorSetCount = 1,
                .pDescriptorCounts = &variableCount
            };

            VkDescriptorSetAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = &countInfo,
                .descriptorPool = pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout
            };

            VkDescriptorSet bindlessSet;
            EWE_VK(vkAllocateDescriptorSets, logicalDevice, &allocInfo, &set);
        }

        BindlessDescriptor::BindlessDescriptor(LogicalDevice& logicalDevice)
            : logicalDevice{logicalDevice}
        {
            CreateLayout();
            CreatePool();
            CreateDescriptor();
        }
        BindlessDescriptor::~BindlessDescriptor() {
            vkDestroyDescriptorSetLayout(logicalDevice, layout, nullptr);
            vkDestroyDescriptorPool(logicalDevice, pool, nullptr);
        }

        
        DescriptorIndex BindlessDescriptor::BindImage(VkDescriptorImageInfo const& imageInfo, DescriptorType type){
            
            const uint32_t occupancy_index = static_cast<uint32_t>(type);
            auto& bits = occupancy[occupancy_index];

            for(uint32_t i = 0; i < max_images_per_type; i++){
                if(!bits.test(i)){
                    bits.set(i);

                    VkWriteDescriptorSet write{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set,
                        .dstBinding = occupancy_index,
                        .dstArrayElement = i,
                        .descriptorCount = 1,
                        .descriptorType = VkExpandDescriptorType(type),
                        .pImageInfo = &imageInfo
                    };

                    vkUpdateDescriptorSets(logicalDevice, 1, &write, 0, nullptr);
                    return i;
                }
            }
            EWE_UNREACHABLE;
            return UINT32_MAX; //error silencer
        }

        void BindlessDescriptor::Unbind(DescriptorIndex index, DescriptorType type){
            
            const uint32_t occupancy_index = static_cast<uint32_t>(type);
            auto& bits = occupancy[occupancy_index];

#if EWE_DEBUG_BOOL
            assert(bits.test(index));
#endif
            bits.reset(index);

            /*
            * i dont think this is necessary. POTENTIALLY, use null descriptor feature (idk where it is)
            * validation error: 2 : vkUpdateDescriptorSets(): pDescriptorWrites[0].pImageInfo[0].imageView is VK_NULL_HANDLE.
                The Vulkan spec states: If descriptorType is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, or VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, and the nullDescriptor feature is not enabled, the imageView member of each element of pImageInfo must not be VK_NULL_HANDLE (https://vulkan.lunarg.com/doc/view/1.4.328.1/windows/antora/spec/latest/chapters/descriptorsets.html#VUID-VkWriteDescriptorSet-descriptorType-02997)
            * 
            VkDescriptorImageInfo nullInfo{
                .sampler = VK_NULL_HANDLE,
                .imageView = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };

            VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = set,
                .dstBinding = occupancy_index,
                .dstArrayElement = index,
                .descriptorCount = 1,
                .descriptorType = VkExpandDescriptorType(type),
                .pImageInfo = &nullInfo
            };

            vkUpdateDescriptorSets(logicalDevice, 1, &write, 0, nullptr);
            */
        }
    }//namespace Backend
} //namespace EWE