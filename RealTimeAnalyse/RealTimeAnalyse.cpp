#include <iostream>
#include"PriorityArrangement.h"
#include "FramePacking.h"
#include"offset_arrangement.h"
void pri_test() {
	int id_size = 100;
	std::vector<message> message_set;
	std::unordered_set<int> available_ids;
	available_ids.reserve(id_size);

	for (int i = 0; i < id_size; i++) {
		available_ids.insert(i);
	}
	std::mutex id_mutex;
	message::parallel_generate_messages(message_set, id_size, available_ids, id_mutex); //生成messgae集合
	message::write_messages(message_set,1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	std::vector<std::vector<canfd_frame*>> population;
	std::vector<message> message_set2 = message::read_messages(1, "D:/document/CODE/C++/RealTimeAnalyse/RealTimeAnalyse/input");
	auto individuals = initial_population(population,message_set2, 10);
	assign_offset(population[0]);
	std::cout<<assign_priority(population[0]);
	return;
}

int main()
{

	pri_test();
	return 0;
}

