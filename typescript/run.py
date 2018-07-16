#!/usr/bin/env python

def run(cmd_args):
    import subprocess, os
    process_args = ["node", "-r", "ts-node/register", "program.ts"] + cmd_args
    subprocess.call(process_args)
#end run

# def install(args):
#     raise NotImplementedError("This must be implemented before calling")
# #end run

if __name__=="__main__":
    import sys, os

    if sys.argv[0] == __file__:
        run(sys.argv[1:])
# end main