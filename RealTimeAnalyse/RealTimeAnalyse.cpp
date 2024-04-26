#include <iostream>
#include"PriorityArrangement.h"
int main()
{
	std::vector<message> mset;
	mset.push_back(message(10,10,1,1,4));
	mset.push_back(message(10, 10, 2, 1, 5));
	mset.push_back(message(10, 20, 6, 5, 0));
	mset.push_back(message(10, 40, 9, 8, 7));
	mset.push_back(message(10, 40, 14, 8, 27));
	mset.push_back(message(10,40,30,6,0));
	std::vector<betaset> beta;
	std::vector<int> lower, upper;
	find_interval(mset, lower, upper);
	create_beta(mset, mset[5], lower[5], beta);

	int time = 0;
	std::cout << "是否存在关键期： " << critical_check(mset, time) << std::endl;
	std::cout<< "关键期出现在：" << time;
	return 0;
}

