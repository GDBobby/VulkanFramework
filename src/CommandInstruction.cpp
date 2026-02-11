#include "EightWinds/RenderGraph/Command/Instruction.h"

#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/GlobalPushConstant.h"
#include "EightWinds/Backend/RenderInfo.h"

namespace EWE{
    uint64_t Command::Instruction::GetParamSize(Type type) noexcept {
        switch (type) {
            case Type::BindPipeline: return sizeof(ParamPack::Pipeline);
            case Type::BindDescriptor: return sizeof(VkDescriptorSet);
            case Type::PushConstant: return sizeof(GlobalPushConstant_Raw);
            case Type::BeginRender: return sizeof(VkRenderingInfo);
            case Type::EndRender: return 0;
            
            case Type::Draw: return sizeof(ParamPack::VertexDraw);
            case Type::DrawIndexed: return sizeof(ParamPack::IndexDraw);
            case Type::Dispatch: return sizeof(ParamPack::Dispatch);
            case Type::DrawMeshTasks: return sizeof(ParamPack::DrawMeshTasks);
            //raytracing here
            
            case Type::DrawIndirect: return sizeof(ParamPack::DrawIndirect);
            case Type::DrawIndexedIndirect: return sizeof(ParamPack::DrawIndirect);
            case Type::DispatchIndirect: return sizeof(ParamPack::DispatchIndirect);
            case Type::DrawMeshTasksIndirect: return sizeof(ParamPack::DrawIndirect);
            
            case Type::DrawIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
            case Type::DrawIndexedIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
            case Type::DrawMeshTasksIndirectCount: return sizeof(ParamPack::DrawIndirectCount);
            //case Type::PipelineBarrier: return 0;
            case Type::DS_ViewportScissor: return sizeof(ParamPack::ViewportScissor);
            case Type::DS_ViewportScissorWithCount: return sizeof(ParamPack::ViewportScissorWithCount);
            case Type::BeginLabel: return sizeof(ParamPack::Label);
            case Type::EndLabel: return 0;
            case Type::If: return sizeof(bool);
            //other loop controls
            case Type::EndIf: return 0;

            //bunch of dynamic state that i havent got to yet
        }
        EWE_UNREACHABLE;
        return 0;
    }
} //namespace EWE