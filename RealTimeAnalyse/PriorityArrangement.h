#pragma once
#include <vector>

#include<algorithm>
#include "canfd_frame.h"
#include"math_algorithms.h"
#include<iostream>

//#define DEBUG_OUTPUT
#include"debug_tool.h"


//检测frame_set里的帧集合是否存在关键期，关键期时间通过first_instant返回
bool critical_check(const std::vector<canfd_frame>& frame_set, int& first_instant);
//检测消息集里的消息是否在instant时完成其任务,未完成
bool canMeetDeadlines(const std::vector<message>& messageSet, const int& instant);
int offset_trans(int target, int basis, int T);
//确定每个任务的分析上界和下界
bool find_interval(const std::vector<canfd_frame*>& frame_set, std::vector<int>& lower_bound, std::vector<int>& upper_bound);
class betaset {
public:
    int C;   //执行时间
    int tr;  //release时间
    betaset(int C,int tr):C(C),tr(tr){}
};
//生成β集合
bool create_beta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int lower_bound, std::vector<betaset>& beta);
//计算剩余干涉R_t_i：在时刻upper时 release的优先级 i 任务所面对的之前高优先级任务未完成的剩余执行时间,lower=upper-T_i+D_i
int calc_remain_interf(const canfd_frame& frame, int t, std::vector<betaset>& beta);
//生成η集合
bool  create_eta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int t, int R, std::vector<betaset>& eta);
//计算在t时刻优先级i 任务release后release的新任务产生的影响时间
int calc_create_interf(const canfd_frame& frame, const int t, const int R, const std::vector<betaset>& eta);
//检测任务集可行性
//bool feasibility_check(std::vector<message>& messageSet);
//frame_set为待分配优先级任务集合，向其中标号为taski的任务分配最低的优先级，pri没啥用
bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri);
//按照assign_table的顺序，依次从第到高尝试分配优先级【frame_set为待分配优先级任务集合】
bool feasibility_check(std::vector<canfd_frame*>& frame_set, std::vector<int>& assign_table);
//自动向frame_set中的frame分配优先级【frame_set为待分配优先级任务集合】
bool assign_priority(std::vector<canfd_frame*>& frame_set);
class PriorityArrangement {
    
};
