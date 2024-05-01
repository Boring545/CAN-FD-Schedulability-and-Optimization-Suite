#pragma once
#include"canfd_frame.h"
#include<numeric>
#include<mutex>
#include <thread>
//随机生成一个符合显式条件的打包方案，max_try为一个帧能装载message的最大数量
std::vector<int> generate_individual(const std::unordered_set<message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try = 5);
//随机生成数量为num的population
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population,std::vector<message>& message_set, int num, size_t max_try=5);
//double calc_fitness() {
//
//}
//class FramePacking
//{
//}

