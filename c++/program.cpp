#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "argument_holder.hpp"

int main(int argc, char** argv){
    ArgumentHolder testy;
    std::cout << testy.Parse(argc, argv) << std::endl;
    std::cout << testy.ChecksumName << std::endl;
}