#include "EightWinds/RenderGraph/Command/Execute.h"

#include "EightWinds/CommandBuffer.h"

#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Backend/RenderInfo.h"

#include <cassert>

//#define EXECUTOR_DEBUGGING

namespace EWE{

    namespace Command{
        Executor::Executor(LogicalDevice& logicalDevice) noexcept
        : logicalDevice{logicalDevice}
        {

        }

        namespace Exec{
            struct ExecContext {
                LogicalDevice& device;

                std::vector<Instruction> const& instructions;
                CommandBuffer& cmdBuf;
                HeapBlock<uint8_t> const& paramPool;
                //std::vector<uint8_t> const& barrierPool;

                ParamPack::Pipeline boundPipeline{};

                std::size_t iterator = 0;

                uint8_t frame;
            
    #ifdef EXECUTOR_DEBUGGING
                void Print(){
                    printf("\t[%d]:[%zu]:[%zu]:[%zu]\n", iterator, paramPool.data(), instructions[iterator].paramOffset, &paramPool[instructions[iterator].paramOffset]);
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
            &Exec::Scissor,
            &Exec::ViewportCount,
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
                auto& data = Instruction::GetData<Instruction::BeginRender>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
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
                auto const& pack = Instruction::GetData<Instruction::BindPipeline>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                //ParamPack::Pipeline const& pipePack = *reinterpret_cast<ParamPack::Pipeline const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                assert(pack.pipe != VK_NULL_HANDLE);
                assert(pack.layout != VK_NULL_HANDLE);
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
                auto& push = Instruction::GetData<Instruction::PushConstant>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdPushConstants(ctx.cmdBuf, ctx.boundPipeline.layout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalPushConstant_Raw), &push);
            }


            void Draw(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::VertexDraw const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Instruction::Draw>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdDraw(ctx.cmdBuf, data.vertexCount, data.instanceCount, data.firstVertex, data.firstInstance);
            }

            void DrawIndexed(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::IndexDraw const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Instruction::DrawIndexed>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdDrawIndexed(ctx.cmdBuf, data.indexCount, data.instanceCount, data.firstIndex, data.vertexOffset, data.firstInstance);
            }

            void Dispatch(ExecContext& ctx){
                //auto* data = reinterpret_cast<ParamPack::Dispatch const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
                auto& data = Instruction::GetData<Instruction::Dispatch>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
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
                auto& data = Instruction::GetData<Instruction::DrawMeshTasks>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);

                ctx.device.cmdDrawMeshTasks(ctx.cmdBuf, data.x, data.y, data.z);
            }
            
            void DrawIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Instruction::DrawIndirect>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                vkCmdDrawIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
            }
            void DrawIndexedIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Instruction::DrawIndexedIndirect>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                vkCmdDrawIndexedIndirect(ctx.cmdBuf, data.buffer, data.offset, data.drawCount, data.stride);
            }
            void DispatchIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& data = Instruction::GetData<Instruction::DispatchIndirect>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                vkCmdDispatchIndirect(ctx.cmdBuf, data.buffer, data.offset);
            }
            void DrawMeshTasksIndirect(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Instruction::DrawMeshTasksIndirect>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                ctx.device.cmdDrawMeshTasksIndirect(ctx.cmdBuf, draw.buffer, draw.offset, draw.drawCount, draw.stride);
            }
            void DrawIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Instruction::DrawIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                vkCmdDrawIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            void DrawIndexedIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Instruction::DrawIndexedIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                vkCmdDrawIndexedIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            void DrawMeshTasksIndirectCount(ExecContext& ctx){
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                auto& draw = Instruction::GetData<Instruction::DrawMeshTasksIndirectCount>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                ctx.device.cmdDrawMeshTasksIndirectCount(ctx.cmdBuf, draw.buffer, draw.offset, draw.countBuffer, draw.countBufferOffset, draw.drawCount, draw.stride);
            }
            
            void Viewport(ExecContext& ctx){
                auto& data = Instruction::GetData<Instruction::DS_Viewport>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetViewport(ctx.cmdBuf, 0, 1, &data.viewport);
            }
            void Scissor(ExecContext& ctx){
                auto& data = Instruction::GetData<Instruction::DS_Scissor>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetScissor(ctx.cmdBuf, 0, 1, &data.scissor);
            }

            void ViewportCount(ExecContext& ctx){
                auto& data = Instruction::GetData<Instruction::DS_ViewportCount>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                assert(data.currentViewportCount < ParamPack::ViewportCount::ArbitraryViewportCountLimit);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetViewport(ctx.cmdBuf, 0, data.currentViewportCount, data.viewports);
            }
            void ScissorCount(ExecContext& ctx){
                auto& data = Instruction::GetData<Instruction::DS_ScissorCount>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
                assert(data.currentScissorCount < ParamPack::ScissorCount::ArbitraryScissorCountLimit);
    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                vkCmdSetScissor(ctx.cmdBuf, 0, data.currentScissorCount, data.scissors);
            }

            void BeginLabel(ExecContext& ctx){
    #if EWE_DEBUG_NAMING
                auto& pack = Instruction::GetData<Instruction::BeginLabel>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);
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
                bool const* condition = Instruction::GetData<Instruction::If>(ctx.instructions[ctx.iterator].instruction_pointer, ctx.frame);

    #ifdef EXECUTOR_DEBUGGING
                ctx.Print();
    #endif
                if(*condition){
                    while(ctx.iterator < ctx.instructions.size()){
                        if(ctx.instructions[ctx.iterator].type == Command::Instruction::Type::EndIf){
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
    #if EWE_DEBUG_BOOL
                printf("not enabled currently\n");
    #endif
            }

            void Switch(ExecContext& ctx){
    #if EWE_DEBUG_BOOL
                printf("not enabled currently\n");
    #endif
            }
            void Case(ExecContext& ctx){
    #if EWE_DEBUG_BOOL
                printf("not enabled currently\n");
    #endif
            }
            void Default(ExecContext& ctx){
    #if EWE_DEBUG_BOOL
                printf("not enabled currently\n");
    #endif
            }

        } //namespace Exec

        void Executor::Execute(CommandBuffer& cmdBuf, uint8_t frameIndex) const noexcept {
            Exec::ExecContext ctx{
                .device = logicalDevice, 
                .instructions = instructions, 
                .cmdBuf = cmdBuf, 
                .paramPool = paramPool[frameIndex],
                //.barrierPool = barrierPool,
                .boundPipeline{.pipe = VK_NULL_HANDLE, .layout = VK_NULL_HANDLE,.bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM },
                .frame = frameIndex
            };

            while(ctx.iterator < instructions.size()){
                //validate before creating the executor
                //assert(instructions[iterator].type != CommandInstruction::Type::EndIf && "unscoped endif");
                //assert(instructions[iterator].type != CommandInstruction::Type::LoopEnd && "unscoped loop end");
                //assert(instructions[iterator].type != CommandInstruction::Type::SwitchEnd && "unscoped switch end");
                
                //the cast doesnt matter at all, but it makes it easier to step thru in the debugger
                Exec::CommandFunction* cmdFunc = dispatchTable[static_cast<std::size_t>(instructions[ctx.iterator].type)];
                cmdFunc(ctx);
                ctx.iterator++;
            }
        }
    } //namespace Command
} //namespace EWE