using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.Win32;

// CodeMarkers
using Microsoft.Internal.Performance;

// Dev11 Language Service references
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

// Dev10 Language Service references
using Microsoft.JScript.Authoring.EngineAdapter;
using Microsoft.JScript.Authoring.Interfaces;
using Microsoft.JScript.NamedItem;
using Microsoft.JScript.Authoring.Intellisense;

namespace DirectAuthorTests
{
    [TestClass]
    public class PerformanceTests : DirectAuthorTest
    {
        public new class Paths : DirectAuthorTest.Paths
        {
            private static readonly string PerformanceTestSolutionBasePath = Path.Combine(Paths.BasePath, @"PerformanceTests");
            public new static readonly string FilesPath = Path.Combine(PerformanceTestSolutionBasePath, @"Files");
            public static readonly string CodeMarkersDirectoryPath = Path.Combine(PerformanceTestSolutionBasePath, @"CodeMarkers");
        }

        class CodeMarkerListener 
        {
            private CodeMarkers codeMarker;
            private Process process;
            private List<CodeMarkerListenerEvent> events;
            private Thread listenerThread;
            private bool ready;
            private static Regex ListenrRegex = new Regex(@"(?<pid>[\d]+)[\s]*[:][\s]+[@](?<timestamp>[\d]+)[\s\d]+[:](?<eventId>[\d]+)[\s]+(?<eventText>[^\n\r]*)$", RegexOptions.ExplicitCapture | RegexOptions.Singleline);
            private const int RetryCount = 3;
            private const int StartEventId = 99;        // used to initalize the listener
            private const int EndEventId = 100;         // used to notify the listener to shutdown
            private int targetProcessId;

            public struct CodeMarkerListenerEvent
            {
                public uint processId;  // Sending process ID
                public uint eventId;    // Numeric CodeMarkerEvent ID
                public uint timestamp;  // Time in milliseconds since the call to StartListenin
               
                public CodeMarkerListenerEvent(uint timestamp, uint eventId, uint processId)
                {
                    this.timestamp = timestamp;
                    this.eventId = eventId;
                    this.processId = processId;
                }

                public override string ToString()
                {
                    return String.Format("timestamp: {0}, \teventId: {1} \teventName: {2}", this.timestamp, this.eventId, GetEventName(this.eventId));
                }

                private static string GetEventName(uint eventId)
                {
                    switch (eventId)
                    {
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsBegin: return "ExecuteAsyncRequestsBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsEnd: return "ExecuteAsyncRequestsEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTCursorBegin: return "GetASTCursorBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTCursorEnd: return "GetASTCursorEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTSubTreeBegin: return "GetASTSubTreeBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTSubTreeEnd: return "GetASTSubTreeEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetAsyncRequestsBegin: return "GetAsyncRequestsBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetAsyncRequestsEnd: return "GetAsyncRequestsEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendByteCodeGenerationBegin: return "ByteCodeGenerationBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendByteCodeGenerationEnd: return "ByteCodeGenerationEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecutionBegin: return "ExecutionBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecutionEnd: return "ExecutionEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTBegin: return "GetASTBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTEnd: return "GetASTEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetCompletionsBegin: return "GetCompletionsBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetCompletionsEnd: return "GetCompletionsEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetFunctionHelpBegin: return "GetFunctionHelpBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetFunctionHelpEnd: return "GetFunctionHelpEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetMessageSetBegin: return "GetMessageSetBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetMessageSetEnd: return "GetMessageSetEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetQuickInfoBegin: return "GetQuickInfoBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetQuickInfoEnd: return "GetQuickInfoEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetRegionsBegin: return "GetRegionsBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetRegionsEnd: return "GetRegionsEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendParsingBegin: return "ParsingBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendParsingEnd: return "ParsingEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendRewriteTreeBegin: return "RewriteTreeBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendRewriteTreeEnd: return "RewriteTreeEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendUpdateBegin: return "UpdateBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendUpdateEnd: return "UpdateEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendApplyCommentsBegin: return "ApplyCommentsBegin";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendApplyCommentsEnd: return "ApplyCommentsEnd";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendAbortCalled: return "AbortCalled";
                        case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendHurryCalled: return "HurryCalled";
                    }
                    return "unknown";
                }
            }

            public CodeMarkerListener(CodeMarkers codeMarker, string codeMarkerSinkLocation)
            {
                if (!File.Exists(codeMarkerSinkLocation))
                {
                    throw new FileNotFoundException(String.Format("The file {0} could not be found", codeMarkerSinkLocation), codeMarkerSinkLocation);
                }

                this.codeMarker = codeMarker;
                this.ready = false;
                this.events = new List<CodeMarkerListenerEvent>();
                this.targetProcessId = Process.GetCurrentProcess().Id;

                this.process = new Process();
                this.process.StartInfo.FileName = codeMarkerSinkLocation;
                this.process.StartInfo.CreateNoWindow = true;
                this.process.StartInfo.UseShellExecute = false;
                this.process.StartInfo.RedirectStandardError = true;
                this.process.StartInfo.RedirectStandardOutput = true;
                // only listen to current process, and die after the end event is fired
                this.process.StartInfo.Arguments = string.Format(" -n -p:{0} -d:{1}", this.targetProcessId, CodeMarkerListener.EndEventId);
            }

            private void Listen()
            {
                // Run the process
                this.process.Start();

                while (!process.StandardOutput.EndOfStream)
                {
                    
                    string line = process.StandardOutput.ReadLine();
                    Match match = ListenrRegex.Match(line);
                    if (match.Success)
                    {
                        uint processId = uint.Parse(match.Groups["pid"].ToString());
                        uint timestamp = uint.Parse(match.Groups["timestamp"].ToString());
                        uint eventId = uint.Parse(match.Groups["eventId"].ToString());

                        // skip starting event
                        if (eventId == CodeMarkerListener.StartEventId)
                        {
                            this.ready = true;
                            continue;
                        }

                        // wait on the end event marker
                        if (eventId == CodeMarkerListener.EndEventId)
                            break;

                        // skip events form other processes
                        if (processId != this.targetProcessId)
                            continue;

                        this.events.Add(new CodeMarkerListenerEvent(timestamp, eventId, processId));
                    }
                }

                // wait for the process to exit
                this.process.WaitForExit();
            }

            public void Start()
            {
                listenerThread = new Thread(new ThreadStart(this.Listen));
                listenerThread.Start();
                Thread.Yield(); // allow the listern to initialize before we return
                WaitUntillReady();
            }

            public void End()
            {
                // fire the end event
                this.codeMarker.CodeMarker(CodeMarkerListener.EndEventId);

                int count = 0;
                while (null != process && !process.HasExited && count < CodeMarkerListener.RetryCount)
                {
                    Thread.Sleep(1000 * count);
                    count ++;
                    // no need to fire event again, as the listener is already running
                }

                // if process is still alive, kill it
                if (null != process && !process.HasExited)
                {
                    this.process.Kill();
                    this.process.WaitForExit();
                }
                listenerThread.Join(); // wait for the thread to exit
            }

            public List<CodeMarkerListenerEvent> Events 
            {
                get { return this.events;}
            }

            public void WaitUntillReady()
            {
                // fire the init marker
                this.codeMarker.CodeMarker(CodeMarkerListener.StartEventId);

                int count = 0;
                while (!this.ready && count < CodeMarkerListener.RetryCount)
                {
                    Thread.Sleep(1000 * count);
                    // fire again, as the listener may have missed it
                    this.codeMarker.CodeMarker(CodeMarkerListener.StartEventId);
                    count++;
                }

                if (!this.ready)
                    throw new Exception("Listener process can not be initialized.. ");
            }

            ~CodeMarkerListener()
            {
                if (null != process && !process.HasExited)
                {
                    this.process.Kill();
                    this.process.WaitForExit();
                }
            }
        }

        private CodeMarkers codeMarker;
        private CodeMarkerListener codeMarkerListener;
        private const string CodeMarkersRegKey = @"SOFTWARE\Microsoft\JscriptLS\10.0";
        private static readonly string CodeMarkerSinkFileLocation = Path.Combine(Paths.CodeMarkersDirectoryPath, "CodeMarkerSink.exe");
        private const string CodeMarkerSinkProcessName = "CodeMarkerSink";

        [TestInitialize]
        public void InitializePerformanceTests()
        {
            //_session = new AuthorTestSession(Directory.GetCurrentDirectory());

            // Note: For CodeMarkers to work, the following reg key has to exist, with its default value set:
            //          HKEY_LOCAL_MACHINE\<CodeMarkersRegKey>\Performance
            //          (Default)              REG_SZ                Microsoft.Internal.Performance.CodeMarkers.dll
            InitializeCodeMarkers();
        }

        protected void InitializeCodeMarkers()
        {
#if DEBUG
            if (CodeMarkersEnabled())
            {
                // kill any listerners remaining from previous test runs
                KillActiveCodeMarkersListeners();

                // initialize the code marker library
                codeMarker = CodeMarkers.Instance;
                codeMarker.InitPerformanceDll(CodeMarkerApp.IEPERF, CodeMarkersRegKey);

                // start the code marker listener
                codeMarkerListener = new CodeMarkerListener(codeMarker, CodeMarkerSinkFileLocation);
                codeMarkerListener.Start();
            }
#endif
        }

        [TestCleanup]
        public void CleanupPerformanceTests()
        {
#if DEBUG
            if (CodeMarkersEnabled())
            {
                codeMarkerListener.End();
                codeMarker.UninitializePerformanceDLL(CodeMarkerApp.IEPERF);
                DumpTestSummary();
            }
#endif
        }

        protected void RecordTestTime(Int64 time)
        {
            Console.WriteLine("### TIME: {0} ms", time);
        }

        private bool CodeMarkersEnabled()
        {
            RegistryKey regKey = Registry.LocalMachine.OpenSubKey(CodeMarkersRegKey + @"\Performance");
            if (regKey == null)
            {
                return false;
            }
            object value = regKey.GetValue(null, null);
            return value != null && value.ToString().Length > 0;
        }

        private void KillActiveCodeMarkersListeners()
        {
            var activeListeners = Process.GetProcessesByName(CodeMarkerSinkProcessName);
            foreach (var activeListener in activeListeners)
            {
                activeListener.Kill();
            }
        }

        private void ProcessEventEnd(Stack<CodeMarkerListener.CodeMarkerListenerEvent> eventStack, CodeMarkerListener.CodeMarkerListenerEvent endEvent, uint expectedStartEventId, string bucketName, Dictionary<string, uint> excutionBuckets)
        {
            Assert.IsTrue(eventStack.Count > 0, "Stack is empty.");
            var startEvent = eventStack.Pop();
            uint startEventId = startEvent.eventId;
            Assert.IsTrue(CodeMarkerEvent.Equals(startEventId, expectedStartEventId), string.Format("Acctual: {0}. Expected: {1}", startEventId, expectedStartEventId));
            uint timeDiffrence =  endEvent.timestamp - startEvent.timestamp;

            if (excutionBuckets.ContainsKey(bucketName))
            {
                excutionBuckets[bucketName] += timeDiffrence;
            }
            else 
            { 
                excutionBuckets.Add(bucketName, timeDiffrence);
            }
        }

        private void DumpExecutionBuckets()
        {
            Dictionary<string, uint> excutionBuckets = new Dictionary<string, uint>();
            Stack<CodeMarkerListener.CodeMarkerListenerEvent> eventStack = new Stack<CodeMarkerListener.CodeMarkerListenerEvent>();
            uint startTime = 0;
            uint endTime = 0;

            for (int i = 0; i < codeMarkerListener.Events.Count; i++)
            {
                var e = codeMarkerListener.Events[i];
                uint eventId = e.eventId;

                if (i == 0) startTime = e.timestamp;
                if (i == codeMarkerListener.Events.Count - 1) endTime = e.timestamp;

                switch (eventId)
                {
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendParsingBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendByteCodeGenerationBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecutionBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendRewriteTreeBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetMessageSetBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetFunctionHelpBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetQuickInfoBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetCompletionsBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetRegionsBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendUpdateBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTCursorBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTSubTreeBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetAsyncRequestsBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetDefinitionLocationBegin:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendApplyCommentsBegin:
                        eventStack.Push(e);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendParsingEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendParsingBegin, "Parsing", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendByteCodeGenerationEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendByteCodeGenerationBegin, "Code Generation", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecutionEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecutionBegin, "Execution", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendRewriteTreeEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendRewriteTreeBegin, "Tree Rewriting", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetMessageSetEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetMessageSetBegin, "GetMessageSet Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetFunctionHelpEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetFunctionHelpBegin, "GetFunctionHelp Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTBegin, "GetAST Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetQuickInfoEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetQuickInfoBegin, "GetQuickInfo Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetCompletionsEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetCompletionsBegin, "GetCompletions Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetRegionsEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetRegionsBegin, "GetRegions Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendUpdateEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendUpdateBegin, "Update Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTCursorEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTCursorBegin, "GetASTCursor Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTSubTreeEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetASTSubTreeBegin, "GetASTSubTree Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetAsyncRequestsEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetAsyncRequestsBegin, "GetAsyncRequests Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendExecuteAsyncRequestsBegin, "ExecuteAsyncRequests Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetDefinitionLocationEnd:
                        ProcessEventEnd(eventStack, e, CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendGetDefinitionLocationBegin, "GetDefinitionLocation Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendApplyCommentsEnd: 
                        ProcessEventEnd(eventStack, e, 23666, "ApplyComments Call", excutionBuckets);
                        break;
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendAbortCalled:
                    case CodeMarkerEvent.perfBrowserTools_LanguageServiceBackendHurryCalled:
                        break;
                    default:
                        Assert.Fail("Unexpected Event type: ", e.eventId);
                        break;
                }
            }


            uint totalTime = endTime - startTime;
            if (excutionBuckets.Count > 0)
            {
                Console.WriteLine();
                Console.WriteLine("======================================");
                Console.WriteLine("Time spent at each phase: ");
                Console.WriteLine("======================================");
                foreach (var bucket in excutionBuckets)
                {
                    double percentOfTotalTime = (double)bucket.Value / (double)totalTime;
                    Console.WriteLine("{0}: \t{1} ({2:0%})", bucket.Key, bucket.Value, percentOfTotalTime);
                }
                Console.WriteLine("======================================");
                Console.WriteLine();
            }
        }

        private void DumpRawEvents()
        {
            if (this.codeMarkerListener.Events.Count > 0)
            {
                Console.WriteLine();
                Console.WriteLine("======================================");
                Console.WriteLine("Code Marker Raw Events: ");
                Console.WriteLine("======================================");
                foreach (var e in this.codeMarkerListener.Events)
                {
                    Console.WriteLine(e.ToString());
                }
                Console.WriteLine("======================================");
                Console.WriteLine();
            }
        }

        private void DumpTestSummary()
        {
            DumpExecutionBuckets();
            DumpRawEvents();
        }

        /// <summary>
        /// Override the default parse mechanism to use a different marker for code completion "_|ch|" instead of "|ch|" or simply "|"
        /// </summary>
        protected override ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();

            int start = 0;
            int markerBeginningIndex = 0;
            string markerBeginning = "_|";
            string markerEnd = "|";

            while ((markerBeginningIndex = text.IndexOf(markerBeginning, start)) >= 0)
            {
                builder.Append(text, start, markerBeginningIndex - start);

                int markerEndIndex = text.IndexOf(markerEnd, markerBeginningIndex + markerBeginning.Length);
                if (markerEndIndex <= 0)
                {
                    throw new ArgumentException(string.Format("Could not find the end of the marker. Expected '{0}' after {1}", markerEnd, markerBeginningIndex));
                }
                string data = text.Substring(markerBeginningIndex + markerBeginning.Length, markerEndIndex - (markerBeginningIndex + markerBeginning.Length));
                requests.Add(new Request() { Offset = builder.Length, Data = data });

                start = markerEndIndex + markerEnd.Length;
            }
            builder.Append(text, start, text.Length - start);

            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }

        internal IEnumerable<AuthorParameterHelp> PerformGetParameterHelp(string text, params string[] contextFiles)
        {
            return PerformGetParameterHelp(text, AuthorHostType.ahtBrowser, contextFiles);
        }

        internal IEnumerable<AuthorParameterHelp> PerformGetParameterHelp(string text, AuthorHostType hostType, params string[] contextFiles)
        {
            var results = new List<AuthorParameterHelp>();
            PerformRequests(text, hostType, (context, offset, data, index) =>
            {
                Int64 callTime;

                var help = context.GetParameterHelpAt(offset, out callTime);

                if (String.Compare(data, "r", StringComparison.InvariantCultureIgnoreCase) == 0)
                {
                    RecordTestTime(callTime);
                    results.Add(help);
                }
            }, false, contextFiles);
            return results.ToArray();
        }

        internal string PerformGetRegions(string fileName, int numberOfIterations, int iterationToRecord)
        {
            Int64 callTime;
            StringBuilder result = new StringBuilder();

            Assert.IsTrue(iterationToRecord > 0 && iterationToRecord <= numberOfIterations, "iterationToRecord should be in the range of [1, numberOfIterations].");

            var file = _session.ReadFile(fileName);
            var context = _session.OpenContext(file);

            for (int i = 0; i < numberOfIterations; i++)
            {
                var regions = context.GetRegions(out callTime);

                Assert.IsTrue(regions != null);

                if ((i+1) == iterationToRecord) // 1 based count
                {
                    RecordTestTime(callTime);
                    result.Append(DumpObject(regions));
                }
            }

            return result.ToString();
        }

        internal IAuthorCompletionSet[] PerformGetCompletions(string text, params string[] contextFiles)
        { 
            return PerformGetCompletions(text, AuthorHostType.ahtBrowser, contextFiles);
        }

        internal IAuthorCompletionSet[] PerformGetCompletions(string text, AuthorHostType hostType, params string[] contextFiles)
        {
            List<IAuthorCompletionSet> result = new List<IAuthorCompletionSet>();

            PerformRequests(text, hostType, (context, offset, data, index) =>
            {
                Int64 callTime;

                var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfMembersFilter, out callTime);

                if (String.Compare(data, "r", StringComparison.InvariantCultureIgnoreCase) == 0)
                {
                    result.Add(completions);
                    RecordTestTime(callTime);
                }

            }, false, contextFiles);

            return result.ToArray();
        }

        internal string PerformGetAST(string fileName, int numberOfIterations, int iterationToRecord)
        {
            Int64 callTime;
            StringBuilder result = new StringBuilder();
            
            Assert.IsTrue(iterationToRecord > 0 && iterationToRecord <= numberOfIterations, "iterationToRecord should be in the range of [1, numberOfIterations].");

            var file = _session.ReadFile(fileName);
            var context = _session.OpenContext(file);
            for (int i = 0; i < numberOfIterations; i++)
            {
                var ast = context.GetASTAsJSON(out callTime);
                if ((i + 1) == iterationToRecord) // 1 based count
                {
                    RecordTestTime(callTime);
                    result.Append(ast);
                }
            }

            return result.ToString();
        }

        internal string PerformGetMessages(string fileName, int numberOfIterations, int iterationToRecord)
        {
            StringBuilder result = new StringBuilder();
            Int64 totalTime;
            Int64 callTime;

            Assert.IsTrue(iterationToRecord > 0 && iterationToRecord <= numberOfIterations, "iterationToRecord should be in the range of [1, numberOfIterations].");

            var file = _session.ReadFile(fileName);
            var context = _session.OpenContext(file);
            for (int i = 0; i < numberOfIterations; i++)
            {
                totalTime = 0;
                context.Update(out callTime);
                totalTime = callTime;
                var messages = context.GetMessages(out callTime);
                totalTime += callTime;

                if ((i + 1) == iterationToRecord) // 1 based count
                {
                    RecordTestTime(totalTime);
                    result.Append(DumpObject(messages));
                }
            }

            return result.ToString();
        }

        internal void PerformGetASTCursor(string fileName, int numberOfIterations, int iterationToRecord)
        {
            Int64 callTime;

            Assert.IsTrue(iterationToRecord > 0 && iterationToRecord <= numberOfIterations, "iterationToRecord should be in the range of [1, numberOfIterations].");

            var file = _session.ReadFile(fileName);
            var context = _session.OpenContext(file);
            for (int i = 0; i < numberOfIterations; i++)
            {
                var cursor = context.GetASTCursor(out callTime);
                if ((i + 1) == iterationToRecord) // 1 based count
                {
                    RecordTestTime(callTime);
                    Assert.IsNotNull(cursor);
                }
            }
        }

        internal IAuthorParseNodeSet[] PerformGetASTSubTree(string fileName, int numberOfIterations, int iterationToRecord)
        {
            HighResolutionTimer timer = new HighResolutionTimer();
            List<IAuthorParseNodeSet> result = new List<IAuthorParseNodeSet>();

            Assert.IsTrue(iterationToRecord > 0 && iterationToRecord <= numberOfIterations, "iterationToRecord should be in the range of [1, numberOfIterations].");

            var file = _session.ReadFile(fileName);
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            Assert.IsNotNull(cursor);

            for (int i = 0; i < numberOfIterations; i++)
            {
                timer.Start();
                var subtree = cursor.GetSubTree();
                timer.End();

                if ((i + 1) == iterationToRecord) // 1 based count
                {
                    result.Add(subtree);
                    RecordTestTime(timer.ElapseTimeInMilliseconds);
                }
            }
            return result.ToArray();
        }
    }

    public class Dev10PerformanceTests : PerformanceTests
    {
        internal string PerformDev10GetCompletions(string text)
        {
            StringBuilder result = new StringBuilder();
            HighResolutionTimer timer = new HighResolutionTimer();

            IActiveScriptAuthor jsAuthor;
            IScriptNode root;
            IScriptEntry entry;
            IScriptEntry2 entry2;
            uint listReq, listAnchor, funAncher;
            int memID, curParam;
            object data;
            IProvideMultipleClassInfo completionList;

            jsAuthor = new ScriptAuthor();
            jsAuthor.GetRoot(out root);
            root.CreateChildEntry(0, 0, null, out entry);
            entry2 = entry as IScriptEntry2;
            entry2.SetText(text);

            timer.Start();
            int hr = entry2.GetInfoFromContext((uint)text.Length,
               (uint)0x0001, out listReq, out listAnchor, out funAncher,
               out memID, out curParam, out data);
            timer.End();

            Assert.IsTrue(hr == 0);

            // retrieve completions
            completionList = data as IProvideMultipleClassInfo;
            ITypeInfo dispClass = null;
            IntPtr dispClassPtr;
            uint iter;
            //int count = 0;
            completionList.GetMultiTypeInfoCount(out iter);
            for (uint i = 0; i < iter; i++)
            {
                completionList.GetInfoOfIndex(i, 0, out dispClassPtr, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero, IntPtr.Zero);

                dispClass = (ITypeInfo)Marshal.GetObjectForIUnknown(dispClassPtr);
                IntPtr pTypeAttr;
                dispClass.GetTypeAttr(out pTypeAttr);
                System.Runtime.InteropServices.ComTypes.TYPEATTR typeAttr = (System.Runtime.InteropServices.ComTypes.TYPEATTR)
                Marshal.PtrToStructure(pTypeAttr, typeof(System.Runtime.InteropServices.ComTypes.TYPEATTR));
                dispClass.ReleaseTypeAttr(pTypeAttr);
                int numVars = typeAttr.cVars;
                int numFuncs = typeAttr.cFuncs;
                int numMembers = numVars + numFuncs;

                // variables
                for (int j = 0; j < numVars; j++)
                {
                    IntPtr pVars;
                    dispClass.GetVarDesc(j, out pVars);
                    System.Runtime.InteropServices.ComTypes.VARDESC vard = (System.Runtime.InteropServices.ComTypes.VARDESC)
                    Marshal.PtrToStructure(pVars, typeof(System.Runtime.InteropServices.ComTypes.VARDESC));
                    dispClass.ReleaseVarDesc(pVars);
                    string[] ns = new string[1];
                    int temp;
                    dispClass.GetNames(vard.memid, ns, 1, out temp);

                    result.AppendLine(FormateCompletion(ns[0], "Field"));
                }

                // functions
                for (int j = 0; j < numFuncs; j++)
                {
                    IntPtr pFunctions;
                    dispClass.GetFuncDesc(j, out pFunctions);
                    System.Runtime.InteropServices.ComTypes.FUNCDESC funcd = (System.Runtime.InteropServices.ComTypes.FUNCDESC)
                    Marshal.PtrToStructure(pFunctions, typeof(System.Runtime.InteropServices.ComTypes.FUNCDESC));
                    dispClass.ReleaseFuncDesc(pFunctions);

                    // Here, we skip restricted, hidden, property_putref
                    if ((funcd.wFuncFlags & (0x1 | 0x08 | 0x40)) != 0)
                    {
                        continue;
                    }

                    string[] ns = new string[1];
                    int temp;
                    dispClass.GetNames(funcd.memid, ns, 1, out temp);

                    string memberType ="Method";
                    /* check for property */
                    if ((funcd.invkind == System.Runtime.InteropServices.ComTypes.INVOKEKIND.INVOKE_PROPERTYGET)
                        || (funcd.invkind == System.Runtime.InteropServices.ComTypes.INVOKEKIND.INVOKE_PROPERTYPUT))
                    {
                        memberType = "Property";
                    }

                    result.AppendLine(FormateCompletion(ns[0], memberType));
                }
            }

            RecordTestTime(timer.ElapseTimeInMilliseconds);
            return result.ToString();
        }

        private string FormateCompletion(string name, string memberType)
        {
            return String.Format(@" {{ Name = ""{0}"" , MemberType = ""{1}"" }}", name, memberType);
        }

        internal string PerformDev10GetMessages(string text)
        {
            StringBuilder result = new StringBuilder();
            HighResolutionTimer timer = new HighResolutionTimer();
           
            IActiveScriptAuthor jsAuthor;
            IScriptNode root;
            IScriptEntry entry;
            IScriptEntry2 entry2;

            jsAuthor = new ScriptAuthor();
            jsAuthor.GetRoot(out root);
            root.CreateChildEntry(0, 0, null, out entry);
            entry2 = entry as IScriptEntry2;
            
            // record the time needed to load the text, as that is where the error checking routine is called
            timer.Start();
            entry2.SetText(text);
            timer.End();

            // retrieve all errors
            IScriptErrorEnumerator errorList;
            entry2.GetErrorEnumerator(out errorList);
             _SCRIPT_ERROR error;
             int errorResult = 0;
             while ((errorResult = errorList.GetNextError(out error)) == 0)
             {
                 result.Append(DumpObject(error));
             }

            RecordTestTime(timer.ElapseTimeInMilliseconds);
            return result.ToString();
        }
    }
}