#pragma once

//https://github.com/Ak-Elements/Onyx/blob/main/onyx/modules/graphics/public/onyx/graphics/rendergraph/rendergraph.h
//https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/


struct RenderGraph{
    void BeginRecording();
    void EndRecording();

    void Execute() const;
};