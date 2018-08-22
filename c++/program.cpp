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
    return std::put_time(std::gmtime(&cTime), "%c %Z");
}

int main(int argc, char** argv){
    ArgumentHolder args;
    if(!args.Parse(argc, argv)){
        std::cout << "Error parsing arguments!" << std::endl;
        PrintUsage();
        return 1;
    }

    Worker work;
    std::cout << "Starting diff of "<< args.DirectoryA << " and " << args.DirectoryB << " ("
        << args.ChecksumName << ")" << std::endl;
    std::cout << "Start time " << GetTime() << std::endl;

    auto resultA = work.scanDirectory(args.DirectoryA.string());
    auto resultB = work.scanDirectory(args.DirectoryB.string());

    work.Reconcile(resultA.get().get(), resultB.get().get(), true);
    work.WriteResult(args, "reference.patch");

    std::cout << std::endl << "End time " << GetTime() << std::endl;
}