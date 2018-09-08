#!/usr/bin/env python3

# This is a basic runner "harness" to manage the different languages and their trials
# check the "runner.py" under each directory for more info

import sys, os, inspect

def help(args = None):    
    __name__ = "__main__"
    supported_operations = [name for name, obj in inspect.getmembers(sys.modules[__name__]) if inspect.isfunction(obj)]
    print()
    print("Language Benchmark Runner")
    print(" 'help' for this text")
    print(" 'init <language name>' to start implementing a new <language>")
    print(" 'run <language> [space-separated arguments]' to run a given <language> implementation with a set of [arguments]")
    print(" 'verify <language> [space-separated arguments]' to check a given <language> against the reference")
    print(" 'benchmark <repetitions> <language> [space-separated arguments]' run an implementation and take an average time")
    print(" 'compare <comma-separated list of languages> <repetitions> [space-separated arguments]' run some implementations and compare the average time")
    print(" 'plot/boxplot <comma-separated list of languages> <repetitions> [space-separated arguments]' benchmark and plot the results")
    print()
# end help

def import_from(module, name):
    # answer taken from https://stackoverflow.com/a/8790077
    module = __import__(module, fromlist=[name])
    return getattr(module, name)
# end import_from

def init(args):
    dir_name = args[0]
    if ',' in dir_name:
        raise ValueError("Cannot have ',' in the implementation name.")

    resolved_path = os.path.join(os.getcwd(), dir_name)
    if not os.path.exists(resolved_path):
        os.mkdir(dir_name)

    run_file_path = os.path.join(resolved_path, "run.py")
    if os.path.exists(run_file_path):
        raise SystemError("This directory already has a 'run.py'")
    
    with open(os.path.join(dir_name,"run.py"), 'w') as run_file:
        run_file.write("""#!/usr/bin/env python

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
        """)

    print("Initialized '{}'".format(dir_name))
    print("Make sure to fill in the 'setup', 'build' and 'run' methods")
#end init

def run(args):
    # save the previous working directory
    working_dir = os.getcwd()
    
    dir_name = args[0]
    if not os.path.exists(os.path.join(os.getcwd(), dir_name) ) :
        raise NameError("Could not find directory '{}'".format(dir_name))

    print("Running trial implemented in '{}'".format(args[0]))
    
    sub_args = args[1:]
    print("Arguments: {}".format(sub_args))
    print("")

    os.chdir(dir_name)

    module_name = dir_name+'.run'
    setup = import_from(module_name, 'setup')
    build = import_from(module_name, 'build')
    run_implementation = import_from(module_name, 'run')

    setup()
    build()
    
    run_implementation(sub_args)
    
    # restore working directory
    os.chdir(working_dir)
#end run

def verify(args):
    from difflib import context_diff

    baseline = 'python'
    comparison = args[0]
    output_file = 'reference.patch'

    run([baseline] + args[1:])
    run(args)
    
    diff = None
    with open(os.path.join(baseline, output_file), 'r') as baseline_output:
        with open(os.path.join(comparison, output_file), 'r') as comparison_output:
            diff = list(context_diff(baseline_output.readlines(), comparison_output.readlines()))

    # This lambda is a shorthand for when verification might fail
    verification_fails_with = lambda entries,pattern : len(entries) > 2 or not all(lines.startswith(pattern) for lines in entries)
    verification_failure = False

    conflicting_lines = [x for x in diff if x[0]=='!']
    if verification_fails_with(conflicting_lines, '! # Results'):
        print("'{}' verification failure: more than one conflicting line found (other than file header)".format(comparison))
        verification_failure = True
    
    additional_lines = [x for x in diff if x[0]=='+']
    if verification_fails_with(additional_lines, '+ {}'.format(os.linesep)):
        print("'{}' verification failure: additional lines of difference are not newlines ({})".format(comparison, os.linesep))
        verification_failure = True

    if verification_failure:
        raise NotImplementedError("Check the implementation of {} against {}".format(comparison, baseline))

    print()
    print("'{}' meets the implementation criteria".format(comparison))
#end verify

def benchmark(args, return_times = False):
    import time
    
    # save the current working dir
    working_dir = os.getcwd()

    repetitions = int(args[0])
    dir_name = args[1]
    sub_args = args[2:]
    times = []

    os.chdir(dir_name)

    module_name = dir_name+'.run'
    setup = import_from(module_name, 'setup')
    build = import_from(module_name, 'build')
    run_implementation = import_from(module_name, 'run')

    setup()
    build()
    print("========== Starting Benchmark ==========")
    for i in range(repetitions):
        start_time = time.perf_counter()
        run_implementation(sub_args)
        end_time = time.perf_counter()
        times.append(round(end_time-start_time,3))
    #end for
    print("========== Finishing Benchmark ==========")

    average = sum(times) / repetitions
    print("")
    print("{} repetitions - average run time: {} seconds".format(repetitions, average))
    print("")
    
    # restore working dir
    os.chdir(working_dir)

    if return_times:
        return times

    return average
#end benchmark

def compare(args, return_time_list = False, print_results = True):
    dir_names = args[0].split(',')
    if len(dir_names) == 1 and dir_names[0] == 'all':
        dir_names = [x for x in os.listdir('.') if os.path.isdir(x) and os.path.exists(os.path.join('.',x,'run.py'))]
        print("Selected by wildcard: {}".format(dir_names))

    repetitions = args[1]
    results = {}

    for implementation in dir_names:
        sub_args = [repetitions, implementation]
        sub_args.extend(args[2:])
        results[implementation] = benchmark(sub_args, return_times=return_time_list)
    # end for

    if not print_results:
        return results

    print("Ran {} iterations of implementation: {}".format(repetitions, dir_names))
    keys = list(results.keys())
    keys.sort()
    for lang in keys:
        print("{}: {} seconds".format(lang, results[lang]))

    return results
#end compare

def __plot(args, use_mean = False):
    # internal, shared implementation of plot

    # do a comparison, but get all results back
    benchmark_results = compare(args, return_time_list = not use_mean, print_results= False)

    ordered_results = []
    avg = lambda collection: sum(collection)/len(collection)
    key_func = lambda x : avg(x[1])
    
    if use_mean:
        # We want avg to be a no-op since it is already averaged
        avg = lambda x: x
    
    ordered_results = [(lang, results) for lang,results in benchmark_results.items()]
    ordered_results.sort(key = key_func)
    
    return ordered_results
#end __plot

def plot(args):
    import pygal

    benchmark_results = __plot(args, use_mean= True)

    bar_chart = pygal.HorizontalBar()
    bar_chart.title = 'Language benchmark results (in seconds, lower is better)'
    [bar_chart.add(x[0], x[1]) for x in benchmark_results]
    bar_chart.render_in_browser()
#end plot

def boxplot(args):
    import pygal

    benchmark_results = __plot(args, use_mean= False)

    box_plot = pygal.Box()
    box_plot.title = 'Language benchmark results'
    [box_plot.add(lang, results) for lang,results in benchmark_results]
    box_plot.render_in_browser()
#end plot

if __name__=="__main__":
    args = sys.argv[1:]
    working_dir = os.getcwd()
    __name__ = "__runner__"

    try:
        operation_name = args[0]
        operation = eval(operation_name)
        operation(args[1:])
    except Exception as e:
        if len(args) > 0:
            print("")
            print("Encountered an issue while running: \"{}\"".format(e))
        help()
        os.chdir(working_dir)
# end main