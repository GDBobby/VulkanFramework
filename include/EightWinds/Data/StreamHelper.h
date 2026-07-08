#pragma once

#include <cstdint>
#include <type_traits>
#include <fstream>

#include <vector>
#include <string>


namespace EWE{

namespace Stream{
	enum class FileType{
		Read,
		Write,
		ReadWrite
	};
}//namespace Stream

	template <typename T>
	concept ReadableConcept = requires(T& s, char* buf, std::size_t n) {
		{ s.read(buf, n) };
	};

	template <typename T>
	concept WritableConcept = requires(T& s, const char* buf, std::size_t n) {
		{ s.write(buf, n) };
	};

	template <typename T>
	concept StreamConcept = ReadableConcept<T> || WritableConcept<T>;


namespace Stream{
	template<StreamConcept S>
	constexpr FileType DeduceFileType(){
		if constexpr(ReadableConcept<S> && WritableConcept<S>){
			return FileType::ReadWrite;
		}
		else if constexpr(ReadableConcept<S>){
			return FileType::Read;
		}
		else if constexpr(WritableConcept<S>){
			return FileType::Write;
		}
		else{
			static_assert(false);
		}
	}

	template<StreamConcept S>
	static constexpr FileType fileType = DeduceFileType<S>();

	template<StreamConcept StreamT>
	struct StreamOps;

	template<>
	struct StreamOps<std::ifstream> {

		static constexpr FileType ft = fileType<std::ifstream>;

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
		static constexpr FileType ft = fileType<std::ofstream>;
		template<typename T>
		static void Process(std::ofstream& s, T const& data) {
			s.write(reinterpret_cast<const char*>(&data), sizeof(T));
		}
		static void Process(std::ofstream& s, const char* data, std::size_t size) {
			s.write(data, size);
		}
	};

	template<StreamConcept StreamT>
	class Operator {
	public:
		static constexpr FileType ft = fileType<StreamT>;

		template <typename T>
		using QualifiedPtr = std::conditional_t<ft == FileType::Write, T const*, T*>;
		template <typename T>
		using QualifiedRef = std::conditional_t<ft == FileType::Write, T const&, T&>;

		[[nodiscard]] explicit Operator(StreamT& s) : stream(s) {}

		template<typename T>
		Operator& Process(T& data) {
			StreamOps<StreamT>::Process(stream, data);
			return *this;
		}
		template<typename T>
		Operator& Process(T* data, std::size_t size) {
			StreamOps<StreamT>::Process(stream, reinterpret_cast<QualifiedPtr<char>>(data), size);
			return *this;
		}
		
		template<typename T>
		requires(ft != Stream::FileType::ReadWrite)
		Operator& Process(QualifiedRef<std::vector<T>> data) {
			std::size_t size_buffer = data.size();
			StreamOps<StreamT>::Process(stream, size_buffer);
			if constexpr(ft == Stream::FileType::Read){
				data.resize(size_buffer);
			}
			if(size_buffer != 0){
				StreamOps<StreamT>::Process(stream, reinterpret_cast<QualifiedPtr<char>>(&data[0]), size_buffer * sizeof(T));
			}
			return *this;
		}
		
		Operator& Process(QualifiedRef<std::string> data)
		requires(ft != Stream::FileType::ReadWrite)
		{
			std::size_t size_buffer = data.size();
			StreamOps<StreamT>::Process(stream, size_buffer);
			if constexpr(ft == Stream::FileType::Read){
				data.resize(size_buffer);
			}

			if(size_buffer != 0){
				StreamOps<StreamT>::Process(stream, QualifiedPtr<char>(&data[0]), size_buffer);
			}
			return *this;
		}

		template<typename T>
		requires(ft == FileType::Read || ft == FileType::ReadWrite)
		T ReadValue(){
			T ret;
			Process(ret);
			return ret;
		}

	private:
		StreamT& stream;
	};

	template<StreamConcept StreamT>
	class OperatorRAII {
	public:
		static constexpr FileType ft = fileType<StreamT>;
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