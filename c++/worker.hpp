#pragma once

#include <boost/filesystem.hpp>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <tuple>

#include "argument_holder.hpp"
#include "file_result.hpp"

typedef std::unordered_map<std::string, std::vector<FileResult>> patch_result;

typedef std::tuple<patch_result, patch_result> reconcile_result;

namespace fs = boost::filesystem;

class Worker{
private:
    // Result of the last reconcile operation if it was saved
    std::shared_ptr<reconcile_result> lastReconcile;

    // Internal implementation of Scan Directory
    std::shared_ptr<FileResult> scanDirectoryInternal(std::string path);

    // Hashes 
    std::string hashFile(std::string filepath);

public:
    // Asynchronously run scanDirectory
    std::future<std::shared_ptr<FileResult>> scanDirectory(std::string path);

    // Run the reconcile operation
    void Reconcile(FileResult* a, FileResult* b, bool keepResult);

    // Write the results to a file
    void WriteResult(ArgumentHolder& args, std::string destination);
};