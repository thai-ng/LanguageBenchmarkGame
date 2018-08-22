#include <iostream>

#include <boost/filesystem.hpp>
#include <cryptopp/integer.h>

#include "worker.hpp"

namespace fs = boost::filesystem;

// Internal implementation of Scan Directory
std::shared_ptr<FileResult> Worker::scanDirectoryInternal(std::string path){
    // source: https://stackoverflow.com/questions/18233640/boostfilesystemrecursive-directory-iterator-with-filter

    std::shared_ptr<FileResult> retVal = std::shared_ptr<FileResult>(new FileResult());
    fs::recursive_directory_iterator end, dirWalker(path);
    while(dirWalker != end){
        auto filepath = dirWalker->path().string();

        if(! fs::is_directory(dirWalker->path())){
            std::cout << filepath << std::endl;
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

}

// Write the results to a file
void Worker::WriteResult(ArgumentHolder& args, std::string destination){

}