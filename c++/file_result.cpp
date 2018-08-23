#include <limits>
#include <iomanip>
#include <sstream>

#include "file_result.hpp"

// ctor
FileResult::FileResult(){
    this->filepath = "";
    this->hash = "";
    this->size = -1;
    this->timeModified = 0;
}

FileResult::FileResult(std::string path, std::string hash, long filesize, std::time_t mdate)
: filepath(path), hash(hash), size(filesize), timeModified(mdate){ }

// Equality operator
bool FileResult::operator==(const FileResult& rhs){
    return this->size == rhs.size
        && std::difftime(this->timeModified, rhs.timeModified) < std::numeric_limits<double>::epsilon()
        && this->hash == rhs.hash
        && this->filepath == rhs.filepath;
}

// String representation
std::string FileResult::toString(){
    std::ostringstream buff;
    buff << this->filepath 
        << " (" << std::put_time(std::gmtime(&this->timeModified), FileResult::dateFormat)
        << " | " << this->size << " bytes)";
    
    return buff.str();
}