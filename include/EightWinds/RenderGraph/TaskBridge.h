#pragma once

#include "EightWinds/VulkanHeader.h"
#include "EightWinds/RenderGraph/GPUTask.h"

#include "EightWinds/Data/PerFlight.h"

#include "EightWinds/RenderGraph/Resources.h"

namespace EWE{

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