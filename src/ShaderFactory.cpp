#include "EightWinds/ShaderFactory.h"

#include <cassert>

namespace EWE{
    
	//this hash isnt working correctly


	Shader* ShaderFactory::GetShaderIfExist(std::string const& path) {
		auto modFind = shaderModuleMap.find(path);
		if (modFind == shaderModuleMap.end()) {
			return nullptr;
		}
		else {
			std::unique_lock<std::mutex> uniqLock{ shaderMapMutex };
			modFind->second.usageCount++;
			return modFind->second.shader;
		}
	}

	Shader* ShaderFactory::GetShader(std::string_view filepath) {
		auto modFind = shaderModuleMap.find(filepath);
		if (modFind == shaderModuleMap.end()) {
			auto empRet = shaderModuleMap.emplace(filepath, new Shader(logicalDevice, filepath));
			assert(empRet.second);
			return empRet.first->second.shader;
		}
		else {
			shaderMapMutex.lock();
			modFind->second.usageCount++;
			return modFind->second.shader;
		}
	}
	Shader* ShaderFactory::CreateShader(std::string_view filepath, const std::size_t dataSize, const void* data) {
		assert(shaderModuleMap.find(filepath) == shaderModuleMap.end());

		auto empRet = shaderModuleMap.emplace(filepath, new Shader(logicalDevice, filepath, dataSize, data));
		assert(empRet.second);
		return empRet.first->second.shader;
	}

	void ShaderFactory::DestroyShader(Shader& shader) {
		if (shader.shaderStageCreateInfo.module != VK_NULL_HANDLE) {
			shaderMapMutex.lock();
			
			auto findRet = shaderModuleMap.find(shader.filepath);
			if (findRet == shaderModuleMap.end()) {
#if EWE_DEBUG_BOOL
				printf("trying to delete a shader that's not in the shader moduel map\n");
#endif
			}
			else {
				findRet->second.usageCount--;
				if (findRet->second.usageCount <= 0) {
					vkDestroyShaderModule(logicalDevice.device, shader.shaderStageCreateInfo.module, nullptr);
					shaderModuleMap.erase(findRet);
				}
			}

			shaderMapMutex.unlock();
#if PIPELINE_HOT_RELOAD
			return;
#endif
			EWE_UNREACHABLE;
		}
	}

	void ShaderFactory::DestroyAllShaders() {
		shaderMapMutex.lock();
		for (auto iter = shaderModuleMap.begin(); iter != shaderModuleMap.end(); iter++) {
			vkDestroyShaderModule(logicalDevice.device, iter->second.shader->shaderStageCreateInfo.module, nullptr);
		}
		shaderModuleMap.clear();
		shaderMapMutex.unlock();
	}

}