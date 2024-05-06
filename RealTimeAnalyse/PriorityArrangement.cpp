
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
bool find_interval(const std::vector<canfd_frame*>& frame_set, std::vector<int>& lower_bound, std::vector<int>& upper_bound) {
    if (frame_set.empty()) return false;
    std::vector<int> omax(frame_set.size());
    std::vector<unsigned long long>temp_p(frame_set.size());
    omax[0] = frame_set[0]->offset; //o_iΪ [0,i]��������offset
    temp_p[0] = frame_set[0]->get_period(); //temp_piΪ[0,i]�������С������

    for (size_t j = 1; j < frame_set.size(); j++) {
        omax[j] = std::max(omax[j - 1], frame_set[j]->offset);
        temp_p[j] = my_algorithm::lcm(temp_p[j - 1], (unsigned long long)frame_set[j]->get_period());
    }
    lower_bound.resize(frame_set.size());
    upper_bound.resize(frame_set.size());
    lower_bound[0] = 0; upper_bound[0] = temp_p[0];
    for (size_t i = 1; i < frame_set.size(); i++) {
        int quotient = ceil(static_cast<double>(omax[i - 1]) / frame_set[i]->get_period());
        lower_bound[i] = static_cast<int>(quotient * frame_set[i]->get_period());
        upper_bound[i] = lower_bound[i] + temp_p[i];
    }
    return true;
}
bool create_beta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int lower_bound, std::vector<betaset>& beta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    upper = lower_bound;
    lower = upper - frame.get_period() + frame.get_deadline();
    /*if (lower >= upper) return false;*/
    //Ĭ��deadline<period

    for (size_t i = 0; i < frame_set.size(); i++) {
        if (frame_set[i] == &frame) continue;
        int offset = offset_trans(frame_set[i]->offset, frame.offset, frame_set[i]->get_period());
        if (offset < upper) {
            for (int j = 0; j * frame_set[i]->get_period() + offset < upper; j++) {
                betaset temp_b(frame_set[i]->get_exec_time(), j * frame_set[i]->get_period() + offset);
                beta.push_back(temp_b);
            }
        }
    }
    std::sort(beta.begin(), beta.end(), [](const betaset& a, const betaset& b) {return a.tr < b.tr; });
    return true;
}


bool  create_eta(const std::vector<canfd_frame*>& frame_set, const canfd_frame& frame, int t, int R, std::vector<betaset>& eta) {
    int c = 0, tr = 0;
    int upper = 0, lower = 0;
    lower = R + t;
    upper = lower + frame.get_deadline();
    for (size_t i = 0; i < frame_set.size(); i++) {
        if (frame_set[i] == &frame) continue;
        int offset = offset_trans(frame_set[i]->offset, frame.offset, frame_set[i]->get_period());
        //tr=O+m*T,Ѱ�ҡ�lower��upper�������п��ܵ�trȡֵ��o����frame_set[i].offset��T��frame_set[i].period��mΪ����
        int m = ceil(((double)lower - offset) / (double)frame_set[i]->get_period());
        if (m * frame_set[i]->get_period() + offset < upper) {
            for (int j = 0; (m + j) * frame_set[i]->get_period() + offset < upper; j++) {
                betaset temp_b(frame_set[i]->get_exec_time(), (m + j) * frame_set[i]->get_period() + offset);
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
bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri, const std::vector<int>& lower, const std::vector<int>& upper) {
    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
   //      
    int t = 0, R = 0, K = 0;
    std::vector<betaset> beta, eta;

    while (t < upper[taski]) {
        create_beta(frame_set, *frame_set[taski], lower[taski], beta);
        R = calc_remain_interf(*frame_set[taski], lower[taski], beta);
        create_eta(frame_set, *frame_set[taski], lower[taski], R, eta);
        K = std::max(0, calc_create_interf(*frame_set[taski], lower[taski], R, eta));
        
        if (frame_set[taski]->get_exec_time() + R + K > frame_set[taski]->get_deadline()) {
            //����i��Զ���ܿ��У���ʹ������Ҳ������
            beta.clear();
            eta.clear();
            DEBUG_MSG("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " > D:", frame_set[taski]->get_deadline());
            DEBUG_MSG("����", frame_set[taski]->get_id(), "  �������ȼ�", pri, "ʧ��");
            return false;
        }
        DEBUG_MSG("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " <= D:", frame_set[taski]->get_deadline());
        beta.clear();
        eta.clear();
        t = t + frame_set[taski]->get_period();
    }


    return true;
}
//���������ȼ�֡����frame_set�У����Ҫ�����Ϊtaski��֡������͵����ȼ���pri�����ܷ����
bool feasibility_check(std::vector<canfd_frame*>& frame_set, int taski, int pri) {
    //TODO ���ܴ���һ�����⣬��ÿ��ѭ�����Ǵ�δ�������ȼ������񼯺���ѡȡһ������ȥ��̽�������ȼ�����ʹ�����񼯺�Խ��ԽС��
    //     beta��eta�Ⱥ��������Ƿ�ֻ�ÿ���δ���伯���أ���������Ǻ�������ʹ��һ���µģ����ϸ��£�ɾ���ѷ������ȼ����񣩵����񼯺�
   //      
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    find_interval(frame_set, lower, upper);
    std::vector<betaset> beta, eta;

    while (t < upper[taski]) {
        create_beta(frame_set, *frame_set[taski], lower[taski], beta);
        R = calc_remain_interf(*frame_set[taski], lower[taski], beta);
        create_eta(frame_set, *frame_set[taski], lower[taski], R, eta);
        K = std::max(0, calc_create_interf(*frame_set[taski], lower[taski], R, eta));
        DEBUG_MSG("exec:", frame_set[taski]->get_exec_time(), "+ R:", R, " + K:", K, " > ", frame_set[taski]->get_deadline());
        if (frame_set[taski]->get_exec_time() + R + K > frame_set[taski]->get_deadline()) {
            //����i��Զ���ܿ��У���ʹ������Ҳ������
            beta.clear();
            eta.clear();
            DEBUG_MSG("����", frame_set[taski]->get_id(), "  �������ȼ�", pri, "ʧ��");
            return false;
        }
        beta.clear();
        eta.clear();
        t = t + frame_set[taski]->get_period();
    }


    return true;
}

bool feasibility_check(std::vector<canfd_frame*>& frame_set, std::vector<int>& assign_table) {
    int t = 0, R = 0, K = 0;
    std::vector<int> lower, upper;
    if (!find_interval(frame_set, lower, upper)) { return false; }
    std::vector<betaset> beta, eta;
    int taski = 0;

    std::vector<canfd_frame*> frame_set_copy(frame_set.begin(), frame_set.end());

    int pri = frame_set.size();
    for (int i = 0; pri > 0; pri--, i++) {
        //ȷ��frame_set[assign_table[i]]��Ӧ�������Ƿ���У�������������ȼ�pri����pendingSet��ɾ��������
        //if (feasibility_check(pendingSet,assign_table[i])) {
        //}
        //��һ���ҵ������ж�Ӧ�����λ��
        auto it2 = std::find_if(frame_set_copy.begin(), frame_set_copy.end(), [&](const canfd_frame* frame) {
            return frame->get_id() == assign_table[i];
            });
        if (it2 != frame_set_copy.end() && feasibility_check(frame_set_copy, std::distance(frame_set_copy.begin(), it2), pri)) {
            /*auto it = frame_set_copy.begin() + std::distance(pendingSet.begin(), it2);*/
            /*(*it)->set_priority(pri);*/
            //���ȼ�Ӧ�����ɹ���ͳһ����
            DEBUG_MSG("����", (*it2)->get_id(), "  �������ȼ�", pri, "�ɹ�");
            frame_set_copy.erase(it2);
            //pendingSet.erase(it2);
        }
        else {
            std::cout << "=============================== " << std::endl;
            std::cout << "����" << (*it2)->get_id() << "  �������ȼ�" << pri << "ʧ��" << "  �������������" << std::endl;
            std::cout << "=============================== " << std::endl;
            return false;
        }
    }
    std::cout << "=============================== " << std::endl;
    std::cout << "���ȼ�����ɹ������� " << std::endl;
    for (size_t i = 0; i < assign_table.size(); i++) {
        frame_set[assign_table[i]]->set_priority(frame_set.size() - i);
        std::cout << "����: " << frame_set[assign_table[i]]->get_id() << "  ���ȼ��� " << frame_set[assign_table[i]]->get_priority() << std::endl;
    }
    std::cout << "=============================== " << std::endl;

    return true;
}
bool assign_priority(std::vector<canfd_frame*>& frame_set) {
    std::vector<canfd_frame*> frame_set_copy(frame_set.begin(), frame_set.end());
    //std::vector<canfd_frame*> copyset;
    std::vector<int> lower, upper;

    bool unassigned = true;
    for (int pri = frame_set.size()-1; pri >= 0; pri--) {
        unassigned = true;
        find_interval(frame_set_copy, lower, upper);
        for (size_t i = 0; i < frame_set_copy.size(); i++) {
            if (feasibility_check(frame_set_copy,i, pri,lower,upper)) {
                auto it = frame_set_copy.begin() + i;
                (*it)->set_priority(pri);
                //copyset.push_back(*it);
                DEBUG_MSG("����", frame_set_copy[i]->get_id(), "  �������ȼ�", pri, "�ɹ�������������������");
                frame_set_copy.erase(it);  //��δ���伯���У�ɾ������ɹ���frmae��Ȼ���Է�����һ�����ȼ�
                unassigned = false;
                break;
            }
        }
        if (unassigned) {
           
            std::cout << "=============================== " << std::endl;
            std::cout << "���ȼ�����ʧ�ܡ����� " << std::endl;
            std::cout << "=============================== " << std::endl;
            return false;
        }
        lower.clear();
        upper.clear();
    }

    std::cout << "=============================== " << std::endl;
    std::cout << "���ȼ�����ɹ������� " << std::endl;
    for (size_t i = 0; i < frame_set.size(); i++) {
        std::cout << "����: " << frame_set[i]->get_id() << "  ���ȼ��� " << frame_set[i]->get_priority() << std::endl;
    }
    std::cout << "=============================== " << std::endl;
    return true;
}



