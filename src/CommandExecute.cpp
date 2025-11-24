#include "EightWinds/Command/Execute.h"

#include <cassert>

namespace EWE{

    namespace Exec{
        struct ExecContext {
            std::vector<CommandInstruction> const& instructions;
            std::size_t& iterator;
            CommandBuffer& cmdBuf;
        };

        using CommandFunction = void(ExecContext& ctx);



        

        void BindPipeline(ExecContext& ctx);
        void BindDescriptor(ExecContext& ctx);
        void BindVertexBuffer(ExecContext& ctx);
        void BindIndexBuffer(ExecContext& ctx);
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
        void LoopBegin(ExecContext& ctx);
        void Switch(ExecContext& ctx);
        void Case(ExecContext& ctx);
        void Default(ExecContext& ctx);

        
        void IfStatement(ExecContext& ctx);
    } //namespace Exec

    static constexpr auto dispatchTable = std::array{
        &Exec::BindPipeline,     //0
        &Exec::BindDescriptor,   //1
        &Exec::BindVertexBuffer, //2
        &Exec::BindIndexBuffer,  //3
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

        void IfStatement(ExecContext& ctx){
            while(ctx.iterator < ctx.instructions.size()){
                if(ctx.instructions[ctx.iterator].type == CommandInstruction::Type::EndIf){
                    ctx.iterator++;
                    return;
                }
                dispatchTable[ctx.instructions[ctx.iterator].type](ctx);
            }
            EWE_UNREACHABLE;
        }
    } //namespace Exec

    void CommandExecutor::Execute(CommandBuffer& cmdBuf){
        std::size_t iterator = 0;
        Exec::ExecContext ctx{instructions, iterator, cmdBuf};

        while(iterator < instructions.size()){
            //validate before creating the executor
            //assert(instructions[iterator].type != CommandInstruction::Type::EndIf && "unscoped endif");
            //assert(instructions[iterator].type != CommandInstruction::Type::LoopEnd && "unscoped loop end");
            //assert(instructions[iterator].type != CommandInstruction::Type::SwitchEnd && "unscoped switch end");
            dispatchTable[instructions[iterator].type](ctx);
        }
    }
} //namespace EWE