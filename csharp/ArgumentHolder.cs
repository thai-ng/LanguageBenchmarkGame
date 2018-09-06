using System;
using System.IO;
using System.Security.Cryptography;

namespace LanguageBenchmark
{
    class ArgumentHolder
    {
        public ArgumentHolder(Program.Arguments inArgs)
        {
            // HACK: combine paths and get the base dir to get a uniform name
            this.DirectoryA = Path.GetDirectoryName(Path.Combine(inArgs.DirectoryA,"x"));
            this.DirectoryB = Path.GetDirectoryName(Path.Combine(inArgs.DirectoryB, "x"));

            this.ignoreUnchanged = inArgs.ignoreUnchanged;
            this.checksumName = this.selectChecksum(inArgs);
        }

        public string DirectoryA { get; protected set; }

        public string DirectoryB { get; protected set; }

        public bool ignoreUnchanged { get; protected set; }

        public string checksumName { get; protected set; }

        private string selectChecksum(Program.Arguments inArgs)
        {
            if(inArgs.useMD5){ return "MD5"; }
            else if(inArgs.useSHA1){ return "SHA1"; }
            else if(inArgs.useSHA256){ return "SHA256"; }

            return "MD5";
        }
    }
}
