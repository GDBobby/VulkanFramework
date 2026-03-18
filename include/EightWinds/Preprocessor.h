#pragma once

#ifndef EWE_DEBUG
#define EWE_DEBUG_BOOL false
#else
#define EWE_DEBUG_BOOL true
#endif


#include <string_view>

#define EWE_USING_EXCEPTIONS true

#ifdef EWE_DEBUG_NAME_FORCE
#define EWE_DEBUG_NAMING EWE_DEBUG_NAME_FORCE
#else
#define EWE_DEBUG_NAMING EWE_DEBUG_BOOL
#endif

#ifdef EWE_CALL_STACK_FORCE
#define EWE_CALL_STACK_DEBUG EWE_CALL_STACK_FORCE
#else
//check if stacktrace is enabled first, clang doesnt support it
#define EWE_CALL_STACK_DEBUG (EWE_DEBUG_BOOL && HAVE_STD_STACKTRACE)
#endif

static inline void EWE_Debug_Breakpoint() {
    #if EWE_DEBUG_BOOL
        #if defined(_MSC_VER)
            __debugbreak();

        #elif defined(__clang__)
            #if __has_builtin(__builtin_debugtrap)
                __builtin_debugtrap();
            #else
                __builtin_trap();
            #endif
        #elif defined(__GNUC__) || defined(__GNUG__)
            #if defined(__i386__) || defined(__x86_64__)
                __asm__ volatile("int $3");
            #else
                __builtin_trap();
            #endif
        #endif
    #else

    #endif
}


#if EWE_DEBUG_BOOL
    #include <cstdio> //printf
#endif

inline void EWE_ASSERT(bool statement, std::string_view print = "failed assert") {
#if EWE_DEBUG_BOOL
    if (!statement) {
        printf("%s\n", print.data());
        EWE_Debug_Breakpoint();
    }
#endif
}


#if EWE_DEBUG_BOOL
#include <utility>

    #ifdef _MSC_VER
        #define EWE_UNREACHABLE EWE_ASSERT(false, "unreachable"); std::unreachable()
    #elif defined(__GNUC__) || defined(__clang__)
        #define EWE_UNREACHABLE EWE_ASSERT(false, "unreachable"); std::unreachable()
    #else
        #define EWE_UNREACHABLE throw std::runtime_error("unreachable code")
    #endif
#else
    #define EWE_UNREACHABLE std::unreachable()
#endif