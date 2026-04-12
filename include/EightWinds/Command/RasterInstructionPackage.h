#pragma once

#include "EightWinds/Command/InstructionPackage.h"

#include "EightWinds/TaskRasterConfig.h"

namespace EWE{
namespace Command{
	struct RasterPackage : public InstructionPackage{
        [[nodiscard]] RasterPackage();
		static constexpr auto allowed_instructions = {
		};
        TaskRasterConfig payload;


		std::span<const Inst::Type> GetAllowedInstructions() override final {return std::span{allowed_instructions.begin(), allowed_instructions.end()};}
	};
} //namespace Command
} //namepsace EWE