#include "EightWinds/Command/ObjectInstructionPackage.h"

namespace EWE{
namespace Command{
    ObjectPackage::ObjectPackage()
    : InstructionPackage{Command::InstructionPackage::Object}
    {

    }

    ObjectPackage::DrawType ObjectPackage::GetDrawType() const{
        for(auto& inst : paramPool.instructions){
            switch(inst){
                case Inst::Draw:
                case Inst::DrawIndexed:
                case Inst::DrawIndirect:
                case Inst::DrawIndexedIndirect:
                case Inst::DrawIndirectCount:
                case Inst::DrawIndexedIndirectCount:
                    return DrawType::Vertex;
                case Inst::DrawMeshTasks:
                case Inst::DrawMeshTasksIndirect:
                case Inst::DrawMeshTasksIndirectCount:
                    return DrawType::Mesh;
                default: break;
            }
        }
        return DrawType::INVALID;
    };

} //namespace COmmand
} //namespace EWE