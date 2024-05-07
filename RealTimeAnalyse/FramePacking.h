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
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size, int frame_count, size_t max_try=5);
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size, size_t max_try = 5);
class FramePacking
{
public:
	std::vector<message>&message_list;
	using FitnessFunction = std::function<double(std::vector<canfd_frame*>)>;
	canfd_utils canfd_setting;
	int population_size = 10;
	int frame_num = 9;//TODO 这两个数值可以通过message_list的信息自动确定
	FramePacking(std::vector<message>& _message_list, canfd_utils _canfd_setting):message_list(_message_list){
		this->canfd_setting = _canfd_setting;
	}
	//计算fitness的同时，也尝试分配优先级
	double calc_fitness(std::vector<canfd_frame*> frame_list) {
		return 1.0 / (assign_priority(frame_list) +calc_bandwidth_utilization(frame_list));
	}
	//计算带宽利用率
double calc_bandwidth_utilization(const std::vector<canfd_frame*>& frameSet) {
    double BWU = 0;
    for (size_t i = 0; i < frameSet.size(); i++) {
        BWU += ((double)canfd_setting.calc_wctt(frameSet[i]->get_paylaod_size()) / frameSet[i]->get_period());
    }
    return BWU;
}
	std::vector<canfd_frame> message_pack() {
		std::vector<std::vector<canfd_frame*>> population;   //存储种群
		//这里生成的种群中的个体不一定都可调度
		auto individuals = initial_population(population, message_list, population_size);  //individuals中的每个个体表示一个映射关系，index下标对应的message被分配到值对应的frame处
		std::vector<FitnessPopulationPair>fp(population.size());   //存储每个个体和其fitness的对应关系

		for (size_t i = 0; i < population.size(); i++) {
			assign_offset(population[i]);//先为每个个体中的数据帧分配合适的offset
			fp.push_back(FitnessPopulationPair(population[i], individuals[i],calc_fitness(population[i])));//计算每个个体的fitness，并尝试分配优先级
		}
		//按fitness降序排列种群中个体
		std::sort(fp.begin(), fp.end(), [](const FitnessPopulationPair& a, const FitnessPopulationPair& b) {
			return  b.fitness< a.fitness; });

		int individual_size = individuals[0].size();

		std::random_device rd; // 用于生成种子
		std::mt19937 gen(rd()); // 伪随机数生成器
		double mean = individual_size / 2; //希望交叉点相对位于中间，使得更多遗传信息被交换
		double stddev = 1.0;
		std::normal_distribution<double> dis(mean, stddev); // 均匀分布的随机整数生成器


		//TODO 使得两个种群个体生成后代
		for (size_t i = 0; i < std::ceil(population.size()/2.0); ++i) {
			int crossover_point = 0; //获取随机交叉点
			do {
				crossover_point = (int)dis(gen);
			} while (crossover_point<1 || crossover_point>individual_size - 2);//存在死循环风险
			fp[i].create_child(fp[i + 1], crossover_point,this);
		}
		for (size_t i = 0; i < std::floor(population.size() / 2.0); ++i) {
			int crossover_point = 0; //获取随机交叉点
			do {
				crossover_point = (int)dis(gen);
			} while (crossover_point<1 || crossover_point>individual_size - 2);//存在死循环风险
			fp[i].create_child(fp[population.size()-1-i], crossover_point,this);
		}


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
		std::vector<canfd_frame> x;
		return x;



	}
private:
	//FitnessFunction calc_fitness;
	class FitnessPopulationPair {

	public:
		double fitness=0;
		std::vector<canfd_frame*>* frames;
		std::vector<int>individual;
		FitnessPopulationPair() { frames = nullptr; }
		FitnessPopulationPair(std::vector<canfd_frame*>& _frames, std::vector<int>& _individual,double _fitness):frames(&_frames), individual(_individual),fitness(_fitness){}
		FitnessPopulationPair create_child(FitnessPopulationPair &fp,int crossover_point, FramePacking* outer) {

			//TODO将individual和fp.individual以crossover_point为中心，交换对方的内容，生成新的两个individual
			std::vector<int> child1 = individual;
			std::vector<int> child2 = (fp.individual);
			child1.reserve(individual.size());
			child2.reserve(individual.size());
			// 将 individual 的内容拆分为两部分并交叉到 child1 和 child2
			child1.assign(individual.begin(), individual.begin() + crossover_point);
			child1.insert(child1.end(), fp.individual.begin() + crossover_point, fp.individual.end());

			child2.assign(fp.individual.begin(), fp.individual.begin() + crossover_point);
			child2.insert(child2.end(), individual.begin() + crossover_point, individual.end());

			int frame_num1 = *std::max_element(child1.begin(), child1.end());
			int frame_num2 = *std::max_element(child2.begin(), child2.end());
			std::vector<int> count1(frame_num1,0), count2(frame_num2,0);

			for (int i = 0; i < individual.size(); i++) {
				if (count1[i] == 0) {
					count1[i]= outer->message_list[child1[i]].period;
				}
				else {
					count1[i] = my_algorithm::gcd(outer->message_list[child1[i]].period, count1[i]);
				}
				if (count2[i] == 0) {
					count2[i] = outer->message_list[child2[i]].period;
				}
				else {
					count2[i] = my_algorithm::gcd(outer->message_list[child2[i]].period, count2[i]);
				}
			}
			double avg1 = std::accumulate(count1.begin(), count1.end(), 0) / frame_num1; //child1的平均周期
			double avg2 = std::accumulate(count2.begin(), count2.end(), 0) / frame_num2; //child2的平均周期

			double score1 = my_algorithm::normalizeValue((double)frame_num1, 0.0, 1.0) + my_algorithm::normalizeValue(avg1, 0.0, 1.0);
			double score2 = my_algorithm::normalizeValue((double)frame_num2, 0.0, 1.0) + my_algorithm::normalizeValue(avg2, 0.0, 1.0);
			auto& child = child1;
			auto& fnum = frame_num1;
			if (score1 < score2) {
				child = child2;
				fnum = frame_num2;
			}
			else {
				child = child1;
				fnum = frame_num1;
			}

			// 创建一个随机数引擎
			std::random_device rd;
			std::mt19937 gen(rd());
			// 创建一个均匀分布对象，范围是 [0, 1)
			std::uniform_real_distribution<double> dis_P(0.0, 1.0);
			double mutation_P = 0.9;
			double P = dis_P(gen);

			std::uniform_real_distribution<int> dis_index(0, (*frames).size() - 1);
			int index = dis_index(gen);

			//变异,随机选一个数据帧，将其内容插到其他数据帧中
			if (P > mutation_P) {

				for (size_t i = 0; i < individual.size(); i++) {
					if (child[i] == index) {
						child[i] = dis_index(gen);
					}
				}
				fnum -= 1;
			}
			//TODO 根据child生成一个frame数组
			std::vector<canfd_frame*>* child_frame_list_p=new std::vector<canfd_frame*>(fnum,nullptr);
			auto& child_frame_list = (*child_frame_list_p);
			//canfd_frame* frame = new canfd_frame(i);
			for (size_t i = 0; i < individual.size(); i++) {
				//遍历child，将对应的message插入对应的frame中
				if(child_frame_list[child[i]]==nullptr){
					canfd_frame* frame = new canfd_frame(child[i]<index?child[i]:child[i]-1);
					child_frame_list[child[i]] = frame;
				}
				if (child_frame_list[child[i]]->add_message(outer->message_list[i])) {
					std::cerr << "if (child_frame_list[child[i]]->add_message(outer->message_list[i])) ERRORRRRRRRRRRR\n";
					abort();
				}
			}
			assign_offset(child_frame_list);
			assign_priority(child_frame_list);//有必要修改assign_priority使得，即使不能调度也能强行分配。
			return FitnessPopulationPair(child_frame_list, child, outer->calc_fitness(child_frame_list));

			//严重问题 它似乎认为 优先级一定能分配成功，但由于打包的问题，有的任务会无法完成

			//TODO 根据两个child，累计每一个个体的每一个帧中，相同周期的message的最大出现次数，每个个体维护一个最大个数，选择最大次数最大的个体，如都一致，选择数据帧小的个体
			//并对其依照概率P选择一个已经打包的帧，将其中的信号后随机拆分到其他几个帧中，依次来减少一个帧
			// //【很可能该操作会因为其他数据帧很满而无法拆分或降低可调度性】，
			// 因此建议P选择不要太大
			//生成一个VECTOR<canfd_frame> ，分配offset并检测可调度性
			// 如果不具备可调度性，选择那些高优先级任务，尝试将它们合并，减少高优先级任务的数量，从而减少其对低优先级任务的干扰
			// 但是有一个问题，就是合并高优先级帧，会可能会导致合并后的高优先级帧周期更小，从而产生更多的影响
			// 应该设计一种能增高数据帧周期的打包方式，也要兼顾减少高优先级数据帧的数量
			// 如果仍不能通过检测，则从父代中随机选择几个高fitness个体作为子代。
			//生成一个新的FitnessPopulationPair并返回




		}
	};

};



