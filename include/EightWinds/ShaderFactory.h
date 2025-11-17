#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Shader.h"

#include <concepts>
#include <unordered_map>
#include <mutex>

namespace EWE{
    //attempting to avoid globals

        struct ShaderModuleTracker {
            Shader* shader;
            
            int16_t usageCount;
            template<class... Args>
            requires (std::is_constructible_v<Shader, Args...>)
            explicit ShaderModuleTracker(std::in_place_t, Args&&... args) : shader(Construct<Shader>(std::forward<Args>(args)...)), usageCount(1) {}

            explicit ShaderModuleTracker(Shader* shader) : shader{ shader }, usageCount{ 1 } {}
        };
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
    struct ShaderFactory{
        LogicalDevice& logicalDevice;
        [[nodiscard]] explicit ShaderFactory(LogicalDevice& logicalDevice) noexcept
        : logicalDevice{logicalDevice}
        {}

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