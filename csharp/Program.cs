using System;
using System.Threading.Tasks;
using CommandLine;

namespace LanguageBenchmark
{
    class Program
    {
        public class Arguments
        {
            [Value(0, MetaName="DirectoryA", Required = true, HelpText="Directory to parse")]
            public string DirectoryA { get; set; }

            [Value(1, MetaName="DirectoryB", Required = true, HelpText="Directory to parse")]
            public string DirectoryB { get; set; }

            [Option('u',"ignore-unchanged", Default = false, HelpText="Ignore unchagned files in the final output")]
            public bool ignoreUnchanged { get; set; }

            [Option("md5", HelpText="MD5 hash (default)", SetName="md5")]
            public bool useMD5 { get; set; }

            [Option("sha1", HelpText="SHA1 hash", SetName="sha1")]
            public bool useSHA1 { get; set; }

            [Option("sha256", HelpText="SHA256 hash", SetName="sha256")]
            public bool useSHA256 { get; set; }   
        }

        static void Main(string[] args)
        {
            Parser.Default
                .ParseArguments<Arguments>(args)
                .WithParsed<Arguments>(
                    parsedArgs => 
                    {
                        ArgumentHolder argHolder = new ArgumentHolder(parsedArgs);
                        Console.WriteLine($"Starting diff of '{argHolder.DirectoryA}' and '{argHolder.DirectoryB}' ({argHolder.checksumName})");
                        Console.WriteLine($"Starting at {DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")}");
                        var worker = new Worker(argHolder.checksumName);
                        
                        var scanA = worker.ScanDirectory(argHolder.DirectoryA);
                        var scanB = worker.ScanDirectory(argHolder.DirectoryB);
                        Task.WaitAll(scanA, scanB);

                        var reconcileOperation = worker.Reconcile(scanA.Result, scanB.Result);
                        Task.WaitAll(worker.WriteResults(argHolder, reconcileOperation, "reference.patch"));

                        Console.WriteLine($"Finished at {DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")}");
                    })
                .WithNotParsed(
                    // Just exit the program
                    _ => System.Environment.Exit(1)
                );
        }
    }
}
