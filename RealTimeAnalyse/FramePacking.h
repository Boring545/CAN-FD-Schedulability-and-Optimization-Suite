#pragma once
#include"canfd_frame.h"
#include<numeric>
#include<mutex>
#include <thread>
//�������һ��������ʽ�����Ĵ��������max_tryΪһ��֡��װ��message���������
std::vector<int> generate_individual(const std::unordered_set<message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try = 5);
//�����������Ϊnum��population
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population,std::vector<message>& message_set, int num, size_t max_try=5);
//double calc_fitness() {
//
//}
//class FramePacking
//{
//}

