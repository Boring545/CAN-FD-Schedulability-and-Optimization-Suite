#include "FramePacking.h"

//TODO ���źŰ����Ŵ��㷨���д��
//1.��ʼ������� ����ֻ֡װ��һ��message��ʹ��MILP�������㷨�ĵĽ����Ϊ��ʼ�������
//ʹ��first fit decrease�������д����Ҫ��װ��ʱ������frame����������
//ÿװ��һ��message����Ҫ������message���frame������WCTT��Deadline
//�������n�����������Ĳ�ַ����أ�

std::vector<int> generate_individual(const std::unordered_set<message*>& message_p_set, size_t max_try) {

    // ����һ�����壬��һ�������е�ÿ�� vector �洢һ�� canfd_frame
    std::vector<int> individual(message_p_set.size(), -1);
    std::vector<canfd_frame> frame_list;
    frame_list.reserve(message_p_set.size());

    std::random_device rd;
    std::mt19937 gen(rd());

    // ����һ�������������ѡ����Ϣ
    std::unordered_set<message*> message_p_set_copy(message_p_set.begin(), message_p_set.end());


    // ������Ⱥ�е�һ������
    for (size_t i = 0; !message_p_set_copy.empty(); ++i) {
        // ����һ�� canfd_frame ����ӿ���Ԫ�أ���������� max_try ��
        canfd_frame frame(i);
        bool added = false;
        for (size_t try_count = 0; try_count < max_try || added == false; ++try_count) {
            std::uniform_int_distribution<int> index_dist(0, message_p_set_copy.size() - 1);
            // �� message_p_set �����ѡ��һ��������Ϣָ��
            auto it = message_p_set_copy.begin();
            std::advance(it, index_dist(gen));
            message* selected_message = *it;

            if (frame.add_message(*selected_message)) {
                individual[selected_message->id] = frame.get_id();//����Ĭ��messageID��0��ʼ��frameҲ��
                added = true;
                message_p_set_copy.erase(selected_message);
                if (message_p_set_copy.empty()) break;
            }
            
        }

        //// �����ɵ� canfd_frame ��ӵ�������
        /*frame_list.emplace_back(frame);*/
    }
    return individual;
}
std::vector<std::vector<int>> initial_population(std::vector<message>& message_set, int num, size_t max_try) {

    //size_t num_messages = message_set.size();

    // ����һ�� unordered_set ���ڸ��ٿ�����Ϣָ��
    std::unordered_set<message*> message_p_set;
    message_p_set.reserve(message_set.size());
    for (message& m : message_set) {
        message_p_set.insert(&m);
    }

    //һ���߳̾������10����������(һ�������Ӧһ������)
    size_t individuals_per_thread = std::max((int)ceil((double)num / std::thread::hardware_concurrency()), (10));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(num) / individuals_per_thread)));

    // ���� vector �洢���ɵ���Ⱥ
    /*std::vector<std::vector<canfd_frame>> population;*/
    std::vector< std::vector<std::vector<int>>> results;
    results.resize(num_threads);

    std::vector<std::vector<int>> individuals;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; i++) {
        int thread_index = i;
        threads.emplace_back([&]() {
            for (size_t j = 0; j < individuals_per_thread && (thread_index * individuals_per_thread + j) < num; ++j) {
                std::unordered_set<message*> message_p_set_copy = message_p_set;
                // ����������� message �ĺ���
                auto indiv = generate_individual(message_p_set_copy, max_try);
                // �����ɵ� message ��ӵ����̵߳���Ϣ������
                results[thread_index].push_back(indiv);
            }
            });
    }
    // �ȴ������߳̽���
    for (auto& thread : threads) {
        thread.join();
    }

    // ��ÿ���߳����ɵ���Ϣ�ϲ��� message_set ��
    for (auto& thread_result : results) {
        for (auto& indiv : thread_result) {
            individuals.emplace_back(std::move(indiv));

        }
    }
    return individuals;





    //// �ظ�������Ⱥֱ��������Ϣ��������
    //while (!message_p_set.empty()) {
    //    // ����һ�����壬��һ�������е�ÿ�� vector �洢һ�� canfd_frame
    //    std::vector<int> individual(num_messages,-1);
    //    std::vector<canfd_frame> frame_list;
    //    // ������Ⱥ�е�һ������
    //    for (size_t i = 0; i < num_messages; ++i) {


    //        // ����һ�������������ѡ����Ϣ
    //        std::unordered_set<message*> message_p_set_copy = message_p_set;

    //        // ����һ�� canfd_frame ����ӿ���Ԫ�أ���������� max_try ��
    //        canfd_frame frame;
    //        bool added = false;
    //        for (size_t try_count = 0; try_count < max_try||added==false; ++try_count) {
    //            // �� message_p_set �����ѡ��һ��������Ϣָ��
    //            auto it = message_p_set_copy.begin();
    //            std::advance(it, rand() % message_p_set_copy.size());
    //            message* selected_message = *it;

    //            if (frame.add_message(*selected_message)) {
    //                individual[selected_message->id] = frame.get_id();//����Ĭ��messageID��0��ʼ��frameҲ��
    //                added = true;
    //            }
    //            message_p_set_copy.erase(selected_message);
    //        }

    //        // �����ɵ� canfd_frame ��ӵ�������
    //        frame_list.push_back(frame);
    //    }

    //    // ��������ӵ���Ⱥ��
    //    population.push_back(frame_list);
    //}
    //return population;
}