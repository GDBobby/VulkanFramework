#include "EightWinds/Pipeline/PipelineSystem.h"

#include <cassert>

namespace EWE{

		Pipeline* PipelineSystem::At(PipelineID pipeID) {
			auto foundPipe = pipelineMap.find(pipeID);
			assert(foundPipe != pipelineMap.end());
			return foundPipe->second;
		}
		void PipelineSystem::Emplace(std::string const& pipeName, PipelineID pipeID, Pipeline* pipe) {
			assert(!pipelineMap.contains(pipeID));
			pipelineMap.emplace(pipeID, pipe);
			pipelineNames.emplace(pipeID, pipeName);
		}
		bool PipelineSystem::OptionalDestructAt(PipelineID pipeID) {
			auto foundPipe = pipelineMap.find(pipeID);

			if (foundPipe != pipelineMap.end()) {
				delete foundPipe->second;
				pipelineMap.erase(foundPipe);
				return true;
			}
			return false;
		}
		void PipelineSystem::DestructAt(PipelineID pipeID) {
#if PIPELINE_HOT_RELOAD
			pipelineNames.erase(pipeID);
#endif
			assert(OptionalDestructAt(pipeID));
		}

		void PipelineSystem::DestructAll() {
			for (auto& pipe : pipelineMap) {
				delete pipe.second;
			}
			pipelineMap.clear();
			pipelineNames.clear();
		}


#if PIPELINE_HOT_RELOAD
		void PipelineSystem::DrawImgui() {

			if (ImGui::TreeNode("Pipelines")) {
				for (auto& pipePair : pipelineMap) {
					auto& pipe = pipePair.second;
					auto* graphicsPipe = dynamic_cast<GraphicsPipeline*>(pipe);
					auto* computePipe = dynamic_cast<ComputePipeline*>(pipe);
				
					std::string extension{};
					auto& pipeName = pipelineNames.at(pipePair.first);
					//for (auto& pipeName : pipelineNames) {

						extension = "##ps";
						extension += std::to_string(pipePair.first);
						ImGui::Checkbox(extension.c_str(), &pipe->enabled);
						ImGui::SameLine();
						if (ImGui::TreeNode(pipeName.c_str())) {

							if (pipe->stalePipeline != VK_NULL_HANDLE) {
								pipe->framesSinceSwap++;
								printf("ticking stale pipeline for destruction - %d : %s\n", pipe->framesSinceSwap, pipeName.c_str());
								if (pipe->framesSinceSwap > (MAX_FRAMES_IN_FLIGHT * 2 + 1)) {
									//if (pipe->pipe->framesSinceSwap > 200) {
									EWE_VK(vkDestroyPipeline, VK::Object->vkDevice, pipe->stalePipeline, nullptr);

									printf("destroying stale pipeline, sleeping\n");
									//std::this_thread::sleep_for(std::chrono::seconds(2));
									pipe->stalePipeline = VK_NULL_HANDLE;
									pipe->framesSinceSwap = 0;
								}
							}
							else {
								pipe->pipeLayout->DrawImgui();

								extension = "reload layout##ps";
								extension += std::to_string(pipePair.first);
								if (ImGui::Button(extension.c_str())) {
									//holdingReloadPipe = pipe->pipe;
									//pipe->pipe = Construct<EWEPipeline>({ holdingReloadPipe->copyStringStruct, holdingReloadPipe->copyConfigInfo });
									//holdingReloadPipe = Construct<EWEPipeline>({ pipe->pipe->copyStringStruct, pipe->pipe->copyConfigInfo });
									pipe->HotReload(true);
								}

								extension = "specialization info##ps";
								extension += std::to_string(pipePair.first);
								if (ImGui::TreeNode(extension.c_str())) {
									for (auto& stage : pipe->copySpecInfo) {
										ImGui::Text("%s - %d", magic_enum::enum_name(stage.key.value).data(), stage.value.size());
										uint16_t extVal = 0;
										for (auto& val : stage.value) {
											ImGui::Text("%d : %s : type(%s)", val.constantID, val.name.c_str(), magic_enum::enum_name(val.type).data());
											switch (val.type) {
												case Shader::ST_INT: {
													int* data = reinterpret_cast<int*>(val.value);
													std::string extValStr = std::to_string(val.constantID) + std::string("##int") + std::to_string(extVal);
													ImGui::InputInt(extValStr.c_str(), data);
													break;
												}
												case Shader::ST_UINT: {

													unsigned int* data = reinterpret_cast<unsigned int*>(val.value);
													std::string extValStr = std::to_string(val.constantID) + std::string("##uint") + std::to_string(extVal);
													ImGui::InputScalar(extValStr.c_str(), ImGuiDataType_U32, data);
													break;
												}
												case Shader::ST_BOOL: {
													bool* data = reinterpret_cast<bool*>(val.value);
													std::string extValStr = std::to_string(val.constantID) + std::string("##bool") + std::to_string(extVal);
													ImGui::Checkbox(extValStr.c_str(), data);
													break;
												}
												case Shader::ST_FLOAT: {
													float* data = reinterpret_cast<float*>(val.value);
													std::string extValStr = std::to_string(val.constantID) + std::string("##float") + std::to_string(extVal);
													ImGui::InputFloat(extValStr.c_str(), data);
													break;
												}
												case Shader::ST_DOUBLE: {
													double* data = reinterpret_cast<double*>(val.value);
													std::string extValStr = std::to_string(val.constantID) + std::string("##double") + std::to_string(extVal);
													ImGui::InputDouble(extValStr.c_str(), data);
													break;
												}
												default:
													ImGui::TextUnformatted("Unknown type");
													break;
											}
											extVal++;
										}
									}


									ImGui::TreePop();
								}

								extension = "reload pipeline only##ps";
								extension += std::to_string(pipePair.first);
								if (ImGui::Button(extension.c_str())) {
									pipe->HotReload(false);
								}
							}
							//pipe->pipeLayout->Imgui();
							if (graphicsPipe) {
								graphicsPipe->copyConfigInfo.RenderIMGUI();
							}
							ImGui::TreePop();
						}

					//}
				}
				ImGui::TreePop();
			}
		}
#endif
}