#pragma once
#pragma once
#include <iostream>

// ��ӡ������Ϣ����
inline void debug_print() {}

// �ݹ��������ӡ��һ��������Ȼ����������ӡʣ�����
template<typename T, typename... Args>
inline void debug_print(T first, Args... args) {
    std::cout << first; // ��ӡ��һ������
    debug_print(args...); // �ݹ���ô�ӡʣ�����
}

// ������Լ���
enum class DebugLevel {
    INFO,
    DEBUG1,
    DEBUG2,
    DEBUG3,
    DEBUG4,
};

// Ĭ�ϵ��Լ���Ϊ INFO
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DebugLevel::INFO
#endif

//===============================
#define DEBUG_OUTPUT


//===============================



// ������Ϣ��
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


//ʹ�÷��� DEBUG_MSG( x, y, str);    ע�⣺Ҫ������debug_tool.hǰ����  DEBUG_OUTPUT