#pragma once

#include "EightWinds/RenderGraph/Command/InstructionPackage.h"
#include "EightWinds/ObjectRasterConfig.h"

namespace EWE{

	struct RasterPackage : public InstructionPackage{
		ObjectRasterConfig config;
		
		virtual void Record(Command::Record& record) override = 0;
		virtual void Undefer(void* data);
	};
} //namepsace EWE