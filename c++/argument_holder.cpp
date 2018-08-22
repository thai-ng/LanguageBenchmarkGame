#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <cryptopp/cryptlib.h>
#include <cryptopp/adler32.h>
#include <cryptopp/crc.h>
#include <cryptopp/md5.h>
#include <cryptopp/sha.h>

#include "argument_holder.hpp"

ArgumentHolder::ArgumentHolder(){
    this->DirectoryA = "";
    this->DirectoryB = "";
    this->Checksum = checksum_ptr(new CryptoPP::Weak::MD5());
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
    std::string checksumName;
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

            checksumName = (*entry).substr(2);
            hasHashOption = true;
        }
    }

    this->setHash(checksumName);
    return true;
}

void ArgumentHolder::setHash(std::string hashName){
    // Not very elegant, but simple
    if(hashName == "md5"){
        // Default option
        return;
    }
    else if(hashName == "crc"){
        this->Checksum = checksum_ptr( new CryptoPP::CRC32());
    }
    else if(hashName == "adler32"){
        this->Checksum = checksum_ptr( new CryptoPP::Adler32());
    }
    else if(hashName == "sha1"){
        this->Checksum = checksum_ptr( new CryptoPP::SHA1());
    }
    else if(hashName == "sha256"){
        this->Checksum = checksum_ptr( new CryptoPP::SHA256());
    }
}

#undef CRYPTOPP_ENABLE_NAMESPACE_WEAK