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
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, size_t population_size, size_t max_try = 5);
using message_list_ptr = std::vector<message>*; //ֱ������ϵͳ�е�Ψһmessage���ϣ�����ʱ���ͷ�
using frame_list_ptr = std::vector<canfd_frame*>*; //ÿ�����������ά��һ��frame���ϣ�����ʱҪ�ͷſռ�
using message_list = std::vector<message>; //ֱ������ϵͳ�е�Ψһmessage���ϣ�����ʱ���ͷ�
using frame_list = std::vector<canfd_frame*>; //ÿ�����������ά��һ��frame���ϣ�����ʱҪ�ͷſռ�
using message_map_list = std::vector<int>;

//��װ��һ��canfd_frame
class packing_scheme {
public:
	frame_list_ptr frames_p=nullptr;
	message_list_ptr messages_p = nullptr;
	message_map_list individual;//����individual�Զ�����frames����
	double fitness = 0;
	canfd_utils* canfd_setting = nullptr;
	std::mt19937 engine;
	bool schedulability_label=false;//true �ɵ��ȣ�Ĭ��false���ɵ���
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
		////TODO �ڴ�й¶����
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
			//����child������Ӧ��message�����Ӧ��frame��
			//����Ĭ��individual��Ԫ���Ǵ�0~1������
			if (frames[individual[i]]->add_message(messages[i])==false) {
				std::cerr << "if (child_frame_list[child[i]]->add_message(outer->message_list[i])) ERRORRRRRRRRRRR\n";
				abort();
			}
		}
		assign_offset(frames);
		schedulability_label = assign_priority(frames);//�������ȼ�
		fitness = calc_fitness();
	}
	//�������������
	double calc_bandwidth_utilization() {
		double BWU = 0;
		auto& frames = *frames_p;
		for (size_t i = 0; i < frames.size(); i++) {
			BWU += ((double)canfd_setting->calc_wctt(frames[i]->get_paylaod_size()) / frames[i]->get_period());
		}
		return BWU;
	}
	//����fitness
	double calc_fitness() {
		auto& frames = *frames_p;
		return 1.0 / ((schedulability_label?0:1) + calc_bandwidth_utilization());
	}
	//�����Ŵ��㷨�����Ӵ�
	packing_scheme create_child_scheme(packing_scheme& other) {
		//TODO��individual��fp.individual��crossover_pointΪ���ģ������Է������ݣ������µ�����individual
		std::uniform_int_distribution<int> cross_point_dist(1, messages_p->size() - 2);// ���ȷֲ����������������
		int crossover_point = cross_point_dist(engine);

		message_map_list map1;
		message_map_list map2;
		// �� individual �����ݲ��Ϊ�����ֲ����浽 child1 �� child2��������ܵ��³���map��id�����������⡿
		map1.assign(individual.begin(), individual.begin() + crossover_point);
		map1.insert(map1.end(), other.individual.begin() + crossover_point, other.individual.end());

		map2.assign(other.individual.begin(), other.individual.begin() + crossover_point);
		map2.insert(map2.end(), individual.begin() + crossover_point, individual.end());

		//��ȡmap���ʶ�����֡����
		int frame_num1 = *std::max_element(map1.begin(), map1.end())+1;
		int frame_num2 = *std::max_element(map2.begin(), map2.end())+1;

		//count ��¼��ÿ������֡������
		std::vector<int> count1(frame_num1, 0), count2(frame_num2, 0);

		//����ÿ������֡���ڵ�gcd
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

		double avg1 = std::accumulate(count1.begin(), count1.end(), 0) / frame_num1; //child1��ƽ������
		double avg2 = std::accumulate(count2.begin(), count2.end(), 0) / frame_num2; //child2��ƽ������

		//���ݸ÷����������ָߵ��Ǹ��Ӵ�
		double score1 = my_algorithm::normalizeValue((double)frame_num1, 0.0, 1.0) + my_algorithm::normalizeValue(avg1, 0.0, 1.0);
		double score2 = my_algorithm::normalizeValue((double)frame_num2, 0.0, 1.0) + my_algorithm::normalizeValue(avg2, 0.0, 1.0);


		message_map_list& map = map1;
		int& frame_num = frame_num1;
		if (score1 < score2) {
			map =map2;//�����Ӵ�2
			frame_num = frame_num2;
		}
		else {
			map = map1;//�����Ӵ�1
			frame_num = frame_num1;
		}

		// ����һ�����ȷֲ����󣬷�Χ�� [0, 1)
		std::uniform_real_distribution<double> dis_P(0.0, 1.0);
		double mutation_P = 0.9; //P����0.9���ܱ��죬����Ϊ10%
		double P = dis_P(engine);

		frame_list& frames = *frames_p;
		std::uniform_int_distribution<int> index_dist(0, frame_num-1);
		int index = index_dist(engine);//���ѡһ������֡

		//����,���ѡһ������֡���������ݲ嵽��������֡��
		if (P > mutation_P) {

			for (size_t i = 0; i < individual.size(); i++) {
				while (map[i] == index) {
					map[i] = index_dist(engine);
				}
				//if (map[i] > index) {
				//	map[i]--;//ȥ��������֡����ű�������Զ���Сһ��
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
	int frame_num = 9;//TODO ��������ֵ����ͨ��message_list����Ϣ�Զ�ȷ��
	std::mt19937 engine;
	FramePacking(std::vector<message>& _message_list, canfd_utils& _canfd_setting):message_list(_message_list){
		this->canfd_setting = _canfd_setting;
		this->engine = std::mt19937(std::random_device{}());
	}

	//ʹ���Ŵ��㷨���
	packing_scheme message_pack() {
		std::vector<std::vector<canfd_frame*>> population;   //�洢��Ⱥ
		//�������ɵ���Ⱥ�еĸ��岻һ�����ɵ���
		auto individuals = initial_population(population, message_list, population_size);  //individuals�е�ÿ�������ʾһ��ӳ���ϵ��index�±��Ӧ��message�����䵽ֵ��Ӧ��frame��

		std::vector<packing_scheme>schemes;   //�洢ÿ���������fitness�Ķ�Ӧ��ϵ
		std::vector<packing_scheme>new_schemes;   //�洢ÿ���������fitness�Ķ�Ӧ��ϵ
		schemes.reserve(population.size());
		for (size_t i = 0; i < population.size(); i++) {
			schemes.emplace_back(message_list, individuals[i], canfd_setting);
		}
		//��fitness����������Ⱥ�и���,fitnessԽ��Խ��
		std::sort(schemes.begin(), schemes.end(), [](const packing_scheme& a, const packing_scheme& b) {
			return  b.fitness < a.fitness; });
		int min_bandwidth_utilization = schemes[0].calc_bandwidth_utilization();
		int iteration_count = 1;
		int max_iter_num = 500;
		std::unordered_set<int> index_set;

		packing_scheme best_scheme= schemes[0];

		do {
			//ʹ��������Ⱥ�������ɺ��,�����װ��new_schemes����ʱ�洢һ��
			for (size_t i = 0; i < std::ceil(population.size() / 2.0); ++i) {
				new_schemes.push_back(schemes[i].create_child_scheme(schemes[i + 1]));
			}
			for (size_t i = 0; i < std::floor(population.size() / 2.0); ++i) {
				new_schemes.push_back(schemes[i].create_child_scheme(schemes[population.size() - 1 - i]));
			}


			//����±�ȡ�ü���
			for (int i = 0; i < population.size(); i++) {
				index_set.insert(i);
			}
			for (int i = population.size(); i < schemes.size(); i++) {
				//���ڲ��ɵ��ȵ��Ӵ�
				if (schemes[i].schedulability_label == false) {
					//TODO ����һ�£��������¿ɵ���
					std::uniform_int_distribution<int> index_dist(0, index_set.size() / 2);// ���ȷֲ����������������
					int index = index_dist(engine);
					auto it = index_set.begin();
					std::advance(it, index);
					schemes[i] = schemes[*it]; //���ѡ���fitness����������ɵ��ȵĺ��ӣ���ΧΪpopulationǰ�벿��

					index_set.erase(it); //���±�ȡ�ü�����ɾ���Ѿ��ù����±�
				}
				else {
					//���Ե��ȵ��Ӵ�ֱ�ӱ���
					schemes[i] = new_schemes[i];
				}
			}
			new_schemes.clear();

			//���°�fitness��������
			std::sort(schemes.begin(), schemes.end(), [](const packing_scheme& a, const packing_scheme& b) {
				return  b.fitness < a.fitness; });
			if (best_scheme.fitness < schemes[0].fitness) {
				best_scheme = schemes[0];
			}

			//������max_iter_num�κ��Զ��˳�
			iteration_count++;
		} while (iteration_count < max_iter_num);

		return best_scheme;

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

	

};



