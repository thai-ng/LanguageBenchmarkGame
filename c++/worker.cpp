#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <cryptopp/filters.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include "worker.hpp"

namespace fs = boost::filesystem;

Worker::Worker(const checksum_ptr instance) : checksumInstance(instance) {}

void Worker::populateSetWithKeys(std::unordered_set<std::string>& set, scan_result& result){
    for(auto& entry : result){
        set.insert(entry.second->filepath);
    }
}

std::string Worker::hashFile(std::string filepath){
    using namespace CryptoPP;

    checksum_ptr checksum = checksum_ptr((CryptoPP::HashTransformation*)this->checksumInstance->Clone());
    std::string digest;
    FileSource fileSource(filepath.c_str(), true, new HashFilter(*checksum, new HexEncoder(new StringSink(digest))));

    return digest;
}

// Internal implementation of Scan Directory
scan_result Worker::scanDirectoryInternal(std::string path){
    // source: https://stackoverflow.com/questions/18233640/boostfilesystemrecursive-directory-iterator-with-filter

    // Paths comes in as "/a", so the cut index accounts for the leftmost separator removal with +1
    int cutIndex = path.length() + 1;
    scan_result retVal;
    fs::recursive_directory_iterator end, dirWalker(path);
    
    while(dirWalker != end){
        auto filepathInfo = dirWalker->path();
        auto filepath = filepathInfo.string();

        if(! fs::is_directory(filepathInfo)){
            // Build a new result with a path that does NOT include the original path being scanned
            std::string shortenedPath = filepath.substr(cutIndex, filepath.length() - cutIndex);
            auto result = std::shared_ptr<FileResult>(
                new FileResult(
                    shortenedPath,
                    this->hashFile(filepath),
                    fs::file_size(filepathInfo),
                    fs::last_write_time(filepathInfo)
                ));
            
            retVal[shortenedPath] = result;
        }
        
        ++dirWalker;
    }

    return retVal;
}

// Asynchronously run scanDirectory
std::future<scan_result> Worker::scanDirectory(std::string path){
    return std::async(std::launch::async, &Worker::scanDirectoryInternal, this, path);
}

// Run the reconcile operation
void Worker::Reconcile(scan_result& resultA, scan_result& resultB, bool keepResult){
    typedef std::unordered_set<std::string> string_set;
    
    string_set pathsA, pathsB;
    this->populateSetWithKeys(pathsA, resultA);
    this->populateSetWithKeys(pathsB, resultB);
    
    string_set suspectedConflicts;
    for(auto& entryA : pathsA){
        if(pathsB.find(entryA) != pathsB.end()){
            suspectedConflicts.insert(entryA);
        }
    }

    string_set unchangedPaths;
    for(auto& entry : suspectedConflicts){
        string_set::iterator entryA = pathsA.find(entry), entryB = pathsB.find(entry);
        if(entryA == pathsA.end() || entryB == pathsB.end()){
            continue;
        }

        FileResult& entryInfoA = *resultA[*entryA];
        FileResult& entryInfoB = *resultB[*entryB];

        if(entryInfoA == entryInfoB){
            unchangedPaths.insert(entry);
        }
    }

    string_set conflicts;
    //TODO:

    std::cout << "It worked!" << std::endl;
}

// Write the results to a file
void Worker::WriteResult(std::string dirA, std::string dirB, std::string destination, bool ignoreUnchanged){
    // TODO!
}

#undef CRYPTOPP_ENABLE_NAMESPACE_WEAK