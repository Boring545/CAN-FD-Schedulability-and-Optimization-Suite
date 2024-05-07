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
		return 1.0 / (assign_priority(frame_list) +calc_bandwidth_utilization(frame_list));
	}
	//�������������
double calc_bandwidth_utilization(const std::vector<canfd_frame*>& frameSet) {
    double BWU = 0;
    for (size_t i = 0; i < frameSet.size(); i++) {
        BWU += ((double)canfd_setting.calc_wctt(frameSet[i]->get_paylaod_size()) / frameSet[i]->get_period());
    }
    return BWU;
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

		int individual_size = individuals[0].size();

		std::random_device rd; // ������������
		std::mt19937 gen(rd()); // α�����������
		double mean = individual_size / 2; //ϣ����������λ���м䣬ʹ�ø����Ŵ���Ϣ������
		double stddev = 1.0;
		std::normal_distribution<double> dis(mean, stddev); // ���ȷֲ����������������


		//TODO ʹ��������Ⱥ�������ɺ��
		for (size_t i = 0; i < std::ceil(population.size()/2.0); ++i) {
			int crossover_point = 0; //��ȡ��������
			do {
				crossover_point = (int)dis(gen);
			} while (crossover_point<1 || crossover_point>individual_size - 2);//������ѭ������
			fp[i].create_child(fp[i + 1], crossover_point,this);
		}
		for (size_t i = 0; i < std::floor(population.size() / 2.0); ++i) {
			int crossover_point = 0; //��ȡ��������
			do {
				crossover_point = (int)dis(gen);
			} while (crossover_point<1 || crossover_point>individual_size - 2);//������ѭ������
			fp[i].create_child(fp[population.size()-1-i], crossover_point,this);
		}


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
		std::vector<int>individual;
		FitnessPopulationPair() { frames = nullptr; }
		FitnessPopulationPair(std::vector<canfd_frame*>& _frames, std::vector<int>& _individual,double _fitness):frames(&_frames), individual(_individual),fitness(_fitness){}
		FitnessPopulationPair create_child(FitnessPopulationPair &fp,int crossover_point, FramePacking* outer) {

			//TODO��individual��fp.individual��crossover_pointΪ���ģ������Է������ݣ������µ�����individual
			std::vector<int> child1 = individual;
			std::vector<int> child2 = (fp.individual);
			child1.reserve(individual.size());
			child2.reserve(individual.size());
			// �� individual �����ݲ��Ϊ�����ֲ����浽 child1 �� child2
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
			double avg1 = std::accumulate(count1.begin(), count1.end(), 0) / frame_num1; //child1��ƽ������
			double avg2 = std::accumulate(count2.begin(), count2.end(), 0) / frame_num2; //child2��ƽ������

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

			// ����һ�����������
			std::random_device rd;
			std::mt19937 gen(rd());
			// ����һ�����ȷֲ����󣬷�Χ�� [0, 1)
			std::uniform_real_distribution<double> dis_P(0.0, 1.0);
			double mutation_P = 0.9;
			double P = dis_P(gen);

			std::uniform_real_distribution<int> dis_index(0, (*frames).size() - 1);
			int index = dis_index(gen);

			//����,���ѡһ������֡���������ݲ嵽��������֡��
			if (P > mutation_P) {

				for (size_t i = 0; i < individual.size(); i++) {
					if (child[i] == index) {
						child[i] = dis_index(gen);
					}
				}
				fnum -= 1;
			}
			//TODO ����child����һ��frame����
			std::vector<canfd_frame*>* child_frame_list_p=new std::vector<canfd_frame*>(fnum,nullptr);
			auto& child_frame_list = (*child_frame_list_p);
			//canfd_frame* frame = new canfd_frame(i);
			for (size_t i = 0; i < individual.size(); i++) {
				//����child������Ӧ��message�����Ӧ��frame��
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
			assign_priority(child_frame_list);//�б�Ҫ�޸�assign_priorityʹ�ã���ʹ���ܵ���Ҳ��ǿ�з��䡣
			return FitnessPopulationPair(child_frame_list, child, outer->calc_fitness(child_frame_list));

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



