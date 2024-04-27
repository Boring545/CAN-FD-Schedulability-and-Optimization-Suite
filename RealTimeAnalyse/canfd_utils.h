#pragma once
class canfd_utils {
public:
	int data_rate = 1000000; // 数据段速度，默认值为 1 Mbps
	int arbitration_rate = 1000000; // 仲裁段速度，默认值为 1 Mbps

	//TODO wctt计算

};