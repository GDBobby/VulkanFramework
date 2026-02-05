#pragma once

#include <cstdint>
#include <type_traits>
#include <fstream>


namespace EWE{
	namespace Stream{
		template<typename Stream, typename T>
		void Helper(Stream& stream, T& data){
			if constexpr(std::is_same_v<Stream, std::ifstream>){
				stream.read(reinterpret_cast<char*>(&data), sizeof(T));
			}
			else if constexpr(std::is_same_v<Stream, std::ofstream>){
				stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
			}
		}
	}
}