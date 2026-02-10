#pragma once

#include <cstdint>
#include <type_traits>
#include <fstream>


namespace EWE{
	namespace Stream{
		template<typename StreamT>
		struct StreamOps;

		template<>
		struct StreamOps<std::ifstream> {
			template<typename T>
			static void Process(std::ifstream& s, T& data) {
				s.read(reinterpret_cast<char*>(&data), sizeof(T));
			}
			static void Process(std::ifstream& s, char* data, std::size_t size) {
				s.read(data, size);
			}
		};

		template<>
		struct StreamOps<std::ofstream> {
			template<typename T>
			static void Process(std::ofstream& s, T& data) {
				s.write(reinterpret_cast<const char*>(&data), sizeof(T));
			}

			static void Process(std::ofstream& s, const char* data, std::size_t size) {
				s.write(data, size);
			}
		};

		template<typename StreamT>
		class Operator {
		public:
			[[nodiscard]] explicit Operator(StreamT& s) : stream(s) {}

			template<typename T>
			Operator& Process(T& data) {
				StreamOps<StreamT>::Process(stream, data);
				return *this;
			}

			template<typename T>
			Operator& Process(T* data, std::size_t size) {
				StreamOps<StreamT>::Process(stream, reinterpret_cast<char*>(data), size);
				return *this;
			}
			template<typename T>
			Operator& Process(T const* data, std::size_t size) const {
				StreamOps<StreamT>::Process(stream, reinterpret_cast<char const*>(data), size);
				return *this;
			}

		private:
			StreamT& stream;
		};

		template<typename StreamT>
		class OperatorRAII {
		public:
			[[nodiscard]] explicit OperatorRAII(std::string_view file_location) 
				: stream{ file_location } 
			{}

			template<typename T>
			OperatorRAII& Process(T& data) {
				StreamOps<StreamT>::Process(stream, data);
				return *this;
			}

			template<typename T>
			OperatorRAII& Process(T* data, std::size_t size) {
				StreamOps<StreamT>::Process(stream, reinterpret_cast<char*>(data), size);
				return *this;
			}
			template<typename T>
			OperatorRAII& Process(T const* data, std::size_t size) const {
				StreamOps<StreamT>::Process(stream, reinterpret_cast<char const*>(data), size);
				return *this;
			}

		private:
			StreamT stream;
		};
	} //namespace Stream
} //namespace EWE