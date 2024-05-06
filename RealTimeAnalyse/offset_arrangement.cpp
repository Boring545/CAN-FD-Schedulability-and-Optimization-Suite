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
	int g = calc_time_granularity(frame_set); //����ʱ������g
	std::vector<int> released(frame_set_copy[frame_set_copy.size() - 1]->get_period() / g, 0); //��¼ÿ��֡��releaseʱ��
	
	int temp_interval = 0,max_interval = 0;//������С���ؼ��
	int left = 0, right = 0, max_left = 0, max_right = 0; //���ؼ����������
	int min_load = frame_set_copy.size();//�������С����
	
	//��˳���frame_set_copy�е�frame���η���offset
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
			//����ѭ����������
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
			//����ȷ����������
			if (max_interval < temp_interval) {
				max_left = left;
				max_right = right;
				max_interval = temp_interval;
			}
		}
		//offsetλ�����������е�
		int new_offset = g * ((max_left  + max_interval / 2) % Ti);
		frame_set_copy[i]->offset = new_offset;

		int index = (0 * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;//index��Ϊ�������releaseʱ�̣���ʱ�����ʼ��Ϊ��һ���㣬��0*
		int k = 0;
		//��ǵ�ǰ����֡���е�releaseλ��
		while (index < (int)released.size()) {
			released[index] += 1;
			k++;
			index = (k * frame_set_copy[i]->get_period() + frame_set_copy[i]->offset) / g;
		}


	}
	return true;
}