#pragma once
inline void debug_print() {}
// 递归情况：打印第一个参数，然后调用自身打印剩余参数
template<typename T, typename... Args>
inline void debug_print(T first, Args... args) {
    std::cout << first ; // 打印第一个参数
    debug_print(args...); // 递归调用打印剩余参数
}
#define  DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#define DEBUG_MSG(...) std::cout << "[DEBUG] "; debug_print(__VA_ARGS__);std::cout<<std::endl;
#else
#define DEBUG_MSG(...)
#endif
//使用方法 DEBUG_MSG( x, y, str);    注意：要在引用debug_tool.h前定义  DEBUG_OUTPUT