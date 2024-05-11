
#include "PriorityArrangement.h"
int offset_trans(int target, int basis, int T) {
    if (target < basis) {
        target = (int)ceil(((double)basis - target) / T) * T + target;
    }
    return target - basis;
}
//检测frame_set里的帧集合是否存在关键期，关键期时间通过first_instant返回
bool critical_check(const std::vector<canfd_frame>& frame_set, int& first_instant){
    int temp_period = 0, temp_gcd = 0;
    int x = 0, y = 0, h = 0, k = 0, t = 0;
    canfd_frame temp_m = frame_set[0];
    if (frame_set.size() < 2) return false;
    for (size_t i = 1; i < frame_set.size(); i++) {
        temp_gcd = my_algorithm::extended_gcd(temp_m.get_period(), frame_set[i].get_period(), x, y);
        temp_period = temp_m.get_period() * frame_set[i].get_period() / temp_gcd;
        if (abs(temp_m.offset - frame_set[i].offset) % temp_gcd == 0) {
            h = abs(temp_m.offset - frame_set[i].offset) / temp_gcd;
            k = ceil(((double)(abs(x)) * h * temp_m.get_period()) / temp_period);
            t = (h * x) * temp_m.get_period() + k * temp_period;
            temp_m.offset = t - static_cast<int>(floor(t / temp_period)) * temp_period;
            temp_m.set_period(temp_period);
        }
        else {
            return false;
        }
    }
    first_instant = t;
    return true;
}
bool find_interval(const std::vector<canfd_frame*>& frame_set, std::vector<int>& lower_bound, std::vector<int>& upper_bound) {
    if (frame_set.empty()) return false;
    std::vector<int> omax(frame_set.size(),0); //omax为frame_set中，除了自己index对应的frame外最大的offset
    unsigned long long temp_p= frame_set[0]->get_period(); //temp_p为frame_set中所有周期的最小公倍数
    for (size_t j = 0; j < frame_set.size(); j++) {
        for (size_t i = 0; i < frame_set.size(); i++) {
            if (i == j) {
                continue;
            }
            else {
                omax[j] = std::max(omax[j], frame_set[i]->offset);
            }
        }
        temp_p = my_algorithm::lcm(temp_p, (unsigned long long)frame_set[j]->get_period());
    }

    lower_bound.resize(frame_set.size());
    upper_bound.resize(frame_set.size());
    for (size_t i = 0; i < frame_set.size(); i++) {
        int quotient = ceil(double(omax[i]) / frame_set[i]->get_period());
        lower_bound[i] = (int)(quotient * frame_set[i]->get_period());
        upper_bound[i] = lower_bound[i] + temp_p;
    }
    return true;
}
bool create_beta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int lower_bound, std::vector<betaset>& beta) {
    int upper = lower_bound;
    int lower = upper - frame.get_period() + frame.get_deadline();
    /*if (lower >= upper) return false;*/
    //默认deadline<period

    for (size_t i = 0; i < frame_set.size(); i++) {
        if (frame_set[i] == &frame) continue;
        int offset = offset_trans(frame_set[i]->offset, frame.offset, frame_set[i]->get_period());
        if (offset < upper) {
            for (int j = 0; j * frame_set[i]->get_period() + offset < upper; j++) {
                beta.emplace_back(frame_set[i]->get_exec_time(), j * frame_set[i]->get_period() + offset);
            }
        }
    }
    std::sort(beta.begin(), beta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}


bool  create_eta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int t, double R, std::vector<betaset>& eta) {
    int lower = R + t;
    int upper = lower + frame.get_deadline();
    for (size_t i = 0; i < frame_set.size(); i++) {
        if (frame_set[i] == &frame) continue;
        int offset = offset_trans(frame_set[i]->offset, frame.offset, frame_set[i]->get_period());
        //tr=O+m*T,寻找【lower，upper】内所有可能的tr取值，o就是frame_set[i].offset，T是frame_set[i].period，m为常数
        int m = ceil(((double)lower - offset) / (double)frame_set[i]->get_period());
        if (m * frame_set[i]->get_period() + offset < upper) {
            for (int j = 0; (m + j) * frame_set[i]->get_period() + offset < upper; j++) {
                eta.emplace_back(frame_set[i]->get_exec_time(), (m + j) * frame_set[i]->get_period() + offset);
            }
        }
    }
    std::sort(eta.begin(), eta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}

double calc_remain_interf(const canfd_frame& frame, int t, std::vector<betaset>& beta) {
    double R = 0;
    int time = t - frame.get_period() + frame.get_deadline();
    time = 0;
    for (const betaset& b : beta) {
        //time为之前一次任务m的启动时间，因为其他任务都能比任务m先执行，
        //R为已经考虑的任务的执行的时间和，R+time<tr表示之前的任务自从release后，
        //其一定执行完毕，下一个任务不需要再考虑以前的任务了。
        if (b.tr >= time + R) {
            R = 0;
            time = b.tr;
        }
        //int temp = b.tr - time;
        R = R + b.C;
    }
    //积累的任务在R+time时候才能彻底完成， R + time - t表示在当前考虑的任务release后，还需等待之前的任务多久
    R = R + time - t;
    if (R < 0) {
        R = 0;
    }
    return R;
}

double calc_create_interf(const canfd_frame& frame, const int t, const int R, const std::vector<betaset>& eta) {
    int next_free = R + t;
    double K = 0;
    double total_created = R;
    for (const betaset& e : eta) {
        total_created += e.C;
        if (next_free < e.tr) { next_free = e.tr; }
        K = K + std::min(t + (double)frame.get_deadline() - next_free, e.C);
        next_free = std::min(t + (double)frame.get_deadline(), next_free + e.C);
    }

    return K;
}
////待分配优先级帧集合frame_set中，如果要给编号为taski的帧分配最低的优先级（pri），能否可行
//bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri, const std::vector<int>& lower, const std::vector<int>& upper) {
//    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
//    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
//   //      
//    int t = 0;
//    double R = 0, K = 0;
//    std::vector<betaset> beta, eta;
//
//    while (t < upper[taski]) {
//        create_beta(frame_set, *frame_set[taski], lower[taski], beta);
//        R = calc_remain_interf(*frame_set[taski], lower[taski], beta);
//        create_eta(frame_set, *frame_set[taski], lower[taski], R, eta);
//        K = std::max(0.0, calc_create_interf(*frame_set[taski], lower[taski], R, eta));
//        
//        if (frame_set[taski]->get_exec_time() + R + K > frame_set[taski]->get_deadline()) {
//            //任务i永远不能可行，这使得任务集也不可行
//            beta.clear();
//            eta.clear();
//            DEBUG_MSG_DEBUG3("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " > D:", frame_set[taski]->get_deadline());
//            DEBUG_MSG_DEBUG3("任务", frame_set[taski]->get_id(), "  分配优先级", pri, "失败");
//            return false;
//        }
//        DEBUG_MSG_DEBUG3("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " <= D:", frame_set[taski]->get_deadline());
//        beta.clear();
//        eta.clear();
//        t = t + frame_set[taski]->get_period();
//    }
//
//
//    return true;
//}
//score评价可行性有多差，score为0是完全可行的，大于0表示不可行，越大越不可行
double feasibility_check_with_score(std::vector<canfd_frame*>& frame_set, int taski, int pri, const std::vector<int>& lower, const std::vector<int>& upper) {
    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
   //      
    int t = 0;
    double R = 0, K = 0;
    std::vector<betaset> beta, eta;
    int score = 0;

    while (t < upper[taski]) {
        create_beta(frame_set, *frame_set[taski], lower[taski], beta);
        R = calc_remain_interf(*frame_set[taski], lower[taski], beta);
        create_eta(frame_set, *frame_set[taski], lower[taski], R, eta);
        K = std::max(0.0, calc_create_interf(*frame_set[taski], lower[taski], R, eta));

        if (frame_set[taski]->get_exec_time() + R + K > frame_set[taski]->get_deadline()) {
            //任务i永远不能可行，这使得任务集也不可行
            beta.clear();
            eta.clear();
            DEBUG_MSG_DEBUG3("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " > D:", frame_set[taski]->get_deadline());
            DEBUG_MSG_DEBUG3("任务", frame_set[taski]->get_id(), "  分配优先级", pri, "失败");
            score = score + (frame_set[taski]->get_exec_time() + R + K - frame_set[taski]->get_deadline());
        }
        DEBUG_MSG_DEBUG3("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " <= D:", frame_set[taski]->get_deadline());
        beta.clear();
        eta.clear();
        t = t + frame_set[taski]->get_period();
    }


    return score;
}
////待分配优先级帧集合frame_set中，如果要给编号为taski的帧分配最低的优先级（pri），能否可行
//bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri) {
//    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
//    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
//   //      
//    int t = 0, R = 0, K = 0;
//    std::vector<int> lower, upper;
//    find_interval(frame_set, lower, upper);
//    std::vector<betaset> beta, eta;
//
//    while (t < upper[taski]) {
//        create_beta(frame_set, *frame_set[taski], lower[taski], beta);
//        R = calc_remain_interf(*frame_set[taski], lower[taski], beta);
//        create_eta(frame_set, *frame_set[taski], lower[taski], R, eta);
//        K = std::max(0.0, calc_create_interf(*frame_set[taski], lower[taski], R, eta));
//        DEBUG_MSG_DEBUG3("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " > ", frame_set[taski]->get_deadline());
//        if (frame_set[taski]->get_exec_time() + R + K > frame_set[taski]->get_deadline()) {
//            //任务i永远不能可行，这使得任务集也不可行
//            beta.clear();
//            eta.clear();
//            DEBUG_MSG_DEBUG3("任务", frame_set[taski]->get_id(), "  分配优先级", pri, "失败");
//            return false;
//        }
//        beta.clear();
//        eta.clear();
//        t = t + frame_set[taski]->get_period();
//    }
//
//
//    return true;
//}

//bool feasibility_check(std::vector<canfd_frame*>& frame_set, std::vector<int>& assign_table) {
//    int t = 0, R = 0, K = 0;
//    std::vector<int> lower, upper;
//    if (!find_interval(frame_set, lower, upper)) { return false; }
//    std::vector<betaset> beta, eta;
//    int taski = 0;
//
//    std::vector<canfd_frame*> frame_set_copy(frame_set.begin(), frame_set.end());
//
//    int pri = frame_set.size();
//    for (int i = 0; pri > 0; pri--, i++) {
//        //确认frame_set[assign_table[i]]对应的任务是否可行，可行则分配优先级pri并从pendingSet中删除该任务
//        //if (feasibility_check(pendingSet,assign_table[i])) {
//        //}
//        //第一步找到数组中对应任务的位置
//        auto it2 = std::find_if(frame_set_copy.begin(), frame_set_copy.end(), [&](const canfd_frame* frame) {
//            return frame->get_id() == assign_table[i];
//            });
//        if (it2 != frame_set_copy.end() && feasibility_check(frame_set_copy, std::distance(frame_set_copy.begin(), it2), pri)) {
//            /*auto it = frame_set_copy.begin() + std::distance(pendingSet.begin(), it2);*/
//            /*(*it)->set_priority(pri);*/
//            //优先级应在最后成功后统一分配
//            DEBUG_MSG_DEBUG3("任务", (*it2)->get_id(), "  分配优先级", pri, "成功");
//            frame_set_copy.erase(it2);
//            //pendingSet.erase(it2);
//        }
//        else {
//            DEBUG_MSG_DEBUG2("=============================== ");
//            DEBUG_MSG_DEBUG2("任务", (*it2)->get_id(), "  分配优先级", pri , "失败", "  分配策略有问题");
//            DEBUG_MSG_DEBUG2("=============================== ");
//            return false;
//        }
//    }
//    DEBUG_MSG_DEBUG2("=============================== ");
//    DEBUG_MSG_DEBUG2("优先级分配成功！！！ ");
//    for (size_t i = 0; i < assign_table.size(); i++) {
//        frame_set[assign_table[i]]->set_priority(frame_set.size() - i);
//        DEBUG_MSG_DEBUG2("任务: ", frame_set[assign_table[i]]->get_id(), "  优先级： ", frame_set[assign_table[i]]->get_priority());
//    }
//    DEBUG_MSG_DEBUG2("=============================== ");
//
//    return true;
//}
//参数使用canfd_frame*的原因是，有必要创建一个副本来表示未分配成功的frame集合，使用指针一方面减少拷贝的开销，一方面还能通过副本操作原本
//可能会面对一个frame集合，其中部分frame是空的
bool assign_priority(std::vector<canfd_frame*>& frame_set,int schedule_require) {
    std::vector<canfd_frame*> frame_set_copy;
    for (auto& cfp : frame_set) {
        //get_deadline=-1表示该帧从未被使用过
        if (cfp->get_deadline()!=-1) {
            frame_set_copy.push_back(cfp);
        }
    }
    int actual_amount = frame_set_copy.size();//实际需要分配优先级的帧数量
    std::vector<int> lower, upper;


    bool unassigned = true,schedule_flag=true;
    double min_score = DBL_MAX, temp_score = DBL_MAX;
    auto it = frame_set_copy.begin();

    for (int pri = actual_amount -1; pri >= 0; pri--) {
        unassigned = true;
        find_interval(frame_set_copy, lower, upper);
        temp_score=min_score = DBL_MAX;
        //填充可选任务集合
        for (size_t index = 0; index < frame_set_copy.size(); index++) {
            temp_score = feasibility_check_with_score(frame_set_copy, index, pri, lower, upper);
            if (temp_score ==0) {
                it = frame_set_copy.begin() + index;
                (*it)->set_priority(pri);
                DEBUG_MSG_DEBUG2("任务", (*it)->get_id(), "  分配优先级", pri, "成功！！！！！！！！！");
                frame_set_copy.erase(it);  //从未分配集合中，删除分配成功的frmae，然后尝试分配下一个优先级
                unassigned = false;
                break;
            }
            else if(schedule_require) {
                if (temp_score < min_score) {
                    min_score = temp_score;
                    it = frame_set_copy.begin() + index;
                }
            }
        }
        if (unassigned) {
            if (schedule_require) {
                (*it)->set_priority(pri);
                DEBUG_MSG_DEBUG2("任务", (*it)->get_id(), "  强行 分配优先级", pri, "成功！！！！！！！！！");
                frame_set_copy.erase(it);  //从未分配集合中，删除分配成功的frmae，然后尝试分配下一个优先级
                schedule_flag = false;
            }
            else {
                DEBUG_MSG_DEBUG2("=============================== ");
                DEBUG_MSG_DEBUG2("优先级分配失败。。。 ");
                DEBUG_MSG_DEBUG2("=============================== ");
                return false;
            }

        }
        lower.clear();
        upper.clear();
    }

    DEBUG_MSG_DEBUG2("=============================== ");
    DEBUG_MSG_DEBUG2("优先级分配成功！！！ ");
    for (size_t i = 0; i < frame_set.size(); i++) {
        if (frame_set[i]->get_deadline() != -1) {
            DEBUG_MSG_DEBUG2("任务: " , frame_set[i]->get_id() , "  优先级： " , frame_set[i]->get_priority());
        }

    }
    DEBUG_MSG_DEBUG2("=============================== ");
    return schedule_flag;
}


