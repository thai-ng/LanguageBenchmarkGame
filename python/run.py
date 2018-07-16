#!/usr/bin/env python

def run(args):
    import subprocess, os
    print(args)
    process_args = ["python3", os.path.join(os.getcwd(), "program.py")] + args
    subprocess.call(process_args)
#end run

if __name__=="__main__":
    import sys, os

    if os.path.basename(sys.argv[0]) == __file__:
        run(sys.argv[1:])
# end main