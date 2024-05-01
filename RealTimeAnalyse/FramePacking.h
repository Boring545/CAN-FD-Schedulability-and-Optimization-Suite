#pragma once

#include<numeric>
#include<mutex>
#include <thread>
#include <functional>
#include"PriorityArrangement.h"
#include"offset_arrangement.h"
//随机生成一个符合显式条件的打包方案，max_try为一个帧能装载message的最大数量
std::vector<int> generate_individual(const std::unordered_set< message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try = 5);
//随机生成数量为num的population
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int num, size_t max_try=5);


class FramePacking
{
public:
	std::vector<message>&message_list;
	using FitnessFunction = std::function<double(std::vector<canfd_frame*>)>;
	canfd_utils canfd_setting;
	int num = 5;
	FramePacking(std::vector<message>& _message_list, canfd_utils _canfd_setting):message_list(_message_list){
		this->canfd_setting = _canfd_setting;
	}
	//计算fitness的同时，也尝试分配优先级
	double calc_fitness(std::vector<canfd_frame*> frame_list) {
		return 1.0 / (assign_priority(frame_list) + canfd_utils().calc_bandwidth_utilization(frame_list));
	}
	std::vector<canfd_frame> message_pack() {
		std::vector<std::vector<canfd_frame*>> population;
		auto individuals = initial_population(population, message_list, num);
		std::vector<FitnessPopulationPair>fp(population.size());
		for (size_t i = 0; i < population.size(); i++) {
			assign_offset(population[i]);
			fp.push_back(FitnessPopulationPair(population[i], calc_fitness(population[i])));
		}
		//fitness降序排列
		std::sort(fp.begin(), fp.end(), [](const FitnessPopulationPair& a, const FitnessPopulationPair& b) {
			return  b.fitness< a.fitness; });
		//TODO 选择一些个体，让他们生成后代，方法为单点交叉：选择序号 i 为1~ceiling（p/2）的个体，将 i 和 i+1 进行crossover；另选择序号 j为1~floor（p/2）的个体，将其与 n+1-j 号的个体crossover
		//crossover选用单点交叉，从父母个体中随机选择相同index作为交叉点，交换对面的另一半到自己身上，
		//生成的两个新结果即为子代，要从子代中选择一个保留并淘汰另一个。
		//淘汰策略为：策略1：如果绝大部分信号周期一致，则保留拆分结果中帧数更少（子集更少）的子代；
		//策略2：如果信号周期彼此不同，累加计算在同一帧中相同周期信号出现的次数，保留该次数最大的子代

		//对得到的后代，依概率P变异，随机选择一个被打包的帧，将其中信号随机的拆分到其他的帧中

		//后代组成的种群也要分配offset，分配优先级，计算fitness。 
		// 对于得到的不可调度的子代，要确定哪个帧不可以调度，然后随机选几个比它高的高优先级帧，将他们合并，
		// 然后重新分析可调度性，如果还是不行
		//建议依照概率丢掉无法调度的个体，然后从父代中随机挑选几个个体替换。

		//直到计算到U不怎么变了为止，或者迭代到极限次数，选择fitness的最优个体作为我们的迭代方案





	}
private:
	//FitnessFunction calc_fitness;
	class FitnessPopulationPair {

	public:
		double fitness;
		std::vector<canfd_frame*>& frames;
		FitnessPopulationPair(std::vector<canfd_frame*>& _frames, double _fitness):frames(_frames),fitness(_fitness){}
	};

};



