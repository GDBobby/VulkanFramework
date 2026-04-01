#pragma once

#include "EightWinds/Command/Instruction.h"

#include <array>
#include <meta>

namespace EWE{
namespace Command{
	static constexpr auto CollectInstructions(){
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
} //namespace Command
} //namespace EWE