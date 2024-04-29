#pragma once
#include"canfd_frame.h"
class canfd_utils {
private:
	int data_rate = 1000000; // 数据段速度，默认值为 1 Mbps
	int arbitration_rate = 1000000; // 仲裁段速度，默认值为 1 Mbps
	double t_arb;
	double t_data;
public:
	//初始化canfd系统基本信息
	bool init_canfd_info(int dataRate,int arbRate) {
		this->data_rate = dataRate;
		t_data = 1.0 / dataRate;
		this->arbitration_rate = arbRate;
		t_arb = 1.0 / arbRate;
		return true;
	}
	//计算最坏情况下的传输时间
	int calc_wctt(canfd_frame frame) {
		int p = frame.get_paylaod_size();
		int wctt = 32 * t_arb + (28 + 5 * ceil(p - 16 / 64.0) + 10.0 * p) * t_data;
		return wctt;
	}
	//计算最好情况下的传输时间
	int calc_bctt(canfd_frame frame) {
		int p = frame.get_paylaod_size();
		int bctt = 29 * t_arb + (27 + 5 * ceil(p - 16 / 64.0) + 8.0 * p) * t_data;
		return bctt;
	}
	//计算带宽利用率
	double calc_bandwidth_utilization(std::vector<canfd_frame>frameSet) {
		double BWU = 0;
		for (size_t i = 0; i < frameSet.size(); i++) {
			BWU += ((double)calc_wctt(frameSet[i]) / frameSet[i].get_period());
		}
		return BWU;
	}
	//TODO 计算数据帧能否按时完成，数据帧可调度性检测
};