#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/Command/CommandBuffer.h"
#include "EightWinds/RenderGraph/Command/Record.h"

#include <cassert>

namespace EWE{

    PushTracker::PushTracker(GlobalPushConstant* ptr) noexcept
    : pushAddress{ptr}
    {
        for(uint8_t i = 0; i < GlobalPushConstant::buffer_count; i++){
            buffers[i].buffer = nullptr;
        }
        for(uint8_t i = 0; i < GlobalPushConstant::texture_count; i++){
            textures[i].image = nullptr;
        }
    }

    GPUTask::GPUTask(LogicalDevice& logicalDevice, Queue& queue, CommandRecord& cmdRecord, std::string_view name) 
        : logicalDevice{logicalDevice}, 
        queue{queue},
        commandExecutor{logicalDevice},
        name{name}
    {
        assert(!cmdRecord.hasBeenCompiled);
        //cmdRecord.Optimize(); <--- EVENTUALLY
        const uint64_t full_data_size = cmdRecord.records.back().paramOffset + CommandInstruction::GetParamSize(cmdRecord.records.back().type);

        commandExecutor.instructions = cmdRecord.records;
        commandExecutor.paramPool.resize(full_data_size);
        const std::size_t param_pool_address = reinterpret_cast<std::size_t>(commandExecutor.paramPool.data());
        cmdRecord.FixDeferred(param_pool_address);
        for(auto& push_off : cmdRecord.push_offsets){
            std::size_t temp_addr = reinterpret_cast<std::size_t>(push_off);
            //pushTrackers.emplace_back(reinterpret_cast<GlobalPushConstant*>(temp_addr + param_pool_address));
        }
        uint64_t blitIndex = 0;
        for (auto const& inst : cmdRecord.records) {
            if (inst.type == CommandInstruction::Type::BeginRender) {
                renderTracker = new RenderTracker();
            }
            if(inst.type == CommandInstruction::Type::Blit) {
                //auto& blitBack = blitTrackers.emplace_back();
                //blitBack.dstImage.resource = nullptr;
                //blitBack.srcImage.resource = nullptr;
            }
        }

        //all validations will be here
        //theres some non-validation stuff here, like collapsing empty branches
        //maybe split out optimization into a different loop
#if EWE_DEBUG_BOOL
        assert(cmdRecord.ValidateInstructions());
#endif
       cmdRecord.hasBeenCompiled = true;
    

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

    int GPUTask::AddImagePin(Image* image, VkPipelineStageFlags2 stage, VkAccessFlags2 accessMask, VkImageLayout layout){
        explicitImageState.push_back(
            new Resource<Image>{
                .image = image, 
                .usage = ImageUsageData{
                    .stage = stage,
                    .accessMask = accessMask,
                    .layout = layout
                }
            }
        );

        return explicitImageState.size() - 1;
    }
    int GPUTask::AddImagePin(Image* image, ImageUsageData const& usage){
        explicitImageState.push_back(
            new Resource<Image>{
                .image = image, 
                .usage = usage
            }
        );
        return explicitImageState.size() - 1;
    }
    int GPUTask::AddBufferPin(Buffer* buffer, BufferUsageData const& usage){
        explicitBufferState.push_back(
            new Resource<Buffer>{
                .buffer = buffer, 
                .usage = usage
            }
        );
        return explicitBufferState.size() - 1;
    }
    int GPUTask::AddBufferPin(Buffer* buffer, VkPipelineStageFlags2 stage, VkAccessFlags2 accessMask) {
        explicitBufferState.push_back(
            new Resource<Buffer>{
                .buffer = buffer, 
                .usage = BufferUsageData{
                    .stage = stage,
                    .accessMask = accessMask
                }
            }
        );
        return explicitBufferState.size() - 1;
    }

    void GPUTask::SetResource(int pin, Image& image) {
        explicitImageState[pin]->image = &image;
    }
    void GPUTask::SetResource(int pin, Buffer& buffer) {
        explicitBufferState[pin]->buffer = &buffer;
    }

    void GPUTask::SetRenderInfo() {
        assert(renderTracker != nullptr);
        renderTracker->compact.Expand(&renderTracker->vk_data);
        
        bool hasBeginRender = false;
        //there's only going to be one BeginRender, and this search will only be performed once on construction
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
    }
    void GPUTask::UpdateFrameIndex(uint8_t frameIndex) {
        renderTracker->compact.Update(&renderTracker->vk_data, frameIndex);
    }
} //namespace EWE