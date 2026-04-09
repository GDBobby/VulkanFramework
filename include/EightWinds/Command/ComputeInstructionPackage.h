#pragma once

#include "EightWinds/Command/InstructionPackage.h"
#include "EightWinds/Shader.h"

namespace EWE{
namespace Command{
	struct ComputePackage : public InstructionPackage{
        [[nodiscard]] ComputePackage();
		static constexpr auto allowed_instructions = {
			Inst::Push,
			Inst::Dispatch

            Inst::BeginLabel,
            Inst::EndLabel,

			Inst::If,
			Inst::EndIf
		};
        struct Payload{
            //do some smart hiding so they cant add vertex and compute and mesh and raytracing at the same time
            Shader* shader;
            //spec constants? for dispatch group sizes
        };
        Payload payload;


		std::span<const Inst::Type> GetAllowedInstructions() override final {return std::span{allowed_instructions.begin(), allowed_instructions.end()};}
	};
} //namespace Command
} //namepsace EWE