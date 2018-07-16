#!/usr/bin/env python

# This is a basic runner "harness" to manage the different languages and their trials
# check the "runner.py" under each directory for more info

import sys, os, inspect

def help(args = None):    
    supported_operations = [name for name, obj in inspect.getmembers(sys.modules[__name__]) if inspect.isfunction(obj)]
    print("Language Benchmark Runner")
    print(" 'help' for this text")
    print(" 'init <language name>' to start implementing a new <language>")
    print(" 'run <language> [space-separated arguments]' to run a given <language> implementation with a set of [arguments]")
    print(" 'benchmark <repetitions> <language> [space-separated arguments]' run an implementation and take an average time")
# end help

def init(args):
    dir_name = args[0]
    resolved_path = os.path.join(os.getcwd(), dir_name)
    if not os.path.exists(resolved_path):
        os.mkdir(dir_name)

    run_file_path = os.path.join(resolved_path, "run.py")
    if os.path.exists(run_file_path):
        raise SystemError("This directory already has a 'run.py'")
    
    with open(os.path.join(dir_name,"run.py"), 'w') as run_file:
        run_file.write("""#!/usr/bin/env python

def run(args):
    raise NotImplementedError("This must be implemented before running")
#end run

def install(args):
    raise NotImplementedError("This must be implemented before running")
#end run

if __name__=="__main__":
    import sys, os

    if os.path.basename(sys.argv[0]) == __file__:
        run(sys.argv[1:])
# end main

        """)

    print("Initialized '{}'".format(dir_name))
    print("Make sure to fill in the 'run' and 'install' methods")
#end init

def run(args):
    dir_name = args[0]
    if not os.path.exists(os.path.join(os.getcwd(), dir_name) ) :
        raise NameError("Could not find directory '{}'".format(dir_name))

    print("Running trial implemented in '{}'".format(args[0]))
    
    sub_args = args[1:]
    print("Arguments: {}".format(sub_args))
    print("")

    # Change dir, and load the runner from there. This may or may not override some functions
    # NOTE: DON'T EXEC RANDOM FILES
    exec(open(os.path.join('.',dir_name,'run.py')).read())
    run(sub_args)
#end run

def benchmark(args):
    repetitions = int(args[0])
    dir_name = args[1]
    sub_args = args[2:]
    times = []

    os.chdir(dir_name)
    exec(open(os.path.join('.','run.py')).read())
    import time
    print("========== Starting Benchmark ==========")
    for i in range(repetitions):
        start_time = time.time()
        run(sub_args)
        end_time = time.time()
        times.append(end_time-start_time)
    #end for
    print("========== Finishing Benchmark ==========")

    average = sum(times) / repetitions
    print("")
    print("{} repetitions - average run time: {} seconds".format(repetitions, average))
#end benchmark

if __name__=="__main__":
    args = sys.argv[1:]
    working_dir = os.getcwd()

    try:
        operation_name = args[0]
        operation = eval(operation_name)
        operation(args[1:])
    except Exception, e:
        print("")
        print("Encountered an issue while running: \"{}\"\n".format(e))
        help()
        os.chdir(working_dir)
# end main