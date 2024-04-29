#include <iostream>
#include"PriorityArrangement.h"
int main()
{
	std::vector<message> mset;
	mset.push_back(message(10,10,1,1,4,1));
	mset.push_back(message(10, 10, 2, 1, 5,2));
	mset.push_back(message(10, 20, 6, 5, 0,3));
	mset.push_back(message(10, 40, 9, 8, 7,4));
	mset.push_back(message(99, 40, 14, 8, 27,5));
	mset.push_back(message(10,40,30,6,0,6));
	//std::vector<betaset> beta,eta;
	//std::vector<int> lower, upper;
	//find_interval(mset, lower, upper);
	//int i = 5;
	//create_beta(mset, mset[i], lower[i], beta);
	//int R=calc_remain_interf(mset[i], lower[i], beta);
	//create_eta(mset, mset[i], lower[i], R, eta);
	//int K = calc_create_interf(mset[i], lower[i],R,eta);
	//std::cout << "R:" << R << "  K:" << K << std::endl;
	//int time = 0;
	//if (mset[i].exec_time + R + K >= mset[i].deadline) {
	//	std::cout << "可分配："  << std::endl;
	//}
	std::vector<int> v = { 5,6,2,4,3,1 };
	//feasibility_check(mset, v);



	//TODO 
	/*assign_priority(mset);*/
	std::vector<message> message_set;
	message::parallel_generate_messages(message_set, 5000);
	message::write_messages(message_set,1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");

	return 0;
}

