#pragma once
#include <vector>
#include <stdexcept>
#include<algorithm>
#include "canfd_frame.h"
#include<iostream>
template<class T>
T gcd(T a, T b);

template<class T>
T extended_gcd(T a, T b, int& x, int& y);

template<class T>
T lcm(T a, T b);
//���mset�е���Ϣ���Ƿ���ڹؼ��ڣ���һ�ιؼ���ֵͨ��first_instant����
bool critical_check(const std::vector<message>& mset, int& first_instant);
//�����Ϣ�������Ϣ�Ƿ���instantʱ���������,δ���
bool canMeetDeadlines(const std::vector<message>& messageSet, const int& instant);
int offset_trans(int target, int basis, int T);
//ȷ��ÿ������ķ����Ͻ���½�
bool find_interval(const std::vector<message>& messageSet, std::vector<int>& lower_bound, std::vector<int>& upper_bound);
class betaset {
public:
    int C;   //ִ��ʱ��
    int tr;  //releaseʱ��
    betaset(int C,int tr):C(C),tr(tr){}
};
//���ɦ¼���
bool create_beta(const std::vector<message>& messageSet, const message& m, int lower_bound, std::vector<betaset>& beta);
//����ʣ�����R_t_i����ʱ��upperʱ release�����ȼ� i ��������Ե�֮ǰ�����ȼ�����δ��ɵ�ʣ��ִ��ʱ��,lower=upper-T_i+D_i
int calc_remain_interf(const message& m, int t, std::vector<betaset>& beta);
//���ɦǼ���
bool  create_eta(const std::vector<message>& messageSet, const message& m, int t, int R, std::vector<betaset>& eta);
//������tʱ�����ȼ�i ����release��release�������������Ӱ��ʱ��
int calc_create_interf(const message& m, int t,int R, const std::vector<betaset>& eta);
//������񼯿�����
bool feasibility_check(std::vector<message>& messageSet);
bool feasibility_check(std::vector<message>& messageSet, int taski,int pri);
bool feasibility_check(std::vector<message>& messageSet, std::vector<int>& assign_table);
//���ȼ�����
bool assign_priority(std::vector<message>& messageSet);
class PriorityArrangement {
    
};
