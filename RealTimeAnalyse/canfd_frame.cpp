#include "canfd_frame.h"



//�ļ��ṹ��ID��int��   datasize(int)    period(int)  deadline(int)    priority��int��   exec_time��int��  data
//�ļ���ʼΪ������Ϣ�������ȡ�����У��˺�ÿ�б�ʾһ��message�����ݼ�ʹ��\t�ָ�
//data_size, period, deadline, priority, exec_time ��0��priority ��[0, 2047], data size ��Ӧ���� 64 bytes
//deadline ӦС�� period, exec_time ӦС�� deadline
std::vector<message> message::read_messages(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<message> messages;

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return messages; // ���ؿ� vector
    }

    std::string line;
    int line_number = 0;

    // ����������Ϣ
    while (std::getline(file, line) && !line.empty()) {
        line_number++;
    }

    while (std::getline(file, line)) {
        line_number++;

        // ʹ�� stringstream �ָ�������
        std::istringstream line_stream(line);
        std::string id_str, data_size_str, period_str, deadline_str, priority_str, exec_time_str, data;

        // �����ȡ���Ʊ��Ϊ�ָ������ֶ�
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

        // ���ַ���ת��Ϊ��Ӧ������ֵ
        int id = std::stoi(id_str);
        int data_size = std::stoi(data_size_str);
        int period = std::stoi(period_str);
        double deadline = std::stod(deadline_str);
        int priority = std::stoi(priority_str);
        int exec_time = std::stoi(exec_time_str);

        // ����Ϸ��Լ��
        if (data_size < 0 || period < 0 || deadline < 0 || priority < 0 || exec_time < 0 || data_size > 64 || priority > 2047) {
            std::cerr << "Error: ���Ϸ������� λ�� �� " << line_number << "\n(��ʾ��data_size, period, deadline, priority, exec_time Ӧ�Ǹ�, priority Ӧ���� [0 , 2047], data size ��Ӧ���� 64 bytes)" << std::endl;
            continue; // ������ǰ��
        }

        if (deadline > period || exec_time > deadline) {
            std::cerr << "Error: ���Ϸ������� λ�� �� " << line_number << " (deadline ӦС�� period, exec_time ӦС�� deadline)" << std::endl;
            continue; // ������ǰ��
        }

        // ���� message ������ӵ� messages ��
        message m(id, data_size, period, deadline, priority, exec_time, data);
        messages.push_back(m);
    }

    file.close(); // �ر��ļ�

    return messages;
}
//��message_setд��filename�ڣ�append=trueʹ��ֱ�����ϴε����ݺ�׷��д��
void message::write_messages(const std::vector<message>& message_set, const std::string& filename, bool append) {
    std::ofstream file;
    if (append) {
        file.open(filename, std::ios::app); // ׷��ģʽ
    }
    else {
        file.open(filename); // Ĭ��ģʽ����׷��
    }

    if (!file.is_open()) {
        std::cerr << "Error:�޷����ļ� " << filename << std::endl;
        return;
    }
    // д����Ϣ������Ϣ�����ڷ�׷��ģʽ��д��
    if (!append) {
        file << "�ļ��ṹ��ID��int��\tdatasize(int)\tperiod(int)\tdeadline(int)\tpriority��int)\texec_time��int��\tdata(�ַ���)\n"<<"===============================================================\n\n";
    }

    // ���д��ÿ�� message
    for (const auto& m : message_set) {
        file << m.id << '\t' << m.data_size << '\t' << m.period << '\t' << m.deadline << '\t' << m.priority << '\t' << m.exec_time << '\t' << m.data << '\n';
    }
    file.close();
}
void message::write_messages(const std::vector<message>& message_set, int ecu_id, const std::string& directory, bool append) {
    // �����ļ���
    std::string filename = directory + "/ecu" + std::to_string(ecu_id) + "_messages.txt";
    // ����ԭʼ����
    write_messages(message_set, filename, append);
}

std::vector<message> message::read_messages(int ecu_id, const std::string& directory) {
    // �����ļ���
    std::string filename = directory + "/ecu" + std::to_string(ecu_id) + "_messages.txt";
    return read_messages(filename);
}
message message::generate_random_message(std::unordered_set<int>& available_ids, std::mutex& id_mutex,int period_base) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::unique_lock<std::mutex> lock(id_mutex); // �� id_mutex ���м���

    if (available_ids.empty()) {
        return message(); // �������id�ļ���Ϊ�գ�����Ĭ�ϵ� message ����
    }

    // �������������
    auto it = available_ids.begin();
    std::advance(it, std::uniform_int_distribution<int>(0, available_ids.size() - 1)(gen));

    // ��ȡ���ѡ���id
    int id = *it;

    // ����ѡ���id�ӿ���id�������Ƴ�
    available_ids.erase(it);

    lock.unlock(); // ����

    // ��������� period��deadline��priority��exec_time �� data_size

    //////////////////�����������
    std::uniform_int_distribution<int> period_mul_dist(1,10); 
    int period = period_base*period_mul_dist(gen);

    //////////////������ɽ�ֹ����
    double mean = (double)period;
    double stddev = 3.0; // ��׼��
    // ������̬�ֲ�����
    std::normal_distribution<double> deadline_dist(mean, stddev);
    // ������̬�ֲ��������
    double deadline_double;
    do {
        deadline_double = deadline_dist(gen);
    } while (deadline_double <= canfd_utils().worst_wctt || deadline_double >= period); // ��֤ deadline �� [worst_wctt, period] ��Χ��
    double deadline =deadline_double;

    /////////////����������ȼ�����ɾ����
    std::uniform_int_distribution<int> priority_dist(0, 2047);
    int priority = priority_dist(gen);


    ////////////////�������ִ��ʱ����ɾ����
    mean = 1.0; // ϣ��������1
    stddev = 2; // ��׼������Ϊ 0.5 ���С��ֵ
    // ������̬�ֲ�����
    std::normal_distribution<double> exec_time_dist(mean, stddev);

    // ������̬�ֲ��������
    double exec_time_double;
    do {
        exec_time_double = exec_time_dist(gen);
    } while (exec_time_double > deadline); // ��֤ exec_time �� deadline��Χ��

    // �������ȡ����Ϊ exec_time
    int exec_time = std::max(1, (int)exec_time_double);

    ////////////////////�����������
    mean = 32;
    stddev = 3;
    // ������̬�ֲ�����
    std::normal_distribution<double> data_size_dist(mean, stddev);
    double data_size=0;
    do {
        data_size = (int)data_size_dist(gen);
    } while (data_size < 0 || data_size > 512);//���ݳ��ȵ�λΪb λ�����64byte����512λ

    std::string data;
    for (int i = 0; i < data_size; ++i) {
        char random_char = static_cast<char>(gen() % 26 + 'a');
        data.push_back(random_char);
    }

    // �������ɵ� message ����
    return message(id, data_size, period, deadline, priority, exec_time, data);
}



//void message::parallel_generate_messages(std::vector<message>& message_set, size_t num_messages) {
//    // ��ȡϵͳ֧�ֵ��߳�����
//    size_t num_threads = std::thread::hardware_concurrency();
//    // ����ÿ���߳����ɵ���Ϣ����
//    size_t messages_per_thread = num_messages / num_threads;
//
//    // �����̲߳�������Ϣ
//    std::vector<std::vector<message>> thread_message_sets(num_threads);
//    std::vector<std::thread> threads;
//    for (size_t i = 0; i < num_threads; ++i) {
//        threads.emplace_back([&](size_t thread_index) {
//            // ��ÿ���߳���������Ϣ
//            for (size_t j = 0; j < messages_per_thread; ++j) {
//                // ����������� message �ĺ���
//                message new_message = generate_random_message();
//                // �����ɵ� message ��ӵ����̵߳���Ϣ������
//                thread_message_sets[thread_index].push_back(new_message);
//            }
//            }, i);
//    }
//
//    // �ȴ������߳̽���
//    for (auto& thread : threads) {
//        thread.join();
//    }
//
//    // ��ÿ���߳����ɵ���Ϣ�ϲ��� message_set ��
//    for (const auto& thread_message_set : thread_message_sets) {
//        message_set.insert(message_set.end(), thread_message_set.begin(), thread_message_set.end());
//    }
//}
void message::parallel_generate_messages(std::vector<message>& message_set, size_t num_messages, std::unordered_set<int>& available_ids, std::mutex& id_mutex) {
    size_t messages_per_thread = std::max((int)ceil((double)num_messages / std::thread::hardware_concurrency()), (50));
    size_t num_threads = std::min(std::thread::hardware_concurrency(), (unsigned int)(std::ceil((double)(num_messages) / messages_per_thread)));
    // �����̲߳�������Ϣ
    std::vector<std::vector<message>> thread_message_sets(num_threads);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_threads; i++) {
        const int thread_index = i;
        threads.emplace_back([&, thread_index]() { // ���� i ���������Ƶ� lambda ���ʽ��
            // ��ÿ���߳���������Ϣ
            for (size_t j = 0; j < messages_per_thread && (thread_index * messages_per_thread + j) < num_messages; ++j) {
                // ����������� message �ĺ���
                message new_message = generate_random_message(available_ids, id_mutex);
                // �����ɵ� message ��ӵ����̵߳���Ϣ������
                thread_message_sets[thread_index].push_back(new_message);
            }
            });
    }


    // �ȴ������߳̽���
    for (auto& thread : threads) {
        thread.join();
    }

    // ��ÿ���߳����ɵ���Ϣ�ϲ��� message_set ��
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
            //��������ȼ�������Ϣ��������ȼ�
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
        // ���ֳ�����Χ��11λ�������������ֵΪ2047��
        return ""; // ���ؿ��ַ�����ʾ����
    }
    std::string identifier = "";
    for (int i = 10; i >= 0; i--) {
        // ��鵱ǰλ�Ƿ�Ϊ1
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
    // ������ݳߴ�������� payload �ߴ磬�������� payload �ߴ�
    return -1;
}

//����identifier��ͬ������priority
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
//��frame�����Ϣm��ͬ������data_size��payload_size��deadline��period
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
                //TODO ִ��ʱ��exec_time�Ƿ���Ҫ���뿼�ǣ� ����exec_timeӦ��С��deadline
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

//TODO ��Ҫ�޸�
//��frame�����Ϣ�����е�������Ϣ��ͬ������data_size��payload_size��deadline��period
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
        //TODO ���¸���Ŀ�����Ϣ
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
        //TODO ��Ҫ����offset��ô����
        if (this->add_message_list(frame.message_p_list, priority_flag)) {
            return true;
        }
        else {
            return false;
        }
    }