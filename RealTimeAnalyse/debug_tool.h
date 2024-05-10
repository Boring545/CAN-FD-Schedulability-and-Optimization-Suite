#pragma once
#pragma once
#include <iostream>

// 打印调试信息函数
inline void debug_print() {}

// 递归情况：打印第一个参数，然后调用自身打印剩余参数
template<typename T, typename... Args>
inline void debug_print(T first, Args... args) {
    std::cout << first; // 打印第一个参数
    debug_print(args...); // 递归调用打印剩余参数
}

// 定义调试级别
enum class DebugLevel {
    INFO,
    DEBUG1,
    DEBUG2,
    DEBUG3,
    DEBUG4,
};

// 默认调试级别为 INFO
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DebugLevel::INFO
#endif

//===============================
#define DEBUG_OUTPUT


//===============================



// 调试消息宏
#ifdef DEBUG_OUTPUT

#define DEBUG_MSG_INFO(...) \
    do { \
        if (static_cast<int>(DebugLevel::INFO) <= static_cast<int>(DEBUG_LEVEL)) { \
            std::cout << "[INFO] "; \
            debug_print(__VA_ARGS__); \
            std::cout << std::endl; \
        } \
    } while (0)

#define DEBUG_MSG_DEBUG1(...) \
    do { \
        if (static_cast<int>(DebugLevel::DEBUG1) <= static_cast<int>(DEBUG_LEVEL)) { \
            std::cout << "[DEBUG1] "; \
            debug_print(__VA_ARGS__); \
            std::cout << std::endl; \
        } \
    } while (0)

#define DEBUG_MSG_DEBUG2(...) \
    do { \
        if (static_cast<int>(DebugLevel::DEBUG2) <= static_cast<int>(DEBUG_LEVEL)) { \
            std::cout << "[DEBUG2] "; \
            debug_print(__VA_ARGS__); \
            std::cout << std::endl; \
        } \
    } while (0)

#define DEBUG_MSG_DEBUG3(...) \
    do { \
        if (static_cast<int>(DebugLevel::DEBUG3) <= static_cast<int>(DEBUG_LEVEL)) { \
            std::cout << "[DEBUG3] "; \
            debug_print(__VA_ARGS__); \
            std::cout << std::endl; \
        } \
    } while (0)

#define DEBUG_MSG_DEBUG4(...) \
    do { \
        if (static_cast<int>(DebugLevel::DEBUG4) <= static_cast<int>(DEBUG_LEVEL)) { \
            std::cout << "[DEBUG4] "; \
            debug_print(__VA_ARGS__); \
            std::cout << std::endl; \
        } \
    } while (0)

#else
#define DEBUG_MSG_INFO(...)
#define DEBUG_MSG_DEBUG1(...)
#define DEBUG_MSG_DEBUG2(...)
#define DEBUG_MSG_DEBUG3(...)
#define DEBUG_MSG_DEBUG4(...)
#endif


//使用方法 DEBUG_MSG( x, y, str);    注意：要在引用debug_tool.h前定义  DEBUG_OUTPUT