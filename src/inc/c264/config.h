#pragma once

#ifdef EXPORT_C264
    #ifdef _MSC_VER
        #define API __declspec(dllexport)
    #elif __linux__
        #define API __attribute__((visibility("default")))
    #else
        #define API
    #endif
#else
    #define API __declspec(dllimport)
#endif

#define CONFIG_SOURCE 1
#define CONFIG_SOURCE_FILE 1
#define CONFIG_PARSER 1
#define CONFIG_FORCE_LOG_POSITION 1

