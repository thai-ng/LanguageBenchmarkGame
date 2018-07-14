import * as crypto from 'crypto'

export class ArgumentHolder{
    directoryA: string = "";
    directoryB: string = "";

    checksumName: string = "";
    checksumGenerator: Function = () => {};

    public directoryInfoIsMissing(){
        return typeof this.directoryA == "undefined" || typeof this.directoryB == "undefined"
            || this.directoryA.length < 3 || this.directoryB.length < 3
    }

    public verifyAndSelectChecksum(args: any){
        const options = ['md5', 'sha1', 'sha256'];
        let selection = "";
        options.forEach(functionName => {
            let possibleSelection = args[functionName];
            if(possibleSelection && selection){
                throw new Error(`Expected only option '${selection}' or '${functionName}'`);
            }

            if(selection.length < 1){
                selection = functionName;
            }
        });

        if(selection.length < 1){
            selection = 'md5';
        }

        this.checksumName = selection;
    }

    public getChecksumObject(){
        return crypto.createHash(this.checksumName)
    }
}