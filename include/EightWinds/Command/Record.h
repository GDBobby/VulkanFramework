#pragma once

#include "EightWinds/VulkanHeader.h"

#include "EightWinds/Command/InstructionPointer.h"
#include "EightWinds/Command/Instruction.h"

#include "EightWinds/Data/RuntimeArray.h"

#include <string_view>
#include <vector>
#include <filesystem>


namespace EWE{
    //forward declared just to save a bit of compile time
    //the implementation doesnt matter here
    struct Pipeline;
    struct PipelineBarrier;
    struct CommandBuffer;
    struct GlobalPushConstant_Raw;
    struct RenderInfo;
    struct LogicalDevice;

    //if i want compile time optimization, i need to change how the data handles are done
    //i dont think InstructionPointer is goign to play nicely with constexpr, and
    //vectors dont work with constexpr either, which is how the param_pool is currently setup.
    //the parampool could probably be a span tho

    //i need a merge option, so that tasks can be modified more easily
    //and commandrecord can be passed around as a pre-package kind of thing
    
namespace Command{
    struct Record{
        [[nodiscard]] Record() = default;
        [[nodiscard]] Record(std::filesystem::path const& file_location);

        //idk how to handle copies right now
        Record(Record const&) = delete;
        Record& operator=(Record const&) = delete;
        Record(Record&&) = delete;
        Record& operator=(Record&&) = delete;

        std::string name{};
        bool hasBeenCompiled = false;
        std::vector<Instruction> records{};

        void Append(Record const& other);
        Record& operator<<(Record const& other){
            Append(other);
            return *this;
        }

        //a pointer is returned so that
        InstructionPointerAdjuster* Add(Inst::Type type, bool external_memory = false);

        template<Inst::Type IType>
        requires (std::meta::is_complete_type(^^ParamPack<IType>))
        auto* Add(bool external_memory = false) {
            auto* inst = Add(IType, external_memory);
            if constexpr(IType == Inst::Push){
                return inst->CastTo<GlobalPushConstant_Raw>();
            }
            else if constexpr(IType == Inst::BeginRender){
                return inst->CastTo<VkRenderingInfo>();
            }
            else{
                return inst->template CastTo<ParamPack<IType>>();
            }
        }

        template<Inst::Type IType>
        requires(!std::meta::is_complete_type(^^ParamPack<IType>))
        void Add(bool external_memory = false) {
            Add(IType, external_memory);
        }

        std::size_t CalculateSize() const noexcept;
        void FixDeferred(const PerFlight<std::size_t> pool_address) noexcept;
#if EWE_DEBUG_BOOL
        bool ValidateInstructions() const;
#endif

        static void WriteInstructions(std::string_view file_location, const std::span<const Inst::Type> instructions);
        void WriteInstructions(std::string_view file_location);
        static RuntimeArray<Inst::Type> ReadInstructions(std::string_view file_location);
    };
}//namespace Command
}//namespace EWE