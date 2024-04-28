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
    std::string data; //用于装载传输的信息，默认不填写，大小可能有限制
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
    int payload_size = 0; //payload尺寸和数据尺寸不完全一样，取值有0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes
    int deadline=-1;  //TODO deadline和period也许需要在最开始给出一个默认值
    int period;     
    std::string identifier; // 优先级标识,一共11位二进制数，表示1~2048，需要将其换算为10进制存在priority中
    int priority=-1;
    int exec_time=0;   //数据帧在系统内的执行时间，是所有装载任务的执行时间和
    int id;

    //同步更新data_size、payload_size
    bool update_data_size(int size) {
        this->data_size = size;
        this->payload_size = payload_size_trans(size);
        return true;
    }
public:
    int offset=0;
    CAN_Frame_Type type;    // 数据帧类型


    static int max_data_size;   // 最大数据负载，默认为 64
    std::vector<message>* message_list; // 所装载的消息集合

    //创建canfd帧时，要么用于传递控制消息，要么用于包裹message来组成数据帧，控制消息估计会自动提供优先级，数据帧优先级由所传递的任务决定，故数据帧可不给优先级
    //创建控制帧
    bool create_canfd_frame(canfd_frame& _frame,int id ,CAN_Frame_Type _type, std::string _identifier, std::vector<message>* _message_list = nullptr);
    bool create_canfd_frame(canfd_frame& _frame,int id, CAN_Frame_Type _type, std::vector<message>* _message_list);
    //将二进制优先级换算为整数优先级
    static int priority_trans(std::string identifier);
    ////将整数优先级换算为二进制优先级
    static std::string priority_trans(int priority);
    //将数据尺寸转换为合适的payload尺寸，payload取值有0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48 or 64 bytes，要求能装下数据
    static int payload_size_trans(int size);
public:
    //设置identifier，同步更新priority
    bool set_identifier(std::string _identifier);
    bool set_priority(int priority);
    //对周期的修改要慎重，最好别乱改，系统会在添加任务后自动计算
    bool set_period(int p) {
        this->period = p;
        return true;
    }

    //向frame添加消息m，同步更新data_size、payload_size，deadline、period，如priority_flag=true，优先级将根据任务优先级自动更新
    bool add_message(message& m,bool priority_flag = false);
    //向frame添加消息集合中的所有消息，同步更新data_size、payload_size，deadline、period,如priority_flag=true，优先级将根据任务优先级自动更新
    bool add_messageset(std::vector<message>& messageSet,bool priority_flag=false);
    //合并两个数据帧,, bool priority_flag=true 则自动更新优先级
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


