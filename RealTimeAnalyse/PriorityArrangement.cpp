
#include "PriorityArrangement.h"

template<class T>
T gcd(T a, T b) {
    if (a <= 0 || b <= 0) {
        throw std::invalid_argument("Arguments must be positive numbers.");
    }
    while (b != 0) {
        T temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

template<class T>
T extended_gcd(T a, T b, int& x, int& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int x1, y1;
    T gcd_val = extended_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd_val;
}

template<class T>
T lcm(T a, T b) {
    if (a <= 0 || b <= 0) {
        throw std::invalid_argument("Arguments must be positive numbers.");
    }
    return (a * b) / gcd(a, b);
}

bool critical_check(const std::vector<message>& mset, int& first_instant) {
    int temp_period = 0, temp_gcd = 0;
    int x = 0, y = 0, h = 0, k = 0, t = 0;
    message temp_m = mset[0];
    if (mset.size() < 2) return false;
    for (size_t i = 1; i < mset.size(); i++) {
        temp_gcd = extended_gcd(temp_m.period, mset[i].period, x, y);
        temp_period = temp_m.period * mset[i].period / temp_gcd;
        if (abs(temp_m.offset - mset[i].offset) % temp_gcd == 0) {
            h = abs(temp_m.offset - mset[i].offset) / temp_gcd;
            k = static_cast<int>(ceil((static_cast<double>(abs(x)) * h * temp_m.period) / temp_period));
            t = (h * x) * temp_m.period + k * temp_period;
            temp_m.offset = t - static_cast<int>(floor(t / temp_period)) * temp_period;
            temp_m.period = temp_period;
        }
        else {
            return false;
        }
    }
    first_instant = t;
    return true;
}
int offset_trans(int target, int basis,int T) {
    if (target < basis) {
        target = ceil((basis - target) / static_cast<double>(T))*T + target;
    }
    return target-basis;
}
bool find_interval(const std::vector<message>& messageSet, std::vector<int>& lower_bound, std::vector<int>& upper_bound) {
    if (messageSet.empty()) return false;
    std::vector<int> omax(messageSet.size()),temp_p(messageSet.size());
    omax[0]= messageSet[0].offset; //o_i为 [0,i]任务的最大offset
    temp_p[0]= messageSet[0].period; //temp_pi为[0,i]任务的最小公倍数

    for (size_t j = 1; j < messageSet.size(); j++) {
        omax[j] = std::max(omax[j-1], messageSet[j].offset);
        temp_p[j] = lcm(temp_p[j - 1], messageSet[j].period);
    }
    lower_bound.resize(messageSet.size());
    upper_bound.resize(messageSet.size());
    lower_bound[0] = 0; upper_bound[0] = temp_p[0];
    for (size_t i = 1; i < messageSet.size(); i++) {
        int quotient = ceil(static_cast<double>(omax[i - 1]) / messageSet[i].period);
        lower_bound[i] = static_cast<int>(quotient * messageSet[i].period);
        upper_bound[i] = lower_bound[i] + temp_p[i];
    }
    return true;
}
bool create_beta(const std::vector<message>& messageSet,const message& m, int lower_bound, std::vector<betaset>& beta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    upper = lower_bound;
    lower = upper - m.period + m.deadline;
    /*if (lower >= upper) return false;*/
    //默认deadline<period

    for (size_t i = 0; i < messageSet.size(); i++) {
        if (&(messageSet[i]) == &m) continue;
        //tr=o+m*T,寻找【lower，upper】内所有可能的tr取值，o就是messageSet[i].offset，T是messageSet[i].period，m为常数
        //int m = ceil((lower - messageSet[i].offset) / (double)messageSet[i].period);
        //if (m * messageSet[i].period + messageSet[i].offset < upper) {
        //    for (size_t j = 0; (m + j) * messageSet[i].period + messageSet[i].offset <= upper; j++) {
        //        betaset temp_b(messageSet[i].exec_time, (m + j) * messageSet[i].period + messageSet[i].offset);
        //        beta.push_back(temp_b);
        //    }
        
        int offset = offset_trans(messageSet[i].offset, m.offset, messageSet[i].period);
        if (offset < upper) {
            for (int j = 0; j * messageSet[i].period + offset < upper; j++) {
                betaset temp_b(messageSet[i].exec_time, j * messageSet[i].period + offset);
                beta.push_back(temp_b);
            }
        }
    }
    std::sort(beta.begin(), beta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}
bool  create_eta(const std::vector<message>& messageSet, const message& m, int t, int R, std::vector<betaset>& eta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    lower = R + t; 
    upper = lower + m.deadline;
    for (size_t i = 0; i < messageSet.size(); i++) {
        if (&(messageSet[i]) == &m) continue;
        int offset = offset_trans(messageSet[i].offset, m.offset, messageSet[i].period);
        //tr=O+m*T,寻找【lower，upper】内所有可能的tr取值，o就是messageSet[i].offset，T是messageSet[i].period，m为常数
        int m = ceil((lower - offset) / (double)messageSet[i].period);
        if (m * messageSet[i].period + offset < upper) {
            for (int j = 0; (m + j) * messageSet[i].period + offset < upper; j++) {
                betaset temp_b(messageSet[i].exec_time, (m + j) * messageSet[i].period + offset);
                eta.push_back(temp_b);
            }
        }
        //int m = ceil((lower - messageSet[i].offset) / (double)messageSet[i].period);
        //if (m * messageSet[i].period + messageSet[i].offset < upper) {
        //    for (size_t j = 0; (m + j) * messageSet[i].period + messageSet[i].offset < upper; j++) {
        //        betaset temp_b(messageSet[i].exec_time, (m + j) * messageSet[i].period + messageSet[i].offset);
        //        eta.push_back(temp_b);
        //    }
        //}
    }
    std::sort(eta.begin(), eta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}
int calc_remain_interf(const message& m,int t, std::vector<betaset>& beta) {
    int R = 0;
    int time= t - m.period + m.deadline;
    time = 0;
    for (betaset b : beta) {
        //time为之前一次任务m的启动时间，因为其他任务都能比任务m先执行，
        //R为已经考虑的任务的执行的时间和，R+time<tr表示之前的任务自从release后，
        //其一定执行完毕，下一个任务不需要再考虑以前的任务了。
        if (b.tr >= time + R) {
            R = 0;
            time = b.tr;
        }
        int temp =  b.tr- time;
        R =R+ b.C;
    }
    R = R +time-t;
    if (R < 0) {
        R = 0;
    }
    return R;
}

int calc_create_interf(const message& m, const int t, const int R, const std::vector<betaset>& eta) {
    int next_free = R + t;
    int K = 0;
    int total_created = R;
    for (betaset e : eta) {
        total_created += e.C;
        if (next_free < e.tr) { next_free = e.tr; }
        K = K + std::min(t + m.deadline - next_free, e.C);
        next_free = std::min(t + m.deadline, next_free + e.C);
    }

    return K;
}

bool feasibility_check(std::vector<message>& messageSet) {
    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
   //      
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    find_interval(messageSet, lower, upper);
    std::vector<betaset> beta, eta;
    for (size_t i = 0; i < messageSet.size(); i++) {
        t = 0;
        while (t < upper[i]) {
            create_beta(messageSet, messageSet[i], lower[i], beta);
            R = calc_remain_interf(messageSet[i], lower[i], beta);
            create_eta(messageSet, messageSet[i], lower[i], R, eta);
            K = calc_create_interf(messageSet[i], lower[i], R, eta);
            if (messageSet[i].exec_time + R + K > messageSet[i].deadline) {
                //任务i永远不能可行，这使得任务集也不可行
                beta.clear();
                eta.clear();
                return false;
            }
            t = t + messageSet[i].period;
            beta.clear();
            eta.clear();
        }
    }
    return true;
}
bool feasibility_check(std::vector<message>& messageSet, int taski,int pri) {
    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
   //      
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    find_interval(messageSet, lower, upper);
    std::vector<betaset> beta, eta;

    create_beta(messageSet, messageSet[taski], lower[taski], beta);
    R = calc_remain_interf(messageSet[taski], lower[taski], beta);
    create_eta(messageSet, messageSet[taski], lower[taski], R, eta);
    K = calc_create_interf(messageSet[taski], lower[taski], R, eta);
    DEBUG_MSG("R:", R, "  K:", K);
    //std::cout << "R:" << R << "  K:" << K << std::endl;
    if (messageSet[taski].exec_time + R + K > messageSet[taski].deadline) {
        //任务i永远不能可行，这使得任务集也不可行
        beta.clear();
        eta.clear();
        DEBUG_MSG("任务" , taski , "  分配优先级" , pri , "失败");
        //std::cout << "任务" << taski << "  分配优先级" << pri << "失败" << std::endl;
        return false;
    }
    beta.clear();
    eta.clear();
   
    return true;
}
bool feasibility_check(std::vector<message>& messageSet, std::vector<int>&assign_table) {
    //TODO 可能存在一个问题，即每次循环都是从未分配优先级的任务集合中选取一个任务去试探分配优先级，这使得任务集合越来越小。
    //     beta，eta等后续计算是否只用考虑未分配集合呢？解决方法是后续计算使用一个新的，不断更新（删除已分配优先级任务）的任务集合
   //      
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    find_interval(messageSet, lower, upper);
    std::vector<betaset> beta, eta;
    int taski=0;

    std::vector<message*> pendingSet_p;
    std::vector<message> pendingSet;
    for (message& m : messageSet) {
        pendingSet_p.push_back(&m);
        pendingSet.push_back(m);
    }

    int pri= messageSet.size();
    for (int i = 0; pri > 0; pri--, i++) {
        //确认messageSet[assign_table[i]]对应的任务是否可行，可行则分配优先级pri并从pendingSet中删除该任务
        //if (feasibility_check(pendingSet,assign_table[i])) {
        //}
        //第一步找到数组中对应任务的位置
        auto it2 = std::find_if(pendingSet.begin(), pendingSet.end(), [&](const message& m) {
            return m.id == assign_table[i];
            });
        if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
            auto it = pendingSet_p.begin() + std::distance(pendingSet.begin(), it2);
            (*it)->priority = pri;
            DEBUG_MSG("任务" , it2->id , "  分配优先级" , pri , "成功");
            //std::cout << "任务" << it2->id << "  分配优先级" << pri << "成功" << std::endl;
            pendingSet_p.erase(it);
            pendingSet.erase(it2);

        }
        else {
            std::cout << "=============================== " << std::endl;
            std::cout << "任务" << it2->id << "  分配优先级" << pri << "失败" << "  分配策略有问题" << std::endl;
            std::cout << "=============================== " << std::endl;
            return false;
        }
    }
    std::cout << "=============================== " << std::endl;
    std::cout << "优先级分配成功！！！ " << std::endl;
    for (const message& m : messageSet) {
        std::cout << "任务: " << m.id << "  优先级： " << m.priority << std::endl;
    }
    std::cout << "=============================== " << std::endl;
    
    return true;
}
bool canMeetDeadlines(const std::vector<message>& messageSet) {
    // 如果消息数量小于 2，则无法产生关键期冲突
    if (messageSet.size() < 2) {
        return true;
    }

    // 遍历消息集合中的所有消息对
    for (size_t i = 0; i < messageSet.size(); ++i) {
        for (size_t j = i + 1; j < messageSet.size(); ++j) {
            // 计算两个消息之间的最大公约数和最小公倍数
            int gcd_val = gcd(messageSet[i].period, messageSet[j].period);
            int lcm_val = lcm(messageSet[i].period, messageSet[j].period);

            // 如果两个消息之间的偏移量小于它们的最大公约数，则存在关键期冲突
            if (abs(messageSet[i].offset - messageSet[j].offset) < gcd_val) {
                return false;
            }

            // 如果两个消息之间的偏移量大于它们的最小公倍数，则不存在关键期冲突
            if (abs(messageSet[i].offset - messageSet[j].offset) > lcm_val) {
                continue;
            }

            // 如果两个消息之间的偏移量大于等于它们的最大公约数且小于等于它们的最小公倍数，则需要进一步检查
            // 这种情况下，需要检查是否存在重复周期
            int time_difference = abs(messageSet[i].offset - messageSet[j].offset);
            if (time_difference % gcd_val == 0) {
                // 如果偏移量是最大公约数的倍数，则存在关键期冲突
                return false;
            }
        }
    }

    // 如果遍历完所有消息对都没有发现关键期冲突，则所有消息都能按时完成
    return true;
}

bool assign_priority(std::vector<message>& messageSet) {
    std::vector<message*> pendingSet_p;
    std::vector<message> pendingSet;
    for (message& m : messageSet) {
        pendingSet_p.push_back(&m);
        pendingSet.push_back(m);
    }
    bool unassigned = true;
    for (int pri = messageSet.size(); pri > 0; pri--) {
        unassigned = true;
        for (auto it = pendingSet_p.begin(); it != pendingSet_p.end(); ++it) {
            auto it2 = pendingSet.begin() + (int)std::distance(pendingSet_p.begin(), it);
            //if (feasibility_check(pendingSet, (int)std::distance(pendingSet_p.begin(), it),pri)) {
            //    (*it)->priority = pri;
            //    pendingSet_p.erase(it); 
            //    pendingSet.erase(it2);
            //    unassigned = false;
            //    break;

            //}
            if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
                (*it)->priority = pri;
                DEBUG_MSG("任务" , it2->id , "  分配优先级" , pri , "成功");
                //std::cout << "任务" << it2->id << "  分配优先级" << pri << "成功" << std::endl;
                pendingSet_p.erase(it);
                pendingSet.erase(it2);
                unassigned = false;
                break;
            }
        }
        if (unassigned) {
            return false;
            std::cout << "=============================== " << std::endl;
            std::cout << "优先级分配失败。。。 " << std::endl;
            std::cout << "=============================== " << std::endl;
        }

    }

    std::cout << "=============================== " << std::endl;
    std::cout << "优先级分配成功！！！ " << std::endl;
    for (const message& m : messageSet) {
        std::cout << "任务: " <<m.id<< "  优先级： "<<m.priority<< std::endl;
    }
    std::cout << "=============================== " << std::endl;
    return true;
}