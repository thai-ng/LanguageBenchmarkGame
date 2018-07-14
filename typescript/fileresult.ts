export class FileResult{
    filepath: string;
    hash: string;
    size: number;
    date: number;

    constructor(filepath: string, filehash: string, size: number, date: number){
        this.filepath = filepath
        this.hash = filehash
        this.size = size
        this.date = date
    }

    public equals(another: FileResult) : boolean{
        return this.hash == another.hash
            && this.filepath == another.filepath
            && this.size == another.size
            && this.date == another.date
    }

    public toString() : string{
        return `${this.filepath} (${this.date} | ${this.size} bytes)`
    }
}