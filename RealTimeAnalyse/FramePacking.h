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
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size, int frame_count, size_t max_try=5);
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size, size_t max_try = 5);
class FramePacking
{
public:
	std::vector<message>&message_list;
	using FitnessFunction = std::function<double(std::vector<canfd_frame*>)>;
	canfd_utils canfd_setting;
	int population_size = 10;
	int frame_num = 9;//TODO ��������ֵ����ͨ��message_list����Ϣ�Զ�ȷ��
	FramePacking(std::vector<message>& _message_list, canfd_utils _canfd_setting):message_list(_message_list){
		this->canfd_setting = _canfd_setting;
	}
	//����fitness��ͬʱ��Ҳ���Է������ȼ�
	double calc_fitness(std::vector<canfd_frame*> frame_list) {
		return 1.0 / (assign_priority(frame_list) + canfd_utils().calc_bandwidth_utilization(frame_list));
	}
	std::vector<canfd_frame> message_pack() {
		std::vector<std::vector<canfd_frame*>> population;   //�洢��Ⱥ
		//�������ɵ���Ⱥ�еĸ��岻һ�����ɵ���
		auto individuals = initial_population(population, message_list, population_size);  //individuals�е�ÿ�������ʾһ��ӳ���ϵ��index�±��Ӧ��message�����䵽ֵ��Ӧ��frame��
		std::vector<FitnessPopulationPair>fp(population.size());   //�洢ÿ���������fitness�Ķ�Ӧ��ϵ

		for (size_t i = 0; i < population.size(); i++) {
			assign_offset(population[i]);//��Ϊÿ�������е�����֡������ʵ�offset
			fp.push_back(FitnessPopulationPair(population[i], individuals[i],calc_fitness(population[i])));//����ÿ�������fitness�������Է������ȼ�
		}
		//��fitness����������Ⱥ�и���
		std::sort(fp.begin(), fp.end(), [](const FitnessPopulationPair& a, const FitnessPopulationPair& b) {
			return  b.fitness< a.fitness; });
		//TODO ʹ��������Ⱥ�������ɺ��

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
		std::vector<canfd_frame> x;
		return x;



	}
private:
	//FitnessFunction calc_fitness;
	class FitnessPopulationPair {

	public:
		double fitness=0;
		std::vector<canfd_frame*>* frames;
		std::vector<int>*individual;
		FitnessPopulationPair() { frames = nullptr; individual = nullptr; }
		FitnessPopulationPair(std::vector<canfd_frame*>& _frames, std::vector<int>& _individual,double _fitness):frames(&_frames), individual(&_individual),fitness(_fitness){}
		void create_child(FitnessPopulationPair &fp) {
			std::random_device rd; // ������������
			std::mt19937 gen(rd()); // α�����������
			double mean = individual->size() / 2;
			double stddev = mean * 1.1;
			std::normal_distribution<double> dis(mean, stddev); // ���ȷֲ����������������
			int crossover_point = 0; //��ȡ��������
			do {
				crossover_point = (int)dis(gen);
			} while (crossover_point<1 || crossover_point>individual->size() - 2);
			//TODO��individual��fp.individual��crossover_pointΪ���ģ������Է������ݣ������µ�����individual
			std::vector<int> child1 = *individual;
			std::vector<int> child2 = *(fp.individual);
			child1.reserve(individual->size());
			child2.reserve(individual->size());
			// �� individual �����ݲ��Ϊ�����ֲ����浽 child1 �� child2
			child1.assign(individual->begin(), individual->begin() + crossover_point);
			child1.insert(child1.end(), fp.individual->begin() + crossover_point, fp.individual->end());

			child2.assign(fp.individual->begin(), fp.individual->begin() + crossover_point);
			child2.insert(child2.end(), individual->begin() + crossover_point, individual->end());
			//�������� ���ƺ���Ϊ ���ȼ�һ���ܷ���ɹ��������ڴ�������⣬�е�������޷����

			//TODO ��������child���ۼ�ÿһ�������ÿһ��֡�У���ͬ���ڵ�message�������ִ�����ÿ������ά��һ����������ѡ�����������ĸ��壬�綼һ�£�ѡ������֡С�ĸ���
			//���������ո���Pѡ��һ���Ѿ������֡�������е��źź������ֵ���������֡�У�����������һ��֡
			// //���ܿ��ܸò�������Ϊ��������֡�������޷���ֻ򽵵Ϳɵ����ԡ���
			// ��˽���Pѡ��Ҫ̫��
			//����һ��VECTOR<canfd_frame> ������offset�����ɵ�����
			// ������߱��ɵ����ԣ�ѡ����Щ�����ȼ����񣬳��Խ����Ǻϲ������ٸ����ȼ�������������Ӷ�������Ե����ȼ�����ĸ���
			// ������һ�����⣬���Ǻϲ������ȼ�֡������ܻᵼ�ºϲ���ĸ����ȼ�֡���ڸ�С���Ӷ����������Ӱ��
			// Ӧ�����һ������������֡���ڵĴ����ʽ��ҲҪ��˼��ٸ����ȼ�����֡������
			// ����Բ���ͨ����⣬��Ӹ��������ѡ�񼸸���fitness������Ϊ�Ӵ���
			//����һ���µ�FitnessPopulationPair������




		}
	};

};



