#include "canfd_frame.h"

int canfd_frame::max_data_size = 64;

canfd_frame::canfd_frame(std::string identifier, CAN_Frame_Type type, int data_size) 
    : identifier(identifier), type(type), data_size(0) {}

bool canfd_frame::add_data(message &m) {
    if (max_data_size - data_size < m.data_size) {
        return false;
    } else {
        data_size += m.data_size;
        message_list.push_back(m);
        return true;
    }
}
