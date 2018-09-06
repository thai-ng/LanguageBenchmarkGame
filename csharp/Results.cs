using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace LanguageBenchmark
{
    public class Results
    {
        public enum ReconcileOperation
        {
            ADD = '+',
            UNCHANGED = '=',
            CONFLICT = '!'
        }

        public class ScanResult : ConcurrentDictionary<string, FileResult> {}
        public class PatchResult : Dictionary<ReconcileOperation, List<FileResult>> {}
        public class ReconcileResult : Tuple<PatchResult, PatchResult>
        {
            public ReconcileResult(PatchResult a, PatchResult b): base(a,b){}
        }
    }
}
