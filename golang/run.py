#!/usr/bin/env python

def setup():
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
        