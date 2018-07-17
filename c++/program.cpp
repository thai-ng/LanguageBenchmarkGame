#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv){
    std::vector<std::string> arguments;
    auto loadArguments = [&](const char* c_str){ arguments.push_back(std::string(c_str)); };
    
    // ignore the 1st argument since it's the program name
    std::for_each(&argv[1], &argv[argc], loadArguments);
}