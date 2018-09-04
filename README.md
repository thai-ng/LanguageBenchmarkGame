# Toy Language Benchmarking Trials

Inspired by Debian's "Game of Benchmark" and my own desire to learn new languages and a fervent wish to find _"the fastest language"_ for a particular project, this project was born. The idea is rather than make a purely computationally heavy task (like Mandelbrot fractals) that doesn't represent a langauge's actual use case, the "trial" is designed to be more practical while also introducing more practical scenarios (like file I/O).

The trial is simple:
* Scan two directories
* Generate the checksum of all the files in both directories
* Compare the checksums and generate a patch to reconcile changes in both directories

A couple of details for the implementation of a program for the trial
* Needs to have MD5, SHA1 and SHA256 as checksums. Adler32 and CRC32 are optional
* Default checksum is MD5, though other hashes/checksums should be flags if available (e.g. "adler32", "crc32", "sha1")
* The two directories are passed in as command line arguments
* Conflicts should be identified in the patch, but not resolved
* Final output of the program is a directory patch file (check reference implementation)

Finally, remember the goal is to make the fastest program possible that can complete the trial. As such, some languages might have better support for features for handling I/O workloads and multithreading. Therefore:
* Any and all libraries are allowed
* Multithreading, async I/O and any fancy language features are fair game. As long as it runs fast :)
* Any number of threads/processes can be spawned
* The process(es) shouldn't use any OS-specific calls to make itself faster
* You only really care about wall-clock time, optimize against that