#include "offset_arrangement.h"
//�������������ڵ�����������Եõ�ʱ�䵥λg��������T1=2,T2=6,��ôg=2��Ϊ��Сʱ�䵥λ��ʹ���������µ�ʱ��߶�release time ����gΪ��λ��������ʵ�ʵ�ʱ��.
int calc_time_granularity(const std::vector<canfd_frame*>& frame_set) {
	if (frame_set.empty()) { return -1; }
	int g = frame_set[0]->get_period();
	for (size_t i = 1; i < frame_set.size(); i++) {
		g = my_algorithm::gcd(g, frame_set[i]->get_period());
	}
	return g;
}
//��
bool  assign_offset(std::vector<canfd_frame*>& frame_set) {
	if (frame_set.empty()) {
		return false;
	}
	std::vector<canfd_frame*> frame_set_copy(frame_set.begin(), frame_set.end());

	//����������������
	std::sort(frame_set_copy.begin(), frame_set_copy.end(),
		[](const canfd_frame* a, const canfd_frame* b) {
			return a->get_period() < b->get_period();
		});
	int g = calc_time_granularity(frame_set);
	std::vector<int> released(frame_set_copy[frame_set_copy.size() - 1]->get_period() / g, 0);
	int index = 0, max_interval = 0, left = 0, right = 0, temp = 0, max_left = 0, max_right = 0;
	
	//��˳���frame_set_copy�е�frame���η���offset
	for (size_t i = 0; i < frame_set_copy.size(); i++) {
		index = (0 * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;
		max_interval = 0, max_left = 0, max_right = 0;
		//
		for (int k = 0; index < (int)released.size(); ) {
			//TODO ˫����������¼�������
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