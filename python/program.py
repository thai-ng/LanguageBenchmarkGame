from scanutil import FileResult
import os, sys, checksums
import argparse, datetime

def select_checksum(args_dict):
    """ Returns a pointer to the constructor of the correct checksum class  """
    retVal = checksums.MD5
    options = {'adler32': checksums.Adler32, 'crc32': checksums.CRC32, 'sha1': checksums.SHA1, 'sha256': checksums.SHA2, 'md5': checksums.MD5}

    for name, function in options.items():
        if args_dict[name]:
            retVal = function

    return retVal
# end select_checksum

def scan_directory(dir_name, checksum):
    """Returns a dictionary of filename to checksum object"""
    BUFFER_SIZE = 64 * 1024 # 4KB

    ret_val = {}
    for root, _, files in os.walk(dir_name):
        for a_file in files:
            filepath = os.path.join(root, a_file)
            check_obj = checksum()
            # source: https://stackoverflow.com/questions/22058048/hashing-a-file-in-python
            with open(filepath, 'rb') as current_file:
                while True:
                    file_data = current_file.read(BUFFER_SIZE)
                    if not file_data:
                        break
                    check_obj.update(file_data)
            # end current_file

            canonical_name = os.path.join(root.replace(dir_name, ''), a_file).lstrip(os.path.sep)
            file_info = os.stat(filepath)
            ret_val[canonical_name] = FileResult(canonical_name, check_obj.hexdigest(), file_info.st_size, file_info.st_mtime)
        # end file in directory
    # end os.walk

    return ret_val
#end scan_directory

def reconcile(info_a, info_b):
    """
    Returns a tuple with the changes that must be applied between info_a and info_b
    How reconciliation works:
    > "ADDED" (+): a file should be added
    > "UNCHANGED" (=): a file is common in both directories (same name, hash, size and modified date)
    > "CONFLICT" (!): files in the same path have different hashes, modified dates, but same name
    """
    paths_a = set(info_a.keys())
    paths_b = set(info_b.keys())

    suspected_conflicts = paths_a & paths_b
    # paths_c = (paths_a | paths_b) - suspected_conflicts 
    unchaged_paths = set([x for x in suspected_conflicts if info_a[x] == info_b[x]])
    conflicts = suspected_conflicts - unchaged_paths
    
    patch_info_a = {}
    patch_info_a["ADD"] = [info_b[x] for x in (paths_b - paths_a)]
    patch_info_a["UNCHANGED"] = [info_a[x] for x in unchaged_paths]
    patch_info_a["CONFLICT"] = [info_a[x] for x in conflicts]
    
    patch_info_b = {}
    patch_info_b["ADD"] = [info_a[x] for x in (paths_a - paths_b)]
    patch_info_b["UNCHANGED"] = [info_b[x] for x in unchaged_paths]
    patch_info_b["CONFLICT"] = [info_b[x] for x in conflicts]

    return (patch_info_a,patch_info_b)
# end reconcile

def write_patch_results(directory_name, patch_info, result_file, ignore_unchanged):
    result_file.write("{0}\n".format(directory_name))
    symbol_map = {"ADD": "+", "UNCHANGED": "=", "CONFLICT": "!"}

    # Serialize and sort the results
    serial_results = [(symbol_map[op], a_file) for op, files in patch_info.items() for a_file in files]
    serial_results.sort(key=lambda element: element[1].filepath)
    
    for line in serial_results:
        if ignore_unchanged and line[0]=="=":
            continue

        result_file.write("{0} {1}\n".format(*line))
    #end for
# end write_patch_results

def write_result(patch_tuple, args):
    """Creates the final output of the program: a patch recommendation"""
    with open("reference.patch", 'w') as result_file:
        result_file.write("# Results for {}\n".format(datetime.datetime.now()))
        result_file.write("# Reconciled '{0}' '{1}'\n".format(args.directory_a, args.directory_b))
        
        write_patch_results(args.directory_a, patch_tuple[0], result_file, args.ignore_unchanged)
        result_file.write("\n")
        write_patch_results(args.directory_b, patch_tuple[1], result_file, args.ignore_unchanged)
        result_file.write("\n")
    #end file
#end write_result

def main(args):
    # choose the checksum strategy and scan the files
    checksum = select_checksum(args.__dict__)

    print("Starting diff of '{0}' and '{1}' ({2})".format(args.directory_a, args.directory_b, checksum))
    print("Starting at {0}".format(datetime.datetime.now()))
    scan_a = scan_directory(args.directory_a, checksum)
    scan_b = scan_directory(args.directory_b, checksum)

    # check the changes and write to the file
    changes = reconcile(scan_a, scan_b)
    write_result(changes, args)
    print("Finished at {0}".format(datetime.datetime.now()))
#end main
	
def setup_arguments():
    parser = argparse.ArgumentParser(description="Reference implementation of the language benchmarking trial")
    parser.add_argument('directory_a', metavar="dir_a", type=str, help="Directory to parse")
    parser.add_argument('directory_b', metavar="dir_b", type=str, help="Directory to parse")
    parser.add_argument('--ignore-unchanged', '-u', action='store_true', help="Ignore unchagned files in the final output")

    checksum_action = parser.add_mutually_exclusive_group()
    checksum_action.add_argument("--md5", action='store_true', help="MD5 hash (default)")
    checksum_action.add_argument("--adler32", action='store_true', help="Adler 32-bit checksum")
    checksum_action.add_argument("--crc32", action='store_true', help="Cyclic Redundancy Check 32-bit checksum")
    checksum_action.add_argument("--sha1", action='store_true', help="SHA1 hash")
    checksum_action.add_argument("--sha256", action='store_true', help="SHA256 hash")
    return parser
#end setup_arguments

if __name__=="__main__":
    argmaker = setup_arguments()
    args = argmaker.parse_args()
    
    # Make absolute paths before handing it off
    args.directory_a = os.path.abspath(args.directory_a)
    args.directory_b = os.path.abspath(args.directory_b)
    main(args)