#include "canfd_frame.h"

int canfd_frame::max_data_size = 64;

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
        std::getline(line_stream, data, '\t');

        // 将字符串转换为对应的整数值
        int id = std::stoi(id_str);
        int data_size = std::stoi(data_size_str);
        int period = std::stoi(period_str);
        int deadline = std::stoi(deadline_str);
        int priority = std::stoi(priority_str);
        int exec_time = std::stoi(exec_time_str);

        // 输入合法性检查
        if (data_size < 0 || period < 0 || deadline < 0 || priority < 0 || exec_time < 0 || data_size > 64 || priority > 2047) {
            std::cerr << "Error: 不合法的输入 位于 行 " << line_number << "\n(提示：data_size, period, deadline, priority, exec_time 应非负, priority 应属于 [0 , 2047], data size 不应超过 64 bytes)" << std::endl;
            continue; // 跳过当前行
        }

        if (deadline >= period || exec_time < deadline) {
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
        std::cerr << "Error: Unable to open file " << filename << std::endl;
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
//随机生成一个合规的messgae
message message::generate_random_message() {
    std::random_device rd;
    std::mt19937 gen(rd());

    // 生成随机的数据
    std::uniform_int_distribution<int> id_dist(0, 999);
    std::uniform_int_distribution<int> period_dist(1, 100);
    std::uniform_int_distribution<int> deadline_dist(0, period_dist(gen) - 1);
    std::uniform_int_distribution<int> priority_dist(0, 2047);
    std::uniform_int_distribution<int> exec_time_dist(0, deadline_dist(gen));

    int id = id_dist(gen);
    int period = period_dist(gen);
    int deadline = deadline_dist(gen);
    int priority = priority_dist(gen);
    int exec_time = exec_time_dist(gen);

    // 生成随机的 data
    std::uniform_int_distribution<int> data_size_dist(0, 64);
    int data_size = data_size_dist(gen);
    std::string data;
    // 生成随机数据
    for (int i = 0; i < data_size; ++i) {
        char random_char = static_cast<char>(gen() % 26 + 'a'); // 生成随机字符
        data.push_back(random_char);
    }

    return message(id, data_size, period, deadline, priority, exec_time, data);
}
// 并行生成随机 message 的函数
void message::parallel_generate_messages(std::vector<message>& message_set, size_t num_messages) {
    std::mutex mutex; // 互斥锁，用于保护共享资源 messages

    // 获取系统支持的线程数量
    size_t num_threads = std::thread::hardware_concurrency();
    // 计算每个线程生成的消息数量
    size_t messages_per_thread = num_messages / num_threads;

    // 创建线程并生成消息
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            // 在每个线程中生成消息
            for (size_t j = 0; j < messages_per_thread; ++j) {
                // 调用生成随机 message 的函数
                message new_message = generate_random_message();
                // 对共享资源 messages 进行加锁
                std::lock_guard<std::mutex> lock(mutex);
                // 将生成的 message 添加到 messages 中
                message_set.push_back(new_message);
            }
            });
    }

    // 等待所有线程结束
    for (auto& thread : threads) {
        thread.join();
    }
}


bool canfd_frame::create_canfd_frame(canfd_frame& _frame,int _id, CAN_Frame_Type _type, std::string _identifier, std::vector<message>* _message_list ) {
    if (_message_list == nullptr) {
        return false;
    }
    else {
        if (_frame.add_messageset(*_message_list)) {
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
bool canfd_frame::create_canfd_frame(canfd_frame& _frame,int _id, CAN_Frame_Type _type, std::vector<message>* _message_list) {
    if (_message_list == nullptr) {
        return false;
    }
    else {
        if (_frame.add_messageset(*_message_list,true)) {
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
        this->update_data_size(this->data_size += m.data_size);
        (this->message_list)->push_back(m);
        this->deadline = std::min(this->deadline, m.deadline);
        this->exec_time += m.exec_time;
        if(priority_flag){ this->set_priority(std::min(this->priority, m.priority)); }
        if ((this->message_list)->empty()) {
            this->period = m.period;
        }
        else {
            this->period = my_algorithm::gcd(this->period, m.period);
        }

        return true;
    }
}
//向frame添加消息集合中的所有消息，同步更新data_size、payload_size，deadline、period
bool canfd_frame::add_messageset(std::vector<message>& messageSet, bool priority_flag ) {
    if (messageSet.empty()) return false;
    int accumulate_size = 0, min_deadline = this->deadline, temp_period = messageSet[0].period;
    int min_pri = this->priority, accumulate_exec=0;
    for (size_t i = 0; i < messageSet.size(); i++) {
        accumulate_size += messageSet[i].data_size;
        if (messageSet[i].deadline < min_deadline) {
            min_deadline = messageSet[i].deadline;
        }
        if (messageSet[i].priority < min_pri) {
            min_pri = messageSet[i].priority;
        }
        temp_period = my_algorithm::gcd(temp_period, messageSet[i].period);
        accumulate_exec += messageSet[i].exec_time;
    }

    if (max_data_size - data_size < accumulate_size) {
        return false;
    }
    else {
        this->update_data_size(this->data_size + accumulate_size);
        for (message& m : messageSet) {
            this->message_list->push_back(m);
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
        if (this->add_messageset(*(frame.message_list), priority_flag)) {
            return true;
        }
        else {
            return false;
        }
    }