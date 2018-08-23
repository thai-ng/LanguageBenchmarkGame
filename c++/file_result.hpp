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

    // init ctor
    FileResult(std::string path, std::string hash, long filesize, std::time_t mdate);

    // Equality operator
    bool operator==(const FileResult& rhs);

    // String representation
    std::string toString();

private:
    const char* dateFormat = "%Y-%m-%d %H:%M:%S";
};