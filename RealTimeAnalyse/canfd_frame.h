#pragma once
#include <vector>
#include <string>
#include <numeric>
#include"math_algorithms.h"
class message {
public:
    int data_size; // ���ݳߴ�
    
    int period;    // ����
    int offset;
    int priority;  //���ȼ�
    int deadline;   //��Ϣ��ֹ����
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
    int data_size = 0;      // ��װ�����ݳ��ȣ�Ĭ��Ϊ��
    int payload_size = 0; //payload�ߴ�����ݳߴ粻��ȫһ����ȡֵ��0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes
    int deadline=-1;  //TODO deadline��periodҲ����Ҫ���ʼ����һ��Ĭ��ֵ
    int period;      
public:
    std::string identifier; // ���ȼ���ʶ
    CAN_Frame_Type type;    // ����֡����
    
    static int max_data_size;   // ������ݸ��أ�Ĭ��Ϊ 64
    std::vector<message*> message_list; // ��װ�ص���Ϣ����

    canfd_frame(std::string identifier, CAN_Frame_Type type, int data_size = 0)
        : identifier(identifier), type(type) {}
    ~canfd_frame() {
        message_list.clear();
    }
    //�����ݳߴ�ת��Ϊ���ʵ�payload�ߴ磬payloadȡֵ��0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes��Ҫ����װ������
    int payload_size_trans(int size) {
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
    bool add_message(message& m) {
        if (max_data_size - data_size < m.data_size) {
            return false;
        }
        else {
            this->data_size += m.data_size;
            this->payload_size = payload_size_trans(this->data_size);
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
            if (messageSet[i].deadline < min_deadline) { // ��������� deadline() ������Ӧ���� deadline������Ҫ����
                min_deadline = messageSet[i].deadline;
            }
            temp_period= gcd(temp_period, messageSet[i].period);
        }

        if (max_data_size - data_size < accumulate_size) {
            return false;
        }
        else {
            data_size += accumulate_size;
            this->payload_size = payload_size_trans(this->data_size);
            for (message& m : messageSet) {
                message_list.push_back(&m);
            }
            //TODO ���¸���Ŀ�����Ϣ
            this->deadline = min_deadline;
            this->period = temp_period;
            return true;
        }
    }
    int get_paylaod_size() {
        return this->payload_size;
    }
    int get_period() {
        return this->period;
    }
    //TODO ������֡��������Ϣ���ᵼ������֡��deadline��data_size�����������仯����Ҫʹ��Ҫ�仯��������Ϊprivate
};


