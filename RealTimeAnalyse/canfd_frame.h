#pragma once
#include <vector>
#include <string>
#include <numeric>
#include"math_algorithms.h"
class message {
public:
    int data_size; // 数据尺寸
    int period;    // 周期
    int offset;
    int priority;  //优先级
    int deadline;   //消息截止日期
    int exec_time;
    int id;
    message(int size, int period, int deadline,int exec_time,int offset = 0, int id = -1,int priority=0) : id(id),data_size(size), period(period), deadline(deadline), exec_time(exec_time),priority(priority),offset(offset) {}
};

enum class CAN_Frame_Type {
    Data_Frame,
    Remote_Frame,
    Error_Frame,
    Overload_Frame,
    Interframe_Space
};
class canfd_frame {
private:
    int data_size = 0;      // 已装载数据长度，默认为空
    int deadline=-1;  //TODO deadline和period也许需要在最开始给出一个默认值
    int period;      
public:
    std::string identifier; // 优先级标识
    CAN_Frame_Type type;    // 数据帧类型
    
    static int max_data_size;   // 最大数据负载，默认为 64
    std::vector<message*> message_list; // 所装载的消息集合

    canfd_frame(std::string identifier, CAN_Frame_Type type, int data_size = 0)
        : identifier(identifier), type(type) {}
    ~canfd_frame() {
        message_list.clear();
    }
    bool add_message(message& m) {
        if (max_data_size - data_size < m.data_size) {
            return false;
        }
        else {
            data_size += m.data_size;
            message_list.push_back(&m);
            this->deadline = std::min(this->deadline, m.deadline);
            if (this->message_list.empty()) {
                this->period = m.period;
            }
            else {
                this->period = gcd(this->period, m.period);
            }

            return true;
        }
    }
    bool add_messageset(std::vector<message>&messageSet) {
        if (messageSet.empty()) return false;
        int accumulate_size = 0, min_deadline = this->deadline,temp_period=messageSet[0].period;
        for (int i = 0; i < messageSet.size(); i++) {
            accumulate_size += messageSet[i].data_size;
            if (messageSet[i].deadline < min_deadline) { // 这里调用了 deadline() 函数，应该是 deadline，不需要括号
                min_deadline = messageSet[i].deadline;
            }
            temp_period= gcd(temp_period, messageSet[i].period);
        }

        if (max_data_size - data_size < accumulate_size) {
            return false;
        }
        else {
            data_size += accumulate_size;
            for (message& m : messageSet) {
                message_list.push_back(&m);
            }
            //TODO 更新更多的可能信息
            this->deadline = min_deadline;
            this->period = temp_period;
            return true;
        }
    }

    //TODO 对数据帧中增加消息，会导致数据帧的deadline，data_size等特征发生变化，故要使得要变化的特征设为private
};


