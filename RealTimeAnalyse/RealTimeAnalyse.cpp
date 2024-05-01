#include <iostream>
#include"PriorityArrangement.h"
#include "FramePacking.h"
#include"offset_arrangement.h"
void paper_test() {
	//canfd_frame* f1 = new canfd_frame(4, 1, 1, 10, 0);
//canfd_frame* f2 = new canfd_frame(5, 1, 2, 10, 1);
//canfd_frame* f3 = new canfd_frame(0, 5, 6, 20, 2);
//canfd_frame* f4 = new canfd_frame(7, 8, 9, 40, 3);
//canfd_frame* f5 = new canfd_frame(27, 8, 14, 40, 4);
//canfd_frame* f6 = new canfd_frame(0, 6, 30, 40, 5);
//std::vector< canfd_frame*> cl;
//cl.push_back(f1);
//cl.push_back(f2);
//cl.push_back(f3);
//cl.push_back(f4);
//cl.push_back(f5);
//cl.push_back(f6);
//std::vector<int> table = { 4,5,1,3,2,0 };
////使用论文例子，优先级分配算法结果同论文一致
//std::cout << feasibility_check(cl, table);
//return;
}
void pri_test() {



	int id_size = 20;
	int max_try = 10;
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
	auto individuals = initial_population(population,message_set2, max_try);
	assign_offset(population[0]);
	assign_priority(population[0]);



}
//TODO 1.修改初始打包算法，
//应该在打包时避免将deadline相差太大，周期的最小公因数过小的消息打包在一个数据帧中

int main()
{

	pri_test();
	return 0;
}

