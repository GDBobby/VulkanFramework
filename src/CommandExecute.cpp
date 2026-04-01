#include "EightWinds/Command/Execute.h"

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Backend/RenderInfo.h"

#include "EightWinds/Command/Record.h"

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
            for (uint8_t i = 0; i < max_frames_in_flight; i++) {
                paramPool[i].Resize(full_data_size);
                param_pool_addresses[i] = reinterpret_cast<std::size_t>(paramPool[i].Data());
            }
            record.FixDeferred(param_pool_addresses);
            EWE_ASSERT(record.ValidateInstructions());
            record.hasBeenCompiled = true;
        }

        namespace Exec{
            struct ExecContext {
                LogicalDevice& device;

                std::vector<Instruction> const& instructions;
                CommandBuffer& cmdBuf;
                HeapBlock<uint8_t> const& paramPool;
                //std::vector<uint8_t> const& barrierPool;

                ParamPack<Inst::BindPipeline> boundPipeline{};

                std::size_t iterator = 0;

                uint8_t frame;
            
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
            //define command functions here
            void BeginRender(ExecContext& ctx) {
                auto& data = Instruction::GetData<Inst::BeginRender>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                //auto* data = reinterpret_cast<VkRenderingInfo const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
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
                auto const& pack = Instruction::GetData<Inst::BindPipeline>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                //ParamPack::Pipeline const& pipePack = *reinterpret_cast<ParamPack::Pipeline const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                EWE_ASSERT(pack.pipe != VK_NULL_HANDLE);
                EWE_ASSERT(pack.layout != VK_NULL_HANDLE);
                ctx.boundPipeline = pack;

    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdBindPipeline(ctx.cmdBuf, pack.bindPoint, pack.pipe);
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
                //auto* push = reinterpret_cast<GlobalPushConstant_Raw const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& push = Instruction::GetData<Inst::Push>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdPushConstants(ctx.cmdBuf, ctx.boundPipeline.layout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalPushConstant_Raw), &push);
            }


            void Draw(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::VertexDraw const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Inst::Draw>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdDraw(ctx.cmdBuf, data.vertexCount, data.instanceCount, data.firstVertex, data.firstInstance);
            }

            void DrawIndexed(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::IndexDraw const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Inst::DrawIndexed>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdDrawIndexed(ctx.cmdBuf, data.indexCount, data.instanceCount, data.firstIndex, data.vertexOffset, data.firstInstance);
            }

            void Dispatch(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::Dispatch const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Inst::Dispatch>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
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
                //auto* data = reinterpret_cast<ParamPack::DrawMeshTasks const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Inst::DrawMeshTasks>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);

                ctx.device.cmdDrawMeshTasks(ctx.cmdBuf, data.x, data.y, data.z);
            }
            
            void DrawIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Inst::DrawIndirect>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                vkCmdDrawIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
            }
            void DrawIndexedIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Inst::DrawIndexedIndirect>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                vkCmdDrawIndexedIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
            }
            void DispatchIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Inst::DispatchIndirect>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                vkCmdDispatchIndirect(ctx.cmdBuf, data.buffer, data.offset);
            }
            void DrawMeshTasksIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Inst::DrawMeshTasksIndirect>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                ctx.device.cmdDrawMeshTasksIndirect(ctx.cmdBuf, draw.buffer, draw.offset, draw.drawCount, draw.stride);
            }
            void DrawIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Inst::DrawIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                vkCmdDrawIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            void DrawIndexedIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Inst::DrawIndexedIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                vkCmdDrawIndexedIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            void DrawMeshTasksIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Inst::DrawMeshTasksIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                ctx.device.cmdDrawMeshTasksIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            
            void Viewport(ExecContext& ctx){
                auto& data = Instruction::GetData<Inst::DS_Viewport>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetViewport(ctx.cmdBuf, 0, 1, &data.viewport);
            }
            void Scissor(ExecContext& ctx){
                auto& data = Instruction::GetData<Inst::DS_Scissor>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetScissor(ctx.cmdBuf, 0, 1, &data.scissor);
            }

            void ViewportCount(ExecContext& ctx){
                auto& data = Instruction::GetData<Inst::DS_ViewportCount>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                EWE_ASSERT(data.currentViewportCount < ParamPack<Inst::DS_ViewportCount>::ArbitraryViewportCountLimit);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetViewport(ctx.cmdBuf, 0, data.currentViewportCount, data.viewports);
            }
            void ScissorCount(ExecContext& ctx){
                auto& data = Instruction::GetData<Inst::DS_ScissorCount>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                EWE_ASSERT(data.currentScissorCount < ParamPack<Inst::DS_ScissorCount>::ArbitraryScissorCountLimit);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetScissor(ctx.cmdBuf, 0, data.currentScissorCount, data.scissors);
            }

            void BeginLabel(ExecContext& ctx){
    #if EWE_DEBUG_NAMING
                auto& pack = Instruction::GetData<Inst::BeginLabel>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);
                VkDebugUtilsLabelEXT labelUtil{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                    .pNext = nullptr,
                    .pLabelName = pack.name,
                    .color = {pack.red, pack.green, pack.blue, 1.f}
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
                bool const* condition = Instruction::GetData<Inst::If>(ctx.instructions[ctx.iterator].instruction_pointer.get(), ctx.frame);

    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                if(*condition){
                    while(ctx.iterator < ctx.instructions.size()){
                        if(ctx.instructions[ctx.iterator].type == Inst::Type::EndIf){
    #ifdef EXECUTOR_DEBUGGING
                            ctx.Print();
    #endif
                            return;
                        }
                        dispatchTable[static_cast<std::size_t>(ctx.instructions[ctx.iterator].type)](ctx);
                        ctx.iterator++;
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
            Exec::ExecContext ctx{
                .device = logicalDevice, 
                .instructions = record.records, 
                .cmdBuf = cmdBuf, 
                .paramPool = paramPool[frameIndex],
                //.barrierPool = barrierPool,
                .boundPipeline{.pipe = VK_NULL_HANDLE, .layout = VK_NULL_HANDLE,.bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM },
                .frame = frameIndex
            };

            while(ctx.iterator < ctx.instructions.size()){
                //validate before creating the executor
                //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::EndIf && "unscoped endif");
                //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::LoopEnd && "unscoped loop end");
                //EWE_ASSERT(instructions[iterator].type != CommandInst::Type::SwitchEnd && "unscoped switch end");
                
                //the cast doesnt matter at all, but it makes it easier to step thru in the debugger
                auto const& cmd_type = ctx.instructions[ctx.iterator].type;
                std::size_t const& cmd_index = static_cast<std::size_t>(cmd_type);
                Exec::CommandFunction* cmdFunc = dispatchTable[cmd_index];
                cmdFunc(ctx);
                ctx.iterator++;
            }
        }
    } //namespace Command
} //namespace EWE