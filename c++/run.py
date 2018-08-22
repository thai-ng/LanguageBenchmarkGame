#!/usr/bin/env python

def setup():
    import os, subprocess, datetime, pip
    if os.path.exists(os.path.join(os.getcwd(), "setup.log")):
        print("'setup.log' exists. C++ implementation setup correctly")
        return

    # We assume pip is installed at least
    # TODO
    #pip.main(['install'])
    # install libboost-filesystem-dev libcrypto++-dev libcrypto++9v5
    raise NotImplementedError("This must be implemented before running")
#end run

def build():
    raise NotImplementedError("This must be implemented before running")
#end run

def run(cmd_args):
    raise NotImplementedError("This must be implemented before running")
#end run

if __name__=="__main__":
    import sys, os

    if os.path.basename(sys.argv[0]) == __file__:
        run(sys.argv[1:])
# end main
        