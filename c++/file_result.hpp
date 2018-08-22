#pragma once

#include <string>
#include <ctime>

struct FileResult{
    std::string filepath;
    std::string hash;
    long size;
    std::time_t timeModified;

    // ctor
    FileResult();

    // Equality operator
    bool operator==(const FileResult& rhs);

    // String representation
    std::string toString();

private:
    const char* dateFormat = "%Y-%m-%d %H:%M:%S";
};