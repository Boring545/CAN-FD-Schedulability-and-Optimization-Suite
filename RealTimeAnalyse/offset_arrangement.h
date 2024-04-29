#pragma once
#include"canfd_frame.h"
#include<algorithm>
#include <numeric>
//数据帧的offset一般为随机值，现在希望通过offset分配算法，减少任务在同一时间集中执行
//从而使得offset更接近真实值
//from：Pushing the limits of CAN - scheduling frames with offsetsprovides a major performance boost

//计算所有流周期的最大公因数可以得到时间单位g，比如有T1=2,T2=6,那么g=2作为最小时间单位，使得我们用新的时间尺度release time （以g为单位）来代替实际的时间.
int calc_time_granularity(const std::vector<canfd_frame>& frame_set) {
	if (frame_set.empty()) { return -1; }
	int g = frame_set[0].get_period();
	for (size_t i = 1; i < frame_set.size(); i++) {
		g = my_algorithm::gcd(g, frame_set[i].get_period());
	}
	return g;
}
//向
bool  assign_offset(std::vector<canfd_frame>& frame_set) {
	if (frame_set.empty()) {
		return false;
	}
	std::vector<canfd_frame*> frame_set_p(frame_set.size());
	frame_set_p[0] = &frame_set[0];
	int last = 0,t=0;
	for (size_t i=1; i < frame_set.size(); i++) {
		if (frame_set_p[last]->get_period() <= frame_set[i].get_period()) {
			frame_set_p[last+1]=(&frame_set[i]);
		}
		else {
			// 否则找到适当的位置插入当前元素的指针
			int j = frame_set_p.size() - 1;
			while (j >= 0 && frame_set_p[j]->get_period() > frame_set[i].get_period()) {
				j--;
			}
			// 将当前元素的指针插入到 frame_set_p 中合适的位置
			frame_set_p.insert(frame_set_p.begin() + j + 1, &frame_set[i]);
		}
	}
	int g = calc_time_granularity(frame_set);
	std::vector<int> released(frame_set_p[frame_set_p.size() - 1]->get_period() / g, 0);
	int index = 0, max_interval = 0, left = 0, right = 0, max_offset, temp = 0,max_left=0,max_right=0;
	for (size_t i=0; i < frame_set_p.size(); i++) {
		index = (0 * frame_set_p[i]->get_period() + frame_set_p[i]->offset) / g;
		for (int k = 0;   index< released.size(); ) {
			//TODO 双向搜索，记录最大区间
			left = index;
			right = index;
			temp = 0;
			while (released[left] == 0|| frame_set_p[i]->get_period()>temp) {
				if (left == 0) { left = released.size(); }
				left--;
				temp++;
			}
			while (released[right] == 0 || frame_set_p[i]->get_period() > temp) {
				if (right == released.size() - 1) { right = -1; }
				right++;
				temp++;
			}
			if (frame_set_p[i]->get_period() == temp) temp = frame_set_p[i]->get_period() - 1;
			if (max_interval<temp) {
				max_left = left;
				max_right = right;
				max_interval = temp;
			}
			released[index] += 1;

			k++;
			index = (k * frame_set_p[i]->get_period() + frame_set_p[i]->offset) / g;
		}

		int new_offset = (max_left + 1 + max_interval / 2) % released.size();
		frame_set_p[i]->offset = new_offset;
	}

}


