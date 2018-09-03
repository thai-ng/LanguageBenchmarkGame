#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <cryptopp/filters.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include "utils.hpp"
#include "worker.hpp"

namespace fs = boost::filesystem;

// for ease, we'll implement some set operators
inline string_set operator-(const string_set& lhs, const string_set& rhs){
    string_set difference;
    for(auto& entry : lhs){
        if(rhs.find(entry) == rhs.end()){
            // If a suspected conflict is not in unchanged entry, it must be a real conflict
            difference.insert(entry);
        }
    }

    return difference;
}

inline string_set operator&(const string_set& lhs, const string_set& rhs){
    string_set intersection;
    for(auto& entry : lhs){
        if(rhs.find(entry) != rhs.end()){
            // If a suspected conflict is not in unchanged entry, it must be a real conflict
            intersection.insert(entry);
        }
    }
    
    return intersection;
}

Worker::Worker(const checksum_ptr instance) : checksumInstance(instance) {}

void Worker::populateSetWithKeys(std::unordered_set<std::string>& set, scan_result& result){
    for(auto& entry : result){
        set.insert(entry.second->filepath);
    }
}

std::string Worker::hashFile(std::string filepath){
    using namespace CryptoPP;

    CryptoPP::HashTransformation* checksum = (CryptoPP::HashTransformation*)this->checksumInstance->Clone();
    std::string digest;
    FileSource fileSource(filepath.c_str(), true, new HashFilter(*checksum, new HexEncoder(new StringSink(digest))));
    delete checksum;

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

// Actually creates the patch data
std::shared_ptr<patch_result> Worker::createPatchData(
    scan_result& src, string_set& pathsSrc,
    scan_result& target, string_set& pathsTarget,
    string_set& unchanged, string_set& conflicts){
    
    std::shared_ptr<patch_result> retVal = std::shared_ptr<patch_result>(new patch_result());
    patch_result &resultMap = *retVal;
    
    string_set additions = pathsSrc - pathsTarget;
    
    resultMap[ReconcileOperation::ADD].reserve(additions.size());
    for(const auto& addition : additions){
        resultMap[ReconcileOperation::ADD].push_back(src[addition]);
    }

    resultMap[ReconcileOperation::UNCHANGED].reserve(unchanged.size());
    for(const auto& entry : unchanged){
        resultMap[ReconcileOperation::UNCHANGED].push_back(target[entry]);
    }

    resultMap[ReconcileOperation::CONFLICT].reserve(conflicts.size());
    for(const auto& entry : conflicts){
        resultMap[ReconcileOperation::CONFLICT].push_back(target[entry]);
    }

    return retVal;
}

// Asynchronously run scanDirectory
std::future<scan_result> Worker::scanDirectory(std::string path){
    return std::async(std::launch::async, &Worker::scanDirectoryInternal, this, path);
}

// Run the reconcile operation
void Worker::Reconcile(scan_result& resultA, scan_result& resultB, bool keepResult){    
    string_set pathsA, pathsB;
    this->populateSetWithKeys(pathsA, resultA);
    this->populateSetWithKeys(pathsB, resultB);
    
    string_set suspectedConflicts = pathsA & pathsB;

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

    string_set conflicts = suspectedConflicts - unchangedPaths;

    std::shared_ptr<patch_result> patchA = 
        this->createPatchData(
            resultB, pathsB,
            resultA, pathsA,
            unchangedPaths, conflicts);
    std::shared_ptr<patch_result> patchB =
        this->createPatchData(
            resultA, pathsA,
            resultB, pathsB,
            unchangedPaths, conflicts);

    if(keepResult){
        this->lastReconcile = std::shared_ptr<reconcile_result>(new reconcile_result(patchA, patchB));
    }
}

// Write an individual patch result
std::stringstream Worker::WritePatchResult(std::string directory, patch_result_ptr result, bool ignoreUnchanged = false){
    typedef std::pair<char, FileResultPtr> line;
    std::stringstream output;
    
    // Flatten the initial structure
    std::vector<line> lines;
    for(auto& operation_set : *result){
        ReconcileOperation operation = operation_set.first;
        auto& entries = operation_set.second;

        if(operation == ReconcileOperation::UNCHANGED && ignoreUnchanged){
            continue;
        }

        for(auto entry : entries){
            lines.push_back(line((char)operation, entry));
        }
    }

    // Sort it by the filepath
    auto sorter = [](const line& a, const line& b) -> bool 
    {
        return a.second->filepath < b.second->filepath;
    };
    std::sort(lines.begin(), lines.end(), sorter);

    // Write out the lines
    output << directory << std::endl;
    for(const line& entry: lines){
        output << entry.first << " " << entry.second->toString() << std::endl;
    }

    return output;
}

// Write the results to a file
void Worker::WriteResult(std::string dirA, std::string dirB, std::string destination, bool ignoreUnchanged){
    std::fstream outFile (destination, std::fstream::out);

    // Asynchronously format the lines before writing
    auto linesA = std::async(
        std::launch::async, 
        &Worker::WritePatchResult, 
        this, 
        dirA, this->lastReconcile->first, ignoreUnchanged);
    auto linesB = std::async(
        std::launch::async, 
        &Worker::WritePatchResult, 
        this, 
        dirB, this->lastReconcile->second, ignoreUnchanged);

    outFile << "# Results for " << GetFormattedDateTime() << std::endl;
    outFile << "# Reconciled '" << dirA << "' '" << dirB << "'" << std::endl;
    outFile << linesA.get().str() << std::endl;
    outFile << linesB.get().str() << std::endl;

    outFile.close();
}

#undef CRYPTOPP_ENABLE_NAMESPACE_WEAK