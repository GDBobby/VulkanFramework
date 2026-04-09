#pragma once

#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/ObjectRasterConfig.h"
#include "EightWinds/Shader.h"

namespace EWE{
namespace Command{
	struct ObjectPackage : public InstructionPackage{
        [[nodiscard]] ObjectPackage();
		static constexpr auto allowed_instructions = {
			Inst::Push,
			Inst::Draw,
			Inst::DrawIndexed,
            Inst::DrawMeshTasks,
            
            Inst::DrawIndirect,
            Inst::DrawIndexedIndirect,
            Inst::DrawMeshTasksIndirect,
            
            Inst::DrawIndirectCount,
            Inst::DrawIndexedIndirectCount,
            Inst::DrawMeshTasksIndirectCount,

            Inst::DS_Viewport,
            Inst::DS_ViewportCount,
            Inst::DS_Scissor,
            Inst::DS_ScissorCount,

            Inst::BeginLabel,
            Inst::EndLabel,

			Inst::If,
			Inst::EndIf
		};
        struct Payload{
            //do some smart hiding so they cant add vertex and compute and mesh and raytracing at the same time
            std::array<Shader*, Shader::Stage::Bits::COUNT> shaders;
            ObjectRasterConfig config;
        };
        Payload payload;


		std::span<const Inst::Type> GetAllowedInstructions() override final {return std::span{allowed_instructions.begin(), allowed_instructions.end()};}
	};
} //namespace Command
} //namepsace EWE