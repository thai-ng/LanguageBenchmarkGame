from scanutil import FileResult
import os, sys, checksums
import argparse

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
    paths_c = (paths_a | paths_b) - suspected_conflicts 
    unchaged_paths = set([x for x in suspected_conflicts if info_a[x] == info_b[x]])
    conflicts = suspected_conflicts - unchaged_paths

    # TODO: Revise below
    staging_info = {}
    staging_info.update({ info_a[x].filepath: info_a[x] for x in paths_c if x in paths_a })
    staging_info.update({ info_b[x].filepath: info_b[x] for x in paths_c if x in paths_b })
    
    directory_patch_a = staging_info.copy()
    directory_patch_a.update({info_b[f].filepath: info_b[f] for f in conflicts})
    
    directory_patch_b = staging_info.copy()
    directory_patch_b.update({info_a[f].filepath: info_a[f] for f in conflicts})

    return (1,)

def write_result(patch_tuple):
    """Creates the final output of the program: a patch recommendation"""
    pass

def main(args):
    # choose the checksum strategy and scan the files
    checksum = select_checksum(args.__dict__)
    scan_a = scan_directory(args.directory_a, checksum)
    scan_b = scan_directory(args.directory_b, checksum)

    # check the changes and write to the file
    changes = reconcile(scan_a, scan_b)
#end main
	
def setup_arguments():
    parser = argparse.ArgumentParser(description="Reference implementation of the language benchmarking trial")
    parser.add_argument('directory_a', metavar="dir_a", type=str, help="Directory to parse")
    parser.add_argument('directory_b', metavar="dir_b", type=str, help="Directory to parse")

    checksum_action = parser.add_mutually_exclusive_group()
    checksum_action.add_argument("--md5", action='store_true', help="MD5 hash (default)")
    checksum_action.add_argument("--adler32", action='store_true', help="Adler 32-bit checksum")
    checksum_action.add_argument("--crc", action='store_true', help="Cyclic Redundancy Check 32-bit checksum")
    checksum_action.add_argument("--sha1", action='store_true', help="SHA1 hash")
    checksum_action.add_argument("--sha2", action='store_true', help="SHA256 hash")
    return parser

if __name__=="__main__":
    argmaker = setup_arguments()
    args = argmaker.parse_args()
    main(args)