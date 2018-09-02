#include "utils.hpp"

std::_Put_time<char> GetFormattedDateTime(){
    auto cTime = std::time(nullptr);
    auto timeFormat = "%Y-%m-%d %H:%M:%S";
    return std::put_time(std::localtime(&cTime), timeFormat);
}