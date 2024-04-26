#pragma once
#include <vector>
#include <string>

class message {
public:
    int data_size; // ���ݳߴ�
    int period;    // ����
    int offset;
    int priority;  //���ȼ�
    int deadline;   //��Ϣ��ֹ����
    int exec_time;
    message(int size, int period, int deadline,int exec_time,int offset = 0, int priority=0) : data_size(size), period(period), deadline(deadline), exec_time(exec_time),priority(priority),offset(offset) {}
};

enum class CAN_Frame_Type {
    Data_Frame,
    Remote_Frame,
    Error_Frame,
    Overload_Frame,
    Interframe_Space
};

class canfd_frame {
public:
    std::string identifier;
    CAN_Frame_Type type;
    int data_size;
    static int max_data_size;
    std::vector<message> message_list;

    canfd_frame(std::string identifier, CAN_Frame_Type type, int data_size);

    bool add_data(message& m);
};

