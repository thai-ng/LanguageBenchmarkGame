// program.ts
import * as commander from 'commander'

function main(){
    // TODO
}

let directory_a = undefined;
let directory_b = undefined;

commander
    .version("0.0.1")
    .description("Typescript implementation of the language benchmarking trial")
    .option("-p", "hit p")
    .arguments("<dir_a> <dir_b>")
    .action(function (dir_a, dir_b){
        directory_a = dir_a;
        directory_b = dir_b;
    });

commander.parse(process.argv)

if (typeof directory_a == "undefined" || typeof directory_b == "undefined" ){ 
    commander.outputHelp();
    process.exit(1)
}

main()