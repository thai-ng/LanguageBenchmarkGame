#include <iostream>
#include <algorithm>
#include <unordered_set>
#include "argument_holder.hpp"

ArgumentHolder::ArgumentHolder(){
    this->DirectoryA = "";
    this->DirectoryB = "";
    this->ChecksumName = "md5";
    this->ShouldIgnoreUnchanged = false;
}

bool ArgumentHolder::Parse(int argc, char** argv){
    if(argc < 3){
        // Not enough arguments
        return false;
    }

    std::vector<std::string> arguments;
    auto loadArguments = [&](const char* c_str){ arguments.push_back(std::string(c_str)); };
    
    // ignore the 1st argument since it's the program name
    std::for_each(&argv[1], &argv[argc], loadArguments);
    
    return this->Parse(arguments);
}

bool ArgumentHolder::Parse(std::vector<std::string>& args){
    fs::path pathA(args[0]), pathB(args[1]);
    
    this->DirectoryA = pathA.normalize();
    this->DirectoryB = pathB.normalize();

    std::unordered_set<std::string> hashOptions = {"--md5", "--crc", "--adler32", "--sha1", "--sha256"};
    bool hasHashOption = false;
    for(unsigned int i=2; i < args.size() ; i++){
        std::string& arg = args[i];

        // Check for ignore unchanged files flag
        if(arg == "--ignore-unchanged" || arg == "-u"){
            this->ShouldIgnoreUnchanged = true;
        }

        // Check for a hash selection
        auto entry = hashOptions.find(arg);
        if(entry != hashOptions.end()){
            if(hasHashOption){
                // error out
                return false;
            }

            this->ChecksumName = (*entry).substr(2);
            hasHashOption = true;
        }
    }

    return true;
}
