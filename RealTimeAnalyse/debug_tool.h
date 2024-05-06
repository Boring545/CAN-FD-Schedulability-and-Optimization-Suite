#pragma once
inline void debug_print() {}
// �ݹ��������ӡ��һ��������Ȼ����������ӡʣ�����
template<typename T, typename... Args>
inline void debug_print(T first, Args... args) {
    std::cout << first ; // ��ӡ��һ������
    debug_print(args...); // �ݹ���ô�ӡʣ�����
}
#define  DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#define DEBUG_MSG(...) std::cout << "[DEBUG] "; debug_print(__VA_ARGS__);std::cout<<std::endl;
#else
#define DEBUG_MSG(...)
#endif
//ʹ�÷��� DEBUG_MSG( x, y, str);    ע�⣺Ҫ������debug_tool.hǰ����  DEBUG_OUTPUT