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
//检测mset中的消息集是否存在关键期，第一次关键期值通过first_instant传回
bool critical_check(const std::vector<message>& mset, int& first_instant);
//检测消息集里的消息是否在instant时完成其任务,未完成
bool canMeetDeadlines(const std::vector<message>& messageSet, const int& instant);
int offset_trans(int target, int basis, int T);
//确定每个任务的分析上界和下界
bool find_interval(const std::vector<message>& messageSet, std::vector<int>& lower_bound, std::vector<int>& upper_bound);
class betaset {
public:
    int C;   //执行时间
    int tr;  //release时间
    betaset(int C,int tr):C(C),tr(tr){}
};
//生成β集合
bool create_beta(const std::vector<message>& messageSet, const message& m, int lower_bound, std::vector<betaset>& beta);
//计算剩余干涉R_t_i：在时刻upper时 release的优先级 i 任务所面对的之前高优先级任务未完成的剩余执行时间,lower=upper-T_i+D_i
int calc_remain_interf(const message& m, int t, std::vector<betaset>& beta);
//生成η集合
bool  create_eta(const std::vector<message>& messageSet, const message& m, int t, int R, std::vector<betaset>& eta);
//计算在t时刻优先级i 任务release后release的新任务产生的影响时间
int calc_create_interf(const message& m, int t,int R, const std::vector<betaset>& eta);
//检测任务集可行性
bool feasibility_check(std::vector<message>& messageSet);
bool feasibility_check(std::vector<message>& messageSet, int taski,int pri);
bool feasibility_check(std::vector<message>& messageSet, std::vector<int>& assign_table);
//优先级分配
bool assign_priority(std::vector<message>& messageSet);
class PriorityArrangement {
    
};
