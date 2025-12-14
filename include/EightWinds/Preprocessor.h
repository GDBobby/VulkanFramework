#pragma once

#ifndef EWE_DEBUG
#define EWE_DEBUG_BOOL false
#else
#define EWE_DEBUG_BOOL true
#endif

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


#if EWE_DEBUG_BOOL
    #include <cassert> //sucks to include this in the header but whatever

    #ifdef _MSC_VER
        #define EWE_UNREACHABLE assert(false)
    #elif defined(__GNUC__) || defined(__clang__)
        #define EWE_UNREACHABLE assert(false);__builtin_unreachable()
    #else
        #define EWE_UNREACHABLE throw std::runtime_error("unreachable code")
    #endif
#else
    #ifdef _MSC_VER
        #define EWE_UNREACHABLE __assume(false)
    #elif defined(__GNUC__) || defined(__clang__)
        #define EWE_UNREACHABLE __builtin_unreachable()
    #else
        #define EWE_UNREACHABLE throw std::runtime_error("unreachable code")
    #endif
#endif