#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <cryptopp/filters.h>
#include <cryptopp/md5.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include "utils.hpp"
#include "worker.hpp"
#include <chrono>

namespace fs = boost::filesystem;

// // for ease, we'll implement some set operators
// inline string_set operator-(const string_set& lhs, const string_set& rhs){
//     string_set difference;
//     for(auto& entry : lhs){
//         if(rhs.find(entry) == rhs.end()){
//             // If a suspected conflict is not in unchanged entry, it must be a real conflict
//             difference.insert(entry);
//         }
//     }

//     return difference;
// }

// inline string_set operator&(const string_set& lhs, const string_set& rhs){
//     string_set intersection;
//     for(auto entry : lhs){
//         if(rhs.find(entry) != rhs.end()){
//             // If a suspected conflict is not in unchanged entry, it must be a real conflict
//             intersection.insert(entry);
//         }
//     }
    
//     return intersection;
// }

Worker::Worker(const checksum_ptr instance) : checksumInstance(instance) {}

std::unordered_set<std::string> Worker::populateSetWithKeys(scan_result const& result){
    std::unordered_set<std::string> set;

    for(auto& entry : result){
        set.insert(entry.second.filepath);
    }

    return set;
}

std::string Worker::hashFile(std::string filepath){
    using namespace CryptoPP;

    auto checksum = CryptoPP::Weak::MD5();
    std::string digest;
    FileSource fileSource(filepath.c_str(), true, new HashFilter(checksum, new HexEncoder(new StringSink(digest))));

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
            
            retVal[shortenedPath] =  FileResult(
                    shortenedPath,
                    this->hashFile(filepath),
                    fs::file_size(filepathInfo),
                    fs::last_write_time(filepathInfo)
            );
        }
        
        ++dirWalker;
    }

    return retVal;
}

// Actually creates the patch data
patch_result Worker::createPatchData(
    scan_result const& src, string_set const& pathsSrc,
    scan_result const& target, string_set const& pathsTarget,
    string_set const& unchanged, string_set const& conflicts){
    
    patch_result resultMap;
    
    std::vector<std::string> additions;
    additions.reserve(pathsSrc.size());
    std::set_difference(std::begin(pathsSrc), std::end(pathsSrc),
                        std::begin(pathsTarget), std::end(pathsTarget),
                        std::back_inserter(additions));
    
    auto& adds = resultMap[ReconcileOperation::ADD];
    adds.reserve(additions.size());
    std::transform(std::begin(additions), std::end(additions), std::back_inserter(adds), [&src](auto& entry) {return src.at(entry);});

    auto& unchangeds = resultMap[ReconcileOperation::UNCHANGED];
    unchangeds.reserve(unchanged.size());
    std::transform(std::begin(unchanged), std::end(unchanged), std::back_inserter(unchangeds), [&target](auto& entry) {return target.at(entry);});
    
    auto& conflicteds = resultMap[ReconcileOperation::CONFLICT];
    conflicteds.reserve(conflicts.size());
    std::transform(std::begin(conflicts), std::end(conflicts), std::back_inserter(conflicteds), [&target](auto& entry) {return target.at(entry);});

    return resultMap;
}

// Asynchronously run scanDirectory
std::future<scan_result> Worker::scanDirectory(std::string path){
    return std::async(std::launch::async, &Worker::scanDirectoryInternal, this, path);
}

// Run the reconcile operation
void Worker::Reconcile(scan_result const& resultA, scan_result const& resultB, bool keepResult){   
    std::vector<std::string> pathsA;
    pathsA.reserve(resultA.size());
    std::transform(std::begin(resultA), std::end(resultA), std::back_inserter(pathsA), [](auto& entry) { return entry.second.filepath; });
    std::sort(std::begin(pathsA), std::end(pathsA));

    std::vector<std::string> pathsB;
    pathsB.reserve(resultB.size());
    std::transform(std::begin(resultB), std::end(resultB), std::back_inserter(pathsB), [](auto& entry) { return entry.second.filepath; });
    std::sort(std::begin(pathsB), std::end(pathsB));

    std::vector<std::string> suspectedConflicts;
    suspectedConflicts.reserve(pathsA.size() + pathsB.size());

    std::set_intersection(std::begin(pathsA), std::end(pathsA),
                          std::begin(pathsB), std::end(pathsB),
                          std::back_inserter(suspectedConflicts));

    std::vector<std::string> unchangedPaths;
    for(auto& entry : suspectedConflicts){
        auto entryA = std::find(std::begin(pathsA), std::end(pathsA), entry);
        auto entryB = std::find(std::begin(pathsB), std::end(pathsB), entry);
        if(entryA == pathsA.end() || entryB == pathsB.end()){
            continue;
        }

        auto& entryInfoA = resultA.at(*entryA);
        auto& entryInfoB = resultB.at(*entryB);

        if(entryInfoA == entryInfoB){
            unchangedPaths.push_back(entry);
        }
    }

    std::vector<std::string> conflicts;
    conflicts.reserve(suspectedConflicts.size());
    std::set_difference(std::begin(suspectedConflicts), std::end(suspectedConflicts),
                        std::begin(unchangedPaths), std::end(unchangedPaths),
                        std::back_inserter(conflicts));

    if(keepResult){
        this->lastReconcile = reconcile_result{
            this->createPatchData(
                resultB, pathsB,
                resultA, pathsA,
                unchangedPaths, conflicts), 
            this->createPatchData(
                resultA, pathsA,
                resultB, pathsB,
                unchangedPaths, conflicts)
        };
    }
}

// Write an individual patch result
std::stringstream Worker::WritePatchResult(std::string directory, patch_result const& result, bool ignoreUnchanged = false) {
    typedef std::pair<char, const FileResult*> line;
    std::stringstream output;
    
    // Flatten the initial structure
    std::vector<line> lines;
    for(const auto& operation_set : result){
        ReconcileOperation operation = operation_set.first;
        auto& entries = operation_set.second;

        if(operation == ReconcileOperation::UNCHANGED && ignoreUnchanged){
            continue;
        }

        for(auto& entry : entries){
            lines.push_back(line{(char)operation, &entry});
        }
    }

    // Sort it by the filepath
    auto sorter = [](const line& a, const line& b) 
    {
        return a.second->filepath < b.second->filepath;
    };
    std::sort(lines.begin(), lines.end(), sorter);

    // Write out the lines
    output << directory << "\n";
    for(const auto& entry: lines){
        output << entry.first << " " << entry.second->toString() << "\n";
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
        dirA, this->lastReconcile.first, ignoreUnchanged);
    auto linesB = std::async(
        std::launch::async, 
        &Worker::WritePatchResult, 
        this, 
        dirB, this->lastReconcile.second, ignoreUnchanged);

    outFile << "# Results for " << GetFormattedDateTime() << "\n";
    outFile << "# Reconciled '" << dirA << "' '" << dirB << "'" << "\n";
    outFile << linesA.get().str() << "\n";
    outFile << linesB.get().str() << "\n";

    outFile.close();
}

#undef CRYPTOPP_ENABLE_NAMESPACE_WEAK