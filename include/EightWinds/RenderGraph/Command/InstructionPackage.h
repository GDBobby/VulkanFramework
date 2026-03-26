#pragma once

#include "EightWinds/RenderGraph/Command/Record.h"
#include "EightWinds/RenderGraph/Command/Instruction.h"

namespace EWE{
	struct InstructionPackage{
        //the name for this package is equal to the name of the record
		Command::Record record;
        std::string& name;
		
		virtual void Undefer(void* data, uint8_t frameIndex);
	};
} //namepsace EWE