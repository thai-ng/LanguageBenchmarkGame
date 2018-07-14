// program.ts
import commander from 'commander'
import * as fs from 'fs'
import * as directory from './directory'
import { ArgumentHolder } from './argumentholder'
import { FileResult } from './fileresult';


async function scanDirectory(directoryPath: string, args: ArgumentHolder) : Promise<Map<string, FileResult>>{
    let retVal = new Map<string,FileResult>()

    await directory.walk(directoryPath, async function(filepath: string) {
        const hasher = args.getChecksumObject()
        const fileStream = fs.createReadStream(filepath)
        const stats = await directory.statAsync(filepath)

        fileStream.pipe(hasher)
        return new Promise(function(resolve, reject) {
            fileStream.on('end', () => {
                const filehash = hasher.digest('hex')
                retVal.set(filepath, 
                    new FileResult(
                        filepath,
                        filehash,
                        stats.size,
                        stats.mtimeMs
                    ))
                resolve()
            })

            fileStream.on('error', reject)
        })
    });

    return retVal
}

async function main(args: ArgumentHolder){
    console.log(`Starting diff of ${args.directoryA} and ${args.directoryB} (checksum: ${args.checksumName})`)
    console.log(`Starting at ${(new Date()).toISOString()}`)

    // Run in parallel, hope it's faster
    let [scan_a, scan_b] = await Promise.all(
        [scanDirectory(args.directoryA, args), 
            scanDirectory(args.directoryB, args)])

    // TODO: Diff
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
    commander.outputHelp()
    process.exit(1)
}

args.verifyAndSelectChecksum(commander);
main(args);