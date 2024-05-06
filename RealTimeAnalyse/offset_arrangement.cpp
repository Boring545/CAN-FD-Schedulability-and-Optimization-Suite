#include "offset_arrangement.h"
//计算所有流周期的最大公因数可以得到时间单位g，比如有T1=2,T2=6,那么g=2作为最小时间单位，使得我们用新的时间尺度release time （以g为单位）来代替实际的时间.
int calc_time_granularity(const std::vector<canfd_frame*>& frame_set) {
	if (frame_set.empty()) { return -1; }
	int g = frame_set[0]->get_period();
	for (size_t i = 1; i < frame_set.size(); i++) {
		g = my_algorithm::gcd(g, frame_set[i]->get_period());
	}
	return g;
}
//向
bool  assign_offset(std::vector<canfd_frame*>& frame_set) {
	if (frame_set.empty()) {
		return false;
	}
	std::vector<canfd_frame*> frame_set_copy(frame_set.begin(), frame_set.end());

	//按照周期升序排列
	std::sort(frame_set_copy.begin(), frame_set_copy.end(),
		[](const canfd_frame* a, const canfd_frame* b) {
			return a->get_period() < b->get_period();
		});
	int g = calc_time_granularity(frame_set); //计算时间粒度g
	std::vector<int> released(frame_set_copy[frame_set_copy.size() - 1]->get_period() / g, 0); //记录每个帧的release时间
	
	int temp_interval = 0,max_interval = 0;//最大的最小负载间隔
	int left = 0, right = 0, max_left = 0, max_right = 0; //负载间隔左右区间
	int min_load = frame_set_copy.size();//间隔内最小负载
	
	//按顺序对frame_set_copy中的frame依次分配offset
	for (size_t i = 0; i < frame_set_copy.size(); i++) {
		temp_interval=max_interval = 0;
		max_left = max_right = -1, left = right = -1;
		
		int Ti= frame_set_copy[i]->get_period() / g;
		min_load = *(std::min_element(released.begin(), released.end()));
		
		for (int j = 0; j < Ti; ++j) {
			if (released[j] == min_load&& temp_interval< Ti) {
				if (left == -1) {
					left = j;
				}
				right = j;
				temp_interval++;
			}
			else {
				left = right = -1;
				temp_interval = 0;
			}
			//考虑循环数组的情况
			if (right == Ti - 1 && released[0] == min_load) {
				for (int m = 0; m < Ti; ++m) {
					if (released[m] == min_load && temp_interval < Ti) {
						right = m;
						temp_interval++;
					}
					else {
						break;
					}
				}
			}
			//重新确定最大空区间
			if (max_interval < temp_interval) {
				max_left = left;
				max_right = right;
				max_interval = temp_interval;
			}
		}
		//offset位于最大空区间中点
		int new_offset = g * ((max_left  + max_interval / 2) % Ti);
		frame_set_copy[i]->offset = new_offset;

		int index = (0 * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;//index点为该任务的release时刻，此时将其初始化为第一个点，故0*
		int k = 0;
		//标记当前数据帧所有的release位置
		while (index < (int)released.size()) {
			released[index] += 1;
			k++;
			index = (k * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;
		}


	}
	return true;
}