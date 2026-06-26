#include "EightWinds/Command/Execute.h"

#include "EightWinds/CommandBuffer.h"
#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/Backend/RenderInfo.h"
#include "EightWinds/Data/Address.h"

//#define EXECUTOR_DEBUGGING

namespace EWE{

namespace Command{

    ParamPack<Inst::BindPipeline> ExecuteParamPool_Internal(LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, ParamPool const& pp, uint8_t frameIndex, ParamPack<Inst::BindPipeline> const& boundPipeline);

namespace Exec{
#if !EWE_DEBUG_BOOL
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
            auto& ret = address.CastToRef<ParamPack<IType>>();
            address.address += sizeof(ParamPack<IType>);
            return ret;
        }

        void Iterate();
    };
#endif

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

    void Ext_Pool(ExecContext& ctx);
#if EWE_DEBUG_BOOL
    void Breakpoint(ExecContext& ctx);
    void DebugFunction(ExecContext& ctx);
#endif

    void Exec(ExecContext& ctx);
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

    &Exec::Ext_Pool,
#if EWE_DEBUG_BOOL
    &Exec::Breakpoint,
    &Exec::DebugFunction
#endif
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
#if EWE_DEBUG_BOOL
        ctx.cmdBuf.debug_currentlyRendering = true;
#endif
        vkCmdBeginRendering(ctx.cmdBuf, &data);
    }
    void EndRender(ExecContext& ctx) {

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

        vkCmdBindPipeline(ctx.cmdBuf, data.bindPoint, data.pipe);
    }

    //i dont know if i bother putting this in the list
    void BindDescriptor(ExecContext& ctx){
        //i dont know where to store the texture descriptor set yet
        VkDescriptorSet* desc = &ctx.device.bindlessDescriptor.set;
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
        vkCmdPushConstants(ctx.cmdBuf, ctx.boundPipeline.layout, VK_SHADER_STAGE_ALL, 0, data.size, data.data);
    }


    void Draw(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Draw>();
        vkCmdDraw(ctx.cmdBuf, data.vertexCount, data.instanceCount, data.firstVertex, data.firstInstance);
    }

    void DrawIndexed(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexed>();
        //is it a mistake to tightly bind these 2?
        //i dont see a common use case where i want to do multiple draw calls per index buffer without instancing
        vkCmdBindIndexBuffer(ctx.cmdBuf, data.indexBuffer.buffer, data.indexBuffer.offset, data.indexBuffer.indexType);
        vkCmdDrawIndexed(ctx.cmdBuf, data.indexCount, data.instanceCount, data.firstIndex, data.vertexOffset, data.firstInstance);
    }

    void Dispatch(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Dispatch>();
        //maybe add an if statement here, check if all groups are above 0?
        vkCmdDispatch(ctx.cmdBuf, data.x, data.y, data.z);
    }
    
    void DrawMeshTasks(ExecContext& ctx){
        //auto* data = reinterpret_cast<ParamPack::DrawMeshTasks const*>(&ctx.paramPool[ctx.pp.instructions[ctx.iterator].paramOffset]);
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasks>();
        ctx.device.cmdDrawMeshTasks(ctx.cmdBuf, data.x, data.y, data.z);
    }
    
    void DrawIndirect(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndirect>();
        if(data.drawCount > 0){ //Inst::If is preferred
            vkCmdDrawIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
        }
    }
    void DrawIndexedIndirect(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexedIndirect>();
        vkCmdDrawIndexedIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
    }
    void DispatchIndirect(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DispatchIndirect>();
        vkCmdDispatchIndirect(ctx.cmdBuf, data.buffer, data.offset);
    }
    void DrawMeshTasksIndirect(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasksIndirect>();
        ctx.device.cmdDrawMeshTasksIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
    }
    void DrawIndirectCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndirectCount>();
        vkCmdDrawIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    void DrawIndexedIndirectCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawIndexedIndirectCount>();
        vkCmdDrawIndexedIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    void DrawMeshTasksIndirectCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DrawMeshTasksIndirectCount>();
        ctx.device.cmdDrawMeshTasksIndirectCount(ctx.cmdBuf, data.buffer, data.offset, data.countBuffer, data.countBufferOffset, data.drawCount, data.stride);
    }
    
    void Viewport(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_Viewport>();
        vkCmdSetViewport(ctx.cmdBuf, 0, 1, &data.viewport);
    }
    void Scissor(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_Scissor>();
        vkCmdSetScissor(ctx.cmdBuf, 0, 1, &data.scissor);
    }

    void ViewportCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_ViewportCount>();
        EWE_ASSERT(data.currentViewportCount<ParamPack<Inst::DS_ViewportCount>::ArbitraryViewportCountLimit);
        vkCmdSetViewport(ctx.cmdBuf, 0, data.currentViewportCount, data.viewports);
    }
    void ScissorCount(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::DS_ScissorCount>();
        EWE_ASSERT(data.currentScissorCount < ParamPack<Inst::DS_ScissorCount>::ArbitraryScissorCountLimit);
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
        ctx.device.BeginLabel(ctx.cmdBuf, &labelUtil);
#endif
    }

    void EndLabel(ExecContext& ctx){
#if EWE_DEBUG_NAMING
        ctx.device.EndLabel(ctx.cmdBuf);
#endif
    }


    void IfStatement(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::If>();
        ctx.iterator++; //need to step past the current if
        while(ctx.iterator < ctx.pp.instructions.size()){
            if(ctx.pp.instructions[ctx.iterator] == Inst::Type::EndIf){
                return;
            }
            if(data){
                ctx.Iterate();
            }
            else{
                ctx.iterator++;
            }
        }
        EWE_UNREACHABLE;
    }

    
    void LoopBegin(ExecContext& ctx){
        //i need to wait and see how this pans out before i code it
        Log::Error("not enabled currently\n");
    }

    void Switch(ExecContext& ctx){
        Log::Error("not enabled currently\n");
    }
    void Case(ExecContext& ctx){
        Log::Error("not enabled currently\n");
    }
    void Default(ExecContext& ctx){
        Log::Error("not enabled currently\n");
    }

    void Ext_Pool(ExecContext& ctx){
        auto& data = ctx.CastAndIncrement<Inst::Ext_Pool>();
        ctx.boundPipeline = ExecuteParamPool_Internal(ctx.device, ctx.cmdBuf, *data.pool, ctx.frame, ctx.boundPipeline);
    }
#if EWE_DEBUG_BOOL
    void Breakpoint(ExecContext& ctx) {
        Log::Warning("inst::breakpoint\n");
        EWE_Debug_Breakpoint();
    }
    void DebugFunction(ExecContext& ctx){
        Log::Warning("debug func\n");
        auto& data = ctx.CastAndIncrement<Inst::DebugFunction>();
        if(data.callback){
            data.callback(ctx);
        }
    }
#endif

} //namespace Exec

    ParamPack<Inst::BindPipeline> ExecuteParamPool_Internal(LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, ParamPool const& _pp, uint8_t frameIndex, ParamPack<Inst::BindPipeline> const& boundPipeline){
        Exec::ExecContext ctx{
            .device = logicalDevice, 
            .pp{_pp},  //its important that this is a view and not a copy or move
            .cmdBuf = cmdBuf,
            .boundPipeline{boundPipeline},
            .iterator = 0,
            .frame = frameIndex,
            .address{_pp.params[frameIndex].memory}
        };
        while(ctx.iterator < ctx.pp.instructions.size()){
            ctx.Iterate();
        }
        return ctx.boundPipeline;
    }

    void ExecuteParamPool(LogicalDevice& logicalDevice, CommandBuffer& cmdBuf, ParamPool const& _pp, uint8_t frameIndex){
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