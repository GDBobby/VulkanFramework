#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/Command/CommandBuffer.h"

#include <cassert>

namespace EWE{

    PushTracker::PushTracker(GlobalPushConstant* ptr) noexcept
    : pushAddress{ptr}
    {
        for(uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++){
            buffers[i].resource = nullptr;
        }
        for(uint8_t i = 0; i < GlobalPushConstant::texture_count; i++){
            textures[i].resource = nullptr;
        }
    }

    GPUTask::~GPUTask(){
#if EWE_DEBUG_BOOL
        printf("need to destruct deferred pointers from CommandRecord, currently memory leak\n");
#endif
        if (renderTracker!= nullptr) {
            delete renderTracker;
        }
    }
    void GPUTask::Execute(CommandBuffer& cmdBuf) {
        assert(cmdBuf.commandPool.queue == queue);
        commandExecutor.Execute(cmdBuf);
    }

    /*
    template<typename T>
    void ReplaceResource(std::vector<Resource<T>*>& used, Resource<T>*& oldRes, T* repRes, bool writes) {
        if (oldRes->resource == repRes) {
            return;
        }
        //this is a removal, but it can be put here for simplicity
        if (repRes == nullptr) {

            return;
        }

        //remove old resource
        assert(oldRes->usageInCurrentTask >= 1);
        oldRes->usageInCurrentTask--;
        if (oldRes->usageInCurrentTask == 0) {
            for (auto iter = used.begin(); iter != used.end(); iter++) {
                if ((*iter) == oldRes) {
                    used.erase(iter);
                }
            }
            delete oldRes;
        }

        //check if its already used in the task
        bool foundInUse = false;
        for (auto& us : used) {
            if (us->resource == repRes) {
                us->usageInCurrentTask++;
                oldRes = us;
                foundInUse = true;
                break;
            }
        }
        //set the new dst image
        if (!foundInUse) {
            //its written to, so the second constructor param is true
            used.push_back(new Resource<T>(repRes, true));
            oldRes = used.back();
        }
    }
    */
    
    void GPUTask::DefineBlitUsage(uint16_t blitIndex, Image* srcImage, Image* dstImage) noexcept {
        assert(blitIndex < blitTrackers.size());
        auto& tracker = blitTrackers[blitIndex];

        //SHOULD i guarantee they're in transfer src/dst optimal?
        if (srcImage != nullptr) {
            tracker.srcImage.resource = srcImage;
            tracker.srcImage.usage.accessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            tracker.srcImage.usage.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        }
        else {
            tracker.srcImage.resource = nullptr;
        }

        if (dstImage != nullptr) {
            tracker.dstImage.resource = dstImage;
            tracker.dstImage.usage.accessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            tracker.dstImage.usage.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        }
        else {
            tracker.dstImage.resource = nullptr;
        }
    }


    void GPUTask::PushBuffer(Buffer* buffer, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData) noexcept{
        assert(slot < GlobalPushConstant::buffer_count);
         
        //first, check whats currently in the requested slot
        auto& desired_slot = pushTrackers[pushIndex].buffers[slot];
        //ReplaceResource(usedBuffers, desired_slot, buffer);
        if (buffer != nullptr) {
            pushTrackers[pushIndex].pushAddress->buffer_addr[slot] = buffer->deviceAddress;
            desired_slot.resource = buffer;
            desired_slot.usage = usageData;
        }
        else {
            pushTrackers[pushIndex].pushAddress->buffer_addr[slot] = 0;
            desired_slot.resource = nullptr;
        }
    }
    void GPUTask::PushImage(Image* img, uint32_t pushIndex, uint8_t slot, ResourceUsageData const& usageData) noexcept{
        assert(slot < (GlobalPushConstant::texture_count + GlobalPushConstant::buffer_count));
        if(slot >= GlobalPushConstant::buffer_count){
            slot -= GlobalPushConstant::buffer_count;
        }
        auto& desired_slot = pushTrackers[pushIndex].textures[slot];

        //ReplaceResource(usedImages, desired_slot, img);
        if (img != nullptr) {
            printf("set up the image texture index\n");
            //pushTrackers[pushIndex].pushAddress->texture_indices[slot] = img->texture_index;
            desired_slot.resource = img;
            desired_slot.usage = usageData;
        }
        else {
            pushTrackers[pushIndex].pushAddress->texture_indices[slot] = -1;
            desired_slot.resource = nullptr;
        }
    }

    void GPUTask::SetRenderInfo() {
        assert(renderTracker != nullptr);
        renderTracker->compact.Expand(&renderTracker->vk_data);
        

#if EWE_DEBUG_BOOL

        bool hasBeginRender = false;
        for (auto& inst : commandExecutor.instructions) {
            if (inst.type == CommandInstruction::Type::BeginRender) {
                VkRenderingInfo** tempAddr = reinterpret_cast<VkRenderingInfo**>(commandExecutor.paramPool.data() + inst.paramOffset);
                *tempAddr = &renderTracker->vk_data.renderingInfo;
                inst.paramOffset;
                hasBeginRender = true;
                break;
            }
        }
        assert(hasBeginRender);
#endif
    }

    /*
    void GPUTask::GenerateInternalSync(){
        assert(false && "disabled, pending removal??");

        //i could optimize this a tiny bit if its assumed push cosntants are 
        //never going to use the same resource twice
        std::vector<Resource<Buffer>*> buffers{};
        for(auto& push : pushTrackers){
            for(uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++){
                if(push.buffers[i].resource != nullptr){
                    buffers.push_back(&push.buffers[i]);
                }
            }
        }

        std::vector<Resource<Image>*> images{};

        std::size_t currentBlitIndex = 0;
        std::size_t currentPushIndex = 0;
        //go thru the push indices
        bool blitScope = currentBlitIndex < blitTrackers.size();
        bool pushScope = currentPushIndex < pushTrackers.size();
        while (blitScope || pushScope) {
            if (blitScope && pushScope) {
                if (reinterpret_cast<std::size_t>(blitTrackers[currentBlitIndex].paramPackAddress) < reinterpret_cast<std::size_t>(pushTrackers[currentPushIndex].pushAddress)) {
                    images.push_back(&blitTrackers[currentBlitIndex].srcImage);
                    images.push_back(&blitTrackers[currentBlitIndex].dstImage);
                    currentBlitIndex++;
                    blitScope = currentBlitIndex < blitTrackers.size();
                }
                //never equal
                else {
                    for (uint8_t i = 0; i < GlobalPushConstant::texture_count; i++) {
                        if (pushTrackers[currentPushIndex].textures[i].resource != nullptr) {
                            images.push_back(&pushTrackers[currentPushIndex].textures[i]);
                        }
                    }
                    currentPushIndex++;
                    pushScope = currentPushIndex < pushTrackers.size();
                }
            }
            else if (blitScope) {
                images.push_back(&blitTrackers[currentBlitIndex].srcImage);
                images.push_back(&blitTrackers[currentBlitIndex].dstImage);
                currentBlitIndex++;
                blitScope = currentBlitIndex < blitTrackers.size();
            }
            else { //push scope guaranteed
                for (uint8_t i = 0; i < GlobalPushConstant::texture_count; i++) {
                    if (pushTrackers[currentPushIndex].textures[i].resource != nullptr) {
                        images.push_back(&pushTrackers[currentPushIndex].textures[i]);
                    }
                }
                currentPushIndex++;
                pushScope = currentPushIndex < pushTrackers.size();
            }
        }

        printf("now all the resources are collected\n");


        struct BarrierPackage {
            std::vector<VkBufferMemoryBarrier2> barriers;
            std::vector<std::pair<uint32_t, uint32_t>> instructionPoint;
        };
        struct ImagePackage {
            std::vector<VkImageMemoryBarrier2> barriers;
            std::vector<std::pair<uint32_t, uint32_t>> instructionPoint;
        };

        BarrierPackage buffer_package{};
        { //buffers
            VkBufferMemoryBarrier2 buff_barrier{};
            buff_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            buff_barrier.pNext = nullptr;
            buff_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            buff_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            for (uint32_t i = 0; i < buffers.size(); i++) {
                const bool startingWrite = GetAccessMaskWrite(buffers[i]->usage.accessMask);
                for (uint32_t j = i + 1; j < buffers.size(); j++) {
                    if (buffers[i]->resource == buffers[j]->resource) {
                        const bool isAWrite = GetAccessMaskWrite(buffers[j]->usage.accessMask);
                        if (startingWrite || isAWrite) {
                            buff_barrier.buffer = buffers[i]->resource->buffer_info.buffer;
                            buff_barrier.srcAccessMask = buffers[i]->usage.accessMask;
                            buff_barrier.srcStageMask = buffers[i]->usage.stage;

                            buff_barrier.dstAccessMask = buffers[j]->usage.accessMask;
                            buff_barrier.dstStageMask = buffers[j]->usage.stage;

                            buff_barrier.offset = buffers[i]->resource->buffer_info.offset;
                            buff_barrier.size = buffers[i]->resource->bufferSize;

                            buffer_package.barriers.push_back(buff_barrier);
                            buffer_package.instructionPoint.emplace_back(i, j);
                        }
                        break;
                    }
                }
            }
        }
        ImagePackage img_package{};
        { //images
            VkImageMemoryBarrier2 img_barrier{};
            img_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            img_barrier.pNext = nullptr;
            img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            VkImageSubresourceRange& subRange = img_barrier.subresourceRange;
            subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subRange.baseArrayLayer = 0;
            subRange.baseMipLevel = 0;

            for (uint32_t i = 0; i < images.size(); i++) {
                const bool startingWrite = GetAccessMaskWrite(images[i]->usage.accessMask);
                for (uint32_t j = i + 1; j < images.size(); j++) {
                    if (images[i]->resource == images[j]->resource) {
                        const bool isAWrite = GetAccessMaskWrite(images[j]->usage.accessMask);
                        if (startingWrite || isAWrite) {
                            img_barrier.image = images[i]->resource->image;
                            img_barrier.srcAccessMask = images[i]->usage.accessMask;
                            img_barrier.srcStageMask = images[i]->usage.stage;

                            img_barrier.dstAccessMask = images[j]->usage.accessMask;
                            img_barrier.dstStageMask = images[j]->usage.stage;

                            img_barrier.oldLayout = images[i]->resource->layout;
                            img_barrier.newLayout = images[i]->resource->layout;

                            //i might want to handle sub-images
                            subRange.layerCount = images[i]->resource->arrayLayers;
                            subRange.levelCount = images[i]->resource->mipLevels;

                            img_package.barriers.push_back(img_barrier);
                            img_package.instructionPoint.emplace_back(i, j);
                        }
                        break;
                    }
                }
            }
        }
    
        printf("i need a simplification of the barriers\n");

        for(auto iter = commandExecutor.instructions.begin(); iter != commandExecutor.instructions.end();){
            if(iter->type == CommandInstruction::Type::PipelineBarrier){
                iter = commandExecutor.instructions.erase(iter);
            }
            else{
                iter++;
            }
        }
        commandExecutor.barrierPool.clear();
        DependencyHeader dependencyHeader;
        std::size_t buffer_barrier_count = buffer_package.barriers.size();
        std::size_t total_barrier_pool_size = buffer_package.barriers.size() * (sizeof(DependencyHeader) + sizeof(VkBufferMemoryBarrier2));
        total_barrier_pool_size += img_package.barriers.size() * (sizeof(DependencyHeader) + sizeof(VkImageMemoryBarrier2));
        auto& barrierPool = commandExecutor.barrierPool;
        //setting to 0 is a waste of processing power, assuming i calculated the size right, and theres 0 gaps
        barrierPool.resize(total_barrier_pool_size);//, 0);
        uint8_t* current_memory_addr = barrierPool.data();
        std::size_t bar_index = 0;
        for(; bar_index < buffer_package.barriers.size(); bar_index++){
            auto& buf_bar = buffer_package.barriers[bar_index];
            dependencyHeader.buffer_count = 1;
            dependencyHeader.image_count = 0;

            //insert the instruction
            commandExecutor.instructions.insert(
                commandExecutor.instructions.begin() + buffer_package.instructionPoint[bar_index].second, 
                CommandInstruction{
                    CommandInstruction::Type::PipelineBarrier, 
                    current_memory_addr - barrierPool.data()
                }
            );
            for(auto after = bar_index + 1; after < buffer_package.barriers.size(); after++){
                //i think its guaranteed to be greater, not sure. worth checking
                buffer_package.instructionPoint[after].second += buffer_package.instructionPoint[after].second >= buffer_package.instructionPoint[bar_index].second;
            }
            for(auto after = 0; after < img_package.barriers.size(); after++){
                img_package.instructionPoint[after].second += img_package.instructionPoint[after].second >= img_package.instructionPoint[bar_index].second;
            }

            memcpy(current_memory_addr, &dependencyHeader, sizeof(DependencyHeader));
            current_memory_addr += sizeof(DependencyHeader);
            memcpy(current_memory_addr, &buf_bar, sizeof(VkBufferMemoryBarrier2));
            current_memory_addr +=  sizeof(VkBufferMemoryBarrier2);
        }
        bar_index = 0;
        for(; bar_index < img_package.barriers.size(); bar_index++){
            auto& img_bar = img_package.barriers[bar_index];
            dependencyHeader.buffer_count = 0;
            dependencyHeader.image_count = 1;

            //insert the instruction
            commandExecutor.instructions.insert(
                commandExecutor.instructions.begin() + img_package.instructionPoint[bar_index].second, 
                CommandInstruction{
                    CommandInstruction::Type::PipelineBarrier, 
                    current_memory_addr - barrierPool.data()
                }
            );
            for(auto after = bar_index + 1; after < img_package.barriers.size(); after++){
                //i think its guaranteed to be greater, not sure. worth checking
                img_package.instructionPoint[after].second += img_package.instructionPoint[after].second >= img_package.instructionPoint[bar_index].second;
            }

            memcpy(current_memory_addr, &dependencyHeader, sizeof(DependencyHeader));
            current_memory_addr += sizeof(DependencyHeader);
            memcpy(current_memory_addr, &img_bar, sizeof(VkImageMemoryBarrier2));
            current_memory_addr +=  sizeof(VkImageMemoryBarrier2);
        }
        assert(current_memory_addr == (barrierPool.data() + total_barrier_pool_size));
    }
    */
} //namespace EWE