#!/usr/bin/env python

output_file_name = 'cpp.out'

def setup():
    import os, datetime
    if os.path.exists(os.path.join(os.getcwd(), "setup.log")):
        print("'setup.log' exists. Thai's C++ implementation setup correctly")
        return

    # We can't really setup this successfully, we need a build system like CMake or scons for xplat support
    print("Need to install libboost-filesystem-dev libcrypto++-dev libcrypto++9v5")
    with open('setup.log', 'w') as logFile:
        logFile.write("# This is an autogenerated file made by 'run.py' on {}\n".format(datetime.datetime.now()))
        logFile.write("# => DO NOT DELETE THIS FILE OR SETUP WILL BE CALLED AGAIN\n")
        logFile.write("\n# Setup completed on {}".format(datetime.datetime.now()))
    #end logFile
#end run

def build():
    import subprocess, os
    
    # remove the previous build
    if os.path.exists(output_file_name):
        os.remove(output_file_name)

    source_files = [x for x in os.listdir('.') if x.endswith('.cpp')]
    c_libs = ['-lboost_system', '-lboost_filesystem', '-lpthread', '-l:libcryptopp.a']
    c_defs = ['-DNDEBUG']

    # For older versions of clang++/g++, the order of the source files matters!
    process_args = ['clang++'] + source_files + ['-std=c++17',  '-Wall', '-pedantic',  '-O3', '-o', output_file_name] + c_libs + c_defs
    subprocess.call(process_args)

    if os.path.exists(output_file_name):
        print("Built C++ implementation as '{}'".format(output_file_name))
    else:
        raise AssertionError("Build failed")
#end run

def run(cmd_args):
    import subprocess
    process_args = ["./{}".format(output_file_name)] + cmd_args
    retcode = subprocess.call(process_args)
    if retcode != 0:
        raise RuntimeError("Program run returned non-zero exit code")
#end run

if __name__=="__main__":
    import sys, os

    if os.path.basename(sys.argv[0]) == __file__:
        run(sys.argv[1:])
# end main
        