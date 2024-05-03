
#include"math_algorithms.h"
namespace my_algorithm {

    //���ɵ�string���һ��Ԫ�ص������0����
    std::string boolvec2string(const std::vector<bool>& boolvec) {
        // ����һ���ַ����洢�ַ���ʾ��ʽ
        std::string char_representation;

        // �������ݣ���ÿ���ֽ�ת��Ϊ�ַ�������ӵ��ַ�����
        for (size_t i = 0; i < boolvec.size(); i += 8) {
            char byte = 0;
            for (size_t j = 0; j < 8 && i + j < boolvec.size(); ++j) {
                byte |= (boolvec[i + j] << (7 - j));
            }
            char_representation.push_back(byte);
        }
        //���ɵ�string���һ��Ԫ�ص������0����
        return char_representation;
    }

    std::string boolvec_to_string01(const std::vector<bool>& boolvec) {
        std::string str;
        str.reserve(boolvec.size());
        for (bool b : boolvec) {
            str.push_back(b ? '1' : '0');
        }
        return str;
    }
    std::vector<bool> string01_to_boolvec(const std::string& string01) {
        std::vector<bool> boolvec;
        boolvec.reserve(string01.size());
        for (char c : string01) {
            if (c == '0') {
                boolvec.push_back(false);
            }
            else if (c == '1') {
                boolvec.push_back(true);
            }
        }
        return boolvec;
    }



    //strΪ���ܴ洢�Ķ��������ݣ�lengthΪ������λ����
    std::vector<bool> string2boolvec(std::string str,size_t length) {
        std::vector<bool> boolvec;
        auto size = str.size();
        // ���ַ����е�ÿ���ַ�ת��Ϊ���������ݣ����洢�� std::vector<bool> ��
        for (size_t index = 0; index < size - 1; index++) {
            for (int i = 7; i >= 0; --i) {
                boolvec.push_back(((char)str[index] >> i) & 1);
            }
        }
        char last_char = (char)str[size-1];

        for (int j = 7; j >= (size*8-length); --j) {
            boolvec.push_back((last_char >> j) & 1);
        }

        return boolvec;
    }
}