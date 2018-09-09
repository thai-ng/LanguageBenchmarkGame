import * as crypto from 'crypto'
import * as path from 'path'

export class ArgumentHolder{
    directoryA: string = "";
    directoryB: string = "";

    checksumName: string = "";
    checksumGenerator: Function = () => {};

    ignoreUnchanged: boolean = false;

    public directoryInfoIsMissing(){
        return typeof this.directoryA == "undefined" || typeof this.directoryB == "undefined"
            || this.directoryA.length < 3 || this.directoryB.length < 3
    }

    public verifyArguments(args: any){
        this.ignoreUnchanged = args["ignoreUnchanged"] || false
        this.selectChecksum(args);
        this.directoryA = this.regularizeDirectoryName(this.directoryA)
        this.directoryB = this.regularizeDirectoryName(this.directoryB)
    }

    private regularizeDirectoryName(somePath: string){
        let cleanPath = path.normalize(somePath)
        if(cleanPath.lastIndexOf('/') == cleanPath.length-1){
            cleanPath = cleanPath.substring(0, cleanPath.length-1)
        }

        return cleanPath
    }

    private selectChecksum(args: any) {
        const options = ['md5', 'sha1', 'sha256'];
        const notImplemented = ['crc32', 'adler32'];
        let selection = "";

        notImplemented.forEach(option =>{
            if(args[option]){
                throw new Error(`Cannot accept checksum '${option}'. It has not been implemented yet`);
            }
        });

        options.forEach(functionName => {
            let possibleSelection = args[functionName];
            if (possibleSelection) {
                if(selection.length > 1){
                    throw new Error(`Expected only option '${selection}' or '${functionName}'`);
                }

                selection = functionName;
            }
        });
        
        if (selection.length < 1) {
            selection = 'md5';
        }
        this.checksumName = selection;
    }

    public getChecksumObject(){
        return crypto.createHash(this.checksumName)
    }
}