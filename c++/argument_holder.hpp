#pragma once

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

struct ArgumentHolder{
    fs::path DirectoryA;

    fs::path DirectoryB;

    std::string ChecksumName;

    bool ShouldIgnoreUnchanged;

    ArgumentHolder();

    // Parse the arguments given
    bool Parse(int argc, char** argv);
    bool Parse(std::vector<std::string>& );
};