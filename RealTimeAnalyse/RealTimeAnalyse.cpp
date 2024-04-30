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

	/*std::vector<int> v = { 5,6,2,4,3,1 };*/
	//feasibility_check(mset, v);



	//TODO 
	/*assign_priority(mset);*/
	int id_size=5000;
	std::vector<message> message_set;
	std::unordered_set<int> available_ids;
	available_ids.reserve(id_size);

	for (int i = 0; i < id_size; i++) {
		available_ids.insert(i);
	}
	std::mutex id_mutex;
	message::parallel_generate_messages(message_set, id_size, available_ids, id_mutex);
	message::write_messages(message_set,1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	std::vector<message> message_set2=message::read_messages(1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	message::print_messages(message_set2);
	return 0;
}

