#pragma once

#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/ObjectRasterConfig.h"

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

		ObjectRasterConfig config;

		std::span<const Inst::Type> GetAllowedInstructions() override final {return std::span{allowed_instructions.begin(), allowed_instructions.end()};}
	};
} //namespace Command
} //namepsace EWE