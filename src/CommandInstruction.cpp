#include "EightWinds/Command/Instruction.h"

#include "EightWinds/Pipeline/PipelineBase.h"
#include "EightWinds/GlobalPushConstant.h"
#include "EightWinds/Backend/RenderInfo.h"

namespace EWE{
    uint64_t CommandInstruction::GetParamSize(Type type) noexcept {
        switch (type) {
            case Type::BindPipeline: return sizeof(Pipeline*);
            case Type::BindDescriptor: return sizeof(VkDescriptorSet);
            case Type::PushConstant: return sizeof(GlobalPushConstant);
            case Type::BeginRender: return sizeof(RenderInfo);
            case Type::EndRender: return 0;
            case Type::Draw: return sizeof(VertexDrawParamPack);
            case Type::DrawIndexed: return sizeof(IndexDrawParamPack);
            case Type::Dispatch: return sizeof(DispatchParamPack);
            case Type::PipelineBarrier: return 0;
            case Type::DS_ViewportScissor: return sizeof(ViewportScissorParamPack);
            case Type::DS_ViewportScissorWithCount: return sizeof(ViewportScissorWithCountParamPack);
            case Type::BeginLabel: return sizeof(LabelParamPack);
            case Type::EndLabel: return 0;
            case Type::If: return sizeof(bool);
            //other loop controls
            case Type::EndIf: return 0;

            //bunch of dynamic state that i havent got to yet
            EWE_UNREACHABLE;
            return 0;
        }
    }
} //namespace EWE