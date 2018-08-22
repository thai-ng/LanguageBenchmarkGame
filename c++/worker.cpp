#include <iostream>
#include <boost/filesystem.hpp>

#include "worker.hpp"

namespace fs = boost::filesystem;

// Internal implementation of Scan Directory
std::shared_ptr<FileResult> Worker::scanDirectoryInternal(std::string path){
    // source: https://stackoverflow.com/questions/18233640/boostfilesystemrecursive-directory-iterator-with-filter

    fs::recursive_directory_iterator end, dirWalker(path);
    while(dirWalker != end){
        auto filepath = dirWalker->path().string();
        std::cout << filepath << std::endl;
        ++dirWalker;
    }

    return nullptr;
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