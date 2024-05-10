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
    int data_rate = 1000000; // ���ݶ��ٶȣ���λĬ��Ϊbps ��1s����data_rate��b(λ) ��Ĭ������Ϊ1Mbps
    int arbitration_rate = 1000000; // �ٲö��ٶȣ���λĬ��ΪMbps ��1s����data_rate��b(λ��Ĭ������Ϊ1Mbps
    double t_arb;   //��λΪs������һ��bit��������
    double t_data;

    
public:
    //��ʼ��canfdϵͳ������Ϣ
    canfd_utils(int dataRate = 1000000, int arbRate = 1000000) {
        this->data_rate = dataRate;
        t_data = 1.0 / dataRate;
        this->arbitration_rate = arbRate;
        t_arb = 1.0 / arbRate;
        worst_wctt = this->calc_wctt(64);
    }
    //���������µĴ���ʱ�� [0.05999,1.019999],����б�Ҫʹ�����ڵ�������2
    double calc_wctt(int paylaod_size, TimeUnit time_unit = TimeUnit::Milliseconds) {
        int p = paylaod_size;
        double wctt = 32 * t_arb + (28 + 5 * ceil(p - 16 / 64.0) + 10.0 * p) * t_data;
        return (int)time_unit * wctt;
    }
    //�����������µĴ���ʱ��
    double calc_bctt(int paylaod_size, TimeUnit time_unit = TimeUnit::Milliseconds) {
        int p = paylaod_size;
        double bctt = 29 * t_arb + (27 + 5 * ceil(p - 16 / 64.0) + 8.0 * p) * t_data;
        return (int)time_unit * bctt;
    }
    ////�������������
    //double calc_bandwidth_utilization(const std::vector<canfd_frame*>& frameSet) {
    //    double BWU = 0;
    //    for (size_t i = 0; i < frameSet.size(); i++) {
    //        BWU += ((double)calc_wctt(frameSet[i]->get_paylaod_size()) / frameSet[i]->get_period());
    //    }
    //    return BWU;
    //}
    //TODO ��������֡�ܷ�ʱ��ɣ�����֡�ɵ����Լ��
};