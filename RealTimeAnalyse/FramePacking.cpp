#include "FramePacking.h"

//TODO ���źŰ����Ŵ��㷨���д��
//1.��ʼ������� ����ֻ֡װ��һ��message��ʹ��MILP�������㷨�ĵĽ����Ϊ��ʼ�������
//ʹ��first fit decrease�������д����Ҫ��װ��ʱ������frame����������
//ÿװ��һ��message����Ҫ������message���frame������WCTT��Deadline
//�������n�����������Ĳ�ַ����أ�

std::vector<int> generate_individual(const std::unordered_set< message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try) {

    // ����һ�����壬��һ�������е�ÿ�� vector �洢һ�� canfd_frame
    std::vector<int> individual(message_p_set.size(), -1);
    frame_list.clear();
    frame_list.reserve(message_p_set.size());

    std::random_device rd;
    std::mt19937 gen(rd());

    // ����һ�������������ѡ����Ϣ
    std::unordered_set<message*> message_p_set_copy(message_p_set.begin(), message_p_set.end());


    // ������Ⱥ�е�һ������
    for (size_t i = 0; !message_p_set_copy.empty(); ++i) {
        // ����һ�� canfd_frame ����ӿ���Ԫ�أ���������� max_try ��
        canfd_frame* frame=new canfd_frame(i);
        bool added = false;
        for (size_t try_count = 0; try_count < max_try || added == false; ++try_count) {
            std::uniform_int_distribution<int> index_dist(0, message_p_set_copy.size() - 1);
            // �� message_p_set �����ѡ��һ��������Ϣָ��
            auto it = message_p_set_copy.begin();
            std::advance(it, index_dist(gen));
            message* selected_message = *it;

            if (frame->add_message(*selected_message)) {
                individual[selected_message->id] = frame->get_id();//����Ĭ��messageID��0��ʼ��frameҲ��
                added = true;
                message_p_set_copy.erase(selected_message);
                if (message_p_set_copy.empty()) break;
            }
            
        }

        //// �����ɵ� canfd_frame ��ӵ�������
        frame_list.emplace_back(frame);
    }
    return individual;
}
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size,int frame_count,  size_t max_try) {
    // ����һ�� unordered_set ���ڸ��ٿ�����Ϣָ��
    std::unordered_set<message*> message_p_set;
    message_p_set.reserve(message_set.size());
    for (message& m : message_set) {
        message_p_set.insert(&m);
    }

    //һ���߳̾������10����������(һ�������Ӧһ������)
    size_t individuals_per_thread = std::max((int)ceil((double)population_size / std::thread::hardware_concurrency()), (10));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(population_size) / individuals_per_thread)));

    // ���� vector �洢���ɵ���Ⱥ
    std::vector< std::vector<std::vector<int>>> indiv_results;
    std::vector< std::vector<std::vector<canfd_frame*>>> canfd_list_results;

    indiv_results.resize(num_threads);
    canfd_list_results.resize(num_threads);

    std::vector<std::vector<int>> individuals;
    std::vector<std::thread> threads;


    for (size_t i = 0; i < num_threads; i++) {
        const int thread_index = i;
        threads.emplace_back([&]() {
            std::vector<canfd_frame*> temp;
            for (size_t j = 0; j < individuals_per_thread && (thread_index * individuals_per_thread + j) < population_size;) {
                // ����������� message �ĺ���,��temp���մ���õ�����֡���ϣ�message_p_set�Ǵ������Ϣ���ϣ�indivΪ������ʽ������֡��message��Ӧ��ϵ
                auto indiv = generate_individual(message_p_set, temp, max_try);
                if (temp.size() == frame_count) {
                    canfd_list_results[thread_index].push_back(temp);
                    // �����ɵ� message ��ӵ����̵߳���Ϣ������
                    indiv_results[thread_index].push_back(indiv);
                    j++;
                }
            }
            });
    }
    // �ȴ������߳̽���
    for (auto& thread : threads) {
        thread.join();
    }

    // ��ÿ���߳����ɵ���Ϣ�ϲ��� message_set ��
    for (auto& thread_result : indiv_results) {
        for (auto& indiv : thread_result) {
            individuals.emplace_back(std::move(indiv));
        }
    }
    for (auto& thread_result : canfd_list_results) {
        for (auto& indiv : thread_result) {
            population.push_back(std::move(indiv));
        }
    }
    return individuals;
}
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size, size_t max_try) {
    // ����һ�� unordered_set ���ڸ��ٿ�����Ϣָ��
    std::unordered_set<message*> message_p_set;
    message_p_set.reserve(message_set.size());
    for (message& m : message_set) {
        message_p_set.insert(&m);
    }

    //һ���߳̾������10����������(һ�������Ӧһ������)
    size_t individuals_per_thread = std::max((int)ceil((double)population_size / std::thread::hardware_concurrency()), (10));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(population_size) / individuals_per_thread)));

    // ���� vector �洢���ɵ���Ⱥ
    std::vector< std::vector<std::vector<int>>> indiv_results;
    std::vector< std::vector<std::vector<canfd_frame*>>> canfd_list_results;

    indiv_results.resize(num_threads);
    canfd_list_results.resize(num_threads);

    std::vector<std::vector<int>> individuals;
    std::vector<std::thread> threads;
    

    for (size_t i = 0; i < num_threads; i++) {
        const int thread_index = i;
        threads.emplace_back([&]() {
            std::vector<canfd_frame*> temp;
            for (size_t j = 0; j < individuals_per_thread && (thread_index * individuals_per_thread + j) < population_size;) {
                // ����������� message �ĺ���,��temp���մ���õ�����֡���ϣ�message_p_set�Ǵ������Ϣ���ϣ�indivΪ������ʽ������֡��message��Ӧ��ϵ
                auto indiv = generate_individual(message_p_set, temp, max_try);
                canfd_list_results[thread_index].push_back(temp);
                // �����ɵ� message ��ӵ����̵߳���Ϣ������
                indiv_results[thread_index].push_back(indiv);
                j++;
            }
            });
    }
    // �ȴ������߳̽���
    for (auto& thread : threads) {
        thread.join();
    }

    // ��ÿ���߳����ɵ���Ϣ�ϲ��� message_set ��
    for (auto& thread_result : indiv_results) {
        for (auto& indiv : thread_result) {
            individuals.emplace_back(std::move(indiv));
        }
    }
    for (auto& thread_result : canfd_list_results) {
        for (auto& indiv : thread_result) {
            population.push_back(std::move(indiv));
        }
    }
    return individuals;
}