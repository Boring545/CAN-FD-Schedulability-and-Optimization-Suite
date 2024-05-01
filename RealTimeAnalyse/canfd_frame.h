#pragma once
#include <vector>
#include <string>
#include <numeric>
#include <fstream>
#include <sstream>
#include<iostream>
#include"math_algorithms.h"
#include <mutex>
#include <random>
#include <thread>
#include <unordered_set>
//�������message��
//1.ͨ�����캯�� 2.ͨ���ļ�д��good����ʹ�ó���ͨ��C++ִ�У����������ֿ���ͨ���ļ���Ϊ�н���JAVA��д��
//�ļ��ṹ��ID��int��   datasize(int)    period(int)  deadline(int)    priority��int��   exec_time��int��  data(���ݳ�����64byte��)   
//�ļ���ʼΪ������Ϣ�������ȡ�����У��˺�ÿ�б�ʾһ��message��
class message {
public:
    int data_size; // ���ݳߴ�
    
    int period;    // ����
    //int offset;
    int priority;  //���ȼ�
    int deadline;   //��Ϣ��ֹ����
    int exec_time;
    int id;
    std::string data; //����װ�ش������Ϣ��Ĭ�ϲ���д����С����������
    message(int _id, int _data_size, int _period, int _deadline, int _priority, int _exec_time, std::string _data = "")
    :id(_id),data_size(_data_size),period(_period),deadline(_deadline),priority(_priority),exec_time(_exec_time){
        data = _data;
    }
    message() {}
    //���ļ���ȡmessageset
    static std::vector<message> read_messages(const std::string& filename);
    static std::vector<message> read_messages(int ecu_id, const std::string& directory);
    //��message_setд��filename�ڣ�append=trueʹ��ֱ�����ϴε����ݺ�׷��д��
    static void write_messages(const std::vector<message>& message_set, const std::string& filename, bool append = false);
    
    static void write_messages(const std::vector<message>& message_set, int ecu_id, const std::string& directory, bool append = false);
    //�������һ���Ϲ��messgae
    /*static message generate_random_message(std::vector<int>& available_ids, std::mutex& id_mutex);*/
    static message generate_random_message(std::unordered_set<int>& available_ids, std::mutex& id_mutex, int period_base);
    // ����������� message �ĺ���,available_ids��ÿ��Ԫ�ش���һ������ʹ�õ�id
    static void parallel_generate_messages(std::vector<message>& message_set, size_t num_messages, std::unordered_set<int>& available_ids, std::mutex& id_mutex);
    
    static void print_messages(const std::vector<message>&message_set);
};


enum class CAN_Frame_Type {
    Data_Frame,
    Remote_Frame,
    Error_Frame,
    Overload_Frame,
    Interframe_Space,
    NULL_FRAME
};
class canfd_frame {
private:
    int data_size = 0;      // ��װ�����ݳ��ȣ�Ĭ��Ϊ��
    int payload_size = 0; //payload�ߴ�����ݳߴ粻��ȫһ����ȡֵ��0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes
    int deadline=-1;  //TODO deadline��periodҲ����Ҫ���ʼ����һ��Ĭ��ֵ
    int period=-1;     
    std::string identifier; // ���ȼ���ʶ,һ��11λ������������ʾ1~2048����Ҫ���任��Ϊ10���ƴ���priority��
    int priority=-1;
    int exec_time=0;   //����֡��ϵͳ�ڵ�ִ��ʱ�䣬������װ�������ִ��ʱ���
    int id=-1;
    CAN_Frame_Type type;    // ����֡����

    //ͬ������data_size��payload_size
    bool update_data_size(int size) {
        this->data_size = size;
        this->payload_size = payload_size_trans(size);
        return true;
    }
public:
    int offset=0;  //����ʹ��assign_offset��������frame��������������ĺ���offset
    


    static int max_data_size;   // ������ݸ��أ�Ĭ��Ϊ 64
    std::vector<message*> message_p_list; // TODO ��ý�message_listǨ�Ƶ�������
    //std::vector<message>* message_list; // ��װ�ص���Ϣ����

    //����canfd֡ʱ��Ҫô���ڴ��ݿ�����Ϣ��Ҫô���ڰ���message���������֡��������Ϣ���ƻ��Զ��ṩ���ȼ�������֡���ȼ��������ݵ����������������֡�ɲ������ȼ�
    //��������֡
    bool create_canfd_frame(canfd_frame& _frame, int _id, CAN_Frame_Type _type, std::string _identifier, std::vector<message*> _message_p_list);
    bool create_canfd_frame(canfd_frame& _frame, int _id, CAN_Frame_Type _type, std::vector<message*> _message_p_list);
    //�����������ȼ�����Ϊ�������ȼ�
    static int priority_trans(std::string identifier);
    ////���������ȼ�����Ϊ���������ȼ�
    static std::string priority_trans(int priority);
    //�����ݳߴ�ת��Ϊ���ʵ�payload�ߴ磬payloadȡֵ��0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes��Ҫ����װ������
    static int payload_size_trans(int size);
public:
    //����identifier��ͬ������priority
    bool set_identifier(std::string _identifier);
    bool set_priority(int priority);
    //�����ڵ��޸�Ҫ���أ���ñ��Ҹģ�ϵͳ�������������Զ�����
    bool set_period(int p) {
        this->period = p;
        return true;
    }

    //��frame�����Ϣm��ͬ������data_size��payload_size��deadline��period����priority_flag=true�����ȼ��������������ȼ��Զ�����
    bool add_message(message& m,bool priority_flag = false);
    //��frame�����Ϣ�����е�������Ϣ��ͬ������data_size��payload_size��deadline��period,��priority_flag=true�����ȼ��������������ȼ��Զ�����
    bool add_message_list(std::vector<message*>& message_p_set,bool priority_flag=false);
    //�ϲ���������֡,, bool priority_flag=true ���Զ��������ȼ�
    bool merge(canfd_frame& frame, bool priority_flag=false);

public:
    int get_priority() const{
        return this->priority;
    }
    int get_paylaod_size() const {
        return this->payload_size;
    }
    int get_period() const {
        return this->period;
    }
    int get_deadline() const {
        return this->deadline;
    }
    int get_exec_time() const {
        return this->exec_time;
    }
    int get_id() const{
        return this->id;
    }
    CAN_Frame_Type get_type() {
        return this->type;
    }

    canfd_frame() {
        type = CAN_Frame_Type::NULL_FRAME;
    }
    canfd_frame(int _id) {
        type = CAN_Frame_Type::NULL_FRAME;
        this->id = _id;
    }
    canfd_frame(int _offset,int _exec,int _deadline,int _period,int _id) {
        offset = _offset;
        exec_time = _exec;
        deadline = _deadline;
        period = _period;
        id = _id;
        type = CAN_Frame_Type::NULL_FRAME;
    }
    ~canfd_frame() {
        this->message_p_list.clear();
        this->identifier.clear();
    }
    void clear() {
        this->message_p_list.clear();
        this->type = CAN_Frame_Type::NULL_FRAME;
        data_size = 0;
        payload_size = 0;
        deadline = -1;
        period = -1;
        identifier.clear();
        priority = -1;
        exec_time = 0;
        id = -1;
        offset = 0;
    }
};

class canfd_utils {
private:
    int data_rate = 1000000; // ���ݶ��ٶȣ�Ĭ��ֵΪ 1 Mbps
    int arbitration_rate = 1000000; // �ٲö��ٶȣ�Ĭ��ֵΪ 1 Mbps
    double t_arb;   //��λΪs
    double t_data;
public:
    //��ʼ��canfdϵͳ������Ϣ
    canfd_utils(int dataRate = 1000000, int arbRate = 1000000) {
        this->data_rate = dataRate;
        t_data = 1.0 / dataRate;
        this->arbitration_rate = arbRate;
        t_arb = 1.0 / arbRate;
    }
    //���������µĴ���ʱ��
    double calc_wctt(int paylaod_size) {
        int p = paylaod_size;
        double wctt = 32 * t_arb + (28 + 5 * ceil(p - 16 / 64.0) + 10.0 * p) * t_data;
        return wctt;
    }
    //�����������µĴ���ʱ��
    double calc_bctt(const canfd_frame& frame) {
        int p = frame.get_paylaod_size();
        double bctt = 29 * t_arb + (27 + 5 * ceil(p - 16 / 64.0) + 8.0 * p) * t_data;
        return bctt;
    }
    //�������������
    double calc_bandwidth_utilization(const std::vector<canfd_frame*>&frameSet) {
        double BWU = 0;
        for (size_t i = 0; i < frameSet.size(); i++) {
            BWU += ((double)calc_wctt(frameSet[i]->get_paylaod_size()) / frameSet[i]->get_period());
        }
        return BWU;
    }
    //TODO ��������֡�ܷ�ʱ��ɣ�����֡�ɵ����Լ��
};