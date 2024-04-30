#pragma once
#include"canfd_frame.h"
#include<numeric>
#include<mutex>
#include <thread>
//随机生成一个符合显式条件的打包方案，max_try为一个帧能装载message的最大数量
std::vector<int> generate_individual(const std::unordered_set<message*>& message_p_set, size_t max_try=5);
//随机生成数量为num的population
std::vector<std::vector<int>> initial_population(std::vector<message>& message_set, int num, size_t max_try=5);
class FramePacking
{
	//std::random_device rd;
	//std::mt19937 gen(rd());


	//std::uniform_int_distribution<int>frame_number_dist(1, message_set.size() - 1);
	//int frame_number = message_set.size(); //先创建信号个数量的frame


	//std::vector<int>individuals(message_set.size());
	//for (int i = 0; i < frame_number; i++) {
	//	individuals[i] = i;
	//	//TODO 在随机生成信号的函数中，增加wctt的限制
	//}
};
////生成很多的frame_set，其已经装载好了message
//void initial_population(std::vector<message>& message_set) {
//
//
//
//
//	std::unordered_set<message*> message_p_set;
//	message_p_set.reserve(message_set.size());
//	for (message& m : message_set) {
//		message_p_set.insert(&m);
//	}
//
//
//	//TODO 先创建一个canfd_frame，随机选取可行元素,向canfd_frame中添加可行元素，最多尝试max_try次。重复该操作直到message分配完。
//	//canfd_frame以vector的形式存储，因为要生成大小为n的种群，即生成n种分配方案，故要生成n个vector<canfd_frame>,
//	//每个个体生成生成方案都维护一个std::unordered_set<message*> message_p_set;，每分配一个message就从message_p_set中删除message
//	//为了方便后续交叉等操作，需要给每个分配方案维护一个individuals数组，其中表示index号message分配到了individuals[i]号frame上，
//
//
//
//
//}

