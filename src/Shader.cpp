#include "EightWinds/Shader.h"

#include "EightWinds/Backend/ShaderVariable.h"
#include "EightWinds/Reflect/Enum.h"
#include "EightWinds/Reflect/Reflect.h"

#include "spirv.hpp"
#include "spirv_common.hpp"
#include "spirvcross/spirv_reflect.hpp"

#include <fstream>

#include <iostream>

#define SANITY_CHECK
#ifdef SANITY_CHECK
#include <string.h>
#endif

#include <filesystem>

#if EWE_DEBUG_BOOL
#define SPV_REFLECT(func, ...) {auto result = func(__VA_ARGS__); \
	if (result != SPV_REFLECT_RESULT_SUCCESS) {Log::Error("failed to read reflected shader data - %s - %s\n", #func, magic_enum::enum_name(result).data());}}
#endif

namespace EWE {

#define c_char_cast(x) reinterpret_cast<const char*>(x)
#define char_cast(x) reinterpret_cast<char*>(x)
	void ShaderMeta::WriteToFile(std::filesystem::path const& path){
		std::ofstream outFile{path, std::ios::binary};
		auto temp_buffer = file_version;
		outFile.write(c_char_cast(&temp_buffer), sizeof(temp_buffer));
		auto const buf_size = buffer_written_to.Size();
		auto const tex_size = texture_written_to.Size();
		outFile.write(c_char_cast(&tex_size), sizeof(tex_size));
		outFile.write(c_char_cast(&buf_size), sizeof(buf_size));
		outFile.write(c_char_cast(buffer_written_to.Data()), sizeof(bool) * buffer_written_to.Size());
		outFile.write(c_char_cast(texture_written_to.Data()), sizeof(bool) * texture_written_to.Size());

		outFile.close();
	}
	bool ShaderMeta::ReadFromFile(std::filesystem::path const& path){
		std::ifstream inFile{path, std::ios::binary};
		auto temp_buffer = file_version;
		inFile.read(char_cast(&temp_buffer), sizeof(temp_buffer));
		if(temp_buffer != file_version){
			inFile.close();
			return false;
		}

		std::size_t buf_size;
		std::size_t tex_size;
		inFile.read(char_cast(&buf_size), sizeof(buf_size));
		inFile.read(char_cast(&tex_size), sizeof(tex_size));
		buffer_written_to.ClearAndResize(buf_size);
		texture_written_to.ClearAndResize(tex_size);

		inFile.read(char_cast(buffer_written_to.Data()), sizeof(bool) * buffer_written_to.Size());
		inFile.read(char_cast(texture_written_to.Data()), sizeof(bool) * texture_written_to.Size());

		inFile.close();
		return true;
	}

	PushConstant::PushConstant() 
	: offset{0}, size{0},
		buffers{}, textures{} 
	{}

	constexpr ShaderVariable::Type ConvertVariableType(spirv_cross::SPIRType::BaseType spirv_var_type) {
		switch (spirv_var_type) {
			case spirv_cross::SPIRType::Boolean:        return ShaderVariable::Type::Bool;

			case spirv_cross::SPIRType::SByte:          return ShaderVariable::Type::Int8;
			case spirv_cross::SPIRType::UByte:          return ShaderVariable::Type::UInt8;
			case spirv_cross::SPIRType::Short:          return ShaderVariable::Type::Int16;
			case spirv_cross::SPIRType::UShort:         return ShaderVariable::Type::UInt16;
			case spirv_cross::SPIRType::Int:            return ShaderVariable::Type::Int32;
			case spirv_cross::SPIRType::UInt:           return ShaderVariable::Type::UInt32;
			case spirv_cross::SPIRType::Int64:          return ShaderVariable::Type::Int64;
			case spirv_cross::SPIRType::UInt64:         return ShaderVariable::Type::UInt64;

			case spirv_cross::SPIRType::Half:           return ShaderVariable::Type::Float16;
			case spirv_cross::SPIRType::Float:          return ShaderVariable::Type::Float32;
			case spirv_cross::SPIRType::Double:         return ShaderVariable::Type::Float64;

			case spirv_cross::SPIRType::Image:          return ShaderVariable::Type::Image;
			case spirv_cross::SPIRType::Sampler:        return ShaderVariable::Type::Sampler;
			case spirv_cross::SPIRType::SampledImage:   return ShaderVariable::Type::SampledImage;
			case spirv_cross::SPIRType::Struct:         return ShaderVariable::Type::Struct;
			case spirv_cross::SPIRType::AtomicCounter:  return ShaderVariable::Type::AtomicCounter;

			case spirv_cross::SPIRType::Void:           return ShaderVariable::Type::Void;
			case spirv_cross::SPIRType::Unknown: {[[fallthrough]];}
			default:                                    return ShaderVariable::Type::Unknown;
		}
	}

	void PrintAllDecorations(std::string_view name, spirv_cross::Bitset const& bitset){
		for(auto& dec : Reflect::Enum::enum_data<spv::Decoration>){
			if(bitset.get(dec.GetUnderlyingValue())){
				Log::Debug("%s has decoration[%s]\n", name.data(), Reflect::Enum::ToString(dec.value).data());
			}
		}
	}

	void ParsePushBufferAddress(spirv_cross::Compiler const& compiler, Shader& shader, spirv_cross::SPIRType const& push_type, uint8_t buffer_member_offset, std::string const& push_mem_name){
		auto const& buf_id = push_type.member_types[buffer_member_offset];
		auto const& buf_type = compiler.get_type(buf_id);
		EWE_ASSERT(buf_type.pointer);

		std::size_t member_offset = 0;
		if(shader.pushRange.buffers.size() > 0){
			member_offset = shader.pushRange.buffers.back().offset + shader.pushRange.buffers.back().size;
		}

		if(buf_type.op == spirv_cross::OpTypeArray){
			EWE_ASSERT(buf_type.array.size() == 1); //not supporting array of arrays here
			for(uint8_t i = 0; i < buf_type.array[0]; i++){
				const std::string buf_name = push_mem_name + std::string("[") + std::to_string(i) + "]";

				shader.pushRange.buffers.push_back(
					PushConstant::Member{
						.name = buf_name,
						.offset = member_offset,
						.size = 8, //ptr in bytes
						.type = PushConstant::Member::Buffer
					}
				);
				member_offset += 8;
			}
		}
		else if (buf_type.op == spirv_cross::OpTypePointer) {
			/*
			ret->name = compiler.get_member_name(var_id, 0);

			ret->pointer = true;
			ret->pointer_depth = var_type.pointer_depth;
			ret->forward_pointer = var_type.forward_pointer;
			*/


			shader.pushRange.buffers.push_back(
				PushConstant::Member{
					.name = push_mem_name,
					.offset = member_offset,
					.size = 8, //ptr in bytes
					.type = PushConstant::Member::Buffer
				}
			);

			EWE_ASSERT(buf_type.basetype == spirv_cross::SPIRType::Struct);
			auto const& member_type_id = buf_type.member_types[0];
			auto const& member_type = compiler.get_type(member_type_id);
			if (member_type.op == spirv_cross::OpTypeRuntimeArray && member_type.basetype == spirv_cross::SPIRType::Struct) {
				//inspect the member type's member type
				//auto const& m_mt_id = member_type.member_types[0];
				//auto const& m_mt = compiler.get_type(m_mt_id);

				auto const& parent_type = compiler.get_type(buf_type.parent_type);
				auto const& parent_child = compiler.get_type(parent_type.member_types[0]);
				auto const& parent_child_name_v1 = compiler.get_name(parent_child.self);
				auto const& parent_child_name_v2 = compiler.get_member_name(parent_type.self, 0);

				auto const& buf_mem_name = compiler.get_member_name(buf_type.self, 0);
				auto const& buf_name = compiler.get_name(buf_type.parent_type);
				auto const& struct_name = compiler.get_member_name(member_type.self, 0);
				auto const& struct_size= compiler.get_declared_struct_size(member_type);
				shader.bufferReferences.push_back(
					Shader::BufferReference(
						parent_child_name_v1,
						struct_size
					)
				);
			}
			else {
				/*
				for (auto& mmt_id : member_type.member_types) {
					auto const& mmt_type = compiler.get_type(mmt_id);
					Log::Debug("%s:%s\n", Reflect::Enum::ToString(mmt_type.basetype).data(), compiler.get_name(mmt_type.self).c_str());
				}
				*/

				auto const& parent_type = compiler.get_type(buf_type.parent_type);
				auto const& parent_name_v1 = compiler.get_name(buf_type.parent_type);
				auto const& parent_name_v2 = compiler.get_name(parent_type.self);
				auto const& parent_child = compiler.get_type(parent_type.member_types[0]);
				auto const& parent_child_name_v1 = compiler.get_name(parent_child.self);
				auto const& parent_child_name_v2 = compiler.get_member_name(parent_type.self, 0);

				auto const& buf_mem_name = compiler.get_member_name(buf_type.self, 0);
				if (parent_child.basetype == spirv_cross::SPIRType::Struct) {
					auto const& second_struct_size = compiler.get_declared_struct_size(parent_child);
					Log::Debug("first member[%s] struct size : %u\n", parent_child_name_v1.c_str(), second_struct_size);
				}
				auto const& struct_size= compiler.get_declared_struct_size(parent_type);
				shader.bufferReferences.push_back(
					Shader::BufferReference(
						parent_name_v1,
						struct_size
					)
				);
			}
		}
		else{
			shader.pushRange.buffers.push_back(
				PushConstant::Member{
					.name = push_mem_name,
					.offset = member_offset,
					.size = 8, //ptr in bytes
					.type = PushConstant::Member::Buffer
				}
			);
		}
	}
	void ParsePushTextureIndex(spirv_cross::Compiler const& compiler, Shader& shader, spirv_cross::SPIRType const& push_type, uint8_t tex_member_offset, std::string const& push_mem_name){
		auto const& var_id = push_type.member_types[tex_member_offset];
		auto const& tex_type = compiler.get_type(var_id);
		EWE_ASSERT(tex_type.basetype == spirv_cross::SPIRType::Int);
		//auto const& tex_name = compiler.get_name(tex_type.self);

		std::size_t member_offset = 0;
		if(shader.pushRange.textures.size() == 0){
			if(shader.pushRange.buffers.size() > 0){
				member_offset = shader.pushRange.buffers.back().offset + shader.pushRange.buffers.back().size;
			}
		}
		else{
			member_offset = shader.pushRange.textures.back().offset + shader.pushRange.textures.back().size;
		}

		if(tex_type.op == spirv_cross::OpTypeArray){
			EWE_ASSERT(tex_type.array.size() == 1); //not supporting array of arrays here
			for(uint8_t i = 0; i < tex_type.array[0]; i++){

				const std::string tex_name = push_mem_name + std::string("[") + std::to_string(i) + "]";

				shader.pushRange.textures.push_back(
					PushConstant::Member{
						.name = tex_name,
						.offset = member_offset,
						.size = 4, //int in bytes
						.type = PushConstant::Member::Texture
					}
				);
				member_offset += 4;
			}
		}
		else{
			shader.pushRange.textures.push_back(
				PushConstant::Member{
					.name = push_mem_name,
					.offset = member_offset,
					.size = 4, //int in bytes
					.type = PushConstant::Member::Texture
				}
			);
		}

		if(tex_type.parent_type != 0){
			auto const& parent_name = compiler.get_name(tex_type.parent_type);
			auto const& parent_type = compiler.get_type(tex_type.parent_type);
		}
		if(tex_type.member_types.size() > 0){
			auto const& tex_child = compiler.get_type(tex_type.member_types[0]);
			auto const& tex_child_name = compiler.get_name(tex_child.self);
			auto const& tex_child_op = tex_child.op;
		}
	}


	void InterpretPushConstant(spirv_cross::Compiler const& compiler, Shader& shader, spirv_cross::ID push_id){		
		
		spirv_cross::SPIRType const& push_type = compiler.get_type(push_id);
		auto const& push_name = compiler.get_name(push_id);
			
		EWE_ASSERT(push_type.basetype == spirv_cross::SPIRType::Struct);
		shader.pushRange.size = compiler.get_declared_struct_size(push_type);

		//compiler.get_decoration(ID id, Decoration decoration)
		PrintAllDecorations("push", compiler.get_decoration_bitset(push_id));

		auto const& push_member_count = push_type.member_types.size();
		/*
		{ //counting buffers / texutres
			uint8_t buffer_count = 0;
			uint8_t texture_count = 0;
			for (uint32_t m = 0; m < push_type.member_types.size(); m++) {
				auto const& member_type = compiler.get_type(push_type.member_types[m]);
				if(member_type.pointer){
					buffer_count++;
				}
				else if(member_type.basetype == spirv_cross::SPIRType::Int){
					texture_count++;
				}
				else if(member_type.basetype == spirv_cross::SPIRType::UInt64){
					Log::Warning("non-pointer address in shader : %s\n", shader.filepath.string().c_str());
				}
				else{
					EWE_ASSERT(false, "invalid spirv type");
				}
			}
		}
		*/
		{ //populating textures / buffers
			for (uint32_t m = 0; m < push_type.member_types.size(); m++) {
				auto const& member_type = compiler.get_type(push_type.member_types[m]);
				if(member_type.pointer){
					ParsePushBufferAddress(compiler, shader, push_type, m, compiler.get_member_name(push_id, m));
				}
				else if(member_type.basetype == spirv_cross::SPIRType::Int){
					ParsePushTextureIndex(compiler, shader, push_type, m, compiler.get_member_name(push_id, m));
				}
				else if(member_type.basetype == spirv_cross::SPIRType::UInt64){
					Log::Warning("non-pointer address in shader : %s\n", shader.name.string().c_str());
				}
				else{
					EWE_ASSERT(false, "invalid spirv type");
				}
			}
		}

		//calculate offsets
		for(uint8_t i = 0; i < shader.pushRange.buffers.size(); i++){
			
		}
	}



	

	std::vector<char> ReadShaderFile(std::filesystem::path const& filepath) {

		std::ifstream shaderFile{};
		shaderFile.open(filepath, std::ios::binary);
		if (!shaderFile.is_open()) {
#if EWE_DEBUG_BOOL
			Log::Error("failed ot open shader file - %s : %s\n", std::filesystem::current_path().string().c_str(), filepath.string().c_str());
			EWE_ASSERT(shaderFile.is_open(), "failed to open shader");
#endif
		}
		shaderFile.seekg(0, std::ios::end);
		const std::size_t fileSize = static_cast<std::size_t>(shaderFile.tellg());
		EWE_ASSERT(fileSize > 0 && "shader is empty");

		shaderFile.seekg(0, std::ios::beg);
		std::vector<char> returnVec(fileSize);
		shaderFile.read(returnVec.data(), fileSize);
		shaderFile.close();
		return returnVec;
	}

	/*
	void AddBinding(spirv_cross::Compiler const& compiler, Backend::Descriptor::LayoutPack& layoutPack, spirv_cross::Resource const& res, VkDescriptorType descType, VkShaderStageFlagBits stageFlag) {
		const uint32_t setIndex = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		
		bool set_contained = false;
		for(auto& set : layoutPack.sets){
			if(set.index == setIndex){
				set_contained = true;
				break;
			}
		}
		if(!set_contained){
			layoutPack.sets.emplace_back(setIndex, Backend::Descriptor::Bindings{});
		}
		
		uint32_t descCount = 1;
		auto const& type = compiler.get_type(res.type_id);
		if (!type.array.empty() && type.array[0] != 0) {
			descCount = type.array[0];
		}
		compiler.get_name(res.id);


		Backend::Descriptor::Set* temp_set_ref = nullptr;
		for(auto& layout : layoutPack.sets){
			if(layout.index == setIndex){
				temp_set_ref = &layout;
				break;
			}
		}
		EWE_ASSERT(temp_set_ref != nullptr);
		
		//this readwrite feedback wont work with device buffer addresses
		//bool isReadable  = !compiler.has_decoration(res.id, spv::DecorationNonReadable);
		bool isWritable  = !compiler.has_decoration(res.id, spv::DecorationNonWritable);
		temp_set_ref->bindings.writes.push_back(isWritable);
		temp_set_ref->bindings.vkBindings.push_back(
			VkDescriptorSetLayoutBinding{
				.binding = compiler.get_decoration(res.id, spv::DecorationBinding),
				.descriptorType = descType,
				.descriptorCount = descCount,
				//this is a notable issue, maybe not significant
				//i think the only way to rectify this, keep it tight while allowing for cross-shader sets,
				//would be to use extensive rendergraph testing
				//tbh, vulkan not allowing the descriptor set itself to have looser usage is crazy
				.stageFlags = VK_SHADER_STAGE_ALL,//bad but going with it for now
				.pImmutableSamplers = nullptr,
			}
		);
	}

	Backend::Descriptor::LayoutPack CreateDescriptorLayoutPack(spirv_cross::Compiler const& compiler, VkShaderStageFlagBits stageFlag) {
		auto const& resources = compiler.get_shader_resources();

		Backend::Descriptor::LayoutPack ret{};// = Construct<Descriptor::LayoutPack>();

#define AddBindingType(vec, descType) for(auto& binding : vec) {AddBinding(compiler, ret, binding, descType, stageFlag);}
		AddBindingType(resources.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		AddBindingType(resources.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		AddBindingType(resources.sampled_images, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		AddBindingType(resources.storage_images, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		AddBindingType(resources.separate_samplers, VK_DESCRIPTOR_TYPE_SAMPLER);
		AddBindingType(resources.separate_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
#undef AddBindingType

		for (auto& set : ret.sets) {
			set.bindings.Sort();
		}

		return ret;
	}
	*/

	void Shader::ReadReflection(const std::size_t dataSize, const void* data) {

		spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(data), dataSize / sizeof(uint32_t));
		auto const& entryPoints = compiler.get_entry_points_and_stages();
		if(entryPoints.size() > 1){
			Log::Warning("multiple entry points[%zu] not supported : in shader[%s]\n", entryPoints.size(), name.string().c_str());
			Log::Warning("\t using entry point : %s\n", entryPoints[0].name.c_str());
		}
		//for (auto& entryPoint : entryPoints) {
			switch (entryPoints[0].execution_model)	{
				case spv::ExecutionModelVertex:   shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
				case spv::ExecutionModelFragment: shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
				case spv::ExecutionModelGLCompute:shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
				case spv::ExecutionModelTaskEXT:  shaderStageCreateInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT; break;
				case spv::ExecutionModelMeshEXT:  shaderStageCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT; break;
				default: EWE_UNREACHABLE; break;
			}
		//}
		auto const& resources = compiler.get_shader_resources();


		//AddBindingType(resources.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		//AddBindingType(resources.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		has_bindless_textures |= resources.sampled_images.size() > 0;
		has_bindless_textures |= resources.storage_images.size() > 0;
		has_bindless_textures |= resources.separate_samplers.size() > 0;
		has_bindless_textures |= resources.separate_images.size() > 0;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
		template for(constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^std::decay_t<decltype(resources)>, std::meta::access_context::current()))){
			if constexpr(requires{resources.[:mem:].size();}){
				if(resources.[:mem:].size() > 0){
					Log::Debug("shader resource member[%s] has %d elements\n", Reflect::GetName<mem>().data(), resources.[:mem:].size());
				}
			}
		}
#pragma GCC diagnostic pop

		pushRange.size = 0;
		pushRange.offset = 0;
		if(resources.push_constant_buffers.size() > 0){
			InterpretPushConstant(compiler, *this, resources.push_constant_buffers[0].base_type_id);
			if(resources.push_constant_buffers.size() > 1){
				Log::Warning("multiple push constants[%zu] not supported : in shader[%s]\n", resources.push_constant_buffers.size(), name.string().c_str());
			}
		}

		shaderStageCreateInfo.pName = "main";
		shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfo.pNext = nullptr;
		shaderStageCreateInfo.pSpecializationInfo = nullptr;
		shaderStageCreateInfo.flags = 0;
		
		//descriptorSets = CreateDescriptorLayoutPack(compiler, shaderStageCreateInfo.stage);

		//defaultSpecConstants.Clear();
		//InterpretSpecializationConstants(compiler, defaultSpecConstants);
	}

	void Shader::CompileModule(const std::size_t dataSize, const void* data) {
		VkShaderModuleCreateInfo shaderCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.codeSize = dataSize,
			.pCode = reinterpret_cast<const uint32_t*>(data)
		};
		EWE_VK(vkCreateShaderModule, logicalDevice.device, &shaderCreateInfo, nullptr, &shaderStageCreateInfo.module);

#if DEBUG_NAMING
		DebugNaming::SetObjectName(shaderStageCreateInfo.module, VK_OBJECT_TYPE_SHADER_MODULE, filepath.data());
#endif
	}


	Shader::Shader(LogicalDevice& _logicalDevice, std::filesystem::path const& fileLocation) 
    : logicalDevice{_logicalDevice}, 
		name{ fileLocation },
		meta{}
	{
		std::vector<char> shaderData = ReadShaderFile(fileLocation);
		//Log::Debug("beginning reflection of : %s\n", fileLocation.string().c_str());
		ReadReflection(shaderData.size(), shaderData.data());
		//Log::Debug("ending reflection of : %s\n", fileLocation.string().c_str());
		CompileModule(shaderData.size(), shaderData.data());
#if EWE_DEBUG_NAMING
		SetDebugName(fileLocation.string());
#endif
		logicalDevice.shaders.Add(this);

	}

	Shader::Shader(LogicalDevice& _logicalDevice, std::filesystem::path const& fileLocation, const std::size_t dataSize, const void* data) 
    : logicalDevice{_logicalDevice}, 
		name{ fileLocation }
	{
		Log::Debug("beginning reflection of : %s\n", fileLocation.string().c_str());
		ReadReflection(dataSize, data);
		Log::Debug("ending reflection of : %s\n", fileLocation.string().c_str());
		CompileModule(dataSize, data);
		SetDebugName(fileLocation.string());
		logicalDevice.shaders.Add(this);
	}

	Shader::Shader(LogicalDevice& _logicalDevice) 
    : logicalDevice{_logicalDevice}, 
		name{} 
	{
		logicalDevice.shaders.Add(this);}
	
	Shader::~Shader() {
		if (shaderStageCreateInfo.module != VK_NULL_HANDLE) {
			vkDestroyShaderModule(logicalDevice.device, shaderStageCreateInfo.module, nullptr);
		}
#if PIPELINE_HOT_RELOAD
		for (auto& staleModule : staleModules) {
			if (staleModule != VK_NULL_HANDLE) {
				vkDestroyShaderModule(logicalDevice.device, staleModule, nullptr);
			}
		}
		staleModules.clear();
#endif
		logicalDevice.shaders.Remove(this);
	}
#if PIPELINE_HOT_RELOAD
	void Shader::HotReload() {
		staleModules.push_back(shaderStageCreateInfo.module);
		shaderStageCreateInfo.module = VK_NULL_HANDLE;
		std::vector<char> shaderData = ReadShaderFile(filepath);
		CompileModule(shaderData.size(), shaderData.data());
		ReadReflection(shaderData.size(), shaderData.data());
	}
#endif

	void Shader::SetDebugName(std::string_view _name) {
#if EWE_DEBUG_NAMING
		logicalDevice.SetObjectName(shaderStageCreateInfo.module, VK_OBJECT_TYPE_SHADER_MODULE, _name);
#endif
	}

} //namespace EWE