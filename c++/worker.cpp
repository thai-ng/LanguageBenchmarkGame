#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <iostream>
#include <boost/filesystem.hpp>
#include <cryptopp/filters.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include "worker.hpp"

namespace fs = boost::filesystem;

Worker::Worker(const checksum_ptr instance) : checksumInstance(instance) {}

std::string Worker::hashFile(std::string filepath){
    using namespace CryptoPP;

    checksum_ptr checksum = checksum_ptr((CryptoPP::HashTransformation*)this->checksumInstance->Clone());
    std::string digest;
    FileSource fileSource(filepath.c_str(), true, new HashFilter(*checksum, new HexEncoder(new StringSink(digest))));

    return digest;
}

// Internal implementation of Scan Directory
std::shared_ptr<FileResult> Worker::scanDirectoryInternal(std::string path){
    // source: https://stackoverflow.com/questions/18233640/boostfilesystemrecursive-directory-iterator-with-filter

    std::shared_ptr<FileResult> retVal = std::shared_ptr<FileResult>(new FileResult());
    fs::recursive_directory_iterator end, dirWalker(path);
    while(dirWalker != end){
        auto filepath = dirWalker->path().string();

        if(! fs::is_directory(dirWalker->path())){
            std::cout << filepath << ": " << this->hashFile(filepath) << std::endl;
            
        }
        
        ++dirWalker;
    }

    return retVal;
}

// Asynchronously run scanDirectory
std::future<std::shared_ptr<FileResult>> Worker::scanDirectory(std::string path){
    return std::async(std::launch::async, &Worker::scanDirectoryInternal, this, path);
}

// Run the reconcile operation
void Worker::Reconcile(FileResult* a, FileResult* b, bool keepResult){
    // TODO!
}

// Write the results to a file
void Worker::WriteResult(std::string dirA, std::string dirB, std::string destination, bool ignoreUnchanged){
    // TODO!
}

#undef CRYPTOPP_ENABLE_NAMESPACE_WEAK