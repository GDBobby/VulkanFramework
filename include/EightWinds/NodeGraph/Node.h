#pragma once

#include "Pin.h"
#include "Link.h"

#include "LAB/Vector.h"
#include "LAB/Transform.h"

#include "EightWinds/CommandBuffer.h"

#include <vector>
#include <string>

namespace EWE{
    struct NodePush{ //128bytes
        //lab::vec4 titleVertices;
        //lab::vec4 backgroundVertices;
        //lab::vec4 foregroundVertices;
        
        lab::vec3 titleColor;
        lab::vec3 foregroundColor;
        lab::vec3 backgroundColor;

        //i could make it a mat3x3 and precompute that on the GPU. its a bit cleaner, but uses more space in the push constant (9 floats instead of 4)
        lab::vec2 position;
        lab::vec2 scale;
        //position, scale
    };

    struct Node{

        std::vector<Pin> pins{};
        std::vector<Link> links{};

        std::string name;

        lab::vec3 titleColor;
        lab::vec3 foregroundColor;
        lab::vec3 backgroundColor;

        float foregroundScale; //what percentage of the width it takes up, width and height separate
        float titleScale; //what percentage of the foreground it takes up, vertical only

        lab::Transform2 transform;
        //no rotation

        uint32_t titleBufferIndex;
        uint32_t backgroundBufferIndex;
        uint32_t borderBufferIndex;

        bool OnClick();

        bool Complete() const {
            return pins.size() == links.size();
        }

        //draw the background, draw the foregound, then draw the title
        void Render(CommandBuffer cmdBuf) const;
    };
}