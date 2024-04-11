#pragma once

#ifdef DEBUG
    #include "Log.hpp"

    #if _MSC_VER
        #include <intrin.h>
        #define DEBUGBREAK() __debugbreak()
    #else
        #define DEBUGBREAK() __builtin_trap()
    #endif

    #define ASSERT_MSG(condition, ...) { if(!(condition)) { FATAL("Assertion '{}' failed at {}:{}", #condition, __FILE__, __LINE__); FATAL(__VA_ARGS__); DEBUGBREAK(); } }
    #define ASSERT(condition) { if(!(condition)) { FATAL("Assertion '{}' failed at {}:{}", #condition, __FILE__, __LINE__); DEBUGBREAK(); } }
#else
    #define DEBUGBREAK()
    #define ASSERT(condition, message)
#endif // DEBUG 
