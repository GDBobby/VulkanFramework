
#if EWE_DEBUG
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