#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/DescriptorSetLayout.h"

#include <vector>
#include <mutex> //factory

namespace EWE {
	struct Shader {
        LogicalDevice& logicalDevice;

        struct Stage { //becomes Shader::Stage
            enum Bits {
                Vertex = 0,
                TessControl,
                TessEval,
                Geometry,
                Task,
                Mesh,
                Fragment,
                Compute,

                COUNT
            };
            Bits value;
            constexpr Stage() : value{ Bits::COUNT } {}
            constexpr Stage(Bits v) : value{ v } {}
            constexpr operator Bits() const {
                return value;
            }

            constexpr Stage(VkShaderStageFlagBits vkStage) {
                switch (vkStage) {
                    case VK_SHADER_STAGE_VERTEX_BIT: value = Bits::Vertex; break;
                    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: value = Bits::TessControl; break;
                    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: value = Bits::TessEval; break;
                    case VK_SHADER_STAGE_GEOMETRY_BIT: value = Bits::Geometry; break;
                    case VK_SHADER_STAGE_TASK_BIT_EXT: value = Bits::Task; break;
                    case VK_SHADER_STAGE_MESH_BIT_EXT: value = Bits::Mesh; break;
                    case VK_SHADER_STAGE_FRAGMENT_BIT: value = Bits::Fragment; break;
                    case VK_SHADER_STAGE_COMPUTE_BIT: value = Bits::Compute; break;
                }
                //have fun debugging this when it doesnt work.
                //EWE_UNREACHABLE;
            }
            constexpr operator VkShaderStageFlagBits() const {
                switch (value) {
                    case Bits::Vertex:		return VK_SHADER_STAGE_VERTEX_BIT;
                    case Bits::TessControl:	return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    case Bits::TessEval:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    case Bits::Geometry:    return VK_SHADER_STAGE_GEOMETRY_BIT;
                    case Bits::Task:		return VK_SHADER_STAGE_TASK_BIT_EXT;
                    case Bits::Mesh:		return VK_SHADER_STAGE_MESH_BIT_EXT;
                    case Bits::Fragment:    return VK_SHADER_STAGE_FRAGMENT_BIT;
                    case Bits::Compute:		return VK_SHADER_STAGE_COMPUTE_BIT;
                }
                //have fun debugging this when it doesnt work
                //EWE_UNREACHABLE;
            }

            constexpr bool operator==(Bits bits) const {
                return value == bits;
            }
            constexpr bool operator==(Stage const other) const {
                return value == other.value;
            }
            constexpr bool operator==(VkShaderStageFlagBits bits) const {
                return value == Stage(bits);
            }
        };

		enum FundamentalType {
			ST_INT,
			ST_UINT,
			ST_BOOL,
			ST_FLOAT,
			ST_DOUBLE,

			ST_COUNT
		};

		struct SpecializationEntry {
#if PIPELINE_HOT_RELOAD
			std::string name{};
#endif
			FundamentalType type;
			uint32_t constantID;
			uint8_t elementCount = 1;
			char value[64]; //64 being the size of a 4x4 matrix, the largest size im supporting rn. i dont even know if thats a thing for spec constants
		};

		struct VkSpecInfo_RAII {
			VkSpecInfo_RAII() {}
			VkSpecInfo_RAII(std::vector<SpecializationEntry> const& specEntries);
			VkSpecInfo_RAII(VkSpecInfo_RAII const& copy);
			VkSpecInfo_RAII& operator=(VkSpecInfo_RAII const& copy) = delete;
			VkSpecInfo_RAII(VkSpecInfo_RAII&& move) noexcept;
			VkSpecInfo_RAII& operator=(VkSpecInfo_RAII&& move) = delete;
			~VkSpecInfo_RAII();
			std::vector<VkSpecializationMapEntry> mapEntries{};
			VkSpecializationInfo specInfo{};
			uint64_t memPtr = 0;
		};


		std::string filepath{}; 
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo;

		Descriptor::LayoutPack descriptorSets;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes{};
		VkPushConstantRange pushRange{};

		std::vector<SpecializationEntry> defaultSpecConstants{};

		explicit Shader(LogicalDevice& logicalDevice, std::string_view fileLocation);
		Shader(LogicalDevice& logicalDevice, std::string_view fileLocation, const std::size_t dataSize, const void* data);
		Shader(LogicalDevice& logicalDevice);
		~Shader();

		bool ValidateVertexInputAttributes(std::vector<VkVertexInputAttributeDescription> const& cpu_side) const;
		VkShaderModule GetVkShader() const {
			return shaderStageCreateInfo.module;
		}
        
#if PIPELINE_HOT_RELOAD
        void HotReload();
#endif

	//protected:
		void CompileModule(const std::size_t dataSize, const void* data);
		void ReadReflection(const std::size_t dataSize, const void* data);
	};

    struct ShaderModuleTracker {
        Shader* shader;

        int16_t usageCount;
        template<class... Args>
            requires (std::is_constructible_v<Shader, Args...>)
        explicit ShaderModuleTracker(std::in_place_t, Args&&... args) : shader(Construct<Shader>(std::forward<Args>(args)...)), usageCount(1) {}

        explicit ShaderModuleTracker(Shader* shader) : shader{ shader }, usageCount{ 1 } {}
    };

    //attempting to avoid globals
    struct StringHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view sv) const noexcept {
            return std::hash<std::string_view>{}(sv);
        }
    };

    struct StringEqual {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const noexcept {
            return a == b;
        }
    };


    //i need to figure out how to intelligently pass this around
    //ideally, LogicalDevice would own a pointer to this
    //but Shader includes logicalDevice
    struct ShaderFactory {
        LogicalDevice& logicalDevice;
        [[nodiscard]] explicit ShaderFactory(LogicalDevice& logicalDevice) noexcept
            : logicalDevice{ logicalDevice }
        {
        }

        [[nodiscard]] Shader* GetShader(std::string_view filepath);
        [[nodiscard]] Shader* CreateShader(std::string_view filepath, const std::size_t dataSize, const void* data);
        void DestroyShader(Shader& shader);
        void DestroyAllShaders();

    private:
        Shader* GetShaderIfExist(std::string const& path);
        std::unordered_map<std::string, ShaderModuleTracker, StringHash, StringEqual> shaderModuleMap;
        std::mutex shaderMapMutex;
    };
}