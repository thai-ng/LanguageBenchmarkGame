// program.ts
import * as commander from 'commander'
import {ArgumentHolder} from './argumentholder'

function main(){
    // TODO
}

let args = new ArgumentHolder();

commander
    .version("0.0.1")
    .description("Typescript implementation of the language benchmarking trial")
    .arguments("<dir_a> <dir_b>")
    .action(function (dir_a, dir_b){
        args.directoryA = dir_a;
        args.directoryB = dir_b;
    });

commander
    .option("-u, --ignore-unchanged", "Ignore unchanged files in the final output")
    .option("--md5", "MD5 hash [default]")
    .option("--sha1", "SHA1 hash")
    .option("--sha256", "SHA256 hash")
    // Fast checksums not supported yet
    //.option("--adler32", "Alder 32-bit checksum")
    //.option("--crc", "Cyclic Redundancy Check 32-bit checksum")

commander.parse(process.argv)

if (args.directoryInfoIsMissing()){
    console.error("Directory information is missing!")
    commander.outputHelp();
    process.exit(1)
}

args.verifyAndSelectChecksum(commander);
main()