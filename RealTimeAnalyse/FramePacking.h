#pragma once
#include"canfd_frame.h"
#include<numeric>
#include<mutex>
#include <thread>
//�������һ��������ʽ�����Ĵ��������max_tryΪһ��֡��װ��message���������
std::vector<int> generate_individual(const std::unordered_set<message*>& message_p_set, size_t max_try=5);
//�����������Ϊnum��population
std::vector<std::vector<int>> initial_population(std::vector<message>& message_set, int num, size_t max_try=5);
class FramePacking
{
	//std::random_device rd;
	//std::mt19937 gen(rd());


	//std::uniform_int_distribution<int>frame_number_dist(1, message_set.size() - 1);
	//int frame_number = message_set.size(); //�ȴ����źŸ�������frame


	//std::vector<int>individuals(message_set.size());
	//for (int i = 0; i < frame_number; i++) {
	//	individuals[i] = i;
	//	//TODO ����������źŵĺ����У�����wctt������
	//}
};
////���ɺܶ��frame_set�����Ѿ�װ�غ���message
//void initial_population(std::vector<message>& message_set) {
//
//
//
//
//	std::unordered_set<message*> message_p_set;
//	message_p_set.reserve(message_set.size());
//	for (message& m : message_set) {
//		message_p_set.insert(&m);
//	}
//
//
//	//TODO �ȴ���һ��canfd_frame�����ѡȡ����Ԫ��,��canfd_frame����ӿ���Ԫ�أ���ೢ��max_try�Ρ��ظ��ò���ֱ��message�����ꡣ
//	//canfd_frame��vector����ʽ�洢����ΪҪ���ɴ�СΪn����Ⱥ��������n�ַ��䷽������Ҫ����n��vector<canfd_frame>,
//	//ÿ�������������ɷ�����ά��һ��std::unordered_set<message*> message_p_set;��ÿ����һ��message�ʹ�message_p_set��ɾ��message
//	//Ϊ�˷����������Ȳ�������Ҫ��ÿ�����䷽��ά��һ��individuals���飬���б�ʾindex��message���䵽��individuals[i]��frame�ϣ�
//
//
//
//
//}

