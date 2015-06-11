using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace DirectAuthorTests
{
    [TestClass]
    public class ScriptLoaderTests : CompletionsBase
    {
        protected static readonly string ContextFile1Name = "context1.js";
        protected static readonly string ContextFile2Name = "context2.js";
        protected static readonly string ContextFile3Name = "context3.js";
        protected static readonly string CompletionFunctionName = "done";
        protected static readonly string ScriptLoaderDirectoryPath = Path.Combine(Paths.FilesPath, @"ScriptLoaders");

        protected void PerformScriptLoaderRequests(string primaryFileText, string loaderFilePath, Action<IAuthorTestContext, int, string, int> action, params IAuthorTestFile[] extraContextFiles)
        {
            var domjsFile = _session.ReadFile(Paths.DomWebPath);
            var loaderFile = _session.ReadFile(loaderFilePath);
            var contextFile1 = _session.FileFromText(@"function context1(p1){context1_type = {context1value : 1};}");
            var contextFile2 = _session.FileFromText(@"function context2(p2){context2_type = {context2value : 2};}");
            var contextFile3 = _session.FileFromText(@"function context3(p3){context3_type = {context3value : 3};}");
            var completionFunction = _session.FileFromText(@" 
                function " + CompletionFunctionName + @"()
                {
                    context1();
                    context2();
                    context3();
                }");

            var offsets = ParseRequests(primaryFileText);
            var primaryFile = _session.FileFromText(offsets.Text);

            var contextFiles = new IAuthorTestFile[extraContextFiles.Length + 3];
            contextFiles[0] = domjsFile;
            contextFiles[1] = loaderFile;
            contextFiles[2] = completionFunction;
            extraContextFiles.CopyTo(contextFiles, 3);

            var context = _session.OpenContext(primaryFile, contextFiles);
            context.AddAsyncScript(ContextFile1Name, contextFile1);
            context.AddAsyncScript(ContextFile2Name, contextFile2);
            context.AddAsyncScript(ContextFile3Name, contextFile3);

            var i = 0;
            foreach (var request in offsets.Requests)
            {
                action(context, request.Offset, request.Data, i++);
            }
        }

        protected void GetCompletion(string loaderFilePath, string loaderCommand, bool scriptRequestInPrimaryFile)
        {
            var primaryFileText = @"
                context1_type.|context1value|; 
                context2_type.|context2value|; 
                context3_type.|context3value|";

            Action<IAuthorTestContext, int, string, int> action = (context, offset, data, index) =>
               {
                   var completions = context.GetCompletionsAt(offset);

                   Assert.IsTrue(completions != null);

                   completions.ToEnumerable().ExpectContains(new[] { data });
               };


            if (scriptRequestInPrimaryFile)
            {
                // prepend the script load request to the primary file
                primaryFileText = loaderCommand + primaryFileText;
                PerformScriptLoaderRequests(primaryFileText, loaderFilePath, action);
            }
            else
            {
                // create a new context file to host the request
                var testFile = _session.FileFromText(loaderCommand);
                PerformScriptLoaderRequests(primaryFileText, loaderFilePath, action, testFile);
            }
        }

        protected void GetFunctionHelp(string loaderFilePath, string loaderCommand, bool scriptRequestInPrimaryFile)
        {
            var primaryFileText = @"
                context1(|p1|);
                context2(|p2|);
                context3(|p3|);";

            Action<IAuthorTestContext, int, string, int> action = (context, offset, data, index) =>
                 {
                     var functionHelp = context.GetParameterHelpAt(offset);

                     Assert.IsTrue(functionHelp != null && functionHelp.FunctionHelp != null);

                     // verify the first parameter is the Data field in the request
                     var parameters = functionHelp.FunctionHelp.GetSignatures().ToEnumerable().Single().GetParameters();
                     parameters.ToEnumerable().First().Name.Expect(data);
                 };

            // perform getFunctionHelp on the file
            if (scriptRequestInPrimaryFile)
            {
                // prepend the script load request to the primary file
                primaryFileText = loaderCommand + primaryFileText;
                PerformScriptLoaderRequests(primaryFileText, loaderFilePath, action);
            }
            else
            {
                // create a new context file to host the request
                var testFile = _session.FileFromText(loaderCommand);
                PerformScriptLoaderRequests(primaryFileText, loaderFilePath, action , testFile);
            }
        }
    }

    [TestClass]
    public class JsDeferTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"JsDefer\jsdefer.js");

        static readonly string loadScriptCommand = String.Format(@"$.deferDef({{
                            context1: {{
                                url: ""{0}"",
                                bare: true
                                }},

                            context2: {{
                                url: ""{1}"",
                                depends: ""context1"",
                                bare: true
                            }},

                            context3: {{
                                url: ""{2}"",
                                depends: [ ""context1"", ""context2"" ],
                                bare:true
                            }}

                        }});
                        $.defer.context3().done({3});", ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);

        [TestMethod]
        public void JsDefer_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void JsDefer_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        public void JsDefer_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void JsDefer_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }
    }

    [TestClass]
    [Ignore] // xhr support is disabled 
    public class LabJsTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"LabJs\lab.js");
        static readonly string loadScriptCommand = String.Format(@"$LAB
                            .script(""{0}"").wait()
                            .script(""{1}"").wait()
                            .script(""{2}"").wait({3});", ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);

        [TestMethod]
        public void LabJs_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void LabJs_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        public void LabJs_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void LabJs_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }
    }

    [TestClass]
    public class HeadJsTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"HeadJS\head.js");
        static readonly string loadScriptCommand = String.Format(@"head.js( ""{0}"", ""{1}"", ""{2}"", {3});",
                        ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);

        [TestMethod]
        public void HeadJs_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void HeadJs_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        public void HeadJs_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void HeadJs_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }
    }

    [TestClass]
    public class RequireJsTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"RequireJs\require.js");
        static readonly string loadScriptCommand = String.Format(@"require([""{0}"", ""{1}"", ""{2}""], {3});",
                        ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);

        [TestMethod]
        [WorkItem(206473)]
        public void RequireJs_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        [WorkItem(206473)]
        public void RequireJs_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        [WorkItem(206473)]
        public void RequireJs_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        [WorkItem(206473)]
        public void RequireJs_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RequireJs_Bootstrapping()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domInitialization = _session.FileFromText(@"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                { 
                                    $tag: 'script',
                                    src: 'require.js',
                                    'data-main' : 'ContextFile.js'
                                }
                            ]
                        }
                    ]
                );");
            var requireJs = _session.ReadFile(loaderFilePath);
            var contextFile = _session.FileFromText(@"contextType = {a:1};", "ContextFile.js");
            var primaryFile = _session.FileFromText(@"contextType.");

            var context = _session.OpenContext(primaryFile, dom, domInitialization, requireJs);
            context.AddAsyncScript("./ContextFile.js", contextFile);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);

            completions.ToEnumerable().ExpectContains("a");
        }
    }

    [TestClass]
    public class YUILoaderTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"YUILoader\yui.js");
        static readonly string loadScriptCommand = String.Format(@"YUI().Get.script([""{0}"", ""{1}"", ""{2}""], {{
                                                onSuccess: {3},
                                                data: {{}}
                                            }});",
                        ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);
        [TestMethod]
        public void YUILoader_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void YUILoader_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        public void YUILoader_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        public void YUILoader_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }
    }

    [TestClass]
    public class YepnopeTests : ScriptLoaderTests
    {
        static readonly string loaderFilePath = Path.Combine(ScriptLoaderDirectoryPath, @"YepNope\yepnope.js");
        static readonly string loadScriptCommand = String.Format(@"  
            yepnope({{ load: '{0}', callback: function() {{ 
                yepnope({{ load: '{1}', callback: function() {{  
                    yepnope({{ load: '{2}', callback: {3} }});
                }} }});
            }} }});
            ", ContextFile1Name, ContextFile2Name, ContextFile3Name, CompletionFunctionName);

        [TestMethod]
        [WorkItem(276835)]
        public void Yepnope_GetCompletion_ScriptRequestInPrimaryFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        [WorkItem(276835)]
        public void Yepnope_GetCompletion_ScriptRequestInContextFile()
        {
            GetCompletion(loaderFilePath, loadScriptCommand, false);
        }

        [TestMethod]
        [WorkItem(276835)]
        public void Yepnope_GetFunctionHelp_ScriptRequestInPrimaryFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, true);
        }

        [TestMethod]
        [WorkItem(276835)]
        public void Yepnope_GetFunctionHelp_ScriptRequestInContextFile()
        {
            GetFunctionHelp(loaderFilePath, loadScriptCommand, false);
        }
    }

    [TestClass]
    public class ScriptLoaderRegressionTests : ScriptLoaderTests
    {
        [TestMethod]
        [WorkItem(203249)]
        public void LoadOnlyScriptTags()
        {
            var domjsFile = _session.ReadFile(Paths.DomWebPath);

            // context file should not be loaded
            var contextFile = _session.FileFromText(@"contextType = {a:1};");

            var primaryFile = _session.FileFromText(@"
                var divElement = document.createElement(""div"");
                divElement.src = ""ContextFile.js"";
                document.body.appendChild(divElement);

                var checkbox = document.createElement('<input name=""box"">');
                checkbox.src = ""ContextFile.js"";
                document.head.appendChild(checkbox);

                var someElement = {src:""ContextFile.js""};
                document.head.insertBefore(someElement, docuemnt.head.firstChild);

                contextType.");

            var context = _session.OpenContext(primaryFile, domjsFile);
            context.AddAsyncScript("ContextFile.js", contextFile);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);

            completions.ToEnumerable().ExpectNotContains(new string[] { "a" });
        }

        [TestMethod]
        [WorkItem(280357)]
        public void DocuemntWrite()
        {
            var domjsFile = _session.ReadFile(Paths.DomWebPath);
            var contextFile1 = _session.FileFromText(@"contextType1 = {loaded:1};");
            var contextFile2 = _session.FileFromText(@"contextType2 = {loaded:1};");
            var contextFile3 = _session.FileFromText(@"contextType3 = {loaded:1};");
            var contextFile4 = _session.FileFromText(@"shouldNotBeLoaded = {loaded:1};");
            var file = _session.FileFromText(@"
                    document.write('                                                         \
                        <script src =   ""ContextFile1.js"" type=""text/javascript"" ><\/script> \
                        <img src=""ContextFile4.js"" type=""text/javascript"" ><\/img>       \
                        <iframe src=""ContextFile4.js"" ><\/iframe>                          \
                        < script src=""ContextFile2.js"" type=""text/javascript"" \/>        \
                        <script src=\'ContextFile3.js\' type=""text/javascript"" \/>         \
                        < invlaid src=""ContextFile4.js"" \/>                                \
                        ');
                ");
            var context = _session.OpenContext(file, domjsFile);
            context.AddAsyncScript("ContextFile1.js", contextFile1);
            context.AddAsyncScript("ContextFile2.js", contextFile2);
            context.AddAsyncScript("ContextFile3.js", contextFile3);
            context.AddAsyncScript("ContextFile4.js", contextFile4);

            var completions = context.GetCompletionsAt(file.Text.Length);
            completions.ToEnumerable().ExpectContains(new string[] { "contextType1", "contextType2", "contextType3" });
            completions.ToEnumerable().ExpectNotContains(new string[] { "shouldNotBeLoaded" });
        }

        [TestMethod]
        [WorkItem(280357)]
        public void InnerHTML()
        {
            var domjsFile = _session.ReadFile(Paths.DomWebPath);
            var contextFile1 = _session.FileFromText(@"contextType1 = {loaded:1};");
            var file = _session.FileFromText(@"document.getElementById('para').innerHTML = '<script src=""ContextFile1.js""\/> ';");
            var context = _session.OpenContext(file, domjsFile);
            context.AddAsyncScript("ContextFile1.js", contextFile1);
            
            var completions = context.GetCompletionsAt(file.Text.Length);
            completions.ToEnumerable().ExpectContains(new string[] { "contextType1"});
        }

        [TestMethod]
        [WorkItem(280357)]
        public void BingMaps()
        {
            var siteTypesFile = _session.ReadFile(Paths.SiteTypesWebPath);
            var domjsFile = _session.ReadFile(Paths.DomWebPath);
            var mapControleFile = _session.ReadFile(Path.Combine(Paths.FilesPath, @"BingMapsV7.0\mapcontrol.js"));
            var coreFile = _session.ReadFile(Path.Combine(Paths.FilesPath, @"BingMapsV7.0\veapicore.js"));
            var primaryFile = _session.FileFromText(@"Microsoft.Maps.");

            var context = _session.OpenContext(primaryFile, siteTypesFile, domjsFile, mapControleFile);
            context.AddAsyncScript(@"http://ecn.dev.virtualearth.net/mapcontrol/v7.0/js/bin/7.0.2011100111334.47/en-us/veapicore.js", coreFile);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);
            completions.ToEnumerable().ExpectContains(new string[] { "Location", "Map", "Events"});
        }

        [TestMethod]
        [WorkItem(323644)]
        public void ScriptBundles()
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
            var c2 = _session.FileFromText("if (typeof x != 'undefined') x.c2 = 1;", "c2.js");
            var c3 = _session.FileFromText("if (typeof x != 'undefined') x.c3 = 1;", "c3.js");
            var loadContext = _session.FileFromText(@"
                 var x = {};
                 load('c1.js', true);
                 load('c2.js', true);
                 load('c3.js', true);
            ", "loadContext.js");

            var primary1 = _session.FileFromText(@"x.", "primary1.js");
            var primary2 = _session.FileFromText(@"x.", "primary2.js");

            var context1 = _session.OpenContext(primary1, dom, simpleLoader, loadContext);
            context1.AddAsyncScript("c1.js", c1);
            context1.AddAsyncScript("c2.js", c2);
            context1.AddAsyncScript("c3.js", c3);

            var context2 = _session.OpenContext(primary2, dom, simpleLoader, loadContext);
            context2.AddAsyncScript("c1.js", c1);
            context2.AddAsyncScript("c2.js", c2);
            context2.AddAsyncScript("c3.js", c3);

            var completionList1 = context1.GetCompletionsAt(primary1.Text.Length);
            completionList1.ToEnumerable().ExpectContains("c1", "c2", "c3");

            var completionList2 = context2.GetCompletionsAt(primary2.Text.Length);
            completionList2.ToEnumerable().ExpectContains("c1", "c2", "c3");
        }

        [TestMethod]
        [WorkItem(367102)]
        public void LoaderFileScriptTagInHTML()
        {
            // Make sure a loader can find itself in the script tags
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domInitialization = _session.FileFromText(@"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                { 
                                    $tag: 'script',
                                    src: 'mylib.js',
                                    'data-config' : 'foo'
                                },
                                { 
                                    $tag: 'script',
                                    src: 'default.js',
                                }
                            ]
                        }
                    ]
                );");

            var loader = _session.FileFromText(@"
                !function () {
                    var scripts = document.getElementsByTagName('script');
                    var configFound = false;
                    for (var i = 0; i < scripts.length; i++) {
                        var script = scripts[i];

                        if (script.src.indexOf('mylib.js') > -1) {
                            configFound = true;
                            break;
                        }
                    }

                    if (!configFound) return;

                    window.globalVariable = 23;
                }();", "loader.js");
            var primaryFile = _session.FileFromText(@"window.globalVariable.");

            var context = _session.OpenContext(primaryFile, dom, domInitialization, loader);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);

            completions.ToEnumerable().ExpectContains(NumberMethods);
        }

        [TestMethod]
        [WorkItem(439220)]
        public void ExecludeDefaultScripts()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = _session.FileFromText(@"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                {
                                    $tag: 'script',
                                    src: '/js/default.js'
                                }
                            ]
                        }
                    ]
                );");

            var defaultcontextFile = _session.FileFromText(@"contextType = {a:1};", "default.js");
            var loader = _session.FileFromText(@"
                var s = document.createElement('script');
                s.src = '/js/default.js';
                document.head.appendChild(s);", "loader.js");
            var primaryFile = _session.FileFromText(@"contextType.");

            var context = _session.OpenContext(primaryFile, dom, domStructure, loader);
            context.AddAsyncScript("/js/default.js", defaultcontextFile);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);

            // ensure the file was not loaded
            completions.ToEnumerable().ExpectNotContains("a");
        }
    }

    [TestClass]
    public class DedicatedWorkerTests : ScriptLoaderTests
    {
        [TestMethod]
        public void ImportScript()
        {
            var dedicatedWorkerFile = _session.ReadFile(Paths.DedicatedWorkerPath);
            var contextFile = _session.FileFromText(@"contextType = {loaded:1};");
            var primaryFile = _session.FileFromText(@"importScripts('ContextFile.js'); contextType.");
            var context = _session.OpenContext(primaryFile, dedicatedWorkerFile);
            context.AddAsyncScript("ContextFile.js", contextFile);

            var completions = context.GetCompletionsAt(primaryFile.Text.Length);
            completions.ToEnumerable().ExpectContains(new string[] { "loaded" });
        }
    }
}