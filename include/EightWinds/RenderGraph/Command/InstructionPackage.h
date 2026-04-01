#pragma once

#include "EightWinds/RenderGraph/Command/Record.h"
#include "EightWinds/RenderGraph/Command/Instruction.h"

#include "EightWinds/RenderGraph/Command/IP_Helper.h"

#include <array>
#include <meta>

namespace EWE{
namespace Command{

	struct InstructionPackage{
		enum Type{
			Base,
			Raster,
		};

		static constexpr auto allowed_instructions = CollectInstructions();

		Command::Record record;
        std::string& name = record.name;
		
		virtual void Undefer(void* data, uint8_t frameIndex);
		virtual std::span<const Inst::Type> GetAllowedInstructions() {return std::span{allowed_instructions.data(), allowed_instructions.size()};}
	};
} //namespace Command
} //namepsace EWE