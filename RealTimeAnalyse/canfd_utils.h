#pragma once
enum class TimeUnit {
    Nanoseconds = 1000000000,
    Microseconds = 1000000,
    Milliseconds = 1000,
    Seconds = 1
};
class canfd_utils {
public:
    double worst_wctt = 0.0;
private:
    int data_rate = 1000000; // 数据段速度，单位默认为bps 即1s传输data_rate个b(位) ，默认设置为1Mbps
    int arbitration_rate = 1000000; // 仲裁段速度，单位默认为Mbps 即1s传输data_rate个b(位，默认设置为1Mbps
    double t_arb;   //单位为s，传输一个bit所用秒数
    double t_data;
    TimeUnit time_unit = TimeUnit::Milliseconds;
    
public:
    //初始化canfd系统基本信息
    canfd_utils(int dataRate = 1000000, int arbRate = 1000000) {
        this->data_rate = dataRate;
        t_data = (double)time_unit / dataRate;
        this->arbitration_rate = arbRate;
        t_arb = (double)time_unit / arbRate;
        worst_wctt = this->calc_wctt(64);
    }
    //calc_wctt注意使用payload作为接收长度，不是datasize多少位
    //计算最坏情况下的传输时间 [0.05999,1.019999],因此有必要使得周期底数大于2
    //结果单位同time_unit
    double calc_wctt(int paylaod_size) {
        int p = paylaod_size;
        double wctt = 32 * t_arb + (28 + 5 * ceil(p - 16 / 64.0) + 10.0 * p) * t_data;
        return wctt;
    }
    //计算最好情况下的传输时间
    double calc_bctt(int paylaod_size) {
        int p = paylaod_size;
        double bctt = 29 * t_arb + (27 + 5 * ceil(p - 16 / 64.0) + 8.0 * p) * t_data;
        return  bctt;
    }
    ////计算带宽利用率
    //double calc_bandwidth_utilization(const std::vector<canfd_frame*>& frameSet) {
    //    double BWU = 0;
    //    for (size_t i = 0; i < frameSet.size(); i++) {
    //        BWU += ((double)calc_wctt(frameSet[i]->get_paylaod_size()) / frameSet[i]->get_period());
    //    }
    //    return BWU;
    //}
    //TODO 计算数据帧能否按时完成，数据帧可调度性检测
};