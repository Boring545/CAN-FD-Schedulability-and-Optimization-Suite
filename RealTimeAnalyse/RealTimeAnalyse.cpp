#include <iostream>
#include"PriorityArrangement.h"
#include "FramePacking.h"
#include"offset_arrangement.h"


void pri_test() {



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
	//TODO 也许随机生成时，1.应使得执行时间远小于周期 2.应使得messgae的周期间有较大的最大公因数
	//一个方法是周期不应过小，先确定最小周期，其他周期都是该最小周期的倍数
	message::parallel_generate_messages(message_set, id_size, available_ids, id_mutex); //生成messgae集合
	message::write_messages(message_set,1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	std::vector<std::vector<canfd_frame*>> population;
	std::vector<message> message_set2 = message::read_messages(1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	auto individuals = initial_population(population,message_set2, population_size,frame_num,3);
	
	for (auto& x : population) {
		assign_offset(x);
		assign_priority(x);
	}
	



}
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
	//TODO 也许随机生成时，1.应使得执行时间远小于周期 2.应使得messgae的周期间有较大的最大公因数
	//一个方法是周期不应过小，先确定最小周期，其他周期都是该最小周期的倍数
	message::parallel_generate_messages(message_set, id_size, available_ids, id_mutex); //生成messgae集合
	message::write_messages(message_set, 1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	message_set = message::read_messages(1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");


	canfd_utils setting;

	FramePacking fp(message_set, setting);
	auto scheme=fp.message_pack();




}
//TODO 1.修改初始打包算法，
//应该在打包时避免将deadline相差太大，周期的最小公因数过小的消息打包在一个数据帧中

int main()
{

	pri_test2();
	/*bb();*/
	return 0;
}

