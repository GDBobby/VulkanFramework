#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

#include "EightWinds/RenderGraph/GPUTask.h"

namespace EWE{
    struct RenderGraph{
        //i want this to be visually buildable, in a UI
        //i need to start there and work towards here

        void Execute();

        struct Node{
            std::vector<uint16_t> input;
            GPUTask task;
            std::vector<uint16_t> output;
            bool visited = false;
            bool updated_this_iter = false;
        };

        std::vector<Node> nodes{};
        void Execute() const;
    };
}//namespace EWE