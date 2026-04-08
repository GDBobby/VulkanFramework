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
		};

		[[nodiscard]] InstructionPackage();

		static constexpr auto allowed_instructions = CollectInstructions();

        std::string name;
		ParamPool paramPool;
		const Type type;
		
		virtual std::span<const Inst::Type> GetAllowedInstructions() {return std::span{allowed_instructions.data(), allowed_instructions.size()};}

	protected:
		[[nodiscard]] InstructionPackage(Type type);	
	};
} //namespace Command
} //namepsace EWE