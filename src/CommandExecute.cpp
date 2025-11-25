#include "EightWinds/Command/Execute.h"

#include "EightWinds/Command/CommandBuffer.h"

#include "EightWinds/Pipeline/PipelineBase.h"

#include "EightWinds/GlobalPushConstant.h"

#include "EightWinds/Backend/RenderInfo.h"

#include <cassert>

namespace EWE{

    namespace Exec{
        struct ExecContext {
            std::vector<CommandInstruction> const& instructions;
            std::size_t& iterator;
            CommandBuffer& cmdBuf;
            std::vector<uint8_t> const& paramPool;

            //i dont really know what i want to do with the lower data
            //i can do assertions, which will be nice for preventing bugs
            //i could do static analysis for deeper bugs, or potentially even optimization
            //im not sure if its necessary, i might just leave it til i need it
            Pipeline* boundPipeline = nullptr;
            VkPipelineLayout boundLayout = VK_NULL_HANDLE;
            VkPipelineBindPoint currentBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

            //GlobalPushConstant push;
        };

        using CommandFunction = void(ExecContext& ctx);



        

        void BindPipeline(ExecContext& ctx);
        void BindDescriptor(ExecContext& ctx);
        void Push(ExecContext& ctx);
        void SetDynamicState(ExecContext& ctx);
        void BeginRender(ExecContext& ctx);
        void EndRender(ExecContext& ctx);
        void Draw(ExecContext& ctx);
        void DrawIndexed(ExecContext& ctx);
        void Dispatch(ExecContext& ctx);
        void Barrier(ExecContext& ctx);
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
        &Exec::BindPipeline,     //0
        &Exec::BindDescriptor,   //1
        &Exec::Push,             //4
        &Exec::SetDynamicState,  //5
        &Exec::BeginRender,      //6
        &Exec::EndRender,        //7
        &Exec::Draw,             //8
        &Exec::DrawIndexed,      //9
        &Exec::Dispatch,         //10
        &Exec::Barrier,          //11  
        &Exec::BeginLabel,       //12
        &Exec::EndLabel,         //13
        &Exec::IfStatement,      //14
        &Exec::LoopBegin,        //15
        &Exec::Switch,           //16
        &Exec::Case,             //17
        &Exec::Default,          //18
    };

    namespace Exec{
        //define command functions here
        void BindPipeline(ExecContext& ctx){
            Pipeline const* pipeline = reinterpret_cast<Pipeline const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            assert(pipeline != nullptr);
            ctx.boundLayout = pipeline->pipeLayout->vkLayout;
            ctx.currentBindPoint = pipeline->pipeLayout->bindPoint;

            vkCmdBindPipeline(ctx.cmdBuf, pipeline->pipeLayout->bindPoint, pipeline->vkPipe);
        }

        //i dont know if i bother putting this in the list
        void BindDescriptor(ExecContext& ctx){
            //i dont know where to store the texture descriptor set yet
            VkDescriptorSet* desc = nullptr;
            printf("i don't know where to store this yet\n");
            assert(false);
            vkCmdBindDescriptorSets(ctx.cmdBuf, ctx.currentBindPoint, ctx.boundPipeline->pipeLayout->vkLayout, 0, 1, desc, 0, nullptr);
        }

        void Push(ExecContext& ctx){
            auto* push = reinterpret_cast<GlobalPushConstant const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);

            vkCmdPushConstants(ctx.cmdBuf, ctx.boundLayout, VK_SHADER_STAGE_ALL, 0, sizeof(GlobalPushConstant), push);
        }

        void SetDynamicState(ExecContext& ctx){
            printf("this needs to be split out\n");
            assert(false);
        }

        void BeginRender(ExecContext& ctx){

            auto* data = reinterpret_cast<RenderInfo const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            vkCmdBeginRendering(ctx.cmdBuf, &data->renderingInfo);
        }
        void EndRender(ExecContext& ctx){
            vkCmdEndRendering(ctx.cmdBuf);
        }

        void Draw(ExecContext& ctx){
            auto* data = reinterpret_cast<VertexDrawParamPack const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            
            vkCmdDraw(ctx.cmdBuf, data->vertexCount, data->instanceCount, data->firstVertex, data->firstInstance);
        }

        void DrawIndexed(ExecContext& ctx){
            auto* data = reinterpret_cast<IndexDrawParamPack const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            
            vkCmdDrawIndexed(ctx.cmdBuf, data->indexCount, data->instanceCount, data->firstIndex, data->vertexOffset, data->firstInstance);
        }

        void Dispatch(ExecContext& ctx){
            auto* data = reinterpret_cast<uint32_t const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            
            //maybe add an if statement here, check if all groups are above 0?
            vkCmdDispatch(ctx.cmdBuf, data[0], data[1], data[2]);
        }

        void Barrier(ExecContext& ctx){
            assert(false && "this needs to be handled");
        }

        void BeginLabel(ExecContext& ctx){
            auto* data = reinterpret_cast<LabelParamPack const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            
            VkDebugUtilsLabelEXT labelUtil{};
            labelUtil.pLabelName = data->name;
            labelUtil.color[0] = data->red;
            labelUtil.color[1] = data->green;
            labelUtil.color[2] = data->blue;
            labelUtil.color[3] = 1.f;
            labelUtil.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            labelUtil.pNext = nullptr;
            vkCmdBeginDebugUtilsLabelEXT(ctx.cmdBuf, &labelUtil);
        }

        void EndLabel(ExecContext& ctx){
            vkCmdEndDebugUtilsLabelEXT(ctx.cmdBuf);
        }


        void IfStatement(ExecContext& ctx){
            bool const* condition = reinterpret_cast<bool const*>(&ctx.paramPool[ctx.instructions[ctx.iterator].paramOffset]);
            
            if(*condition){
                while(ctx.iterator < ctx.instructions.size()){
                    if(ctx.instructions[ctx.iterator].type == CommandInstruction::Type::EndIf){
                        ctx.iterator++;
                        return;
                    }
                    dispatchTable[ctx.instructions[ctx.iterator].type](ctx);
                }
                EWE_UNREACHABLE;
            }
        }

        
        void LoopBegin(ExecContext& ctx){
            //i need to wait and see how this pans out before i code it
            printf("not enabled currently\n");
        }

        void Switch(ExecContext& ctx){
            printf("not enabled currently\n");
        }
        void Case(ExecContext& ctx){
            printf("not enabled currently\n");
        }
        void Default(ExecContext& ctx){
            printf("not enabled currently\n");
        }

    } //namespace Exec

    void CommandExecutor::Execute(CommandBuffer& cmdBuf) const noexcept {
        std::size_t iterator = 0;
        Exec::ExecContext ctx{instructions, iterator, cmdBuf, paramPool};

        while(iterator < instructions.size()){
            //validate before creating the executor
            //assert(instructions[iterator].type != CommandInstruction::Type::EndIf && "unscoped endif");
            //assert(instructions[iterator].type != CommandInstruction::Type::LoopEnd && "unscoped loop end");
            //assert(instructions[iterator].type != CommandInstruction::Type::SwitchEnd && "unscoped switch end");
            dispatchTable[instructions[iterator].type](ctx);
        }
    }
} //namespace EWE