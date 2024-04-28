#include "canfd_frame.h"

int canfd_frame::max_data_size = 64;




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
            this->period = gcd(this->period, m.period);
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
        temp_period = gcd(temp_period, messageSet[i].period);
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
    bool canfd_frame::merge(canfd_frame& frame) {
        if (this->type != frame.type) { return false; }
        //TODO 需要考虑offset怎么处理
        if (this->add_messageset(*(frame.message_list),true)) {
            return true;
        }
        else {
            return false;
        }
    }