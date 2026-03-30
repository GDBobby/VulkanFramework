#pragma once

#include "EightWinds/RenderGraph/Command/Record.h"
#include "EightWinds/RenderGraph/Command/Instruction.h"

#include <array>
#include <meta>

namespace EWE{
	constexpr auto CollectInstructions(){
		std::array<Inst::Type, std::meta::enumerators_of(^^Inst::Type).size()> ret{};
		static constexpr auto members = std::define_static_array(std::meta::enumerators_of(^^Inst::Type));
		
		std::size_t index = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
		template for(constexpr auto mem : members){
			ret[index++] = [:mem:];
		}
#pragma GCC diagnostic pop
		return ret;
	}

	struct InstructionPackage{
		enum Type{
			Base,
			Raster,
		};

		static constexpr auto allowed_instructions = CollectInstructions();

		Command::Record record{};
        std::string& name = record.name;
		
		virtual void Undefer(void* data, uint8_t frameIndex);
		virtual std::span<const Inst::Type> GetAllowedInstructions() {return std::span{allowed_instructions.data(), allowed_instructions.size()};}
	};
} //namepsace EWE