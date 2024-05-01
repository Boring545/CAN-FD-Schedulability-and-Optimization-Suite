#pragma once
#include"canfd_frame.h"
#include<algorithm>
#include <numeric>
//数据帧的offset一般为随机值，现在希望通过offset分配算法，减少任务在同一时间集中执行
//从而使得offset更接近真实值
//from：Pushing the limits of CAN - scheduling frames with offsetsprovides a major performance boost

//计算所有流周期的最大公因数可以得到时间单位g，比如有T1=2,T2=6,那么g=2作为最小时间单位，使得我们用新的时间尺度release time （以g为单位）来代替实际的时间.
int calc_time_granularity(const std::vector<canfd_frame*>& frame_set);
//向
bool  assign_offset(std::vector<canfd_frame*>& frame_set);


