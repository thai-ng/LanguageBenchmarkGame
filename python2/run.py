#!/usr/bin/env python

def setup():
    print("If you can run this script, setup is complete")
#end run

def build():
    print("No build needed for reference implementation")
#end run

def run(cmd_args):
    import subprocess, os
    # The only change is that we run python2 instead of python3
    process_args = ["python2", os.path.join(os.getcwd(), '..', 'python', "program.py")] + cmd_args
    
    retcode = subprocess.call(process_args)
    if retcode != 0:
        raise RuntimeError("Program run returned non-zero exit code")
#end run

if __name__=="__main__":
    import sys, os

    if sys.argv[0] == __file__:
        run(sys.argv[1:])
# end main