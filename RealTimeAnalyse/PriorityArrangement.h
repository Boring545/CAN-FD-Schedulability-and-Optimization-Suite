#pragma once
#include <vector>

#include<algorithm>
#include "canfd_frame.h"
#include"math_algorithms.h"
#include<iostream>

//#define DEBUG_OUTPUT
#include"debug_tool.h"


//���frame_set���֡�����Ƿ���ڹؼ��ڣ��ؼ���ʱ��ͨ��first_instant����
bool critical_check(const std::vector<canfd_frame>& frame_set, int& first_instant);
//�����Ϣ�������Ϣ�Ƿ���instantʱ���������,δ���
bool canMeetDeadlines(const std::vector<message>& messageSet, const int& instant);
int offset_trans(int target, int basis, int T);
//ȷ��ÿ������ķ����Ͻ���½�
bool find_interval(const std::vector<canfd_frame*>& frame_set, std::vector<int>& lower_bound, std::vector<int>& upper_bound);
class betaset {
public:
    int C;   //ִ��ʱ��
    int tr;  //releaseʱ��
    betaset(int C,int tr):C(C),tr(tr){}
};
//���ɦ¼���
bool create_beta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int lower_bound, std::vector<betaset>& beta);
//����ʣ�����R_t_i����ʱ��upperʱ release�����ȼ� i ��������Ե�֮ǰ�����ȼ�����δ��ɵ�ʣ��ִ��ʱ��,lower=upper-T_i+D_i
int calc_remain_interf(const canfd_frame& frame, int t, std::vector<betaset>& beta);
//���ɦǼ���
bool  create_eta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int t, int R, std::vector<betaset>& eta);
//������tʱ�����ȼ�i ����release��release�������������Ӱ��ʱ��
int calc_create_interf(const canfd_frame& frame, const int t, const int R, const std::vector<betaset>& eta);
//������񼯿�����
//bool feasibility_check(std::vector<message>& messageSet);
//frame_setΪ���������ȼ����񼯺ϣ������б��Ϊtaski�����������͵����ȼ���priûɶ��
bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri);
//����assign_table��˳�����δӵڵ��߳��Է������ȼ���frame_setΪ���������ȼ����񼯺ϡ�
bool feasibility_check(std::vector<canfd_frame*>& frame_set, std::vector<int>& assign_table);
//�Զ���frame_set�е�frame�������ȼ���frame_setΪ���������ȼ����񼯺ϡ�
bool assign_priority(std::vector<canfd_frame*>& frame_set);
class PriorityArrangement {
    
};
