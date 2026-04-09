#include "EightWinds/Command/Execute.h"

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/Command/Record.h"

#include "EightWinds/Data/Address.h"

//#define EXECUTOR_DEBUGGING

namespace EWE{

namespace Command{
    Executor::Executor(LogicalDevice& _logicalDevice, Record& _record) noexcept
    : logicalDevice{_logicalDevice},
        record{_record}
    {

        //EWE_ASSERT(!record.hasBeenCompiled);
        //record.Optimize(); <--- EVENTUALLY
        
        const uint64_t full_data_size = record.CalculateSize();
        
        PerFlight<std::size_t> param_pool_addresses{};
        for (uint8_t frame = 0; frame < max_frames_in_flight; frame++) {
            paramPool.params[frame].Resize(full_data_size);
            param_pool_addresses[frame] = reinterpret_cast<std::size_t>(paramPool.params[frame].memory);
        }
        paramPool.instructions.reserve(record.records.size());
        for(auto& rec : record.records){
            paramPool.instructions.push_back(rec.type);
        }
        record.FixDeferred(param_pool_addresses);
        EWE_ASSERT(record.ValidateInstructions());
        record.hasBeenCompiled = true;
    }

namespace Exec{
    struct ExecContext {
        LogicalDevice& device;
        ParamPool const& pp; //its important that this is a view and not a copy or move, either of which would invalidate pointers

        CommandBuffer& cmdBuf;

        ParamPack<Inst::BindPipeline> boundPipeline{};

        std::size_t iterator = 0;

        uint8_t frame;

        Address address;

        template <Inst::Type IType>
        requires(Inst::GetParamSize(IType) > 0)
        auto& CastAndIncrement(){
            auto& ret = *reinterpret_cast<ParamPack<IType>*>(address.address);
            address.address += Inst::GetParamSize(IType);
            return ret;
        }

        void Iterate();
    
#ifdef EXECUTOR_DEBUGGING
        void Print(){
            Logger::Print<Logger::Debug>("\t[%d]:[%zu]:[%zu]:[%zu]\n", iterator, paramPool.data(), instructions[iterator].paramOffset, &paramPool[instructions[iterator].paramOffset]);
        }
#endif
    };

    using CommandFunction = void(ExecContext& ctx);



    

    void BindPipeline(ExecContext& ctx);
    void BindDescriptor(ExecContext& ctx);
    void Push(ExecContext& ctx);
    void BeginRender(ExecContext& ctx);
    void EndRender(ExecContext& ctx);
    
    void Draw(ExecContext& ctx);
    void DrawIndexed(ExecContext& ctx);
    void Dispatch(ExecContext& ctx);
    void DrawMeshTasks(ExecContext& ctx);
    
    void DrawIndirect(ExecContext& ctx);
    void DrawIndexedIndirect(ExecContext& ctx);
    void DispatchIndirect(ExecContext& ctx);
    void DrawMeshTasksIndirect(ExecContext& ctx);
    
    void DrawIndirectCount(ExecContext& ctx);
    void DrawIndexedIndirectCount(ExecContext& ctx);
    void DrawMeshTasksIndirectCount(ExecContext& ctx);
    
    //void Barrier(ExecContext& ctx);
    void Viewport(ExecContext& ctx);
    void Scissor(ExecContext& ctx);
    void ViewportCount(ExecContext& ctx);
    void ScissorCount(ExecContext& ctx);
    void BeginLabel(ExecContext& ctx);
    void EndLabel(ExecContext& ctx);

    void IfStatement(ExecContext& ctx);

    //i dont think i wnat to commit to more advanced flow control at this very moment
    void LoopBegin(ExecContext& ctx);
    void Switch(ExecContext& ctx);
    void Case(ExecContext& ctx);
    void Default(ExecContext& ctx);
} //namespace Exec

static constexpr auto dispatchTable = std::array{
    &Exec::BindPipeline,
    &Exec::BindDescriptor, 
    &Exec::Push, 
    &Exec::BeginRender, 
    &Exec::EndRender,
    
    &Exec::Draw,
    &Exec::DrawIndexed,
    &Exec::Dispatch,
    &Exec::DrawMeshTasks,
    
    &Exec::DrawIndirect,
    &Exec::DrawIndexedIndirect,
    &Exec::DispatchIndirect,
    &Exec::DrawMeshTasksIndirect,
    
    &Exec::DrawIndirectCount,
    &Exec::DrawIndexedIndirectCount,
    &Exec::DrawMeshTasksIndirectCount,
    
    //&Exec::Barrier, 
    &Exec::Viewport,
    &Exec::ViewportCount,
    &Exec::Scissor,
    &Exec::ScissorCount,
    &Exec::BeginLabel,
    &Exec::EndLabel,
    &Exec::IfStatement,
    &Exec::LoopBegin,
    &Exec::Switch,
    &Exec::Case,
    &Exec::Default,
};

namespace Exec{
    void ExecContext::Iterate(){
        auto const& cmd_type = pp.instructions[iterator];
        std::size_t const& cmd_index = static_cast<std::size_t>(cmd_type);
        Exec::CommandFunction* cmdFunc = dispatchTable[cmd_index];
        cmdFunc(*this);
        iterator++;
    }

    //define command functions here
    void BeginRender(ExecContext& ctx) {
        auto& data = ctx.CastAndIncrement<Inst::BeginRender>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
#if EWE_DEBUG_BOOL
        ctx.cmdBuf.debug_currentlyRendering = true;
#endif
        vkCmdBeginRendering(ctx.cmdBuf, &data);
    }
    void EndRender(ExecContext& ctx) {
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif

#if EWE_DEBUG_BOOL
        ctx.cmdBuf.debug_currentlyRendering = false;
#endif
        vkCmdEndRendering(ctx.cmdBuf);
    }
    void BindPipeline(ExecContext& ctx) {
        auto& data = ctx.CastAndIncrement<Inst::BindPipeline>();
        EWE_ASSERT(data.pipe != VK_NULL_HANDLE);
        EWE_ASSERT(data.layout != VK_NULL_HANDLE);
        ctx.boundPipeline = data;

#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdBindPipeline(ctx.cmdBuf, data.bindPoint, data.pipe);
    }

    //i dont know if i bother putting this in the list
    void BindDescriptor(ExecContext& ctx){
        //i dont know where to store the texture descriptor set yet
        VkDescriptorSet* desc = &ctx.device.bindlessDescriptor.set;
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdBindDescriptorSets(
            ctx.cmdBuf, ctx.boundPipeline.bindPoint, 
            ctx.boundPipeline.layout, 
            0, 1, 
            desc, 
            0, nullptr
        );
    }

    void Push(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Push>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdPushConstants(ctx.cmdBuf, ctx.boundPipeline.layout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalPushConstant_Raw), &data);
    }


    void Draw(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Draw>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdDraw(ctx.cmdBuf, data.vertexCount, data.instanceCount, data.firstVertex, data.firstInstance);
    }

    void DrawIndexed(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexed>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdDrawIndexed(ctx.cmdBuf, data.indexCount, data.instanceCount, data.firstIndex, data.vertexOffset, data.firstInstance);
    }

    void Dispatch(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Dispatch>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        //maybe add an if statement here, check if all groups are above 0?
        vkCmdDispatch(ctx.cmdBuf, data.x, data.y, data.z);
    }
    
    void DrawMeshTasks(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        //auto* data = reinterpret_cast<ParamPack::DrawMeshTasks const*>(&ctx.paramPool[ctx.pp.instructions[ctx.iterator].paramOffset]);
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasks>();

        ctx.device.cmdDrawMeshTasks(ctx.cmdBuf, data.x, data.y, data.z);
    }
    
    void DrawIndirect(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawIndirect>();
        vkCmdDrawIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
    }
    void DrawIndexedIndirect(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexedIndirect>();
        vkCmdDrawIndexedIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
    }
    void DispatchIndirect(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DispatchIndirect>();
        vkCmdDispatchIndirect(ctx.cmdBuf, data.buffer, data.offset);
    }
    void DrawMeshTasksIndirect(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasksIndirect>();
        ctx.device.cmdDrawMeshTasksIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
    }
    void DrawIndirectCount(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawIndirectCount>();
        vkCmdDrawIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    void DrawIndexedIndirectCount(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexedIndirectCount>();
        vkCmdDrawIndexedIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    void DrawMeshTasksIndirectCount(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasksIndirectCount>();
        ctx.device.cmdDrawMeshTasksIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    
    void Viewport(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_Viewport>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdSetViewport(ctx.cmdBuf, 0, 1, &data.viewport);
    }
    void Scissor(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_Scissor>();
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdSetScissor(ctx.cmdBuf, 0, 1, &data.scissor);
    }

    void ViewportCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_ViewportCount>();
        EWE_ASSERT(data.currentViewportCount<ParamPack<Inst::DS_ViewportCount>::ArbitraryViewportCountLimit);
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdSetViewport(ctx.cmdBuf, 0, data.currentViewportCount, data.viewports);
    }
    void ScissorCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_ScissorCount>();
        EWE_ASSERT(data.currentScissorCount < ParamPack<Inst::DS_ScissorCount>::ArbitraryScissorCountLimit);
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        vkCmdSetScissor(ctx.cmdBuf, 0, data.currentScissorCount, data.scissors);
    }

    void BeginLabel(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::BeginLabel>();
#if EWE_DEBUG_NAMING
        VkDebugUtilsLabelEXT labelUtil{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
            .pNext = nullptr,
            .pLabelName = data.name,
            .color = {data.red, data.green, data.blue, 1.f}
        };
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        ctx.device.BeginLabel(ctx.cmdBuf, &labelUtil);
#endif
    }

    void EndLabel(ExecContext& ctx){
#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
#if EWE_DEBUG_NAMING
        ctx.device.EndLabel(ctx.cmdBuf);
#endif
    }


    void IfStatement(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::If>();

#ifdef EXECUTOR_DEBUGGING
        ctx.Print();
#endif
        if(data){
            while(ctx.iterator < ctx.pp.instructions.size()){
                if(ctx.pp.instructions[ctx.iterator] == Inst::Type::EndIf){
#ifdef EXECUTOR_DEBUGGING
                    ctx.Print();
#endif
                    return;
                }
                ctx.Iterate();
            }
            EWE_UNREACHABLE;
        }
        else {
            //passes over the if, so that the endif will be stepped over on return
            ctx.iterator++;
        }
    }

    
    void LoopBegin(ExecContext& ctx){
        //i need to wait and see how this pans out before i code it
        Logger::Print<Logger::Error>("not enabled currently\n");
    }

    void Switch(ExecContext& ctx){
        Logger::Print<Logger::Error>("not enabled currently\n");
    }
    void Case(ExecContext& ctx){
        Logger::Print<Logger::Error>("not enabled currently\n");
    }
    void Default(ExecContext& ctx){
        Logger::Print<Logger::Error>("not enabled currently\n");
    }

} //namespace Exec

    void Executor::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) const noexcept {
        //i dont quite understand why logicaldevice can be passed as a non-const reference from within a const function, 
        //but right now that behavior is depended upon. if that changes in the future, just remove const modifier from func
        ExecuteParamPool(paramPool, logicalDevice, cmdBuf, frameIndex);
    }

    void ExecuteParamPool(ParamPool const& _pp, LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, uint8_t frameIndex){
        Exec::ExecContext ctx{
            .device = logicalDevice, 
            .pp{_pp},  //its important that this is a view and not a copy or move
            .cmdBuf = cmdBuf,
            .boundPipeline{.pipe = VK_NULL_HANDLE, .layout = VK_NULL_HANDLE,.bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM },
            .iterator = 0,
            .frame = frameIndex,
            .address{_pp.params[frameIndex].memory}
        };
        while(ctx.iterator < ctx.pp.instructions.size()){
            //validate before creating the executor
            //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::EndIf && "unscoped endif");
            //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::LoopEnd && "unscoped loop end");
            //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::SwitchEnd && "unscoped switch end");
            
            //auto const& cmd_type = ctx.pp.instructions[ctx.iterator].type;
            //std::size_t const& cmd_index = static_cast<std::size_t>(cmd_type);
            //Exec::CommandFunction* cmdFunc = dispatchTable[cmd_index];
            //cmdFunc(ctx);
            ctx.Iterate();
        }
    }

} //namespace Command
} //namespace EWE