#pragma once
#include"canfd_frame.h"
#include<algorithm>
#include <numeric>
//����֡��offsetһ��Ϊ���ֵ������ϣ��ͨ��offset�����㷨������������ͬһʱ�伯��ִ��
//�Ӷ�ʹ��offset���ӽ���ʵֵ
//from��Pushing the limits of CAN - scheduling frames with offsetsprovides a major performance boost

//�������������ڵ�����������Եõ�ʱ�䵥λg��������T1=2,T2=6,��ôg=2��Ϊ��Сʱ�䵥λ��ʹ���������µ�ʱ��߶�release time ����gΪ��λ��������ʵ�ʵ�ʱ��.
int calc_time_granularity(const std::vector<canfd_frame*>& frame_set);
//��
bool  assign_offset(std::vector<canfd_frame*>& frame_set);


