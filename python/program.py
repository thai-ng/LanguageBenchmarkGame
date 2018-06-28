from scanutil import FileResult
import os, sys, checksums
import argparse, datetime

def select_checksum(args_dict):
    """ Returns a pointer to the constructor of the correct checksum class  """
    retVal = checksums.MD5
    options = {'adler32': checksums.Adler32, 'crc': checksums.CRC32, 'sha1': checksums.SHA1, 'sha2': checksums.SHA2, 'md5': checksums.MD5}

    for name, function in options.items():
        if args_dict[name]:
            print("Using {0} checksum algorithm".format(name.upper()))
            retVal = function

    return retVal
# end select_checksum

def scan_directory(dir_name, checksum):
    """Returns a dictionary of filename to checksum object"""
    BUFFER_SIZE = 64 * 1024 # 4KB

    ret_val = {}
    for root, dirs, files in os.walk(dir_name):
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
    patch_info_a["CONFLICT"] = { x: (info_a[x], info_b[x]) for x in conflicts}
    
    patch_info_b = {}
    patch_info_b["ADD"] = [info_a[x] for x in (paths_a - paths_b)]
    patch_info_b["UNCHANGED"] = [info_b[x] for x in unchaged_paths]
    patch_info_b["CONFLICT"] = { x: (info_a[x], info_b[x]) for x in conflicts}

    return (patch_info_a,patch_info_b)
# end reconcile

def write_patch_results(directory_name, patch_info, result_file, ignore_unchanged):
    result_file.write("{0}\n".format(directory_name))
    symbol_map = {"ADD": "+", "UNCHANGED": "=", "CONFLICT": "!"}
    
    for operation, items in patch_info.items():
        if ignore_unchanged and operation=="UNCHANGED":
            continue

        symbol = symbol_map[operation]
        for file_info in items:
            result_file.write("{0} {1}\n".format(symbol, file_info))
        # end file_info

        result_file.write("\n")
    #end patch_info.items

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
    scan_a = scan_directory(args.directory_a, checksum)
    scan_b = scan_directory(args.directory_b, checksum)

    # check the changes and write to the file
    changes = reconcile(scan_a, scan_b)
    write_result(changes, args)
#end main
	
def setup_arguments():
    parser = argparse.ArgumentParser(description="Reference implementation of the language benchmarking trial")
    parser.add_argument('directory_a', metavar="dir_a", type=str, help="Directory to parse")
    parser.add_argument('directory_b', metavar="dir_b", type=str, help="Directory to parse")
    parser.add_argument('--ignore-unchanged', action='store_true', help="Ignore unchagned files in the final output")

    checksum_action = parser.add_mutually_exclusive_group()
    checksum_action.add_argument("--md5", action='store_true', help="MD5 hash (default)")
    checksum_action.add_argument("--adler32", action='store_true', help="Adler 32-bit checksum")
    checksum_action.add_argument("--crc", action='store_true', help="Cyclic Redundancy Check 32-bit checksum")
    checksum_action.add_argument("--sha1", action='store_true', help="SHA1 hash")
    checksum_action.add_argument("--sha2", action='store_true', help="SHA256 hash")
    return parser
#end setup_arguments

if __name__=="__main__":
    argmaker = setup_arguments()
    args = argmaker.parse_args()
    main(args)