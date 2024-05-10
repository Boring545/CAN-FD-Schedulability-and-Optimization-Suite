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
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, size_t population_size, size_t max_try = 5);
using message_list_ptr = std::vector<message>*; //直接引用系统中的唯一message集合，析构时不释放
using frame_list_ptr = std::vector<canfd_frame*>*; //每个打包方案都维护一组frame集合，析构时要释放空间
using message_list = std::vector<message>; //直接引用系统中的唯一message集合，析构时不释放
using frame_list = std::vector<canfd_frame*>; //每个打包方案都维护一组frame集合，析构时要释放空间
using message_map_list = std::vector<int>;

//封装了一组canfd_frame
class packing_scheme {
public:
	frame_list_ptr frames_p=nullptr;
	message_list_ptr messages_p = nullptr;
	message_map_list individual;//根据individual自动生成frames集合
	double fitness = 0;
	canfd_utils* canfd_setting = nullptr;
	std::mt19937 engine;
	bool schedulability_label=false;//true 可调度，默认false不可调度
	packing_scheme(){}
	packing_scheme(const packing_scheme& other) {
		frames_p = other.frames_p;
		messages_p = other.messages_p;
		individual = other.individual;
		fitness = other.fitness;
		canfd_setting = other.canfd_setting;
		engine = other.engine;
		schedulability_label = other.schedulability_label;
	}
	~packing_scheme() {
		//auto& frames = *frames_p;
		//for (size_t i = 0; i < frames.size(); i++) {
		//	delete frames[i];
		//}
		////TODO 内存泄露风险
		///*delete frames_p;*/
	}
	packing_scheme(message_list& _message_list, message_map_list& _message_map, canfd_utils& _canfd_config) {
		engine = std::mt19937(std::random_device{}());
		messages_p = &_message_list;
		individual = _message_map;
		canfd_setting = &_canfd_config;

		int frames_size = *std::max_element(individual.begin(), individual.end())+1;

		frames_p = new frame_list(frames_size);
		frame_list& frames = (*frames_p);
		for (size_t i = 0; i < frames_size; i++) {
			canfd_frame* frame_p = new canfd_frame(i);
			frames[i]=frame_p;
		}

		message_list& messages = *(messages_p);
		for (size_t i = 0; i < individual.size(); i++) {
			//遍历child，将对应的message插入对应的frame中
			//这里默认individual的元素是从0~1连续的
			if (frames[individual[i]]->add_message(messages[i])==false) {
				std::cerr << "if (child_frame_list[child[i]]->add_message(outer->message_list[i])) ERRORRRRRRRRRRR\n";
				abort();
			}
		}
		assign_offset(frames);
		schedulability_label = assign_priority(frames);//分配优先级
		fitness = calc_fitness();
	}
	//计算带宽利用率
	double calc_bandwidth_utilization() {
		double BWU = 0;
		auto& frames = *frames_p;
		for (size_t i = 0; i < frames.size(); i++) {
			BWU += ((double)canfd_setting->calc_wctt(frames[i]->get_paylaod_size()) / frames[i]->get_period());
		}
		return BWU;
	}
	//计算fitness
	double calc_fitness() {
		auto& frames = *frames_p;
		return 1.0 / ((schedulability_label?0:1) + calc_bandwidth_utilization());
	}
	//按照遗传算法生成子代
	packing_scheme create_child_scheme(packing_scheme& other) {
		//TODO将individual和fp.individual以crossover_point为中心，交换对方的内容，生成新的两个individual
		std::uniform_int_distribution<int> cross_point_dist(1, messages_p->size() - 2);// 均匀分布的随机整数生成器
		int crossover_point = cross_point_dist(engine);

		message_map_list map1;
		message_map_list map2;
		// 将 individual 的内容拆分为两部分并交叉到 child1 和 child2【交叉可能导致出现map里id不连续的问题】
		map1.assign(individual.begin(), individual.begin() + crossover_point);
		map1.insert(map1.end(), other.individual.begin() + crossover_point, other.individual.end());

		map2.assign(other.individual.begin(), other.individual.begin() + crossover_point);
		map2.insert(map2.end(), individual.begin() + crossover_point, individual.end());

		//获取map里标识的最大帧数量
		int frame_num1 = *std::max_element(map1.begin(), map1.end())+1;
		int frame_num2 = *std::max_element(map2.begin(), map2.end())+1;

		//count 记录了每个数据帧的周期
		std::vector<int> count1(frame_num1, 0), count2(frame_num2, 0);

		//计算每个数据帧周期的gcd
		message_list& messages = *messages_p;
		for (int i = 0; i < messages.size(); i++) {
			if (count1[map1[i]] == 0) {
				count1[map1[i]] = messages[i].period;
			}
			else {
				count1[map1[i]] = my_algorithm::gcd(messages[i].period, count1[map1[i]]);
			}
			if (count2[map2[i]] == 0) {
				count2[map2[i]] = messages[i].period;
			}
			else {
				count2[map2[i]] = my_algorithm::gcd(messages[i].period, count2[map2[i]]);
			}
		}

		double avg1 = std::accumulate(count1.begin(), count1.end(), 0) / frame_num1; //child1的平均周期
		double avg2 = std::accumulate(count2.begin(), count2.end(), 0) / frame_num2; //child2的平均周期

		//根据该分数，保留分高的那个子代
		double score1 = my_algorithm::normalizeValue((double)frame_num1, 0.0, 1.0) + my_algorithm::normalizeValue(avg1, 0.0, 1.0);
		double score2 = my_algorithm::normalizeValue((double)frame_num2, 0.0, 1.0) + my_algorithm::normalizeValue(avg2, 0.0, 1.0);


		message_map_list& map = map1;
		int& frame_num = frame_num1;
		if (score1 < score2) {
			map =map2;//保留子代2
			frame_num = frame_num2;
		}
		else {
			map = map1;//保留子代1
			frame_num = frame_num1;
		}

		// 创建一个均匀分布对象，范围是 [0, 1)
		std::uniform_real_distribution<double> dis_P(0.0, 1.0);
		double mutation_P = 0.9; //P大于0.9才能变异，概率为10%
		double P = dis_P(engine);

		frame_list& frames = *frames_p;
		std::uniform_int_distribution<int> index_dist(0, frame_num-1);
		int index = index_dist(engine);//随机选一个数据帧

		//变异,随机选一个数据帧，将其内容插到其他数据帧中
		if (P > mutation_P) {

			for (size_t i = 0; i < individual.size(); i++) {
				while (map[i] == index) {
					map[i] = index_dist(engine);
				}
				//if (map[i] > index) {
				//	map[i]--;//去掉空数据帧，编号比他大的自动缩小一号
				//}
			}
			frame_num -= 1;
		}
		
		return packing_scheme(messages, map, *(this->canfd_setting));
	}
};

class FramePacking
{
public:
	std::vector<message>&message_list;
	canfd_utils canfd_setting;
	int population_size = 10;
	int frame_num = 9;//TODO 这两个数值可以通过message_list的信息自动确定
	std::mt19937 engine;
	FramePacking(std::vector<message>& _message_list, canfd_utils& _canfd_setting):message_list(_message_list){
		this->canfd_setting = _canfd_setting;
		this->engine = std::mt19937(std::random_device{}());
	}

	//使用遗传算法打包
	packing_scheme message_pack() {
		std::vector<std::vector<canfd_frame*>> population;   //存储种群
		//这里生成的种群中的个体不一定都可调度
		auto individuals = initial_population(population, message_list, population_size);  //individuals中的每个个体表示一个映射关系，index下标对应的message被分配到值对应的frame处

		std::vector<packing_scheme>schemes;   //存储每个个体和其fitness的对应关系
		std::vector<packing_scheme>new_schemes;   //存储每个个体和其fitness的对应关系
		schemes.reserve(population.size());
		for (size_t i = 0; i < population.size(); i++) {
			schemes.emplace_back(message_list, individuals[i], canfd_setting);
		}
		//按fitness降序排列种群中个体,fitness越大越好
		std::sort(schemes.begin(), schemes.end(), [](const packing_scheme& a, const packing_scheme& b) {
			return  b.fitness < a.fitness; });
		int min_bandwidth_utilization = schemes[0].calc_bandwidth_utilization();
		int iteration_count = 1;
		int max_iter_num = 500;
		std::unordered_set<int> index_set;

		packing_scheme best_scheme= schemes[0];

		do {
			//使得两个种群个体生成后代,后代先装到new_schemes里临时存储一下
			for (size_t i = 0; i < std::ceil(population.size() / 2.0); ++i) {
				new_schemes.push_back(schemes[i].create_child_scheme(schemes[i + 1]));
			}
			for (size_t i = 0; i < std::floor(population.size() / 2.0); ++i) {
				new_schemes.push_back(schemes[i].create_child_scheme(schemes[population.size() - 1 - i]));
			}


			//随机下标取用集合
			for (int i = 0; i < population.size(); i++) {
				index_set.insert(i);
			}
			for (int i = population.size(); i < schemes.size(); i++) {
				//对于不可调度的子代
				if (schemes[i].schedulability_label == false) {
					//TODO 补救一下，让他重新可调度
					std::uniform_int_distribution<int> index_dist(0, index_set.size() / 2);// 均匀分布的随机整数生成器
					int index = index_dist(engine);
					auto it = index_set.begin();
					std::advance(it, index);
					schemes[i] = schemes[*it]; //随机选择高fitness祖先替代不可调度的孩子，范围为population前半部分

					index_set.erase(it); //从下标取用集合中删除已经用过的下标
				}
				else {
					//可以调度的子代直接保留
					schemes[i] = new_schemes[i];
				}
			}
			new_schemes.clear();

			//重新按fitness降序排序
			std::sort(schemes.begin(), schemes.end(), [](const packing_scheme& a, const packing_scheme& b) {
				return  b.fitness < a.fitness; });
			if (best_scheme.fitness < schemes[0].fitness) {
				best_scheme = schemes[0];
			}

			//迭代到max_iter_num次后自动退出
			iteration_count++;
		} while (iteration_count < max_iter_num);

		return best_scheme;

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

	

};



