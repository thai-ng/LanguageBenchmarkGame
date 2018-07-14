import * as fs from 'fs'
import * as util from 'util'
import * as path from 'path'

export const readdirAsync = util.promisify(fs.readdir)
export const statAsync = util.promisify(fs.stat)

export async function visit(files: Array<string>, root: string, action: (filepath: string) => void){
    // array.forEach is apparently synchronous, so might as well do this
    for(const file of files){
        let filepath = path.join(root, file)
        let stats = await statAsync(filepath)
        if(stats.isDirectory()){
            walk(filepath, action)
        }
        else if (typeof action != "undefined"){
            await action(filepath)
        }
    }
}

export async function walk(basePath: string, action: (filepath: string) => void){
    let files = await readdirAsync(basePath)
    await visit(files, basePath, action)
}