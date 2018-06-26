import os, sys, hashlib
import argparse

def scan_directory(dir_name):
    """Returns a dictionary of filename to checksum"""

    pass

def main(args):
    print(args)
    pass
	
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