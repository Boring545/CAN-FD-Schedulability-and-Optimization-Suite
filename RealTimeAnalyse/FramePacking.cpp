#include "FramePacking.h"

//TODO 将信号按照遗传算法进行打包
//1.初始打包方案 数据帧只装载一个message、使用MILP或其他算法的的结果作为初始打包方案
//使用first fit decrease方案进行打包，要求装载时不超过frame承载能力，
//每装载一个message，都要检测加入message后的frame，满足WCTT≤Deadline
//如何生成n种满足条件的拆分方案呢？

std::vector<int> generate_individual(const std::unordered_set< message*>& message_p_set, std::vector<canfd_frame*>& frame_list, size_t max_try) {

    // 创建一个个体，即一个个体中的每个 vector 存储一个 canfd_frame
    std::vector<int> individual(message_p_set.size(), -1);
    frame_list.clear();
    frame_list.reserve(message_p_set.size());

    std::random_device rd;
    std::mt19937 gen(rd());

    // 创建一个副本用于随机选择消息
    std::unordered_set<message*> message_p_set_copy(message_p_set.begin(), message_p_set.end());


    // 生成种群中的一个个体
    for (size_t i = 0; !message_p_set_copy.empty(); ++i) {
        // 创建一个 canfd_frame 并添加可行元素，最多可以添加 max_try 个
        canfd_frame* frame=new canfd_frame(i);
        bool added = false;
        for (size_t try_count = 0; try_count < max_try || added == false; ++try_count) {
            std::uniform_int_distribution<int> index_dist(0, message_p_set_copy.size() - 1);
            // 从 message_p_set 中随机选择一个可用消息指针
            auto it = message_p_set_copy.begin();
            std::advance(it, index_dist(gen));
            message* selected_message = *it;

            if (frame->add_message(*selected_message)) {
                individual[selected_message->id] = frame->get_id();//这里默认messageID从0开始，frame也是
                added = true;
                message_p_set_copy.erase(selected_message);
                if (message_p_set_copy.empty()) break;
            }
            
        }

        //// 将生成的 canfd_frame 添加到个体中
        frame_list.emplace_back(frame);
    }
    return individual;
}
std::vector<std::vector<int>> initial_population(std::vector<std::vector<canfd_frame*>>& population, std::vector<message>& message_set, int population_size,int frame_count,  size_t max_try) {
    // 创建一个 unordered_set 用于跟踪可用消息指针
    std::unordered_set<message*> message_p_set;
    message_p_set.reserve(message_set.size());
    for (message& m : message_set) {
        message_p_set.insert(&m);
    }

    //一个线程尽量完成10个生成任务(一个任务对应一个个体)
    size_t individuals_per_thread = std::max((int)ceil((double)population_size / std::thread::hardware_concurrency()), (10));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(population_size) / individuals_per_thread)));

    // 创建 vector 存储生成的种群
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
                // 调用生成随机 message 的函数,用temp接收打包好的数据帧集合，message_p_set是传入的消息集合，indiv为数组形式的数据帧、message对应关系
                auto indiv = generate_individual(message_p_set, temp, max_try);
                if (temp.size() == frame_count) {
                    canfd_list_results[thread_index].push_back(temp);
                    // 将生成的 message 添加到该线程的消息集合中
                    indiv_results[thread_index].push_back(indiv);
                    j++;
                }
            }
            });
    }
    // 等待所有线程结束
    for (auto& thread : threads) {
        thread.join();
    }

    // 将每个线程生成的消息合并到 message_set 中
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
    // 创建一个 unordered_set 用于跟踪可用消息指针
    std::unordered_set<message*> message_p_set;
    message_p_set.reserve(message_set.size());
    for (message& m : message_set) {
        message_p_set.insert(&m);
    }

    //一个线程尽量完成10个生成任务(一个任务对应一个个体)
    size_t individuals_per_thread = std::max((int)ceil((double)population_size / std::thread::hardware_concurrency()), (10));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(population_size) / individuals_per_thread)));

    // 创建 vector 存储生成的种群
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
                // 调用生成随机 message 的函数,用temp接收打包好的数据帧集合，message_p_set是传入的消息集合，indiv为数组形式的数据帧、message对应关系
                auto indiv = generate_individual(message_p_set, temp, max_try);
                canfd_list_results[thread_index].push_back(temp);
                // 将生成的 message 添加到该线程的消息集合中
                indiv_results[thread_index].push_back(indiv);
                j++;
            }
            });
    }
    // 等待所有线程结束
    for (auto& thread : threads) {
        thread.join();
    }

    // 将每个线程生成的消息合并到 message_set 中
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