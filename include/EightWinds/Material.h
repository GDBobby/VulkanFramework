#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/Shader.h"
#include "EightWinds/Pipeline/Layout.h"

#include "EightWinds/Data/Color.h"

namespace EWE{
	struct MaterialBase{
		PipeLayout* layout;//point to a layout
		
		//its really just a set of shaders
		//but we're not going to recompile or anything like that
	};
	struct MaterialPBR : public MaterialBase {
			Color_RGB<float> color;
			float roughness;
			float metal;
			//float ao;
			
			float specular;
			float glossiness;
			
			//other stuff
			//float fresnel;
			//float anisotropy;
			//float emissive;
	};
	
	struct OtherMaterial : public MaterialBase{
		//basically just all the data that gets sent to the GPU
		
	};
	
	struct GrassBufferObject{
		float height;
		float density;
		float offset[2];
	};
	
	struct GeneratedGrassMaterial : public MaterialBase, public GrassBufferObject {
	};
}