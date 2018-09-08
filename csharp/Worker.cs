using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace LanguageBenchmark
{
    class Worker
    {
        public string checksumName { get; protected set; }

        public Worker(string checksumName)
        {
            this.checksumName = checksumName;
        }

        public Task<Results.ScanResult> ScanDirectory(string root)
        {
            var cutIndex = root.Length + 1;
            var files = Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories);

            // Return a task that runs this in a parallel foreach
            //  Because Parallel.ForEach doesn't return a task and is not awaitable, 
            //  then we have to wrap it in this way. 
            //  The resulting code is faster than having a List<Task> that is then sent to Task.WaitAll
            //  since Parallel.ForEach scales better.
            return Task.Run(
                () => 
                {
                    var scanResult = new Results.ScanResult();
                    Parallel.ForEach(files, 
                        (filepath) => {
                            var canonicalPath = filepath.Substring(cutIndex);
                            scanResult[canonicalPath] = this.HashFile(filepath, canonicalPath);
                    });

                    return scanResult;
                }
            );
        }

        public Results.ReconcileResult Reconcile(Results.ScanResult a, Results.ScanResult b)
        {
            var pathsA = new HashSet<string>(a.Keys);
            var pathsB = new HashSet<string>(b.Keys);

            var suspectedConflicts = new HashSet<string>(pathsA);
            suspectedConflicts.IntersectWith(pathsB);

            var unchangedFiles = new HashSet<string>();
            foreach(var entry in suspectedConflicts)
            {
                if(a[entry].Equals(b[entry]))
                {
                    unchangedFiles.Add(entry);
                }
            }

            var conflicts = new HashSet<string>(suspectedConflicts);
            conflicts.RemoveWhere(entry => unchangedFiles.Contains(entry));

            return new Results.ReconcileResult(
                this.GeneratePatch(b, a, pathsB, pathsA, unchangedFiles, conflicts),
                this.GeneratePatch(a, b, pathsA, pathsB, unchangedFiles, conflicts)
            );
        }

        public async Task WriteResults(ArgumentHolder args, Results.ReconcileResult patch, string outFilepath)
        {
            using(var outFile = File.Open(outFilepath, FileMode.Create, FileAccess.Write))
            using(var outStream = new StreamWriter(outFile))
            {
                var writeTaskA = Task.Run(() => this.WritePatchResult(args.DirectoryA, patch.Item1, args.ignoreUnchanged));
                var writeTaskB = Task.Run(() => this.WritePatchResult(args.DirectoryB, patch.Item2, args.ignoreUnchanged));

                await outStream.WriteLineAsync($"# Results for {DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")}");
                await outStream.WriteLineAsync($"# Reconciled '{args.DirectoryA}' '{args.DirectoryB}'");
                await outStream.WriteLineAsync(await writeTaskA);
                await outStream.WriteLineAsync();
                await outStream.WriteLineAsync(await writeTaskB);
                await outStream.WriteLineAsync();
            }
        }

        private FileResult HashFile(string filepath, string canonicalPath)
        {
            using(var hasher = HashAlgorithm.Create(this.checksumName))
            {
                var fileStream = new FileStream(filepath, FileMode.Open);
                return new FileResult
                {
                    FilePath = canonicalPath,
                    HashValue = BitConverter.ToString(hasher.ComputeHash(fileStream)).Replace("-",""),
                    Size = fileStream.Length,
                    ModifiedDate = File.GetLastWriteTime(filepath)
                };
            }
        }

        private Results.PatchResult GeneratePatch(
            Results.ScanResult src, Results.ScanResult target,
            HashSet<string> srcPaths, HashSet<string> targetPaths, 
            HashSet<string> unchanged, HashSet<string> conflicts)
        {
            var retVal = new Results.PatchResult();

            var additions = new HashSet<string>(srcPaths);
            int addCount = additions.RemoveWhere(entry => targetPaths.Contains(entry));
            retVal[Results.ReconcileOperation.ADD] = new List<FileResult>(addCount);
            foreach(var entry in additions)
            {
                retVal[Results.ReconcileOperation.ADD].Add(src[entry]);
            }
            
            retVal[Results.ReconcileOperation.UNCHANGED] = new List<FileResult>(unchanged.Count);
            foreach(var entry in unchanged)
            {
                retVal[Results.ReconcileOperation.UNCHANGED].Add(src[entry]);
            }

            retVal[Results.ReconcileOperation.CONFLICT] = new List<FileResult>(conflicts.Count);
            foreach(var entry in conflicts)
            {
                retVal[Results.ReconcileOperation.CONFLICT].Add(target[entry]);
            }

            return retVal;
        }

        private string WritePatchResult(string dir, Results.PatchResult result, bool ignoreUnchanged)
        {
            var buffer = new StringBuilder();
            buffer.AppendLine(dir);

            var lines = new List<Results.Line>();

            // Flatten the results
            foreach(var action in result)
            {
                var operation = action.Key;
                if(operation == Results.ReconcileOperation.UNCHANGED && ignoreUnchanged)
                {
                    continue;
                }

                foreach(var entry in action.Value)
                {
                    lines.Add(new Results.Line(operation, entry));
                }
            }

            // Sort by filename
            lines.Sort((x,y) => { return string.CompareOrdinal(x.Item2.FilePath,  y.Item2.FilePath); });
            foreach(var line in lines)
            {
                buffer.AppendLine($"{(char)line.Item1} {line.Item2}");
            }

            return buffer.ToString();
        }   
    }
}
