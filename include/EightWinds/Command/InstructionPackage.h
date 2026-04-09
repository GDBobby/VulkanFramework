#pragma once

#include "EightWinds/Command/InstructionType.h"
#include "EightWinds/Command/InstructionPointer.h"

#include "EightWinds/Command/IP_Helper.h"
#include "EightWinds/Command/ParamPool.h"

#include <meta>

namespace EWE{
namespace Command{

	struct InstructionPackage{
		enum Type : uint8_t{
			Base,
			Object,
			Compute,
		};

		[[nodiscard]] InstructionPackage();

		static constexpr auto allowed_instructions = {

            Inst::BeginLabel,
            Inst::EndLabel,

			Inst::If,
			Inst::EndIf
		};

		const Type type;
        std::string name;
		ParamPool paramPool;
		
		virtual std::span<const Inst::Type> GetAllowedInstructions() {return std::span{allowed_instructions.begin(), allowed_instructions.end()};}
	
	protected:
		[[nodiscard]] InstructionPackage(Type type);	
	};
} //namespace Command
} //namepsace EWE