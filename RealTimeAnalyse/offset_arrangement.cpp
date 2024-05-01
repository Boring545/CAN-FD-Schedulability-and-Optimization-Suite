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
	int g = calc_time_granularity(frame_set);
	std::vector<int> released(frame_set_copy[frame_set_copy.size() - 1]->get_period() / g, 0);
	int index = 0, max_interval = 0, left = 0, right = 0, temp = 0, max_left = 0, max_right = 0;
	
	//按顺序对frame_set_copy中的frame依次分配offset
	for (size_t i = 0; i < frame_set_copy.size(); i++) {
		index = (0 * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;
		max_interval = 0, max_left = 0, max_right = 0;
		//
		for (int k = 0; index < (int)released.size(); ) {
			//TODO 双向搜索，记录最大区间
			left = index;
			right = index;
			temp = 0;
			while (released[right] == 0 && frame_set_copy[i]->get_period() > temp) {
				if (right == released.size() - 1) { right = -1; }
				right++;
				temp++;
			}
			while (released[left] == 0 && frame_set_copy[i]->get_period() > temp) {
				if (left == 0) { left = released.size(); }
				left--;
				temp++;
			}

			if (frame_set_copy[i]->get_period() == temp) temp = frame_set_copy[i]->get_period() - 1;
			if (max_interval < temp) {
				max_left = left;
				max_right = right;
				max_interval = temp;
			}
			released[index] += 1;

			k++;
			index = (k * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;
		}

		int new_offset = (max_left + 1 + max_interval / 2) % released.size();
		frame_set_copy[i]->offset = new_offset;
	}
	return true;
}