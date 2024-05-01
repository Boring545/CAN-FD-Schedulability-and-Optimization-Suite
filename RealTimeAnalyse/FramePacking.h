#pragma once

#include<numeric>
#include<mutex>
#include <thread>
#include <functional>
#include"PriorityArrangement.h"
#include"offset_arrangement.h"
//�������һ��������ʽ�����Ĵ��������max_tryΪһ��֡��װ��message���������
std::vector<int> generate_individual(const std::unordered_set< message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try = 5);
//�����������Ϊnum��population
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
	//����fitness��ͬʱ��Ҳ���Է������ȼ�
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
		//fitness��������
		std::sort(fp.begin(), fp.end(), [](const FitnessPopulationPair& a, const FitnessPopulationPair& b) {
			return  b.fitness< a.fitness; });
		//TODO ѡ��һЩ���壬���������ɺ��������Ϊ���㽻�棺ѡ����� i Ϊ1~ceiling��p/2���ĸ��壬�� i �� i+1 ����crossover����ѡ����� jΪ1~floor��p/2���ĸ��壬������ n+1-j �ŵĸ���crossover
		//crossoverѡ�õ��㽻�棬�Ӹ�ĸ���������ѡ����ͬindex��Ϊ����㣬�����������һ�뵽�Լ����ϣ�
		//���ɵ������½����Ϊ�Ӵ���Ҫ���Ӵ���ѡ��һ����������̭��һ����
		//��̭����Ϊ������1��������󲿷��ź�����һ�£�������ֽ����֡�����٣��Ӽ����٣����Ӵ���
		//����2������ź����ڱ˴˲�ͬ���ۼӼ�����ͬһ֡����ͬ�����źų��ֵĴ����������ô��������Ӵ�

		//�Եõ��ĺ����������P���죬���ѡ��һ���������֡���������ź�����Ĳ�ֵ�������֡��

		//�����ɵ���ȺҲҪ����offset���������ȼ�������fitness�� 
		// ���ڵõ��Ĳ��ɵ��ȵ��Ӵ���Ҫȷ���ĸ�֡�����Ե��ȣ�Ȼ�����ѡ���������ߵĸ����ȼ�֡�������Ǻϲ���
		// Ȼ�����·����ɵ����ԣ�������ǲ���
		//�������ո��ʶ����޷����ȵĸ��壬Ȼ��Ӹ����������ѡ���������滻��

		//ֱ�����㵽U����ô����Ϊֹ�����ߵ��������޴�����ѡ��fitness�����Ÿ�����Ϊ���ǵĵ�������





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



