using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class DirectAuthorTest
    {
        protected IAuthorTestSession _session;

        public class Paths
        {
            private static readonly string BuildArch = "x86";
#if DEBUG
            private static readonly string BuildType = "chk";
#else
            private static readonly string BuildType = "fre";
#endif

            private static readonly bool InRazzle = !String.IsNullOrEmpty(Environment.GetEnvironmentVariable(@"SDXROOT"));

            private static string GetBinariesPath()
            {
                string binDirBase = null;
                string binariesPath = null;

                if (InRazzle)
                {
                    var buildArch = Environment.GetEnvironmentVariable(@"build.arch");
                    if (buildArch != "x86")
                        Assert.Fail("DirectAuthorTest doesn't run on non-x86 arch");

                    var nttree = Environment.GetEnvironmentVariable(@"_NTTREE");
                    if (String.IsNullOrEmpty(nttree))
                        Assert.Fail("Couldn't set the binaries path. Please start the test environment through Razzle.");

                    // Just the the _NTTREE to find the binary to test     
                    binariesPath = nttree;
                }
                else 
                {
                    var currentDir = Assembly.GetExecutingAssembly().Location;
                    if (currentDir.IndexOf(".binaries.") > 0)
                        binDirBase = currentDir.Substring(0, currentDir.IndexOf(".binaries."));
                    else if (currentDir.IndexOf(@"\inetcore") > 0)
                        binDirBase = currentDir.Substring(0, currentDir.IndexOf(@"\inetcore"));
 
                    if (binDirBase != null)
                        binariesPath = binDirBase + ".binaries." + BuildArch + BuildType;
                    else
                        binariesPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                }
                Assert.IsTrue(Directory.Exists(binariesPath), String.Format("Could not find BinariesPath. Expected: '{0}'", binariesPath));
                System.Diagnostics.Debug.WriteLine(String.Format("# Binaries Path : '{0}'", binariesPath));
                return binariesPath;
            }

            private static string GetReferencesPath(string binariesPath)
            {
                if (String.IsNullOrEmpty(binariesPath))
                    Assert.Fail("Could not find ReferencesPath. Binaries path is invalid.");

                string referencesPath;

                if (InRazzle || binariesPath.IndexOf(".binaries.") > 0)
                    referencesPath = Path.Combine(binariesPath, @"jscript\References");
                else
                    referencesPath = binariesPath;

                Assert.IsTrue(Directory.Exists(referencesPath), String.Format("Could not find ReferencesPath. Expected: '{0}'", referencesPath));
                System.Diagnostics.Debug.WriteLine(String.Format("# References Path : '{0}'", referencesPath));
                return referencesPath;
            }

            protected static readonly string BinariesPath = GetBinariesPath();
            public static readonly string JScript9LSFilePath = Path.Combine(BinariesPath, @"chakrals.dll");
            public static readonly string ReferencesPath = GetReferencesPath(BinariesPath);
            public static readonly string LibHelpPath = Path.Combine(ReferencesPath, @"LibHelp.js");
            public static readonly string SiteTypesWebPath = Path.Combine(ReferencesPath, @"SiteTypesWeb.js");
            public static readonly string SiteTypesWindowsPath = Path.Combine(ReferencesPath, @"SiteTypesWindows.js");
            public static readonly string DomWebPath = Path.Combine(ReferencesPath, @"DomWeb.js");
            public static readonly string DomWindowsPath = Path.Combine(ReferencesPath, @"DomWindows_8.1.js");
            public static readonly string DedicatedWorkerPath = Path.Combine(ReferencesPath, @"DedicatedWorker.js");
            public static readonly string ShowPlainCommentsPath = Path.Combine(ReferencesPath, @"showPlainComments.js");
            public static readonly string JSDocExtensionPath = Path.Combine(ReferencesPath, @"JSDocExtension.js");

            public static readonly string LatestJQueryText = JQueryTestFiles.jquery_1_7;

            public class WinJs
            {
                public static readonly string BaseIntellisensePath = Path.Combine(ReferencesPath, @"base.intellisense.js");
                public static readonly string BaseIntellisenseSetupPath = Path.Combine(ReferencesPath, @"base.intellisense-setup.js");
            }
        }

        //[TestInitialize]
        public void Initialize()
        {
            _session = new AuthorTestSession(Directory.GetCurrentDirectory());
        }

        //[TestCleanup]
        public void Cleanup()
        {
            if (_session != null)
            {
                _session.Close();
                _session = null;
            }
        }

        protected struct Request
        {
            public int Offset;
            public string Data;
        }

        protected struct ParsedRequests
        {
            public string Text;
            public IEnumerable<Request> Requests;
        }

        // Names of additional context files
        private  string[] NamesOfAdditionalContextFile {
            get {
                return new string[] { Paths.LibHelpPath };
            }
        }

        // Contents of additional context files
        protected virtual string[] AdditionalContextFiles
        {
            get
            {
                var libhelp = _session.ReadFile(Paths.LibHelpPath).Text;

                return new string[] { libhelp };
            }
        }

        protected virtual ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();
            for (int i = 0; i < text.Length; i++)
            {
                var ch = text[i];

                // We should be able to check by the symbol structure too, ^|data| will allow that
                if (ch == '^' && i < text.Length - 1 && text[i+1] == '|')
                {
                    continue;
                }
                if (ch == '|'
                    // Don't consider " |" or "||" a cursor location because it is most likely the | or || operators.
                    && (i == 0 || (text[i - 1] != ' ' && text[i - 1] != '|')))
                {
                    string data = null;
                    var j = i + 1;
                    if (j < text.Length)
                    {
                        while (true)
                        {
                            ch = text[j++];
                            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch =='_' || ch == ',' || ch == '!' || ch == '@')
                                continue;
                            if (ch == '|')
                            {
                                data = text.Substring(i + 1, j - i - 2);
                                i = j - 1;
                            }
                            break;
                        }
                    }
                    requests.Add(new Request() { Offset = builder.Length, Data = data });
                }
                else
                    builder.Append(ch);
            }
            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }

        private string[] CombinedContextFiles(string[] contextFiles) {
            var allContextFiles = new List<string>(contextFiles);
            allContextFiles.AddRange(AdditionalContextFiles);
            return allContextFiles.ToArray();
        }

        private string[] NamesOfCombinedContextFiles(string[] contextFiles) {
            var allContextFiles = new List<string>(contextFiles);
            allContextFiles.AddRange(NamesOfAdditionalContextFile);
            return allContextFiles.ToArray();
        }

        protected void PerformRequests(string text, Action<IAuthorTestContext, int, string, int> action) {
            PerformRequests(text, action, true, new string[] {});
        }

        protected void PerformRequests(string text, Action<IAuthorTestContext, int, string, int> action, params string[] contextFiles)
        {
            PerformRequests(text, action, true, contextFiles);
        }

        protected void PerformRequests(string text, Action<IAuthorTestContext, int, string, int> action, bool createFilesFromText, params string[] contextFiles)
        {
            PerformRequests(text, AuthorHostType.ahtBrowser, action, createFilesFromText, contextFiles);
        }

        protected void PerformRequests(string text, AuthorHostType hostType, Action<IAuthorTestContext, int, string, int> action, bool createFilesFromText, params string[] contextFiles)
        {
            List<IAuthorTestFile> files = new List<IAuthorTestFile>();
            foreach (var fileName in NamesOfAdditionalContextFile)
            {
                files.Add(_session.ReadFile(fileName));
            }
            files.AddRange(contextFiles.Select(f => createFilesFromText ? _session.FileFromText(f) : _session.ReadFile(f)).ToArray());

            var offsets = ParseRequests(text);
            var file = _session.FileFromText(offsets.Text, "primaryFile.js");
            var context = _session.OpenContext(file, hostType, files.ToArray());
            var i = 0;
            foreach (var request in offsets.Requests)
            {
                action(context, request.Offset, request.Data, i++);
            }
        }

        internal void PerformCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, AuthorCompletionFlags flags = AuthorCompletionFlags.acfMembersFilter, params string[] contextFiles)
        {
            PerformCompletionRequests(text, action, flags, true, contextFiles);
        }

        internal void PerformCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, bool createFilesFromText, params string[] contextFiles)
        {
            PerformCompletionRequests(text, action, AuthorCompletionFlags.acfMembersFilter, createFilesFromText, contextFiles);
        }

        internal void PerformCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, AuthorCompletionFlags flags , bool createFilesFromText, params string[] contextFiles)
        {
            PerformRequests(text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, flags);
                Assert.IsNotNull(completions);

                var completionItems = completions.ToEnumerable();
                if (completionItems.Any(c => c.Kind != AuthorCompletionKind.ackLabel))
                {
                    // If this was not a label completion, ensure the successful execution flag is set. There should be no timeouts for
                    // perform requests cases, so we should always successfully execute to the breakpoint.
                    Assert.AreNotEqual(AuthorCompletionDiagnosticFlags.acdfNone, completions.DiagnosticFlags & AuthorCompletionDiagnosticFlags.acdfSuccessfulExecution);
                }

                if (data != null && data.Substring(0, 1) == "@")
                {
                    Assert.AreEqual(AuthorCompletionObjectKind.acokDynamic, completions.ObjectKind);
                    data = data.Substring(1);
                }

                action(completionItems, data, index);
            }, createFilesFromText, contextFiles);
        }

        internal void PerformCompletionRequests(string js, params string[] contextFiles)
        {
            PerformCompletionRequests(js, (completions, data, i) =>
            {
                var entries = data.Split(',');
                var shouldContain = HandleTypes(entries.Where(s => s.Length > 0 && s[0] != '!').ToArray());
                var shouldNotContain = HandleTypes(entries.Where(s => s.Length > 0 && s[0] == '!').Select(s => s.Substring(1)).ToArray());
                completions.ExpectContains(shouldContain);
                completions.ExpectNotContains(shouldNotContain);
            }, AuthorCompletionFlags.acfMembersFilter, contextFiles);
        }

        private static string[] HandleTypes(string[] values)
        {
            if (values != null && values.Length == 1)
                switch (values[0])
                {
                    case "Number": return NumberMethods;
                    case "String": return StringMethods;
                    case "Object": return ObjectMethods;
                    case "Array": return ArrayMethods;
                    case "Function": return FunctionMethods;
                    case "Boolean": return BooleanMethods;
                }
            return values;
        }

        //protected void PerformHurriedCompletionRequests(string text, params string[] contextFiles)
        //{
        //    PerformHurriedCompletionRequests(text, (completions, data, index) =>
        //    {
        //        var entries = data.Split(',');
        //        var shouldContain = HandleTypes(entries.Where(s => s.Length > 0 && s[0] != '!').ToArray());
        //        var shouldNotContain = HandleTypes(entries.Where(s => s.Length > 0 && s[0] == '!').Select(s => s.Substring(1)).ToArray());
        //        completions.ExpectContains(shouldContain);
        //        completions.ExpectNotContains(shouldNotContain);
        //    }, contextFiles);
        //}
        //protected void PerformHurriedCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, params string[] contextFiles)
        //{
        //    PerformHurriedCompletionRequests(text, action, true, contextFiles);
        //}

        //protected void PerformHurriedCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, bool createFilesFromText, params string[] contextFiles)
        //{
        //    Action CallPerformRequests = () =>
        //    {
        //        PerformRequests(text, (context, offset, data, index) =>
        //        {
        //            using (IDisposable hurry = ExecutionLimiter(context))
        //            {
        //                var completions = context.GetCompletionsAt(offset);
        //                action(completions.ToEnumerable(), data, index);
        //            }
        //        }, createFilesFromText, contextFiles);
        //    };

        //    WithMTASession(CallPerformRequests);
        //}

        //protected void PerformHurriedCompletionRequests(string text, Action<IAuthorCompletionSet, string> action, params string[] contextFiles)
        //{
        //    PerformHurriedCompletionRequests(text, action, true, contextFiles);
        //}

        //protected void PerformHurriedCompletionRequests(string text, Action<IAuthorCompletionSet, string> action, bool createFilesFromText, params string[] contextFiles)
        //{
        //    Action CallPerformRequests = () =>
        //    {
        //        PerformRequests(text, (context, offset, data, index) =>
        //        {
        //            using (IDisposable hurry = ExecutionLimiter(context))
        //            {
        //                var completions = context.GetCompletionsAt(offset);
        //                action(completions, data);
        //            }
        //        }, createFilesFromText, contextFiles);
        //    };

        //    WithMTASession(CallPerformRequests);
        //}

        protected void PerformParameterRequests(string text, Action<AuthorParameterHelp, string, int> action, int callHurryIn = 0)
        {
            bool enableHurry = callHurryIn != 0;
            Action CallPerformRequests = () =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    using (IDisposable hurry = (enableHurry ? CallHurryIn(context, callHurryIn) : null))
                    {
                        var help = context.GetParameterHelpAt(offset);
                        action(help, data, index);
                    }
                }, AdditionalContextFiles);
            };

            //if (enableHurry)
            //{
            //    WithMTASession(CallPerformRequests);
            //}
            //else
            //{
                CallPerformRequests();
            //}
        }

        //protected void WithMTASession(Action action)
        //{
        //    InMTA(() =>
        //    {
        //        Initialize();
        //        try
        //        {
        //            action();
        //        }
        //        finally
        //        {
        //            Cleanup();
        //        }
        //    });
        //}

        protected void EnsureMessageLogged(IAuthorTestContext context, string msg)
        {
            context.LoggedMessages.Count(m => m == msg).Expect(1);
        }

        protected void AssertAreStructurallyEqual(int[] expected, int[] actual)
        {
            var len = Math.Min(expected.Length, actual.Length);
            for (var i = 0; i < len; i++)
                Assert.AreEqual(expected[i], actual[i]);
            Assert.AreEqual(expected.Length, actual.Length);
        }

        protected void AssertAreStructurallyEqual(object expected, object actual, string logPrefix = "AssertAreStructurallyEqual:")
        {
            if (expected == null)
            {
                Assert.IsNull(actual, logPrefix + " Expected is null but Actual is {0}", actual);
                return;
            }

            var type = expected.GetType();
            if (type.IsEnum || type == typeof(string) || type == typeof(int) || type == typeof(double) || type == typeof(float) || type == typeof(long) || type == typeof(bool))
            {
                Assert.AreEqual(expected, actual, logPrefix + " Value mismatch");
            }
            else if (type.IsArray || expected is IEnumerable<object>)
            {
                var expectedEnum = (IEnumerable<object>)expected;
                var actualEnum = ((IEnumerable)actual).Cast<object>();
                int index = 0;

                foreach (var element in expectedEnum.Zip(actualEnum, (e, a) => new { Expected = e, Actual = a }))
                {
                    AssertAreStructurallyEqual(element.Expected, element.Actual, string.Format("{0}[{1}]", logPrefix, index));
                    index++;
                }

                Assert.AreEqual(expectedEnum.Count(), actualEnum.Count(), message: logPrefix + " Expected count of items is not the same as the actual count!");
            }
            else
            {
                Assert.IsNotNull(actual);
                
                var actualType = actual.GetType();
                
                foreach (var property in type.GetProperties())
                {
                    var actualField = actualType.GetField(property.Name);
                    Assert.IsNotNull(actualField, logPrefix + " Expected field not found on actual result. Field Name: {0}", property.Name);
                
                    try
                    {
                        AssertAreStructurallyEqual(property.GetValue(expected, null), actualField.GetValue(actual), string.Format("{0}.{1}", logPrefix, property.Name));
                    }
                    catch (AssertFailedException ex)
                    {
                        throw new AssertFailedException(
                            string.Format("{0} Error asserting equal values for property '{1}'. \n Expectation: \n\t {2}. \n Actual: \n\t {3}\n", logPrefix, property.Name, expected, actual),
                            ex);
                    }
                }
            }
        }

        protected static bool DumpObject(object value, StringBuilder builder, string indent, Type type = null, Func<string, bool> filter = null)
        {
            Func<string, bool> ShouldDumpMember = name => filter != null ? filter(name) : true;

            if (value == null)
            {
                builder.Append("null");
                return false;
            }
            bool lineEmitted = false;
            type = type ?? value.GetType();
            if (type.Name == "__ComObject")
            {
                type = ApiTypeOf(value) ?? type;
            }
            if (type == typeof(string))
                builder.AppendAsLiteral(value.ToString());
            else if (type.IsEnum)
            {
                builder.Append(value.GetType().Name);
                builder.Append('.');
                builder.Append(value.ToString());
            }
            else if (type == typeof(Int16) || type == typeof(Int32) || type == typeof(Int16) || type == typeof(Int64) || type == typeof(int) || type == typeof(uint) || type == typeof(double) || type == typeof(float) || type == typeof(long) || type == typeof(Boolean))
            {
                builder.Append(value.ToString());
            }
            else if (type == typeof(IntPtr) )
            {
                builder.Append("IntPtr");
            }
            else if (type.IsArray || EnumerateSet(value) != null)
            {
                var a = EnumerateSet(value);
                var elementType = ElementTypeOf(a);
                builder.Append("new [] { ");
                var first = true;
                foreach (var element in a)
                {
                    if (!first) builder.Append(", ");
                    first = false;
                    lineEmitted = DumpObject(element, builder, indent + "    ", elementType, filter) || lineEmitted;
                }
                if (lineEmitted)
                    builder.Append("\r\n" + indent + "}");
                else
                    builder.Append("}");
            }
            else
            {
                bool first = true;
                builder.Append("\r\n" + indent + "new { ");

                // Add fields
                var fields = type.GetFields();
                foreach (var field in fields)
                {
                    var fieldValue = field.GetValue(value);
                    if (fieldValue != null)
                    {
                        if (!ShouldDumpMember(field.Name))
                        {
                            continue;
                        }

                        if (!first) builder.Append(", ");
                        first = false;
                        builder.Append(field.Name);
                        builder.Append(" = ");
                        lineEmitted = DumpObject(fieldValue, builder, indent + "    ", field.FieldType, filter) || lineEmitted;
                    }
                }

               
                // Add properties
                var properties = type.GetProperties();
                foreach (var property in properties)
                {
                    if (property.CanRead)
                    {
                        var propertyValue = property.GetValue(value, null);
                        if (propertyValue != null) 
                        {
                            if (!ShouldDumpMember(property.Name))
                            {
                                continue;
                            }

                            if (!first) builder.Append(", ");
                            first = false;
                            builder.Append(property.Name);
                            builder.Append(" = ");
                            lineEmitted = DumpObject(propertyValue, builder, indent + "    ", property.PropertyType, filter) || lineEmitted;
                        }
                    }
                }

                // Add property like methods.
                var methods = type.GetMethods(BindingFlags.DeclaredOnly | BindingFlags.Instance | BindingFlags.Public);
                foreach (var method in methods)
                {
                    if (method.Name.StartsWith("Get") && method.GetParameters().Length == 0)
                    {
                        if (!ShouldDumpMember(method.Name))
                        {
                            continue;
                        }

                        var methodValue = method.Invoke(value, null);
                        if (methodValue != null)
                        {
                            if (!first) builder.Append(", ");
                            first = false;
                            builder.Append(method.Name.Substring(3));
                            builder.Append(" = ");
                            lineEmitted = DumpObject(methodValue, builder, indent + "    ", method.ReturnType, filter) || lineEmitted;
                        }
                    }
                }
                if (lineEmitted) builder.Append("\r\n" + indent + "}");
                else builder.Append(" }");
                lineEmitted = true;
            }
            return lineEmitted;
        }

        internal static Type ApiTypeOf(object value)
        {
            Type result = null;
            if (TryInterface<IAuthorFunctionHelp>(value, ref result))
                return result;
            return null;
        }

        internal static bool TryInterface<T>(object value, ref Type result) where T: class
        {
            if (value is T)
            {
                result = typeof(T);
                return true;
            }
            return false;
        }

        internal static IEnumerable EnumerateSet(object value)
        {
            var completionSet = value as IAuthorCompletionSet;
            if (completionSet != null) return completionSet.ToEnumerable();
            var messageSet = value as IAuthorMessageSet;
            if (messageSet != null) return messageSet.ToEnumerable();
            var regionSet = value as IAuthorRegionSet;
            if (regionSet != null) return regionSet.ToEnumerable();
            var parameterSet = value as IAuthorParameterSet;
            if (parameterSet != null) return parameterSet.ToEnumerable();
            var signatureSet = value as IAuthorSignatureSet;
            if (signatureSet != null) return signatureSet.ToEnumerable();
            var compatibleWithSet = value as IAuthorCompatibleWithSet;
            if (compatibleWithSet != null) return compatibleWithSet.ToEnumerable();
            return value as IEnumerable;
        }

        internal static Type ElementTypeOf(object value)
        {
            var type = value.GetType();
            if (type.IsArray)
                return type.GetElementType();
            foreach (var intf in type.GetInterfaces())
                if (intf.IsInstanceOfType(typeof(IEnumerable<>)))
                    return intf.GetGenericArguments()[0];
            return null;
        }

        public static string DumpObject(object obj, Func<string, bool> filter = null)
        {
            var builder = new StringBuilder();
            DumpObject(obj, builder, "", null, filter);
            return builder.ToString();
        }

        internal static IDisposable CallHurryIn(IAuthorTestContext context, int milliseconds, int phaseId = 0)
        {
            return new HurryCaller(context, milliseconds, phaseId);
        }

        private class HurryCaller : IDisposable
        {
            IAuthorTestContext _context;
            int _time;
            int _phaseId;
            Thread _worker;

            public HurryCaller(IAuthorTestContext context, int time, int phaseId)
            {
                _context = context;
                _time = time;
                _phaseId = phaseId;
                _worker = new Thread(Worker)
                {
                    Name = "Hurry",
                    IsBackground = true
                };
                _worker.SetApartmentState(ApartmentState.MTA);
                _worker.Start();
            }

            void Worker()
            {
                
                while (_context != null)
                {
                    Thread.Sleep(_time);
                    // Copy the value into a temporary to avoid a null exception when _context is nulled before the call to Hurry() is invoked.
                    var c = _context;
                    if (c != null)
                        c.Hurry(_phaseId);

                }
            }

            public void Dispose()
            {
                _context = null;
                _worker.Join();
            }
        }

        public interface IPhaseReporter : IDisposable
        {
            int ExecuteCount { get; }
        }

        public static IPhaseReporter ExecutionLimiter(IAuthorTestContext context, int? timeout = null)
        {
            int HurryTimeout = timeout ?? 100;
#if DEBUG
            HurryTimeout = HurryTimeout * 5;
#endif

            return new ExecutionLimiterCaller(context, HurryTimeout);
        }

        private class ExecutionLimiterCaller : IPhaseReporter
        {
            IAuthorTestContext _context;
            int _time;
            int _executeCount;
            HurryCaller _caller;

            public ExecutionLimiterCaller(IAuthorTestContext context, int time)
            {
                _context = context;
                if (context != null)
                    context.PhaseChanged += PhaseChanged;
                _time = time;
            }

            public int ExecuteCount { get { return _executeCount; } }

            public void PhaseChanged(AuthorFileAuthoringPhase phase, int phaseId)
            {
                switch (phase)
                {
                    case AuthorFileAuthoringPhase.afpExecuting:
                        StartCaller(phaseId);
                        _executeCount++;
                        break;
                    default:
                        StopCaller();
                        break;
                }
            }

            public void Dispose()
            {
                StopCaller();
                var context = _context;
                if (context != null)
                    context.PhaseChanged -= PhaseChanged;
                _context = null;
            }

            private void StartCaller(int phaseId)
            {
                if (_caller == null)
                    _caller = new HurryCaller(_context, _time, phaseId);
            }

            private void StopCaller()
            {
                var caller = _caller;
                _caller = null;
                if (caller != null) caller.Dispose();
            }
        }

        static ManualResetEvent sync = new ManualResetEvent(false);
        static Queue<Action> work;

        public static void InMTA(Action action)
        {
            Exception e = null;
            work = new Queue<Action>();
            Thread mtaThread = new Thread(MTAWorker);
            mtaThread.SetApartmentState(ApartmentState.MTA);
            mtaThread.Start();

            if (work != null)
            {
                var waiter = new ManualResetEvent(false);
                lock (work)
                    work.Enqueue(() =>
                    {
                        try
                        {
                            try
                            {
                                action();
                            }
                            catch (Exception ex)
                            {
                                e = ex;
                            }
                        }
                        finally
                        {
                            waiter.Set();
                        }
                    });
                sync.Set();
                waiter.WaitOne();
                if (e != null) throw e;
            }

            CloseMTA(); 
            mtaThread.Join();
            mtaThread = null;
       }

        public static void CloseMTA()
        {
            work = null;
            sync.Set();
        }

        static void MTAWorker()
        {
            while (work != null)
            {
                sync.WaitOne(500);
                var lwork = work;
                if (lwork == null) break;
                while (true)
                {
                    Action action = null;
                    lock (lwork)
                    {
                        action = lwork.Count > 0 ? lwork.Dequeue() : null;
                    }
                    if (action != null)
                        action();
                    else
                        break;
                }
            }
        }

        public static readonly string[] NumberMethods = new[]
        {
            "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toExponential", "toFixed", "toLocaleString", "toPrecision", "toString", "valueOf"
        };

        public static readonly string[] StringMethods = new[]
        {
            "anchor", "big", "blink", "bold", "charAt", "charCodeAt", "concat", "constructor", "fixed", "fontcolor", "fontsize", "hasOwnProperty", "indexOf", "isPrototypeOf", "italics", "lastIndexOf", "length", "link", "localeCompare", "match", "propertyIsEnumerable", "replace", "search", "slice", "small", "split", "strike", "sub", "substr", "substring", "sup", "toLocaleLowerCase", "toLocaleString", "toLocaleUpperCase", "toLowerCase", "toString", "toUpperCase", "valueOf"
        };

        public static readonly string[] ObjectMethods = new[]
        {
            "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] ArrayMethods = new[]
        {
            "concat", "constructor", "hasOwnProperty", "isPrototypeOf", "join", "length", "pop", "propertyIsEnumerable", "push", "reverse", "shift", "slice", "sort", "splice", "toLocaleString", "toString", "unshift", "valueOf"
        };

        public static readonly string[] FunctionMethods = new[]
        {
            "apply", "bind", "call", "constructor", "hasOwnProperty", "isPrototypeOf", "length", "propertyIsEnumerable", "prototype", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] BoundFunctionMethods = new[]
        {
            "apply", "bind", "call", "constructor", "hasOwnProperty", "isPrototypeOf", "length", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] Globals = new[]
        {
            "String", "NaN", "Infinity", "undefined", "eval", "parseInt", "parseFloat", "isNaN", "isFinite", "decodeURI", "decodeURIComponent", "encodeURI", "encodeURIComponent", "escape", "unescape", "ScriptEngine", "ScriptEngineMajorVersion", "ScriptEngineMinorVersion", "ScriptEngineBuildVersion", "CollectGarbage", "Object", "Array", "Boolean", "Date", "Function", "Math", "Debug", "Number", "RegExp", "Error", "EvalError", "RangeError", "ReferenceError", "SyntaxError", "TypeError", "URIError" 
        };

        public static readonly string[] RegExpConstructorMethods = new[]
        {
            "length", "prototype", "input", "lastMatch", "lastParen", "leftContext", "rightContext", "index", "constructor", "apply", "bind", "call", "toString", "caller", "arguments", "hasOwnProperty", "propertyIsEnumerable", "isPrototypeOf", "toLocaleString", "valueOf"
        };

        public static readonly string[] RegExpMethods = new[]
        {
            "compile", "constructor", "exec", "global", "ignoreCase", "lastIndex", "multiline", "source", "test"
        };

        public static readonly string[] BooleanMethods = new[]
        {
            "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] IntlMethods = new[]
        {
            "Collator", "NumberFormat", "DateTimeFormat", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] IntlObjectConstructorMethods = new[]
        {
            "supportedLocalesOf", "length", "prototype", "constructor", "apply", "bind", "call", "toString", "caller", "arguments", "hasOwnProperty", "propertyIsEnumerable", "isPrototypeOf", "toLocaleString", "valueOf"
        };

        public static readonly string[] CollatorMethods = new[]
        {
            "compare", "resolvedOptions", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] IntlFormatObjectMethods = new[]
        {
            "format", "resolvedOptions", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] MapMembers = new[]
        {
            "clear", "delete", "forEach", "get", "has", "set", "size", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] WeakMapMembers = new[]
        {
            "delete", "get", "has", "set", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };

        public static readonly string[] SetMembers = new[]
        {
            "add", "clear", "delete", "forEach", "has", "size", "constructor", "hasOwnProperty", "isPrototypeOf", "propertyIsEnumerable", "toLocaleString", "toString", "valueOf"
        };
    }
}
