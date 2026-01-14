#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

    std::function<void(CommandBuffer& cmdBuf)> task_bridge{};

    template<
        typename PrefixT = std::nullptr_t,
        typename SuffixT = std::nullptr_t
    >
    std::function<void(CommandBuffer&, uint8_t)> GenerateBridge(
            PrefixT* prefix,
            GPUTask& task,
            SuffixT* suffix
    ) {
        return [&](CommandBuffer& cmdBuf, uint8_t frameIndex)
            {
                if constexpr (!std::is_same_v<PrefixT, std::nullptr_t>) {
                    prefix->Execute(cmdBuf, frameIndex);
                }

                task.Execute(cmdBuf, frameIndex);

                if constexpr (!std::is_same_v<SuffixT, std::nullptr_t>) {
                    suffix->Execute(cmdBuf, frameIndex);
                }
            };
    }

    /*
    so we can go prefix -> prefix
    OR we can go suffix -> prefix
    but we can't go prefix->suffix
    */

    //do I do it like this or do I put them directly in the task?
    //the main thign is that sometimes an affix will be requested without a task
    //struct TaskAffixes{
    //    TaskPrefix prefix;
    //    TaskSuffix suffix;
    //};
}