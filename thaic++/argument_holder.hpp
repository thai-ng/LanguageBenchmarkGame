#pragma once

#include <cryptopp/cryptlib.h>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

typedef std::shared_ptr<CryptoPP::HashTransformation> checksum_ptr;

struct ArgumentHolder{
    fs::path DirectoryA;

    fs::path DirectoryB;

    checksum_ptr Checksum;

    bool ShouldIgnoreUnchanged;

    ArgumentHolder();

    // Parse the arguments given
    bool Parse(int argc, char** argv);
    bool Parse(std::vector<std::string>& );

private:
    void setHash(std::string hashName);
};