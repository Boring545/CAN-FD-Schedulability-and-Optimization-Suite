
#include "PriorityArrangement.h"
int offset_trans(int target, int basis, int T) {
    if (target < basis) {
        target = ceil(((double)basis - target) / T) * T + target;
    }
    return target - basis;
}
//���frame_set���֡�����Ƿ���ڹؼ��ڣ��ؼ���ʱ��ͨ��first_instant����
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
bool find_interval(const std::vector<canfd_frame>& frame_set, std::vector<int>& lower_bound, std::vector<int>& upper_bound) {
    if (frame_set.empty()) return false;
    std::vector<int> omax(frame_set.size()), temp_p(frame_set.size());
    omax[0] = frame_set[0].offset; //o_iΪ [0,i]��������offset
    temp_p[0] = frame_set[0].get_period(); //temp_piΪ[0,i]�������С������

    for (size_t j = 1; j < frame_set.size(); j++) {
        omax[j] = std::max(omax[j - 1], frame_set[j].offset);
        temp_p[j] = my_algorithm::lcm(temp_p[j - 1], frame_set[j].get_period());
    }
    lower_bound.resize(frame_set.size());
    upper_bound.resize(frame_set.size());
    lower_bound[0] = 0; upper_bound[0] = temp_p[0];
    for (size_t i = 1; i < frame_set.size(); i++) {
        int quotient = ceil(static_cast<double>(omax[i - 1]) / frame_set[i].get_period());
        lower_bound[i] = static_cast<int>(quotient * frame_set[i].get_period());
        upper_bound[i] = lower_bound[i] + temp_p[i];
    }
    return true;
}
bool create_beta(const std::vector<canfd_frame>& frame_set, const canfd_frame& frame, int lower_bound, std::vector<betaset>& beta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    upper = lower_bound;
    lower = upper - frame.get_period() + frame.get_deadline();
    /*if (lower >= upper) return false;*/
    //Ĭ��deadline<period

    for (size_t i = 0; i < frame_set.size(); i++) {
        if (&(frame_set[i]) == &frame) continue;
        int offset = offset_trans(frame_set[i].offset, frame.offset, frame_set[i].get_period());
        if (offset < upper) {
            for (int j = 0; j * frame_set[i].get_period() + offset < upper; j++) {
                betaset temp_b(frame_set[i].get_exec_time(), j * frame_set[i].get_period() + offset);
                beta.push_back(temp_b);
            }
        }
    }
    std::sort(beta.begin(), beta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}


bool  create_eta(const std::vector<canfd_frame>& frame_set, const canfd_frame& frame, int t, int R, std::vector<betaset>& eta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    lower = R + t;
    upper = lower + frame.get_deadline();
    for (size_t i = 0; i < frame_set.size(); i++) {
        if (&(frame_set[i]) == &frame) continue;
        int offset = offset_trans(frame_set[i].offset, frame.offset, frame_set[i].get_period());
        //tr=O+m*T,Ѱ�ҡ�lower��upper�������п��ܵ�trȡֵ��o����frame_set[i].offset��T��frame_set[i].period��mΪ����
        int m = ceil(((double)lower - offset) / (double)frame_set[i].get_period());
        if (m * frame_set[i].get_period() + offset < upper) {
            for (int j = 0; (m + j) * frame_set[i].get_period() + offset < upper; j++) {
                betaset temp_b(frame_set[i].get_exec_time(), (m + j) * frame_set[i].get_period() + offset);
                eta.push_back(temp_b);
            }
        }
    }
    std::sort(eta.begin(), eta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}

int calc_remain_interf(const canfd_frame& frame, int t, std::vector<betaset>& beta) {
    int R = 0;
    int time = t - frame.get_period() + frame.get_deadline();
    time = 0;
    for (betaset b : beta) {
        //timeΪ֮ǰһ������m������ʱ�䣬��Ϊ���������ܱ�����m��ִ�У�
        //RΪ�Ѿ����ǵ������ִ�е�ʱ��ͣ�R+time<tr��ʾ֮ǰ�������Դ�release��
        //��һ��ִ����ϣ���һ��������Ҫ�ٿ�����ǰ�������ˡ�
        if (b.tr >= time + R) {
            R = 0;
            time = b.tr;
        }
        int temp = b.tr - time;
        R = R + b.C;
    }
    R = R + time - t;
    if (R < 0) {
        R = 0;
    }
    return R;
}

int calc_create_interf(const canfd_frame& frame, const int t, const int R, const std::vector<betaset>& eta) {
    int next_free = R + t;
    int K = 0;
    int total_created = R;
    for (betaset e : eta) {
        total_created += e.C;
        if (next_free < e.tr) { next_free = e.tr; }
        K = K + std::min(t + frame.get_deadline() - next_free, e.C);
        next_free = std::min(t + frame.get_deadline(), next_free + e.C);
    }

    return K;
}
//���������ȼ�֡����frame_set�У����Ҫ�����Ϊtaski��֡������͵����ȼ���pri�����ܷ����
bool feasibility_check(std::vector<canfd_frame>& frame_set, int taski, int pri) {
    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
   //      
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    find_interval(frame_set, lower, upper);
    std::vector<betaset> beta, eta;

    create_beta(frame_set, frame_set[taski], lower[taski], beta);
    R = calc_remain_interf(frame_set[taski], lower[taski], beta);
    create_eta(frame_set, frame_set[taski], lower[taski], R, eta);
    K = calc_create_interf(frame_set[taski], lower[taski], R, eta);
    DEBUG_MSG("R:", R, "  K:", K);
    //std::cout << "R:" << R << "  K:" << K << std::endl;
    if (frame_set[taski].get_exec_time() + R + K > frame_set[taski].get_deadline()) {
        //����i��Զ���ܿ��У���ʹ������Ҳ������
        beta.clear();
        eta.clear();
        DEBUG_MSG("����", taski, "  �������ȼ�", pri, "ʧ��");
        //std::cout << "����" << taski << "  �������ȼ�" << pri << "ʧ��" << std::endl;
        return false;
    }
    beta.clear();
    eta.clear();

    return true;
}

bool feasibility_check(std::vector<canfd_frame>& frame_set, std::vector<int>& assign_table) {
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    if (find_interval(frame_set, lower, upper)) { return false; }
    std::vector<betaset> beta, eta;
    int taski = 0;

    std::vector<canfd_frame*> pendingSet_p;
    std::vector<canfd_frame> pendingSet;
    for (canfd_frame& frame : frame_set) {
        pendingSet_p.push_back(&frame);
        pendingSet.push_back(frame);
    }

    int pri = frame_set.size();
    for (int i = 0; pri > 0; pri--, i++) {
        //ȷ��frame_set[assign_table[i]]��Ӧ�������Ƿ���У�������������ȼ�pri����pendingSet��ɾ��������
        //if (feasibility_check(pendingSet,assign_table[i])) {
        //}
        //��һ���ҵ������ж�Ӧ�����λ��
        auto it2 = std::find_if(pendingSet.begin(), pendingSet.end(), [&](const canfd_frame& frame) {
            return frame.get_id() == assign_table[i];
            });
        if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
            auto it = pendingSet_p.begin() + std::distance(pendingSet.begin(), it2);
            /*(*it)->set_priority(pri);*/
            //���ȼ�Ӧ�����ɹ���ͳһ����
            DEBUG_MSG("����", it2->get_id(), "  �������ȼ�", pri, "�ɹ�");
            pendingSet_p.erase(it);
            pendingSet.erase(it2);

        }
        else {
            std::cout << "=============================== " << std::endl;
            std::cout << "����" << it2->get_id() << "  �������ȼ�" << pri << "ʧ��" << "  �������������" << std::endl;
            std::cout << "=============================== " << std::endl;
            return false;
        }
    }
    std::cout << "=============================== " << std::endl;
    std::cout << "���ȼ�����ɹ������� " << std::endl;
    for (size_t i = 0; i < assign_table.size(); i++) {
        frame_set[assign_table[i]].set_priority(frame_set.size() - i);
        std::cout << "����: " << frame_set[assign_table[i]].get_id() << "  ���ȼ��� " << frame_set[assign_table[i]].get_priority() << std::endl;
    }
    std::cout << "=============================== " << std::endl;

    return true;
}
bool assign_priority(std::vector<canfd_frame>& frame_set) {
    std::vector<canfd_frame*> pendingSet_p;
    std::vector<canfd_frame> pendingSet;
    for (canfd_frame& frame : frame_set) {
        pendingSet_p.push_back(&frame);
        pendingSet.push_back(frame);
    }
    bool unassigned = true;
    for (int pri = frame_set.size(); pri > 0; pri--) {
        unassigned = true;
        for (auto it = pendingSet_p.begin(); it != pendingSet_p.end(); ++it) {
            auto it2 = pendingSet.begin() + (int)std::distance(pendingSet_p.begin(), it);

            if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
                (*it)->set_priority(pri);
                DEBUG_MSG("����", it2->get_id(), "  �������ȼ�", pri, "�ɹ�");
                pendingSet_p.erase(it);
                pendingSet.erase(it2);
                unassigned = false;
                break;
            }
        }
        if (unassigned) {
            return false;
            std::cout << "=============================== " << std::endl;
            std::cout << "���ȼ�����ʧ�ܡ����� " << std::endl;
            std::cout << "=============================== " << std::endl;
        }

    }

    std::cout << "=============================== " << std::endl;
    std::cout << "���ȼ�����ɹ������� " << std::endl;
    for (const canfd_frame& frame : frame_set) {
        std::cout << "����: " << frame.get_id() << "  ���ȼ��� " << frame.get_priority() << std::endl;
    }
    std::cout << "=============================== " << std::endl;
    return true;
}












//bool critical_check(const std::vector<message>& mset, int& first_instant) {
//    int temp_period = 0, temp_gcd = 0;
//    int x = 0, y = 0, h = 0, k = 0, t = 0;
//    message temp_m = mset[0];
//    if (mset.size() < 2) return false;
//    for (size_t i = 1; i < mset.size(); i++) {
//        temp_gcd = extended_gcd(temp_m.period, mset[i].period, x, y);
//        temp_period = temp_m.period * mset[i].period / temp_gcd;
//        if (abs(temp_m.offset - mset[i].offset) % temp_gcd == 0) {
//            h = abs(temp_m.offset - mset[i].offset) / temp_gcd;
//            k = static_cast<int>(ceil((static_cast<double>(abs(x)) * h * temp_m.period) / temp_period));
//            t = (h * x) * temp_m.period + k * temp_period;
//            temp_m.offset = t - static_cast<int>(floor(t / temp_period)) * temp_period;
//            temp_m.period = temp_period;
//        }
//        else {
//            return false;
//        }
//    }
//    first_instant = t;
//    return true;
//}
//
//bool find_interval(const std::vector<message>& messageSet, std::vector<int>& lower_bound, std::vector<int>& upper_bound) {
//    if (messageSet.empty()) return false;
//    std::vector<int> omax(messageSet.size()),temp_p(messageSet.size());
//    omax[0]= messageSet[0].offset; //o_iΪ [0,i]��������offset
//    temp_p[0]= messageSet[0].period; //temp_piΪ[0,i]�������С������
//
//    for (size_t j = 1; j < messageSet.size(); j++) {
//        omax[j] = std::max(omax[j-1], messageSet[j].offset);
//        temp_p[j] = lcm(temp_p[j - 1], messageSet[j].period);
//    }
//    lower_bound.resize(messageSet.size());
//    upper_bound.resize(messageSet.size());
//    lower_bound[0] = 0; upper_bound[0] = temp_p[0];
//    for (size_t i = 1; i < messageSet.size(); i++) {
//        int quotient = ceil(static_cast<double>(omax[i - 1]) / messageSet[i].period);
//        lower_bound[i] = static_cast<int>(quotient * messageSet[i].period);
//        upper_bound[i] = lower_bound[i] + temp_p[i];
//    }
//    return true;
//}
//bool create_beta(const std::vector<message>& messageSet,const message& m, int lower_bound, std::vector<betaset>& beta) {
//    int c = 0, tr = 0;
//    int upper = 0, lower = 0;
//    upper = lower_bound;
//    lower = upper - m.period + m.deadline;
//    /*if (lower >= upper) return false;*/
//    //Ĭ��deadline<period
//
//    for (size_t i = 0; i < messageSet.size(); i++) {
//        if (&(messageSet[i]) == &m) continue;
//        int offset = offset_trans(messageSet[i].offset, m.offset, messageSet[i].period);
//        if (offset < upper) {
//            for (int j = 0; j * messageSet[i].period + offset < upper; j++) {
//                betaset temp_b(messageSet[i].exec_time, j * messageSet[i].period + offset);
//                beta.push_back(temp_b);
//            }
//        }
//    }
//    std::sort(beta.begin(), beta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
//    return true;
//}
//
//
//bool  create_eta(const std::vector<message>& messageSet, const message& m, int t, int R, std::vector<betaset>& eta) {
//    int c = 0, tr = 0;
//    int upper = 0, lower = 0;
//    lower = R + t; 
//    upper = lower + m.deadline;
//    for (size_t i = 0; i < messageSet.size(); i++) {
//        if (&(messageSet[i]) == &m) continue;
//        int offset = offset_trans(messageSet[i].offset, m.offset, messageSet[i].period);
//        //tr=O+m*T,Ѱ�ҡ�lower��upper�������п��ܵ�trȡֵ��o����messageSet[i].offset��T��messageSet[i].period��mΪ����
//        int m = ceil(((double)lower - offset) / (double)messageSet[i].period);
//        if (m * messageSet[i].period + offset < upper) {
//            for (int j = 0; (m + j) * messageSet[i].period + offset < upper; j++) {
//                betaset temp_b(messageSet[i].exec_time, (m + j) * messageSet[i].period + offset);
//                eta.push_back(temp_b);
//            }
//        }
//    }
//    std::sort(eta.begin(), eta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
//    return true;
//}
//int calc_remain_interf(const message& m,int t, std::vector<betaset>& beta) {
//    int R = 0;
//    int time= t - m.period + m.deadline;
//    time = 0;
//    for (betaset b : beta) {
//        //timeΪ֮ǰһ������m������ʱ�䣬��Ϊ���������ܱ�����m��ִ�У�
//        //RΪ�Ѿ����ǵ������ִ�е�ʱ��ͣ�R+time<tr��ʾ֮ǰ�������Դ�release��
//        //��һ��ִ����ϣ���һ��������Ҫ�ٿ�����ǰ�������ˡ�
//        if (b.tr >= time + R) {
//            R = 0;
//            time = b.tr;
//        }
//        int temp =  b.tr- time;
//        R =R+ b.C;
//    }
//    R = R +time-t;
//    if (R < 0) {
//        R = 0;
//    }
//    return R;
//}
//
//int calc_create_interf(const message& m, const int t, const int R, const std::vector<betaset>& eta) {
//    int next_free = R + t;
//    int K = 0;
//    int total_created = R;
//    for (betaset e : eta) {
//        total_created += e.C;
//        if (next_free < e.tr) { next_free = e.tr; }
//        K = K + std::min(t + m.deadline - next_free, e.C);
//        next_free = std::min(t + m.deadline, next_free + e.C);
//    }
//
//    return K;
//}
//
//bool feasibility_check(std::vector<message>& messageSet) {
//    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
//    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
//   //      
//    int t = 0, R = 0, K = 0;
//    std::vector<int> lower, upper;
//    find_interval(messageSet, lower, upper);
//    std::vector<betaset> beta, eta;
//    for (size_t i = 0; i < messageSet.size(); i++) {
//        t = 0;
//        while (t < upper[i]) {
//            create_beta(messageSet, messageSet[i], lower[i], beta);
//            R = calc_remain_interf(messageSet[i], lower[i], beta);
//            create_eta(messageSet, messageSet[i], lower[i], R, eta);
//            K = calc_create_interf(messageSet[i], lower[i], R, eta);
//            if (messageSet[i].exec_time + R + K > messageSet[i].deadline) {
//                //����i��Զ���ܿ��У���ʹ������Ҳ������
//                beta.clear();
//                eta.clear();
//                return false;
//            }
//            t = t + messageSet[i].period;
//            beta.clear();
//            eta.clear();
//        }
//    }
//    return true;
//}
//bool feasibility_check(std::vector<message>& messageSet, int taski,int pri) {
//    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
//    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
//   //      
//    int t = 0, R = 0, K = 0;
//    std::vector<int> lower, upper;
//    find_interval(messageSet, lower, upper);
//    std::vector<betaset> beta, eta;
//
//    create_beta(messageSet, messageSet[taski], lower[taski], beta);
//    R = calc_remain_interf(messageSet[taski], lower[taski], beta);
//    create_eta(messageSet, messageSet[taski], lower[taski], R, eta);
//    K = calc_create_interf(messageSet[taski], lower[taski], R, eta);
//    DEBUG_MSG("R:", R, "  K:", K);
//    //std::cout << "R:" << R << "  K:" << K << std::endl;
//    if (messageSet[taski].exec_time + R + K > messageSet[taski].deadline) {
//        //����i��Զ���ܿ��У���ʹ������Ҳ������
//        beta.clear();
//        eta.clear();
//        DEBUG_MSG("����" , taski , "  �������ȼ�" , pri , "ʧ��");
//        //std::cout << "����" << taski << "  �������ȼ�" << pri << "ʧ��" << std::endl;
//        return false;
//    }
//    beta.clear();
//    eta.clear();
//   
//    return true;
//}
//bool feasibility_check(std::vector<message>& messageSet, std::vector<int>&assign_table) {
//    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
//    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
//   //      
//    int t = 0, R = 0, K = 0;
//    std::vector<int> lower, upper;
//    find_interval(messageSet, lower, upper);
//    std::vector<betaset> beta, eta;
//    int taski=0;
//
//    std::vector<message*> pendingSet_p;
//    std::vector<message> pendingSet;
//    for (message& m : messageSet) {
//        pendingSet_p.push_back(&m);
//        pendingSet.push_back(m);
//    }
//
//    int pri= messageSet.size();
//    for (int i = 0; pri > 0; pri--, i++) {
//        //ȷ��messageSet[assign_table[i]]��Ӧ�������Ƿ���У�������������ȼ�pri����pendingSet��ɾ��������
//        //if (feasibility_check(pendingSet,assign_table[i])) {
//        //}
//        //��һ���ҵ������ж�Ӧ�����λ��
//        auto it2 = std::find_if(pendingSet.begin(), pendingSet.end(), [&](const message& m) {
//            return m.id == assign_table[i];
//            });
//        if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
//            auto it = pendingSet_p.begin() + std::distance(pendingSet.begin(), it2);
//            (*it)->priority = pri;
//            DEBUG_MSG("����" , it2->id , "  �������ȼ�" , pri , "�ɹ�");
//            pendingSet_p.erase(it);
//            pendingSet.erase(it2);
//
//        }
//        else {
//            std::cout << "=============================== " << std::endl;
//            std::cout << "����" << it2->id << "  �������ȼ�" << pri << "ʧ��" << "  �������������" << std::endl;
//            std::cout << "=============================== " << std::endl;
//            return false;
//        }
//    }
//    std::cout << "=============================== " << std::endl;
//    std::cout << "���ȼ�����ɹ������� " << std::endl;
//    for (const message& m : messageSet) {
//        std::cout << "����: " << m.id << "  ���ȼ��� " << m.priority << std::endl;
//    }
//    std::cout << "=============================== " << std::endl;
//    
//    return true;
//}
//
//
//bool assign_priority(std::vector<message>& messageSet) {
//    std::vector<message*> pendingSet_p;
//    std::vector<message> pendingSet;
//    for (message& m : messageSet) {
//        pendingSet_p.push_back(&m);
//        pendingSet.push_back(m);
//    }
//    bool unassigned = true;
//    for (int pri = messageSet.size(); pri > 0; pri--) {
//        unassigned = true;
//        for (auto it = pendingSet_p.begin(); it != pendingSet_p.end(); ++it) {
//            auto it2 = pendingSet.begin() + (int)std::distance(pendingSet_p.begin(), it);
//            //if (feasibility_check(pendingSet, (int)std::distance(pendingSet_p.begin(), it),pri)) {
//            //    (*it)->priority = pri;
//            //    pendingSet_p.erase(it); 
//            //    pendingSet.erase(it2);
//            //    unassigned = false;
//            //    break;
//
//            //}
//            if (it2 != pendingSet.end() && feasibility_check(pendingSet, std::distance(pendingSet.begin(), it2), pri)) {
//                (*it)->priority = pri;
//                DEBUG_MSG("����" , it2->id , "  �������ȼ�" , pri , "�ɹ�");
//                //std::cout << "����" << it2->id << "  �������ȼ�" << pri << "�ɹ�" << std::endl;
//                pendingSet_p.erase(it);
//                pendingSet.erase(it2);
//                unassigned = false;
//                break;
//            }
//        }
//        if (unassigned) {
//            return false;
//            std::cout << "=============================== " << std::endl;
//            std::cout << "���ȼ�����ʧ�ܡ����� " << std::endl;
//            std::cout << "=============================== " << std::endl;
//        }
//
//    }
//
//    std::cout << "=============================== " << std::endl;
//    std::cout << "���ȼ�����ɹ������� " << std::endl;
//    for (const message& m : messageSet) {
//        std::cout << "����: " <<m.id<< "  ���ȼ��� "<<m.priority<< std::endl;
//    }
//    std::cout << "=============================== " << std::endl;
//    return true;
//}