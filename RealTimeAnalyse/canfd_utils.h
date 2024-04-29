#pragma once
#include"canfd_frame.h"
class canfd_utils {
private:
	int data_rate = 1000000; // ���ݶ��ٶȣ�Ĭ��ֵΪ 1 Mbps
	int arbitration_rate = 1000000; // �ٲö��ٶȣ�Ĭ��ֵΪ 1 Mbps
	double t_arb;
	double t_data;
public:
	//��ʼ��canfdϵͳ������Ϣ
	bool init_canfd_info(int dataRate,int arbRate) {
		this->data_rate = dataRate;
		t_data = 1.0 / dataRate;
		this->arbitration_rate = arbRate;
		t_arb = 1.0 / arbRate;
		return true;
	}
	//���������µĴ���ʱ��
	int calc_wctt(canfd_frame frame) {
		int p = frame.get_paylaod_size();
		int wctt = 32 * t_arb + (28 + 5 * ceil(p - 16 / 64.0) + 10.0 * p) * t_data;
		return wctt;
	}
	//�����������µĴ���ʱ��
	int calc_bctt(canfd_frame frame) {
		int p = frame.get_paylaod_size();
		int bctt = 29 * t_arb + (27 + 5 * ceil(p - 16 / 64.0) + 8.0 * p) * t_data;
		return bctt;
	}
	//�������������
	double calc_bandwidth_utilization(std::vector<canfd_frame>frameSet) {
		double BWU = 0;
		for (size_t i = 0; i < frameSet.size(); i++) {
			BWU += ((double)calc_wctt(frameSet[i]) / frameSet[i].get_period());
		}
		return BWU;
	}
	//TODO ��������֡�ܷ�ʱ��ɣ�����֡�ɵ����Լ��
};