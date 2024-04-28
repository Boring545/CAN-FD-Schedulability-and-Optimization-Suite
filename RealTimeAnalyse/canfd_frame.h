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
    std::string data; //����װ�ش������Ϣ��Ĭ�ϲ���д����С����������
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
    std::string identifier; // ���ȼ���ʶ,һ��11λ������������ʾ1~2048����Ҫ���任��Ϊ10���ƴ���priority��
    int priority=-1;
    int exec_time=0;   //����֡��ϵͳ�ڵ�ִ��ʱ�䣬������װ�������ִ��ʱ���
    int id;

    //ͬ������data_size��payload_size
    bool update_data_size(int size) {
        this->data_size = size;
        this->payload_size = payload_size_trans(size);
        return true;
    }
public:
    int offset=0;
    CAN_Frame_Type type;    // ����֡����


    static int max_data_size;   // ������ݸ��أ�Ĭ��Ϊ 64
    std::vector<message>* message_list; // ��װ�ص���Ϣ����

    //����canfd֡ʱ��Ҫô���ڴ��ݿ�����Ϣ��Ҫô���ڰ���message���������֡��������Ϣ���ƻ��Զ��ṩ���ȼ�������֡���ȼ��������ݵ����������������֡�ɲ������ȼ�
    //��������֡
    bool create_canfd_frame(canfd_frame& _frame,int id ,CAN_Frame_Type _type, std::string _identifier, std::vector<message>* _message_list = nullptr);
    bool create_canfd_frame(canfd_frame& _frame,int id, CAN_Frame_Type _type, std::vector<message>* _message_list);
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
    bool add_messageset(std::vector<message>& messageSet,bool priority_flag=false);
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

    canfd_frame() {}
    ~canfd_frame() {
        message_list->clear();
    }
};


