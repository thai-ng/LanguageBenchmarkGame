using System;
using System.IO;

namespace LanguageBenchmark
{
    public class FileResult
    {
        public string FilePath { get; set; }

        public string HashValue { get; set; }

        public long Size { get; set; }

        public DateTime ModifiedDate { get; set; }

        public override bool Equals(object obj)
        {
            //
            // See the full list of guidelines at
            //   http://go.microsoft.com/fwlink/?LinkID=85237
            // and also the guidance for operator== at
            //   http://go.microsoft.com/fwlink/?LinkId=85238
            //
            var other = obj as FileResult;
            if (other == null)
            {
                return false;
            }
            
            return Path.Equals(this.FilePath, other.FilePath)
                && this.HashValue == other.HashValue
                && this.Size == other.Size
                && Math.Abs((this.ModifiedDate - other.ModifiedDate).TotalSeconds) < 1;
        }

        public override int GetHashCode(){ return base.GetHashCode(); }

        public override string ToString()
        {
            return $"{this.FilePath} ({this.ModifiedDate.ToLongTimeString()} | {this.Size} bytes)";
        }
    }
}