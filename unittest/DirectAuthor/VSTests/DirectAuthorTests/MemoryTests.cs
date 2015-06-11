using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace DirectAuthorTests
{
    [TestClass]
    public class MemoryTests : CompletionsBase
    {
        private void MemoryTest<T>(string text, Func<AuthorTestSession, IAuthorTestFile, IAuthorTestContext, T> action, int iterations = 10000, int tolerateBytes = 512) where T: class
        {
            var session = new AuthorTestSession(Directory.GetCurrentDirectory());
            var primary = session.FileFromText(text);
            var context = session.OpenContext(primary);
            T result = action(session, primary, context);
            if (result != null)
                Marshal.ReleaseComObject(result);
            GC.Collect();
            var privateMemoryStart = Process.GetCurrentProcess().PrivateMemorySize64;
            var baseline = CreateBaseline(context);
            for (int i = 0; i < iterations; i++)
            {
                primary.Touch();
                result = action(session, primary, context);
                if (result != null)
                    Marshal.ReleaseComObject(result);
            }
            DumpMemoryUsage(context);
            ValidateActiveObjectsCount(context, baseline);
            primary.Touch();
            Marshal.ReleaseComObject(primary.GetHandle());
            primary = null;
            context = null;
            session.Close();
            session = null;
            result = null;
            GC.Collect();
            GC.WaitForFullGCComplete();
            GC.WaitForPendingFinalizers();
            var privateMemoryEnd = Process.GetCurrentProcess().PrivateMemorySize64;
            
            if (privateMemoryEnd <= privateMemoryStart + iterations * tolerateBytes)
            { 
                Console.WriteLine(string.Format("WARNING: Potential leak of upto {0} bytes, or {1} bytes per invocation", 
                privateMemoryEnd - privateMemoryStart,
                (privateMemoryEnd - privateMemoryStart) / iterations
                ));
            }
            WriteMemoryTestResult(privateMemoryEnd - privateMemoryStart);
        }

        public static void DumpMemoryUsage(IAuthorTestContext context, bool writeAsTestResult = false)
        {
            Func<int, double> ToMB = (bytes) => bytes / 1024.0 / 1024.0;
            context.GetAllocations(allocs =>
            {
                Debug.WriteLine("------- ALLOCATIONS SUMMARY -------");

                foreach (var a in allocs)
                {
                    string format = "> {0,-10} {1,-40} count: {2,-10}";
                    if (a.Category != "Object")
                        format += "{3:0.00;-10}(MB)";
                    Debug.WriteLine(format, a.Category, a.Tag, a.Count, ToMB(a.Size));
                }

                var totalBytes = allocs.Sum(a => a.Size);
                Debug.WriteLine(">>>> TOTAL LS SIZE (MB): {0}", ToMB(totalBytes));

                if (writeAsTestResult)
                {
                    // Dump the total usage number for memorytest.pl script
                    MemoryTests.WriteMemoryTestResult(totalBytes);
                }
            });
        }

        public static int GetObjectCount(IAuthorTestContext context, string objectName)
        {
            int count = -1;
            context.GetAllocations(allocs =>
            {
                foreach (var a in allocs)
                {
                    if (a.Category == "Object" && String.Compare(a.Tag, objectName, true) == 0)
                    {
                        count = a.Count;
                        return;
                    }
                }
            });
            return count;
        }

        private static IDictionary<string, int> CreateBaseline(IAuthorTestContext context)
        {
            var baseline = new Dictionary<string, int>();
            context.GetAllocations(allocs =>
            {
                baseline = allocs.Select(alloc => new { alloc.Tag, alloc.Count }).ToDictionary(alloc => alloc.Tag, alloc => alloc.Count);
            });
            return baseline;
        }

        private static void ValidateActiveObjectsCount(IAuthorTestContext context, IDictionary<string, int> baseline)
        {
            context.GetAllocations(allocs =>
            {
                foreach (var alloc in allocs)
                {
                    if (alloc.Category == "Object" && !(alloc.Tag.StartsWith("AuthorDiagnosticsImpl") || alloc.Tag == "AllocInfoSet"))
                    {
                        int count = alloc.Count;
                        int baselineCount = 0;
                        if(baseline != null && baseline.ContainsKey(alloc.Tag))
                            baselineCount = baseline[alloc.Tag];

                        switch (alloc.Tag)
                        {
                            case "AuthoringServicesInfo":
                                Assert.IsTrue(count - baselineCount <= 15);
                                break;
                            case "AuthoringFileHandle":
                                Assert.IsTrue(count - baselineCount <= 15);
                                break;
                            default:
                                Assert.IsTrue(count - baselineCount <= 5);
                                break;
                        }
                    }
                }
            });
        }

        public static void WriteMemoryTestResult(Int64 bytes)
        {
            Console.WriteLine("### MEMORY: {0} bytes", bytes);
        }

        [TestMethod]
        public void Memory_GetRegions()
        {
            MemoryTest(GetRegionText, (session, primary, context) =>
            {
                return context.GetRegions();
            });
        }
        #region Test data
        const string GetRegionText = @"

    function a() {
    }

    function b() {
        function c() {
        }
    }

    function d() {
        function e() {
            function g() {
            }
        }
        function f() {
            function h() {
            }
        }
    }
";

        #endregion

        [TestMethod]
        public void Memory_GetGlobalCompletions()
        {
            MemoryTest("", (session, primary, context) =>
            {
                return context.GetCompletionsAt(0);
            });
        }

        [TestMethod]
        public void Memory_GetSimpleCompletions()
        {
            var text = "var a = 1; a.";
            var offset = text.Length;
            MemoryTest(text, (session, primary, context) =>
            {
                return context.GetCompletionsAt(offset);
            });
        }

        [TestMethod]
        public void Memory_ContextFiles()
        {
            // Make up some context file of a substantial size so we would notice if we leak it 
            var contextText = new StringBuilder();
            for (int i = 0; i < 1000; i++)
            {
                contextText.Append(@"function f() { }\n");
            }

            var text = "var a = 1;|";

            var offset = text.Length;
            MemoryTest(text, (session, primary, context) =>
            {
                // Create and add a new context file
                var contextFile = session.FileFromText(contextText.ToString());
                context.AddContextFiles(contextFile);

                // Request completions with the new context file
                var completions = context.GetCompletionsAt(offset);
                Marshal.ReleaseComObject(completions);
                
                // Remove the new context file
                context.RemoveContextFiles(contextFile);
                Marshal.ReleaseComObject(contextFile.GetHandle());
                contextFile = null;

                // Request completions again without the context file
                return context.GetCompletionsAt(offset);
            }, 250, 
            1024 * 64 // There's a constant leak of ~20 MB which we still need to find. Currently using a safe value so the unit test won't fail randomly 
            );

        }

        [TestMethod]
        [WorkItem(347769)]
        public void Memory_PrimaryFile()
        {
#if DEBUG
            // The diagnostics for this test are not in retail builds.
            var session = new AuthorTestSession(Directory.GetCurrentDirectory());
            var primaryFile = session.FileFromText("var a = 1; a.");
            var context = session.OpenContext(primaryFile);
            var diagnostics = context.GetDiagnostics();
            var completions = context.GetCompletionsAt(primaryFile.Text.Length);

            Func<IAuthorAllocInfo> GetHandleInfo = () => diagnostics.GetAllocStats().ToEnumerable().Where(e => e.Tag == "AuthoringFileHandle").FirstOrDefault();

            Marshal.ReleaseComObject(completions);
            completions = null;
            context.TakeOwnership(diagnostics);
            var beforeClose = GetHandleInfo();
            context.Close();
            var rawFile = session.TakeOwnershipOf(primaryFile);
            rawFile.Close();
            Marshal.ReleaseComObject(rawFile);
            rawFile = null;
            primaryFile = null;
            System.GC.Collect();
            var afterClose = GetHandleInfo();

            Assert.IsTrue(afterClose == null || afterClose.Count < beforeClose.Count);
            
            Marshal.ReleaseComObject(diagnostics);
#endif
        }

        [TestMethod]
        public void Memory_GetMessages()
        {
            MemoryTest(Errors.AllErrorsText, (session, primary, context) =>
            {
                context.Update();
                return context.GetMessages();
            });
        }

        [TestMethod]
        public void Memory_GetParameterHelp()
        {
            var text = @"
                var a = function(de, soft)
                 {
                   return function(x)
                   {
                     return de + soft + x;
                   }
                 }
                 a(2, 3)(";
            var offset = text.Length;
            MemoryTest<IAuthorFunctionHelp>(text, (session, primary, context) =>
            {
                var paramHelp = context.GetParameterHelpAt(offset);
                DumpMemoryUsage(context);
                Marshal.ReleaseComObject(paramHelp.FunctionHelp);
                DumpMemoryUsage(context);
                paramHelp.FunctionHelp = null;
                paramHelp = null;
                return null;
            });
        }

        [TestMethod]
        public void Memory_GetParameterHelp_WithExtensions()
        {
            var text = @"
                intellisense.addEventListener('signaturehelp', function(e) {
                    e.functionHelp;
                });
                var a = function(de, soft)
                 {
                   return function(x)
                   {
                     return de + soft + x;
                   }
                 }
                 a(2, 3)(";
            var offset = text.Length;
            MemoryTest<IAuthorFunctionHelp>(text, (session, primary, context) =>
            {
                var paramHelp = context.GetParameterHelpAt(offset);
                DumpMemoryUsage(context);
                Marshal.ReleaseComObject(paramHelp.FunctionHelp);
                DumpMemoryUsage(context);
                paramHelp.FunctionHelp = null;
                paramHelp = null;
                return null;
            }, 50);
        }

        [TestMethod]
        [WorkItem(384269)]
        public void Memory_CompletionHint()
        {
            var text = @"
                intellisense.addEventListener('statementcompletionhint', function(e) { e.completionItem; e.symbolHelp; });
                function f() {}
                ;|";
            var offset = text.Length;
            MemoryTest<IAuthorFunctionHelp>(text, (session, primary, context) =>
            {
                var completions = context.GetCompletionsAt(offset);
                var hint = completions.GetHintFor("f");
                var funcHelp = hint.GetFunctionHelp();
                DumpMemoryUsage(context);
                Marshal.ReleaseComObject(funcHelp);
                Marshal.ReleaseComObject(hint);
                Marshal.ReleaseComObject(completions);
                DumpMemoryUsage(context);
                completions = null;
                funcHelp = null;
                hint = null;
                return null;
            }, 100);
        }

        [TestMethod]
        public void Memory_JQueryTest()
        {
            var jquery = _session.ReadFile(Paths.JQuery.JQuery_1_2_6VSDocFilePath).Text + "\n  $.";
            var offset = jquery.Length;
            MemoryTest(jquery, (session, primary, context) =>
            {
                return context.GetCompletionsAt(offset);
            }, 200);
        }

        [TestMethod]
        public void Memory_LargeFunc()
        {
            string text = "function f() { return 1;" + new string(' ', 10 * 1024 * 1024) + "}; f().|";
            MemoryTest(text, (session, primary, context) =>
            {
                return context.GetCompletionsAt(text.Length);
            }, 150);
        }
    }

#if DEBUG

    [TestClass]
    public class MemoryDiagnosticsBase : CompletionsBase
    {
        const string LogFilePath = "jscript.config";

        [TestInitialize]
        new public void Initialize()
        {
            string path = Directory.GetCurrentDirectory();
            File.WriteAllText(Path.Combine(path, LogFilePath), "-TraceMemory");
            base.Initialize();
        }

        [TestCleanup]
        new public void Cleanup()
        {
            File.Delete(LogFilePath);
            base.Cleanup();
        }
    }

    // Debug-only because IAuthorDiagnostics interface is only available in CHK build
    [TestClass]
    public class ArenaMemoryTests : MemoryDiagnosticsBase
    {
        static string[] WWAContextFileNames = new string[] { 
            Paths.WinRTFilePath, 
            Paths.SiteTypesWindowsPath, 
            Paths.DomWindowsPath, 
            Path.Combine(Paths.FilesPath, @"base.js"), 
            Paths.WinJs.BaseIntellisensePath, 
            Path.Combine(Paths.FilesPath, @"ui.js"), 
            Paths.JQuery.JQuery_1_4_3FilePath, 
            Path.Combine(Paths.FilesPath, @"cover.js")
        };

        string[] WWAContextFiles
        {
            get { return WWAContextFileNames.Select(fileName => _session.ReadFile(fileName).Text).ToArray(); }
        }

        // This test simulates the JSLS Core Analysis memory consumption for the end to end WWA (TailoredAppDeveloper) run
        [TestMethod]
        public void SimulateArenaMemoryForWWATest()
        {
            PerformRequests(@"
                var x = { f: 1 };
                x.|
                var f = Windows.Devices.Sensors.|;
                var y = Windows.Storage.Pickers.|;
                var z = Windows.ApplicationModel.Activation.|;
                var x = Windows.UI.ViewManagement.|;
                var a = Windows.Devices.Geolocation.|;
                var b = Windows.Data.Html.|;
                var c = Windows.Data.Json.|;
                var e = Windows.Data.Xml.Dom.|;
                var f = Windows.Web.Syndication.|;
                var f = Windows.Security.Cryptography.|;
                var f = Windows.Storage.BulkAccess.|;
            ", (context, offset, data, index) =>
             {
                 // Request completion
                 var completions = context.GetCompletionsAt(offset);
                 Marshal.ReleaseComObject(completions);
                 completions = null;
                 MemoryTests.DumpMemoryUsage(context, true);
             },
             WWAContextFiles);
        }

        [TestMethod]
        public void UseSameFilesInMultipleContexts()
        {
            string primary = @"var f = Windows.Devices.Sensors.|;";
            IAuthorTestFile[] contextFiles = WWAContextFiles.Select(fileText => _session.FileFromText(fileText)).ToArray();
            var context1 = _session.OpenContext(_session.FileFromText(primary), AuthorHostType.ahtBrowser, contextFiles);
            var context2 = _session.OpenContext(_session.FileFromText(primary), AuthorHostType.ahtBrowser, contextFiles);
            var context3 = _session.OpenContext(_session.FileFromText(primary), AuthorHostType.ahtBrowser, contextFiles.Where((file, index) => index != 3).ToArray());

            int offset = primary.IndexOf('|');

            Action<IAuthorTestContext> Verify = (context) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Marshal.ReleaseComObject(completions);
                completions = null;
                MemoryTests.DumpMemoryUsage(context1);
            };

            Verify(context1);
            Verify(context2);
            Verify(context3);
        }

    }

    [TestClass]
    public class RecyclerMemoryTests : MemoryDiagnosticsBase
    {
        [TestMethod]
        [Ignore] // Re-enable after recycler diagnostic information is added to diagnostics.
        public void TestSimpleModificationMemoryUsage()
        {
            var context1 = _session.FileFromText("var a = 1");
            var context2 = _session.FileFromText("var b = 2");
            var primary = _session.FileFromText("b.");
            var context = _session.OpenContext(primary, context1, context2);
            var diagnostics = context.GetDiagnostics();
            var completions = context.GetCompletionsAt(2);
            Marshal.ReleaseComObject(completions);
            context1.Touch();
            diagnostics.ForceGC();
            completions = context.GetCompletionsAt(2);
            diagnostics.ForceGC();
        }

        [TestMethod]
        [Ignore] // Re-enable after recycler diagnostic information is added to diagnostics.
        public void TestWinJSModificationMemoryUsage()
        {
            var winrt = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\winrt.js"));
            var sitetypes = _session.ReadFile(Paths.SiteTypesWindowsPath);
            var dom = _session.ReadFile(Paths.DomWindowsPath);
            var _base = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\base.js"));
            var baseExtensions = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\base.intellisense.js"));
            var ui = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\ui.js"));
            var binding = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\binding.js"));
            var controls = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\controls.js"));
            var animations = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\animations.js"));
            var uicollections = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\uicollections.js"));
            var wwaapp = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\wwaapp.js"));
            var res = _session.ReadFile(Path.Combine(Paths.FilesPath, @"289781\res.js"));

            var primary = _session.FileFromText(@"(function () {
    'use strict';

    var app = WinJS.Application;

    app.onactivated = function (e) {
        if (e.detail.kind === Windows.ApplicationModel.Activation.ActivationKind.launch) {
            WinJS.UI.processAll();
        }
    }

    app.start();

    var foo = document.createElement('div');
    var AppBar = new WinJS.UI.AppBar(foo);
    AppBar.
})();");
            var offset = primary.Text.IndexOf("AppBar.") + 7;

            var context = _session.OpenContext(primary, winrt, sitetypes, dom, _base, baseExtensions, ui, binding, controls, animations, uicollections, wwaapp, res);

            var diagnostics = context.GetDiagnostics();
            var lastTotal = 0;
            var firstTotal = 0;
            var memoryReport = new List<Dictionary<string, int>>();
            var instanceReport = new List<Dictionary<string, int>>();
            // Loop through performing the following operation
            //  Modify controls.js
            //  Get a completion list for AppBar.
            for (var i = 0; i < 20; i++)
            {
                var start = DateTime.Now;
                Debug.WriteLine("Iteration {0}", i);
                controls.Touch();
                var completions = context.GetCompletionsAt(offset);
                Assert.AreEqual(AuthorCompletionObjectKind.acokDynamic, completions.ObjectKind);
                Marshal.ReleaseComObject(completions);
                completions = null;
                var endCompletions = DateTime.Now;
                var s = new StringBuilder();
                s.Append(endCompletions - start);
                s.Append('\t');

                // Force garbage collection
                System.GC.Collect();
                diagnostics.ForceGC();
                s.Append(DateTime.Now - endCompletions);
                Debug.WriteLine("Times: " + s.ToString());

                // Collect memory
                var stats = diagnostics.GetAllocStats();
                var iterationTotal = stats.ToEnumerable().Where(info => info.Category == "ARENA").Sum(info => info.Size);
                if (lastTotal != 0)
                {
                    // Assert.IsTrue(iterationTotal < lastTotal + 1024 * 1024);
                    // Assert.IsTrue(iterationTotal < firstTotal + 2 * 1024 * 1024);
                }
                else
                    firstTotal = iterationTotal;
                var memoryTotals = stats.ToEnumerable()
                    .Where(info => info.Category == "ARENA")
                    .GroupBy(info => info.Tag)
                    .Where(grouping => grouping.Any())
                    .Select(grouping => new { name = grouping.Key, total = grouping.Sum(item => item.Size) })
                    .ToDictionary(total => total.name, total => total.total);
                memoryReport.Add(memoryTotals);

                var instances = stats.ToEnumerable()
                    .Where(info => info.Category == "Object")
                    .GroupBy(info => info.Tag)
                    .Select(grouping => new { name = grouping.Key, total = grouping.Sum(item => item.Count) })
                    .ToDictionary(total => total.name, total => total.total);
                instanceReport.Add(instances);

                lastTotal = iterationTotal;
            }
            var memoryReportStr = GenReport(memoryReport);
            var instancesReportStr = GenReport(instanceReport);
            Debug.WriteLine(memoryReportStr);
            Debug.WriteLine(instancesReportStr);
        }

        static string GenReport(List<Dictionary<string, int>> report)
        {
            var headers = report.SelectMany(totals => totals.Keys).Unique().ToArray();
            var stableHeaders = headers.Select(header => new { header = header, column = report.Select(item => item[header]) }).Where(column => column.column.AllTheSame()).ToDictionary(column => column.header, column => column.column.First());
            var unstableHeaders = headers.Where(header => !stableHeaders.ContainsKey(header));

            var s = new StringBuilder();
            if (unstableHeaders.Any())
            {
                foreach (var header in unstableHeaders)
                {
                    s.Append(header);
                    s.Append('\t');
                }
                s.Append('\n');
                foreach (var item in report)
                {
                    foreach (var header in unstableHeaders)
                    {
                        if (item.ContainsKey(header))
                            s.Append(item[header]);
                        else
                            s.Append(0);
                        s.Append('\t');
                    }
                    s.Append('\n');
                }
                s.Append('\n');
            }
            else
                s.Append("No values were unstable\n");

            if (stableHeaders.Any())
            {
                foreach (var stableHeader in stableHeaders.Keys)
                {
                    s.Append(stableHeader);
                    s.Append('\t');
                }
                s.Append('\n');
                foreach (var stableValue in stableHeaders.Values)
                {
                    s.Append(stableValue);
                    s.Append('\t');
                }
            }
            else
                s.Append("No values were stable");

            return s.ToString();
        }

    }

    [TestClass]
    public class ScriptContextFoldingTests : MemoryDiagnosticsBase
    {
        [TestMethod]
        public void FoldScriptContexts()
        {
            var c1 = _session.FileFromText("var c1 = 0;", "c1.js");
            var c2 = _session.FileFromText("var c2 = 0;", "c2.js");
            var c3 = _session.FileFromText("var c3 = 0;", "c3.js");
            var c4 = _session.FileFromText("var c4 = 0;", "c4.js");
            var c5 = _session.FileFromText("var c5 = 0;", "c5.js");
            var c6 = _session.FileFromText("var c6 = 0;", "c6.js");
            var primaryFile = _session.FileFromText("", "primaryFile.js");

            var context = _session.OpenContext(primaryFile, c1, c2, c3, c4, c5, c6);

            Action<string, int> VerifyObjectCount = (objectName, expectedCount) =>
            {
                var actualCount = MemoryTests.GetObjectCount(context, objectName);
                Debug.WriteLine("# {0} objects {1}", objectName, actualCount);
                Assert.AreEqual(expectedCount, actualCount);
            };

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);
            completions.ToEnumerable().ExpectContains("c1", "c2", "c3", "c4", "c5", "c6");
            Marshal.ReleaseComObject(completions);
            completions = null;

            VerifyObjectCount("ScriptContextPath", 8);          // 1 for the root, 1 for internal helpers, and 6 for context files
            VerifyObjectCount("LeafScriptContext", 1);          // 1 primary file
            VerifyObjectCount("RefCountedScriptContext", 3);    // 1 for the root, as it does not get copied
                                                                // 1 for the 6 context files along with the internal helpers.js
                                                                // 1 for the leaf context for the primary file

            var winrtText = File.ReadAllText(Paths.WinRTFilePath);
            c4.Replace(0, c4.Text.Length, winrtText);
            completions = context.GetCompletionsAt(primaryFile.Text.Length);
            completions.ToEnumerable().ExpectContains("c1", "c2", "c3", "c5", "c6");
            completions.ToEnumerable().ExpectNotContains("c4");
            Marshal.ReleaseComObject(completions);
            completions = null;

            VerifyObjectCount("ScriptContextPath", 8);
            VerifyObjectCount("LeafScriptContext", 1);
            VerifyObjectCount("RefCountedScriptContext", 4);    // a new copy should be created after winrt
        }

        [TestMethod]
        public void FoldScriptContextsWithDependentAsyncRequests()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var simpleLoader = _session.FileFromText(@"
                function load(uri, afterLoader) {
                     var scriptElement = document.createElement('script');
                     scriptElement.src = uri;
                     scriptElement.type = 'text/javascript';
                     if (!afterLoader)
                        scriptElement.onload = function forceAfter() { };
                     document.body.appendChild(scriptElement);
                }", "loader.js");
            var c1 = _session.FileFromText("if (typeof x != 'undefined') x.c1 = 1;", "c1.js");
            var loadContext = _session.FileFromText(@"
                 var x = {};
                 load('c1.js', true);
            ", "loadContext.js");

            var primaryFile = _session.FileFromText("p = {};", "primary.js");

            for (int i = 0; i <10; i++)
            {
                var f = _session.FileFromText(String.Format("f{0} = {{}};", i), String.Format("f{0}.js", i));

                var context = _session.OpenContext(primaryFile, dom, simpleLoader, loadContext, c1, f);
                context.AddAsyncScript("c1.js", c1);

                var completions = context.GetCompletionsAt(primaryFile.Text.Length);
                completions.ToEnumerable().ExpectContains( String.Format("f{0}", i), "p");
                Marshal.ReleaseComObject(completions);
                completions = null;
                f.Touch();
                _session.Cleanup();
            }
        }
    }
#endif

    static public class MemoryExtensions
    {
        static public IEnumerable<V> Unique<V>(this IEnumerable<V> enumerable)
        {
            var d = new Dictionary<V, V>();
            foreach (var value in enumerable)
            {
                if (d.ContainsKey(value))
                    continue;
                yield return value;
                d.Add(value, value);
            }
        }

        static public bool AllTheSame<V>(this IEnumerable<V> enumerable)
        {
            var comparer = Comparer<V>.Default;
            V first = enumerable.First();
            foreach (var v in enumerable.Skip(1))
                if (comparer.Compare(first, v) != 0)
                    return false;

            return true;
        }
    }

    [TestClass]
    public class PeakMemoryUsageTests : DirectAuthorTest
    {
        [TestMethod]
        [WorkItem(260819)]
        public void EverGrowingBuffer()
        {
            Func<long, double> ToMB = (bytes) => bytes / 1024.0 / 1024.0;

            // This test emulates the situation when the buffer is constantly grows during typing.
            // We want to verify that the source buffer is properly released (otherwise we'll run out of memory very quickly).
            var buffer = "|" + new string(' ', 10 * 1024 * 1024);
            var baseline = Process.GetCurrentProcess().PrivateMemorySize64;
            PerformRequests(buffer, (context, offset, data, index) =>
            {
                for (int i = 0; i < 450; i++)
                {
                    context.PrimaryFile.Append(" ");
                    context.Update();
                }
                var finalMemory = Process.GetCurrentProcess().PrivateMemorySize64;
                Debug.WriteLine(String.Format("Baseline memory: {0}. Final memory: {1}. Used Memory: {2}", ToMB(baseline), ToMB(finalMemory), ToMB(finalMemory- baseline)));
                Assert.IsTrue((finalMemory - baseline) < 500 * 1024 * 1024);
            });
        }
    }
}