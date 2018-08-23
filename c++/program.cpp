#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>

#include "argument_holder.hpp"
#include "file_result.hpp"
#include "worker.hpp"

void PrintUsage(){
    using namespace std;
    cout << endl;
    cout << "  Usage: program.out [options] <dir_a> <dir_b>" << endl << endl;
    cout << "  C++ implementation of the language benchmarking trial" << endl << endl;
    cout << "  Options:" << endl << endl;
    cout << "    -u, --ignore-unchanged\t Ignore unchanged files in the final output" << endl;
    cout << "    --md5\t\t\t MD5 Hash [Default]" << endl;
    cout << "    --sha1\t\t\t SHA1 Hash" << endl;
    cout << "    --sha256\t\t\t SHA256 Hash" << endl;
}

auto GetTime(){
    auto cTime = std::time(nullptr);
    auto timeFormat = "%Y-%m-%d %H:%M:%S";
    return std::put_time(std::gmtime(&cTime), timeFormat);
}

int main(int argc, char** argv){
    ArgumentHolder args;
    if(!args.Parse(argc, argv)){
        std::cout << "Error parsing arguments!" << std::endl;
        PrintUsage();
        return 1;
    }

    Worker work(args.Checksum);
    std::cout << "Starting diff of "<< args.DirectoryA << " and " << args.DirectoryB << " ("
        << args.Checksum->AlgorithmName() << ")" << std::endl;
    std::cout << "Start time " << GetTime() << std::endl;

    auto promiseA = work.scanDirectory(args.DirectoryA.string());
    auto promiseB = work.scanDirectory(args.DirectoryB.string());
    auto resultA = promiseA.get(), resultB = promiseB.get();
    
    work.Reconcile(resultA, resultB, true);
    work.WriteResult(args.DirectoryA.string(), args.DirectoryB.string(), "reference.patch", args.ShouldIgnoreUnchanged);

    std::cout << std::endl << "End time " << GetTime() << std::endl;
}