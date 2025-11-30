#include "EightWinds/Preprocessor.h"

#define VMA_IMPLEMENTATION
#if EWE_DEBUG_BOOL
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#endif
#include "vma/vk_mem_alloc.h"