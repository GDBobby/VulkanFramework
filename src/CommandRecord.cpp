#include "EightWinds/RenderGraph/Command/InstructionPointer.h"
#include "EightWinds/RenderGraph/Command/Record.h"
#include "EightWinds/VulkanHeader.h"

namespace EWE{
    namespace Command{
        Record::Record(std::string_view file_location) 
        : name{file_location}
        {
            auto const& temp_insts = ReadInstructions(file_location);
            for(auto const& inst : temp_insts){
                Add(inst);
            }
        }

        std::size_t Record::CalculateSize() const noexcept{
            std::size_t ret = 0;
            for(auto const& inst : records){
                ret += Instruction::GetParamSize(inst.type);
            }
            return ret;
        }

        #if EWE_DEBUG_BOOL
        bool Record::ValidateInstructions() const{
            int64_t current_if_depth = 0;
            int64_t current_label_depth = 0;
            std::vector<uint32_t> if_command_length{};

            bool pipeline_bound = false;

            bool currently_rendering = false;
            for(auto& rec : records){
                switch(rec.type){
                    case Instruction::Type::If:
                        current_if_depth++;
                        if_command_length.push_back(0);
                        EWE_ASSERT(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        continue;
                    case Instruction::Type::EndIf:
                        EWE_ASSERT(current_if_depth >= 1);
                        EWE_ASSERT(if_command_length.size() == current_if_depth); //unnecessary, sanity check
                        current_if_depth--;
                        if(if_command_length.back() == 0){
                            //if its 0, the if and endif instruction can be erased
                        }
                        if_command_length.pop_back();
                        continue;
                    case Instruction::Type::BeginLabel:
                        current_label_depth++;
                        break;
                    case Instruction::Type::EndLabel:
                        current_label_depth--;
                        EWE_ASSERT(current_label_depth >= 0);
                        break;

                    case Instruction::Type::BeginRender:
                        EWE_ASSERT(!currently_rendering);
                        currently_rendering = true;
                        break;
                    case Instruction::Type::EndRender:
                        EWE_ASSERT(currently_rendering);
                        currently_rendering = false;
                        break;
                    case Instruction::Type::BindPipeline:
                        pipeline_bound = true;
                        break;

                    case Instruction::Type::BindDescriptor:
                    case Instruction::Type::Push:
                    case Instruction::Type::DS_Viewport:
                    case Instruction::Type::DS_ViewportCount:
                    case Instruction::Type::DS_Scissor:
                    case Instruction::Type::DS_ScissorCount:
                    case Instruction::Type::Draw:
                    case Instruction::Type::DrawIndexed:
                    case Instruction::Type::Dispatch:
                    case Instruction::Type::DrawMeshTasks:
                    case Instruction::Type::DrawIndirect:
                    case Instruction::Type::DrawIndexedIndirect:
                    case Instruction::Type::DispatchIndirect:
                    case Instruction::Type::DrawMeshTasksIndirect:
                    case Instruction::Type::DrawIndirectCount:
                    case Instruction::Type::DrawIndexedIndirectCount:
                    case Instruction::Type::DrawMeshTasksIndirectCount:

                        EWE_ASSERT(pipeline_bound);
                        break;

                    default:
                        break;
                }
                if(if_command_length.size() > 0){
                    if_command_length.back()++;
                }

            }
            EWE_ASSERT(current_if_depth == 0);
            EWE_ASSERT(current_label_depth == 0);
            return true;
            
        }
        #endif

        /*
        void Record::Compile(GPUTask* constructionPointer, LogicalDevice& logicalDevice, Queue& queue) noexcept {
            EWE_ASSERT(!hasBeenCompiled);
            const uint64_t full_data_size = records.back().paramOffset + Instruction::GetParamSize(records.back().type);

            GPUTask ret{logicalDevice, queue};
            ret.commandExecutor.instructions = records;
            ret.commandExecutor.paramPool.resize(full_data_size);
            const std::size_t param_pool_address = reinterpret_cast<std::size_t>(ret.commandExecutor.paramPool.data());
            for(auto& def_ref : deferred_references){
                def_ref->data += param_pool_address;
                //we convert the initial offset to a real pointer into the paramPool
            }
            for(auto& push_off : push_offsets){
                std::size_t temp_addr = reinterpret_cast<std::size_t>(push_off);
                ret.pushTrackers.emplace_back(reinterpret_cast<GlobalPushConstant*>(temp_addr + param_pool_address));
            }
            uint64_t blitIndex = 0;
            for (auto const& inst : records) {
                if (inst.type == Instruction::Type::BeginRender) {
                    ret.renderTracker = new RenderTracker();
                }
                if(inst.type == Instruction::Type::Blit) {
                    auto& blitBack = ret.blitTrackers.emplace_back();
                    blitBack.dstImage.resource = nullptr;
                    blitBack.srcImage.resource = nullptr;
                }
            }

            //all validations will be here
            //theres some non-validation stuff here, like collapsing empty branches
            //maybe split out optimization into a different loop
        #if EWE_DEBUG_BOOL
            EWE_ASSERT(ValidateInstructions(records));
        #endif
            hasBeenCompiled = true;

            return ret;
        }
        */

        InstructionPointerAdjuster* Record::Add(Instruction::Type type, bool external_memory /*= false*/) {
            if(Instruction::GetParamSize(type) == 0){
                records.push_back(
                    Instruction{type, nullptr}
                );
            }
            else{
                records.push_back(
                    Instruction{type, new InstructionPointerAdjuster()}
                );
                records.back().instruction_pointer->internal = !external_memory;
            }
            return records.back().instruction_pointer;
        }

        void Record::FixDeferred(const PerFlight<std::size_t> pool_address) noexcept {

            std::size_t current_offset = 0;
            for(auto& inst : records){
                if(inst.instruction_pointer){
                    if(inst.instruction_pointer->internal){
                        for(uint8_t frame = 0; frame < max_frames_in_flight; frame++){
                            inst.instruction_pointer->data[frame] = pool_address[frame] + current_offset;
                        }
                        inst.instruction_pointer->adjusted = true;
                        current_offset += Instruction::GetParamSize(inst.type);
                    }
                }
            }
#if EWE_DEBUG_BOOL
            EWE_ASSERT(current_offset = CalculateSize());
#endif
        }

        static constexpr std::size_t record_file_version = 0;
        void Record::WriteInstructions(std::string_view file_location, const std::span<const Instruction::Type> instructions){
            std::ofstream stream_base{file_location.data(), std::ios::binary};
            EWE_ASSERT(stream_base.is_open());
            Stream::Operator<std::ofstream> stream{ stream_base };

            std::size_t temp_buffer = record_file_version;
            stream.Process(temp_buffer);
            EWE_ASSERT(temp_buffer == record_file_version);

            temp_buffer = instructions.size();
            stream.Process(temp_buffer);

            for(auto& inst : instructions){
                stream.Process(inst);
            }
            stream_base.close();
        }

        void Record::WriteInstructions(std::string_view file_location){
            
            RuntimeArray<Instruction::Type> instructions{records.size()};
            for(std::size_t i = 0; i < records.size(); i++){
                instructions[i] = records[i].type;
            }
            WriteInstructions(file_location, instructions);
        }

        RuntimeArray<Instruction::Type> Record::ReadInstructions(std::string_view file_location){
            std::ifstream stream_base{file_location.data(), std::ios::binary};
            EWE_ASSERT(stream_base.is_open());
            Stream::Operator<std::ifstream> stream{ stream_base };

            std::size_t temp_buffer = record_file_version;
            stream.Process(temp_buffer);
            EWE_ASSERT(temp_buffer == record_file_version);

            stream.Process(temp_buffer);

            RuntimeArray<Instruction::Type> ret{temp_buffer};

            for (std::size_t i = 0; i < temp_buffer; i++){
                stream.Process(ret[i]);
            }
            stream_base.close();
            return ret;
        }
    }//namespace Command
} //namespace EWE