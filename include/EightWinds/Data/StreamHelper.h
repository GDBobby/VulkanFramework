#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
#include <fstream>

#include <vector>
#include <string>


namespace EWE {

    namespace Stream {
        enum class FileType {
            Read,
            Write,
            ReadWrite
        };
    } //namespace Stream

    template<typename T>
    concept ReadableConcept = requires(T& s, char* buf, std::size_t n) {
        { s.read(buf, n) };
    };

    template<typename T>
    concept WritableConcept = requires(T& s, const char* buf, std::size_t n) {
        { s.write(buf, n) };
    };

    template<typename T>
    concept StreamConcept = ReadableConcept<T> || WritableConcept<T>;


    namespace Stream {
        template<StreamConcept S>
        constexpr FileType DeduceFileType() {
            if constexpr (ReadableConcept<S> && WritableConcept<S>) {
                return FileType::ReadWrite;
            }
            else if constexpr (ReadableConcept<S>) {
                return FileType::Read;
            }
            else if constexpr (WritableConcept<S>) {
                return FileType::Write;
            }
            else {
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
            static void Process(std::ifstream& s, char* data, std::size_t size) { s.read(data, size); }
        };

        template<>
        struct StreamOps<std::ofstream> {
            static constexpr FileType ft = fileType<std::ofstream>;
            template<typename T>
            static void Process(std::ofstream& s, T const& data) {
                s.write(reinterpret_cast<const char*>(&data), sizeof(T));
            }
            static void Process(std::ofstream& s, const char* data, std::size_t size) { s.write(data, size); }
        };

        template<StreamConcept StreamT>
        class Operator {
        public:
            static constexpr FileType ft = fileType<StreamT>;

            template<typename T>
            using QualifiedPtr = std::conditional_t<ft == FileType::Write, T const*, T*>;
            template<typename T>
            using QualifiedRef = std::conditional_t<ft == FileType::Write, T const&, T&>;

            [[nodiscard]] explicit Operator(StreamT& s) : stream(s) {}

            template<typename T>
                requires(!requires(T& t) {
                    t.begin();
                    t.end();
                    t.size();
                })
            Operator& Process(T& data) {
                StreamOps<StreamT>::Process(stream, data);
                return *this;
            }
            template<typename T>
                requires(!requires(T& t) {
                    t.begin();
                    t.end();
                    t.size();
                })
            Operator& Process(T* data, std::size_t size) {
                StreamOps<StreamT>::Process(stream, reinterpret_cast<QualifiedPtr<char>>(data), size);
                return *this;
            }

            template<typename T>
                requires(ft != Stream::FileType::ReadWrite)
            Operator& Process(QualifiedRef<std::vector<T>> data) {
                std::size_t size_buffer = data.size();
                StreamOps<StreamT>::Process(stream, size_buffer);
                if constexpr (ft == Stream::FileType::Read) {
                    data.resize(size_buffer);
                }
                if (size_buffer != 0) {
                    StreamOps<StreamT>::Process(
                        stream,
                        reinterpret_cast<QualifiedPtr<char>>(&data[0]),
                        size_buffer * sizeof(T)
                    );
                }
                return *this;
            }

            Operator& Process(QualifiedRef<std::string> data)
                requires(ft != Stream::FileType::ReadWrite)
            {
                std::size_t size_buffer = data.size();
                StreamOps<StreamT>::Process(stream, size_buffer);
                if (size_buffer != 0) {
                    if constexpr (ft == Stream::FileType::Read) {
                        data.resize(size_buffer);
                    }
                    StreamOps<StreamT>::Process(stream, QualifiedPtr<char>(&data[0]), size_buffer);
                }
                return *this;
            }

            template<typename T>
                requires(ft == FileType::Read || ft == FileType::ReadWrite)
            T ReadValue() {
                T ret;
                Process(ret);
                return ret;
            }

            int64_t GetFilePos() {
                if constexpr (ft == FileType::Read || ft == FileType::ReadWrite) {
                    return stream.tellg();
                }
                else if constexpr (ft == FileType::Write) {
                    return stream.tellp();
                }
                return std::numeric_limits<int64_t>::max();
            }

            void Seek(std::ios::seekdir seekdir, int64_t offset = 0) {
                if constexpr (ft == FileType::Read || ft == FileType::ReadWrite) {
                    stream.seekg(offset, seekdir);
                }
                else if constexpr (ft == FileType::Write) {
                    stream.seekp(offset, seekdir);
                }
            }

            int64_t CheckDistance(std::ios::seekdir lhs, std::ios::seekdir rhs) {
                EWE_ASSERT(lhs != std::ios::end && rhs != std::ios::beg);
                if (lhs == rhs) {
                    return 0;
                }

                const int64_t original_pos = GetFilePos();

                int64_t lh_pos = 0;
                if (lhs == std::ios::cur) {
                    lh_pos = original_pos;
                }

                int64_t rh_pos = original_pos;
                if (rhs == std::ios::end) {
                    Seek(std::ios::end);
                    rh_pos = GetFilePos();
                    Seek(std::ios::beg, original_pos);
                }
                return rh_pos - lh_pos;
            }


            StreamT& stream;
        };

        template<StreamConcept StreamT>
        class OperatorRAII {
        public:
            static constexpr FileType ft = fileType<StreamT>;
            [[nodiscard]] explicit OperatorRAII(std::string_view file_location) : stream{file_location} {}

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