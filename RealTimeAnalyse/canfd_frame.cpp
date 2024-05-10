#include "canfd_frame.h"



//文件结构：ID（int）   datasize(int)    period(int)  deadline(int)    priority（int）   exec_time（int）  data
//文件开始为描述信息，如果读取到空行，此后每行表示一个message，数据间使用\t分割
//data_size, period, deadline, priority, exec_time ≥0，priority ∈[0, 2047], data size 不应超过 64 bytes
//deadline 应小于 period, exec_time 应小于 deadline
std::vector<message> message::read_messages(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<message> messages;

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return messages; // 返回空 vector
    }

    std::string line;
    int line_number = 0;

    // 跳过描述信息
    while (std::getline(file, line) && !line.empty()) {
        line_number++;
    }

    while (std::getline(file, line)) {
        line_number++;

        // 使用 stringstream 分割行内容
        std::istringstream line_stream(line);
        std::string id_str, data_size_str, period_str, deadline_str, priority_str, exec_time_str, data;

        // 逐个读取以制表符为分隔符的字段
        if (!std::getline(line_stream, id_str, '\t')) {
            continue;
        }
        if (!std::getline(line_stream, data_size_str, '\t')) {
            continue;
        }
        if (!std::getline(line_stream, period_str, '\t')) {
            continue;
        }
        if (!std::getline(line_stream, deadline_str, '\t')) {
            continue;
        }
        if (!std::getline(line_stream, priority_str, '\t')) {
            continue;
        }
        if (!std::getline(line_stream, exec_time_str, '\t')) {
            continue;
        }
        std::getline(line_stream, data);

        // 将字符串转换为对应的整数值
        int id = std::stoi(id_str);
        int data_size = std::stoi(data_size_str);
        int period = std::stoi(period_str);
        double deadline = std::stod(deadline_str);
        int priority = std::stoi(priority_str);
        int exec_time = std::stoi(exec_time_str);

        // 输入合法性检查
        if (data_size < 0 || period < 0 || deadline < 0 || priority < 0 || exec_time < 0 || data_size > 64 || priority > 2047) {
            std::cerr << "Error: 不合法的输入 位于 行 " << line_number << "\n(提示：data_size, period, deadline, priority, exec_time 应非负, priority 应属于 [0 , 2047], data size 不应超过 64 bytes)" << std::endl;
            continue; // 跳过当前行
        }

        if (deadline > period || exec_time > deadline) {
            std::cerr << "Error: 不合法的输入 位于 行 " << line_number << " (deadline 应小于 period, exec_time 应小于 deadline)" << std::endl;
            continue; // 跳过当前行
        }

        // 创建 message 对象并添加到 messages 中
        message m(id, data_size, period, deadline, priority, exec_time, data);
        messages.push_back(m);
    }

    file.close(); // 关闭文件

    return messages;
}
//将message_set写入filename内，append=true使得直接在上次的内容后追加写入
void message::write_messages(const std::vector<message>& message_set, const std::string& filename, bool append) {
    std::ofstream file;
    if (append) {
        file.open(filename, std::ios::app); // 追加模式
    }
    else {
        file.open(filename); // 默认模式，不追加
    }

    if (!file.is_open()) {
        std::cerr << "Error:无法打开文件 " << filename << std::endl;
        return;
    }
    // 写入消息描述信息，仅在非追加模式下写入
    if (!append) {
        file << "文件结构：ID（int）\tdatasize(int)\tperiod(int)\tdeadline(int)\tpriority（int)\texec_time（int）\tdata(字符串)\n"<<"===============================================================\n\n";
    }

    // 逐个写入每个 message
    for (const auto& m : message_set) {
        file << m.id << '\t' << m.data_size << '\t' << m.period << '\t' << m.deadline << '\t' << m.priority << '\t' << m.exec_time << '\t' << m.data << '\n';
    }
    file.close();
}
void message::write_messages(const std::vector<message>& message_set, int ecu_id, const std::string& directory, bool append) {
    // 构造文件名
    std::string filename = directory + "/ecu" + std::to_string(ecu_id) + "_messages.txt";
    // 调用原始函数
    write_messages(message_set, filename, append);
}

std::vector<message> message::read_messages(int ecu_id, const std::string& directory) {
    // 构造文件名
    std::string filename = directory + "/ecu" + std::to_string(ecu_id) + "_messages.txt";
    return read_messages(filename);
}
message message::generate_random_message(std::unordered_set<int>& available_ids, std::mutex& id_mutex,int period_base) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::unique_lock<std::mutex> lock(id_mutex); // 对 id_mutex 进行加锁

    if (available_ids.empty()) {
        return message(); // 如果可用id的集合为空，返回默认的 message 对象
    }

    // 生成随机迭代器
    auto it = available_ids.begin();
    std::advance(it, std::uniform_int_distribution<int>(0, available_ids.size() - 1)(gen));

    // 获取随机选择的id
    int id = *it;

    // 将已选择的id从可用id集合中移除
    available_ids.erase(it);

    lock.unlock(); // 解锁

    // 生成随机的 period、deadline、priority、exec_time 和 data_size

    //////////////////随机生成周期
    std::uniform_int_distribution<int> period_mul_dist(1,10); 
    int period = period_base*period_mul_dist(gen);

    //////////////随机生成截止日期
    double mean = (double)period;
    double stddev = 3.0; // 标准差
    // 创建正态分布对象
    std::normal_distribution<double> deadline_dist(mean, stddev);
    // 生成正态分布的随机数
    double deadline_double;
    do {
        deadline_double = deadline_dist(gen);
    } while (deadline_double <= canfd_utils().worst_wctt || deadline_double >= period); // 保证 deadline 在 [worst_wctt, period] 范围内
    double deadline =deadline_double;

    /////////////随机生成优先级【待删除】
    std::uniform_int_distribution<int> priority_dist(0, 2047);
    int priority = priority_dist(gen);


    ////////////////随机生成执行时【待删除】
    mean = 1.0; // 希望趋近于1
    stddev = 2; // 标准差设置为 0.5 或更小的值
    // 创建正态分布对象
    std::normal_distribution<double> exec_time_dist(mean, stddev);

    // 生成正态分布的随机数
    double exec_time_double;
    do {
        exec_time_double = exec_time_dist(gen);
    } while (exec_time_double > deadline); // 保证 exec_time 在 deadline范围内

    // 将随机数取整作为 exec_time
    int exec_time = std::max(1, (int)exec_time_double);

    ////////////////////随机生成数据
    mean = 32;
    stddev = 3;
    // 创建正态分布对象
    std::normal_distribution<double> data_size_dist(mean, stddev);
    double data_size=0;
    do {
        data_size = (int)data_size_dist(gen);
    } while (data_size < 0 || data_size > 512);//数据长度单位为b 位，最大64byte，即512位

    std::string data;
    for (int i = 0; i < data_size; ++i) {
        char random_char = static_cast<char>(gen() % 26 + 'a');
        data.push_back(random_char);
    }

    // 返回生成的 message 对象
    return message(id, data_size, period, deadline, priority, exec_time, data);
}



//void message::parallel_generate_messages(std::vector<message>& message_set, size_t num_messages) {
//    // 获取系统支持的线程数量
//    size_t num_threads = std::thread::hardware_concurrency();
//    // 计算每个线程生成的消息数量
//    size_t messages_per_thread = num_messages / num_threads;
//
//    // 创建线程并生成消息
//    std::vector<std::vector<message>> thread_message_sets(num_threads);
//    std::vector<std::thread> threads;
//    for (size_t i = 0; i < num_threads; ++i) {
//        threads.emplace_back([&](size_t thread_index) {
//            // 在每个线程中生成消息
//            for (size_t j = 0; j < messages_per_thread; ++j) {
//                // 调用生成随机 message 的函数
//                message new_message = generate_random_message();
//                // 将生成的 message 添加到该线程的消息集合中
//                thread_message_sets[thread_index].push_back(new_message);
//            }
//            }, i);
//    }
//
//    // 等待所有线程结束
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    // 将每个线程生成的消息合并到 message_set 中
//    for (const auto& thread_message_set : thread_message_sets) {
//        message_set.insert(message_set.end(), thread_message_set.begin(), thread_message_set.end());
//    }
//}
void message::parallel_generate_messages(std::vector<message>& message_set, size_t num_messages, std::unordered_set<int>& available_ids, std::mutex& id_mutex) {
    size_t messages_per_thread = std::max((int)ceil((double)num_messages / std::thread::hardware_concurrency()), (50));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(num_messages) / messages_per_thread)));
    // 创建线程并生成消息
    std::vector<std::vector<message>> thread_message_sets(num_threads);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; i++) {
        const int thread_index = i;
        threads.emplace_back([&, thread_index]() { // 捕获 i 变量并复制到 lambda 表达式中
            // 在每个线程中生成消息
            for (size_t j = 0; j < messages_per_thread && (thread_index * messages_per_thread + j) < num_messages; ++j) {
                // 调用生成随机 message 的函数
                message new_message = generate_random_message(available_ids, id_mutex);
                // 将生成的 message 添加到该线程的消息集合中
                thread_message_sets[thread_index].push_back(new_message);
            }
            });
    }


    // 等待所有线程结束
    for (auto& thread : threads) {
        thread.join();
    }

    // 将每个线程生成的消息合并到 message_set 中
    for (auto& thread_message_set : thread_message_sets) {
        for (auto& new_message : thread_message_set) {
            message_set.emplace_back(std::move(new_message));
        }
    }
}



void message::print_messages(const std::vector<message>&message_set) {
    std::cout << "ID\tdatasize(\tperiod\tdeadline\tpriority\texec_time\tdata\n";
    for (size_t i = 0; i < message_set.size(); i++) {
        std::cout << message_set[i].id << '\t' << message_set[i].data_size << '\t' << message_set[i].period << '\t' << message_set[i].deadline << '\t' << message_set[i].priority << '\t' << message_set[i].exec_time << '\t' << message_set[i].data << '\n';
    }
}
bool canfd_frame::create_canfd_frame(canfd_frame& _frame,int _id, CAN_Frame_Type _type, std::string _identifier, std::vector<message*> _message_p_list ) {
    if (_message_p_list.empty()) {
        return false;
    }
    else {
        if (_frame.add_message_list(_message_p_list)) {
            _frame.type = _type;
            _frame.set_identifier(_identifier);
            _frame.id = _id;
            return true;
        }
        else {
            return false;
        }
    }

}
bool canfd_frame::create_canfd_frame(canfd_frame& _frame,int _id, CAN_Frame_Type _type, std::vector<message*> _message_p_list) {
    if (_message_p_list.empty()) {
        return false;
    }
    else {
        if (_frame.add_message_list(_message_p_list,true)) {
            _frame.type = _type;
            _frame.id = _id;
            //这里的优先级就是消息的最大优先级
            return true;
        }
        else {
            return false;
        }
    }

}



int canfd_frame::priority_trans(std::string identifier) {
    if (identifier.size() != 11)
        return -2;
    int pri = 0;
    for (size_t i = 0; i < identifier.size(); i++) {
        if (identifier[i] == '1') {
            pri += (int)pow(2, 10 - i);
        }
        else if (identifier[i] != '0') {
            return -3;
        }
        
    }
    return pri; // Return the decimal number
}
std::string canfd_frame::priority_trans(int priority) {
    if (priority < 0 || priority > 2047) {
        // 数字超出范围（11位二进制数的最大值为2047）
        return ""; // 返回空字符串表示错误
    }
    std::string identifier = "";
    for (int i = 10; i >= 0; i--) {
        // 检查当前位是否为1
        if (priority & (1 << i)) {
            identifier += '1';
        }
        else {
            identifier += '0';
        }
    }
    return identifier;
}
int canfd_frame::payload_size_trans(int size) {
    int payload_sizes[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };
    int num_sizes = sizeof(payload_sizes) / sizeof(payload_sizes[0]);
    for (int i = 0; i < num_sizes; ++i) {
        if (size <= payload_sizes[i]) {
            return payload_sizes[i];
        }
    }
    // 如果数据尺寸大于所有 payload 尺寸，返回最大的 payload 尺寸
    return -1;
}

//设置identifier，同步更新priority
bool canfd_frame::set_identifier(std::string _identifier) {
    int temp = priority_trans(_identifier);
    if (temp > 0) {
        this->priority = temp;
        this->identifier = _identifier;
        return true;
    }
    else {
        return false;
    }
}
bool canfd_frame::set_priority(int priority) {
    std::string temp = priority_trans(priority);
    if (temp.size()==11) {
        this->priority = priority;
        this->identifier = temp;
        return true;
    }
    else {
        return false;
    }
}
//向frame添加消息m，同步更新data_size、payload_size，deadline、period
bool canfd_frame::add_message(message& m, bool priority_flag) {
    if (max_data_size - data_size < m.data_size) {
        return false;
    }

    else {
        double wctt = 0;
        int temp_period = 0;
        double min_deadline = 0;
        if (this->message_p_list.empty() ){
            temp_period = m.period;
            min_deadline = m.deadline;
            wctt = canfd_utils().calc_wctt(canfd_frame::payload_size_trans(m.data_size));
            if (temp_period < min_deadline || min_deadline < wctt ) {
                return false;
            }
        }
        else {
            temp_period = my_algorithm::gcd(this->period, m.period);
            min_deadline = std::min(this->deadline, m.deadline);
            wctt = canfd_utils().calc_wctt(canfd_frame::payload_size_trans(m.data_size + this->data_size));
            if ( min_deadline <= wctt) {
                //TODO 执行时间exec_time是否需要纳入考虑？ 比如exec_time应该小于deadline
                return false;
            }
        }

        this->set_period(temp_period);
        this->deadline = min_deadline;

        this->update_data_size(this->data_size += m.data_size);
        (this->message_p_list).push_back(&m);
        if(priority_flag){ this->set_priority(std::min(this->priority, m.priority)); }

        return true;
    }
}

//TODO 需要修改
//向frame添加消息集合中的所有消息，同步更新data_size、payload_size，deadline、period
bool canfd_frame::add_message_list(std::vector<message*>& message_p_set, bool priority_flag ) {
    if (message_p_set.empty()) return false;
    int accumulate_size = 0, min_deadline = this->deadline, temp_period = message_p_set[0]->period;
    int min_pri = this->priority, accumulate_exec=0;
    for (size_t i = 0; i < message_p_set.size(); i++) {
        accumulate_size += message_p_set[i]->data_size;
        if (message_p_set[i]->deadline < min_deadline) {
            min_deadline = message_p_set[i]->deadline;
        }
        if (message_p_set[i]->priority < min_pri) {
            min_pri = message_p_set[i]->priority;
        }
        temp_period = my_algorithm::gcd(temp_period, message_p_set[i]->period);
        accumulate_exec += message_p_set[i]->exec_time;
    }

    if (max_data_size - data_size < accumulate_size || temp_period < min_deadline || this->deadline < canfd_utils().calc_wctt(canfd_frame::payload_size_trans(accumulate_size + this->data_size)) || min_deadline <= this->exec_time + accumulate_exec) {
        return false;
    }
    else {
        this->update_data_size(this->data_size + accumulate_size);
        for (message* m : message_p_set) {
            this->message_p_list.push_back(m);
        }
        //TODO 更新更多的可能信息
        this->deadline = min_deadline;
        this->period = temp_period;
        this->exec_time += accumulate_exec;
        if (priority_flag) {
            this->set_priority(min_pri);
        }
        return true;
    }
}
    bool canfd_frame::merge(canfd_frame& frame,bool priority_flag) {
        if (this->type != frame.type) { return false; }
        //TODO 需要考虑offset怎么处理
        if (this->add_message_list(frame.message_p_list, priority_flag)) {
            return true;
        }
        else {
            return false;
        }
    }