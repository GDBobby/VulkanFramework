#include "EightWinds/Backend/Descriptor/Bindless.h"

#include "EightWinds/LogicalDevice.h"
#include "EightWinds/DescriptorImageInfo.h"
#include "EightWinds/ImageView.h"
#include "EightWinds/Sampler.h"

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
        bindingFlags.fill(
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 
            | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
            | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
        );
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
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
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
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
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

        //VkDescriptorSet bindlessSet;
        EWE_VK(vkAllocateDescriptorSets, logicalDevice, &allocInfo, &set);
    }

    BindlessDescriptor::BindlessDescriptor(LogicalDevice& _logicalDevice)
        : logicalDevice{_logicalDevice}
    {
        CreateLayout();
        CreatePool();
        CreateDescriptor();
    }
    BindlessDescriptor::~BindlessDescriptor() {
        vkDestroyDescriptorSetLayout(logicalDevice, layout, nullptr);
        vkDestroyDescriptorPool(logicalDevice, pool, nullptr);
    }

    
    TextureIndex BindlessDescriptor::BindImage(DescriptorImageInfo& dii){
        
        const uint32_t occupancy_index = static_cast<uint32_t>(dii.type);
        auto& bits = occupancy[occupancy_index];

        std::unique_lock<std::mutex> binding_lock{binding_mutex};
        for(uint32_t i = 0; i < max_images_per_type; i++){
            if(!bits.test(i)){
                bits.set(i);

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = occupancy_index,
                    .dstArrayElement = i,
                    .descriptorCount = 1,
                    .descriptorType = VkExpandDescriptorType(dii.type),
                    .pImageInfo = &dii.imageInfo
                };

                vkUpdateDescriptorSets(logicalDevice, 1, &write, 0, nullptr);
#if EWE_DEBUG_BOOL
                if (tracker.find(&dii) != tracker.end()) {
                    Log::Error("tracker already contains dii\n");
                }
                for (auto& kvp : tracker) {
                    if (kvp.key->view.view == dii.view.view) {
                        Log::Warning("view equal\n");
                    }
                }
                tracker.push_back(&dii, i);
#endif
                return i;
            }
        }
        EWE_UNREACHABLE;
        return UINT32_MAX; //error silencer
    }

    void BindlessDescriptor::Unbind(DescriptorImageInfo& dii){

        std::unique_lock<std::mutex> binding_lock{binding_mutex};

        const uint32_t occupancy_index = static_cast<uint32_t>(dii.type);
        auto& bits = occupancy[occupancy_index];
        EWE_ASSERT(bits.test(dii.index));
        bits.reset(dii.index);

        tracker.Remove(&dii);

        return;
    }

    inline bool operator==(VkDescriptorImageInfo const lhs, VkDescriptorImageInfo const rhs) {
        return lhs.sampler  == rhs.sampler &&
            lhs.imageView   == rhs.imageView &&
            lhs.imageLayout == rhs.imageLayout;
    }


    TextureIndex BindlessDescriptor::BindImage(VkDescriptorImageInfo raw_dii){
        
        const uint32_t occupancy_index = static_cast<uint32_t>(DescriptorType::Combined);
        auto& bits = occupancy[occupancy_index];

        std::unique_lock binding_lock{binding_mutex};
        for(uint32_t i = 0; i < max_images_per_type; i++){
            if(!bits.test(i)){
                bits.set(i);

                VkWriteDescriptorSet write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = set,
                    .dstBinding = occupancy_index,
                    .dstArrayElement = i,
                    .descriptorCount = 1,
                    .descriptorType = VkExpandDescriptorType(DescriptorType::Combined),
                    .pImageInfo = &raw_dii
                };

                vkUpdateDescriptorSets(logicalDevice, 1, &write, 0, nullptr);
#if EWE_DEBUG_BOOL
                for(auto& kvp : tracker_raw){
                    if(kvp.key == raw_dii){
                        Log::Error("tracker already contains raw_dii\n");
                    }
                }
                for (auto& kvp : tracker_raw) {
                    if (kvp.key.imageView == raw_dii.imageView) {
                        Log::Warning("view equal\n");
                    }
                }
                tracker_raw.push_back(raw_dii, i);
#endif
                return i;
            }
        }
        EWE_UNREACHABLE;
        return UINT32_MAX; //error silencer
    }

    void BindlessDescriptor::UnbindRaw(TextureIndex texture_index){

        std::unique_lock binding_lock{binding_mutex};
        VkDescriptorImageInfo temp;
        bool found = false;
        for(auto& kvp : tracker_raw){
            if(kvp.value == texture_index){
                temp = kvp.key;
                found = true;
                break;
            }
        }
        EWE_ASSERT(found);

        const uint32_t occupancy_index = static_cast<uint32_t>(DescriptorType::Sampled);
        auto& bits = occupancy[occupancy_index];
        EWE_ASSERT(bits.test(texture_index));
        bits.reset(texture_index);

        for(auto iter = tracker_raw.begin(); iter != tracker_raw.end(); iter++){
            if(iter->key == temp){
                tracker_raw.Remove(iter);
                return;
            }
        }
        EWE_UNREACHABLE;
    }

    std::string BindlessDescriptor::GetImageNameForIndex(TextureIndex index) const {
        for (auto& tr : tracker) {
            if (tr.value == index) {
                return tr.key->view.image.name.string();
            }
        }
        return "invalid index";
    }

}//namespace Backend
} //namespace EWE