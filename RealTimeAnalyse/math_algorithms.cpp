
#include"math_algorithms.h"
namespace my_algorithm {

    //生成的string最后一个元素的最后用0补足
    std::string boolvec2string(const std::vector<bool>& boolvec) {
        // 创建一个字符串存储字符表示形式
        std::string char_representation;

        // 遍历数据，将每个字节转换为字符，并添加到字符串中
        for (size_t i = 0; i < boolvec.size(); i += 8) {
            char byte = 0;
            for (size_t j = 0; j < 8 && i + j < boolvec.size(); ++j) {
                byte |= (boolvec[i + j] << (7 - j));
            }
            char_representation.push_back(byte);
        }
        //生成的string最后一个元素的最后用0补足
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



    //str为紧密存储的二进制数据，length为二进制位数量
    std::vector<bool> string2boolvec(std::string str,size_t length) {
        std::vector<bool> boolvec;
        auto size = str.size();
        // 将字符串中的每个字符转换为二进制数据，并存储到 std::vector<bool> 中
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