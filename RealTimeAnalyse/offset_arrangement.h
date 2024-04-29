#pragma once
#include"canfd_frame.h"
#include<algorithm>
#include <numeric>
//����֡��offsetһ��Ϊ���ֵ������ϣ��ͨ��offset�����㷨������������ͬһʱ�伯��ִ��
//�Ӷ�ʹ��offset���ӽ���ʵֵ
//from��Pushing the limits of CAN - scheduling frames with offsetsprovides a major performance boost

//�������������ڵ�����������Եõ�ʱ�䵥λg��������T1=2,T2=6,��ôg=2��Ϊ��Сʱ�䵥λ��ʹ���������µ�ʱ��߶�release time ����gΪ��λ��������ʵ�ʵ�ʱ��.
int calc_time_granularity(const std::vector<canfd_frame>& frame_set) {
	if (frame_set.empty()) { return -1; }
	int g = frame_set[0].get_period();
	for (size_t i = 1; i < frame_set.size(); i++) {
		g = my_algorithm::gcd(g, frame_set[i].get_period());
	}
	return g;
}
//��
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
			// �����ҵ��ʵ���λ�ò��뵱ǰԪ�ص�ָ��
			int j = frame_set_p.size() - 1;
			while (j >= 0 && frame_set_p[j]->get_period() > frame_set[i].get_period()) {
				j--;
			}
			// ����ǰԪ�ص�ָ����뵽 frame_set_p �к��ʵ�λ��
			frame_set_p.insert(frame_set_p.begin() + j + 1, &frame_set[i]);
		}
	}
	int g = calc_time_granularity(frame_set);
	std::vector<int> released(frame_set_p[frame_set_p.size() - 1]->get_period() / g, 0);
	int index = 0, max_interval = 0, left = 0, right = 0, max_offset, temp = 0,max_left=0,max_right=0;
	for (size_t i=0; i < frame_set_p.size(); i++) {
		index = (0 * frame_set_p[i]->get_period() + frame_set_p[i]->offset) / g;
		for (int k = 0;   index< released.size(); ) {
			//TODO ˫����������¼�������
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


