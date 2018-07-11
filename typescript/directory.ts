import * as fs from 'fs'
import * as path from 'path'

export function visit(err: Object, files: Array<string>, root: string, action: (filepath: string) => void){
    if(err){
        return;
    }

    files.forEach(element => {
        let filepath = path.join(root, element)

        fs.stat(filepath, function(err, stats){
            if(stats.isDirectory()){
                walk(filepath, action)
            }
            else if (typeof action != "undefined"){
                action(filepath)
            }
        });
    });
}

export function walk(basePath: string, action: (filepath: string) => void){
    const curry_visit = function(err: Object, files: Array<string>) {
        visit(err, files, basePath, action)
    }

    fs.readdir(basePath, curry_visit)
}