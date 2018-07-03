import * as crypto from 'crypto'

export class ArgumentHolder{
    directoryA: string;
    directoryB: string;

    checksumName: string;
    checksumGenerator: Function;

    directoryInfoIsMissing(){
        return typeof this.directoryA == "undefined" || typeof this.directoryB == "undefined"
            || this.directoryA.length < 3 || this.directoryB.length < 3
    }

    verifyAndSelectChecksum(args: Object){
        const options = ['md5', 'sha1', 'sha256'];
        let selection = undefined;
        options.forEach(functionName => {
            let possibleSelection = args[functionName];
            if(possibleSelection && selection){
                throw new Error(`Expected only option '${selection}' or '${functionName}'`);
            }

            selection = functionName;
        });

        this.checksumName = selection;
        this.checksumGenerator = crypto.createHash
    }

    getChecksumObject(){
        return this.checksumGenerator(this.checksumName)
    }
}