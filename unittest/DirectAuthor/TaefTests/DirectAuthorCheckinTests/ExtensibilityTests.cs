//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ExtensibilityTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the ExtensibilityTests type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using System.IO;
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class ExtensibilityTests : DirectAuthorTest
    {
        [TestInitialize]
        new public void Initialize()
        {
            base.Initialize();
        }

        [TestCleanup]
        new public void Cleanup()
        {
            base.Cleanup();
        }

        [TestMethod]
        [WorkItem(411278)]
        public void Bind()
        {
            PerformCompletionRequests(@"
                function This() {
                    this.num = 1;
                }
                function define(members) {
                    Object.getOwnPropertyNames(members).forEach(function (name) {
                        intellisense.setCallContext(members[name], { 
                            get thisArg() { return new This(); } 
                        });
                    });
                }

                define({ 
                    Foo: function () { this.|num| }
                });
            ");
        }

        [TestMethod]
        [WorkItem(379748)]
        public void WinJSExtension()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var basejs = WinJSTestFiles.latest_base;
            var baseIntellisense = _session.ReadFile(Paths.WinJs.BaseIntellisensePath).Text;
            PerformCompletionRequests(@"
                var page = WinJS.UI.Pages.define('/page', {
                    ready: function (element, options) {
                        element.|hasOwnProperty,childNodes|;
                    }
                });
                var p = new page();
                p.|render|;
            ", dom, basejs, baseIntellisense);

            PerformCompletionRequests(@"
                WinJS.Class.define(function () { this.name = ''; }, {
                    foo: function () {
                        this.|name,foo|;
                    }
                },
                {
                    staticfoo: function () {
                        this.|call|;
                    }
                });
            ", dom, basejs, baseIntellisense);
        }

        [TestMethod]
        public void WinJSPromise()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var basejs = WinJSTestFiles.latest_base;
            var baseIntellisense = _session.ReadFile(Paths.WinJs.BaseIntellisensePath).Text;

            PerformCompletionRequests(@"
                new WinJS.Promise.as(1).done(function(r) {  r.|Number|; }, function (e) { e.|message| });
                new WinJS.Promise.as(1).then(function(r) {  r.|Number|; }, function (e) { e.|message| });
            ", dom, basejs, baseIntellisense);
        }

        [TestMethod]
        [WorkItem(377931)]
        public void IntelliSenseOnExtensibilityAPI()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            var extensibilityIntellisense = _session.ReadFile(Path.Combine(Paths.ReferencesPath, "extensibility.intellisense.js")).Text;

            // statementcompletion event argument
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function (e) { 
                    e.|items,target,targetName,offset,scope|;
                    e.target.|hasOwnProperty|;
                    e.targetName.|charAt|;
                    e.scope.|charAt|;
                    e.offset.|toFixed|;
                    e.items[0].|name,kind,glyph,parentObject,value,comment|;
                    e.items[0].name.|charAt|;
                    e.items[0].kind.|charAt|;
                    e.items[0].glyph.|charAt|;
                    e.items[0].comment.|charAt|;
                    e.items[0].value.|hasOwnProperty|;
                    e.items[0].parentObject.|hasOwnProperty|;
                    var funcComments = intellisense.getFunctionComments(function() {});
                    funcComments.|inside,above,paramComments|;
                    funcComments.paramComments[0].|name,comment|;
                });
            ", dom, extensibilityIntellisense);

            // statementcompletionhint event argument
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletionhint', function (e) { 
                    e.|completionItem,symbolHelp|;
                    e.completionItem.|name,kind,glyph,parentObject,value,comment|;
                    e.symbolHelp.|name,symbolType,symbolDisplayType,elementType,scope,description,locid,helpKeyword,externalFile,externalid,functionHelp|;
                    e.symbolHelp.name.|charAt|;
                    e.symbolHelp.functionHelp.|functionName,signatures|;
                    e.symbolHelp.functionHelp.functionName.|charAt|;
                    e.symbolHelp.functionHelp.signatures[0].|description,locid,helpKeyword,externalFile,externalid,returnValue,params|;
                    e.symbolHelp.functionHelp.signatures[0].returnValue.|type,elementType,description,locid,helpKeyword,externalFile,externalid|;
                    e.symbolHelp.functionHelp.signatures[0].params[0].|name,type,elementType,description,locid,optional|;
                });
            ", dom, extensibilityIntellisense);

            // signaturehelp event argument
            PerformCompletionRequests(@"
                intellisense.addEventListener('signaturehelp', function (e) {
                    e.|target,offset,parentObject,functionHelp|;
                    e.target.|hasOwnProperty|;
                    e.parentObject.|hasOwnProperty|;
                    e.offset.|toFixed|;
                    e.functionHelp.|functionName,signatures|;
                    e.functionHelp.functionName.|charAt|;
                    e.functionHelp.signatures[0].|description,locid,helpKeyword,externalFile,externalid,returnValue,params|;
                    e.functionComments.|inside,above,paramComments|;
                    e.functionComments.paramComments[0].|name,comment|;
                });
            ", dom, extensibilityIntellisense);
        }

        [TestMethod]
        [WorkItem(374669)]
        public void LoggedMessagesWithCompletionHint()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletionhint', function (event) {});
                intellisense.addEventListener('statementcompletion', function () { intellisense.logMessage('Completion'); });
                var f;
                f|
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                var hint = completions.GetHintFor("f");
                context.LoggedMessages.Count(m => m == "Completion").Expect(1);
            });
        }

        [TestMethod]
        [WorkItem(376721)]
        public void SavingExtensibilityObjectBetweenEvents()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests(@"
            var documentItem = null;
            var savedEvent = null;
            var getElementItem = null;
            intellisense.addEventListener('statementcompletion', function (event) {
                savedEvent = event;
                event.items.forEach(function (item) {
                if (item.name == 'document') {
                        documentItem = item;
                        intellisense.logMessage('document item saved');
                    }
                if (item.name == 'getElementById') {
                        getElementItem = item;
                    }
                });
            });
            intellisense.addEventListener('statementcompletionhint', function (event) {
                intellisense.logMessage('savedEvent.items: ' + savedEvent.items);
                savedEvent.items.forEach(function(item) {});
                var item = event.completionItem;
                var savedItem = documentItem;
                if (item.name == 'getElementById') {
                        savedItem = getElementItem;
                    }
                if (savedItem.name == item.name &&
                            savedItem.comments == item.comments) {
                            intellisense.logMessage('Items match');
                        }
                    else {
                            intellisense.logMessage('Items do not match');
                    }
            });
            document.getElementById();
            ;|
        ", (context, offset, data, index) =>
         {
             var completions = context.GetCompletionsAt(offset);
             var hint = completions.GetHintFor("document");
         }, dom);
        }

        [TestMethod]
        [WorkItem(374987)]
        public void AnnotateGlobalVariables()
        {
            PerformRequests(@"
                this.g = undefined;
                intellisense.annotate(this, {
                    /// <field>desc</field>
                    g: undefined
                });
                ;|;
                this.|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("g");
                 hint.Description.Trim().Expect("desc");
             });

            PerformRequests(@"
                var g;
                intellisense.annotate(this, {
                    /// <field>desc</field>
                    g: undefined
                });
                ;|;
                this.|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("g");
                 hint.Description.Trim().Expect("desc");
             });

            // Make sure <field> cannot be used with globabal variable directly (it is OK in annotation).
            PerformRequests(@"
                /// <field>desc</field>
                var g;
                ;|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("g");
                 hint.Description.Expect(null);
             });
        }

        [TestMethod]
        public void UseSimpleComments()
        {
            string extension = @"
                intellisense.addEventListener('statementcompletionhint', function (e) { 
                        intellisense.logMessage('e.completionItem.comments: ' + e.completionItem.comments);
                        e.symbolHelp.description = e.completionItem.comments;
                        if (!e.symbolHelp.description && typeof e.completionItem.value == 'function') {
                            var c = intellisense.getFunctionComments(e.completionItem.value);
                            var inside = c.inside;
                            if (inside) {
                                e.symbolHelp.description = inside;
                                intellisense.logMessage('getFunctionComments: ' + inside);
                            }
                        }
                    });
                ";

            PerformRequests(extension + @"
                var o = {
                    // func descr
                    f: function() { }
                };
                o.|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("f");
                 var funcHelp = hint.GetFunctionHelp();
                 hint.Description.Trim().Expect("func descr");
             });

            PerformRequests(extension + @"
                function outer() {
                    // func descr
                    function f() {}
                }
                ;|;
                
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("outer");
                 hint.Description.Trim().Expect("func descr");
             });

            PerformRequests(extension + @"
                (function() {
                    var o = {};
                    // func descr
                    o.f = function() {};
                    o.|;
                })();
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("f");
                 hint.Description.Trim().Expect("func descr");
             });

            PerformRequests(extension + @"
                // func descr
                function f() {}
                ;|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("f");
                 hint.Description.Trim().Expect("func descr");
             });

            PerformRequests(extension + @"
                // descr line 1
                // descr line 2
                var x1 = {};
                /* descr line 1
                 descr line 2 */
                var x2 = {};
                ;|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("x1");
                 hint = completions.GetHintFor("x2");
             });

            PerformRequests(extension + @"
                var o = {
                    // field descr
                    f: 1
                };
                o.|;
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 var hint = completions.GetHintFor("f");
                 hint.Description.Trim().Expect("field descr");
             });
        }

        [TestMethod]
        public void Version()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (e) { });
                intellisense.logMessage('version: ' + intellisense.version);
                intellisense.|
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetCompletionsAt(offset);
                EnsureMessageLogged(context, "version: 11");
            });
        }

        [TestMethod]
        [WorkItem(311514)]
        public void ReservedWords()
        {
            // Make sure that reserved words are being preserved when extension is being called.
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function (e) { 
                        // force roundtripping of completion list
                        x = e.items;
                    });
                ;|break|;
            ", (completion, data, index) =>
             {
                 completion.ExpectContains("break");
             }, AuthorCompletionFlags.acfAny);
        }

        [TestMethod]
        public void HidingEntriesInStatementScope()
        {
            // In statement scope, extension is not allowed to remove builtin types or scope variables from completion. 
            // Extensions can be used to hide library-specific globals but they are not allowed to hide built-in objects, local variables or parameters since
            // they are never library-specific.
            string extension = @"
                intellisense.addEventListener('statementcompletion', 
                    function (e) { 
                        intellisense.logMessage('>>>> resetting items');
                        e.items = [];
                    });
                    function f2() {} // global function, completion entry can be removed
                ";
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                // global variable, completion entry can be removed
                var x;|!x,Object,Array,JSON,document,window,SVGElement,!f2|
                function f1(p1,p2) { // global function, completion entry can be removed
                    var v1,v2; // scope variables, completion entries can NOT be removed
                    function f3() { // nested function, completion entry can NOT be removed
                        var a,b;|f3,!f1,!f2,p1,p2,a,b,v1,v2,!x,Object,Array,JSON,document,window,SVGElement|
                    }
                }
                this.|!x,Object,Array,JSON,document,window,SVGElement,!f2|;
            ",
                extension, dom);
        }

        [TestMethod]
        [TestCategory("super")]
        public void TargetName()
        {
            string extension = @"
                intellisense.addEventListener('statementcompletion', 
                    function (e) { 
                        intellisense.logMessage('>>>> e.target: ' + e.target + ' e.targetName: ' + e.targetName);
                    });";
            Action<string, string, string> Verify = (src, target, targetName) =>
            {
                PerformRequests(src,
                (context, offset, data, index) =>
                {
                    var help = context.GetCompletionsAt(offset);
                    EnsureMessageLogged(context, ">>>> e.target: " + target + " e.targetName: " + targetName);
                }, extension);
            };

            var undefinedText = "undefined";
            var objectText = "[object Object]";

            Verify(@"var xyz;xyz.|;", undefinedText, "xyz");
            Verify(@"var x = { a: 1 };x.a|;", objectText, "x");
            Verify(@"var x = { a: { b: 1 } }; x.a.|;", objectText, "a");
            Verify(@"var x = { a: { b: 1 } }; x.a.b|;", objectText, "a");
            Verify(@"''.|", "", undefinedText);
            Verify(@"this.|", objectText, "this");
            Verify(@"|", undefinedText, undefinedText);
            Verify(@"function () {|}", undefinedText, undefinedText);
            Verify(@"function () {while(true){if(false){var x;x|}}}", undefinedText, undefinedText);
            Verify(@"class A { }; class B extends A { method() { super.|; } }", objectText, "super");
        }

        [TestMethod]
        [WorkItem(223319)]
        public void ContextTarget()
        {
            // Make sure that context.target is set regardless of the completion target type

            string extension = @"
                intellisense.addEventListener('statementcompletion', 
                    function (e) { 
                        intellisense.logMessage('e.target: ' + e.target);
                        if(e.target)
                            intellisense.logMessage('>>>> OK');
                        else
                            intellisense.logMessage('>>>> NOT OK');
                    });";

            Action<string, string> Verify = (s, targetMessage) =>
            {
                PerformRequests(s,
                (context, offset, data, index) =>
                {
                    var help = context.GetCompletionsAt(offset);
                    EnsureMessageLogged(context, "e.target: " + targetMessage);
                    EnsureMessageLogged(context, ">>>> OK");
                },
                extension);
            };

            Verify(@"var x = 1; x.|", "1");
            Verify(@"var x = 'hi'; x.|", "hi");
            Verify(@"var x = true; x.|", "true");
            Verify(@"var x = {}; x.|", "[object Object]");
            Verify(@"var x = ['hello', 'world']; x.|", "hello,world");
        }

        [TestMethod]
        public void LogMessage()
        {
            PerformRequests(@"
                intellisense.addEventListener('signaturehelp', 
                    function (e) { 
                        intellisense.logMessage('>>>> Hi from extension');
                    });
                ''.split(|);
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                EnsureMessageLogged(context, ">>>> Hi from extension");
                Assert.IsNotNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        [WorkItem(205878)]
        public void Extensions_SourceFileHandleRoundTripping()
        {
            var extensionFileText = @"
                    intellisense.addEventListener('signaturehelp', 
                        function (e) {
                            function logMessage(msg) { intellisense.logMessage(msg); }
                                logMessage('setting function ' + e.functionHelp.functionName + ' description');
                                e.functionHelp.signatures[0].description = 'description from extension';
                        }
                    );";
            var contextFileText = @"function foo(x ,y) {}";

            PerformRequests(@"
                    foo(|context|);
                    
                    function bar(a, b) {}
                    bar(|primary|);

                    Object.getPrototypeOf(|internal|);
                ",
                (context, offset, data, index) =>
                {
                    var help = context.GetParameterHelpAt(offset);
                    help.FunctionHelp.GetSignatures().ToEnumerable().Single().Description.Expect("description from extension");
                    switch (data)
                    {
                        case "context":
                            Assert.IsNotNull(help.FunctionHelp.SourceFileHandle);
                            Assert.AreEqual(help.FunctionHelp.SourceFileHandle, context.GetFileByText(contextFileText).GetHandle());
                            break;
                        case "primary":
                            Assert.IsNotNull(help.FunctionHelp.SourceFileHandle);
                            Assert.AreEqual(help.FunctionHelp.SourceFileHandle, context.PrimaryFile.GetHandle());
                            break;
                        case "internal":
                            Assert.IsNotNull(help.FunctionHelp.SourceFileHandle);
                            break;
                        default:
                            Assert.Fail("unexpected value");
                            break;
                    }

                }, contextFileText, extensionFileText);
        }

        [TestMethod]
        [WorkItem(429869)]
        public void TargetSetFromDocComments()
        {
            string extension = @"
                intellisense.addEventListener('statementcompletion', 
                    function (e) { 
                        intellisense.logMessage('e.target: ' + e.target);
                        if(e.target)
                            intellisense.logMessage('>>>> OK');
                        else
                            intellisense.logMessage('>>>> NOT OK');
                    });";

            PerformRequests(@"
                /// <var type='Number' />
                var listView;
                listView.|;",
            (context, offset, data, index) =>
            {
                var help = context.GetCompletionsAt(offset);
                EnsureMessageLogged(context, ">>>> OK");
            },
            extension);
        }

        [TestMethod]
        [WorkItem(910668)]
        public void GetFileNameInPrimaryFile()
        {
            PerformCompletionRequests(@"
            var name = intellisense.executingScriptFileName;
            (name === ""primaryFile.js"" ? ""yes"" : 0).|String|;
            ");
        }

        [TestMethod]
        [WorkItem(910668)]
        public void GetFileNameInContextFile()
        {
            var contextFile = _session.FileFromText(@"
                   var contextFileName = intellisense.executingScriptFileName;
                ", "contextFile.js");
            var primaryFile = _session.FileFromText(@"
                    (contextFileName === ""contextFile.js"" ? ""yes"" : 0).", "primaryFile.js");

            var context = _session.OpenContext(primaryFile, contextFile);
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);
        }

        [TestMethod]
        [WorkItem(910668)]
        public void GetFileNameInPrimaryFileFromContextFileHelper()
        {
            var contextFile = _session.FileFromText(@"
                   function getFileName() { return intellisense.executingScriptFileName; }
                ", "contextFile.js");
            var primaryFile = _session.FileFromText(@"
                    var name = getFileName();
                    (name === ""primaryFile.js"" ? ""yes"" : 0).", "primaryFile.js");

            var context = _session.OpenContext(primaryFile, contextFile);
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);
        }

        [TestMethod]
        [WorkItem(910668)]
        public void GetFileNameInContextFileFromContextFileHelper()
        {
            var helperFile = _session.FileFromText(@"
                   function getFileName() { return intellisense.executingScriptFileName; }
                ", "helperFile.js");
            var contextFile = _session.FileFromText(@"
                   var contextFileName = getFileName();
                ", "contextFile.js");
            var primaryFile = _session.FileFromText(@"
                    (contextFileName === ""contextFile.js"" ? ""yes"" : 0).", "primaryFile.js");

            var context = _session.OpenContext(primaryFile, helperFile, contextFile);
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);
        }

        [TestMethod]
        [WorkItem(910668)]
        public void GetFileNameInFileWithNoName()
        {
            var primaryFile = _session.FileFromText(@"
                    var name = intellisense.executingScriptFileName;
                    (typeof name === ""undefined"" ? ""yes"" : 0).");

            var context = _session.OpenContext(primaryFile);
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            completion.ToEnumerable().ExpectContains(CompletionsBase.StringMethods);
        }
    }
}