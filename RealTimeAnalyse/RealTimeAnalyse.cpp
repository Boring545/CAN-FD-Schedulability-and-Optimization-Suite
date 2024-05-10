#include <iostream>
#include"PriorityArrangement.h"
#include "FramePacking.h"
#include"offset_arrangement.h"


#define DEBUG_OUTPUT
#include"debug_tool.h"

void pri_test2() {



	int id_size = 20;
	int population_size = 10;
	int frame_num = 10;
	std::vector<message> message_set;
	std::unordered_set<int> available_ids;
	available_ids.reserve(id_size);

	for (int i = 0; i < id_size; i++) {
		available_ids.insert(i);
	}
	std::mutex id_mutex;
	canfd_utils setting;
	//TODO 也许随机生成时，1.应使得执行时间远小于周期 2.应使得messgae的周期间有较大的最大公因数
	//一个方法是周期不应过小，先确定最小周期，其他周期都是该最小周期的倍数
	message::parallel_generate_messages(message_set, id_size, available_ids, id_mutex,setting); //生成messgae集合
	message::write_messages(message_set, 1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	message_set = message::read_messages(1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");




	FramePacking fp(message_set, setting);
	auto scheme=fp.message_pack();
	DEBUG_MSG_INFO("U: " , scheme.calc_bandwidth_utilization());



}
//TODO 1.修改初始打包算法，
//应该在打包时避免将deadline相差太大，周期的最小公因数过小的消息打包在一个数据帧中

int main()
{

	pri_test2();
	/*bb();*/
	return 0;
}

