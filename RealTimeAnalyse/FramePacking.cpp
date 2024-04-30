#include "FramePacking.h"

//TODO 将信号按照遗传算法进行打包
//1.初始打包方案 数据帧只装载一个message、使用MILP或其他算法的的结果作为初始打包方案
//使用first fit decrease方案进行打包，要求装载时不超过frame承载能力，
//每装载一个message，都要检测加入message后的frame，满足WCTT≤Deadline
//如何生成n种满足条件的拆分方案呢？
