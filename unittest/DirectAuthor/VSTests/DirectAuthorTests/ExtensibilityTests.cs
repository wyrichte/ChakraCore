using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class ExtensibilityTests : DirectAuthorTest
    {
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
            var basejs = _session.ReadFile(Path.Combine(Paths.WinJs.LatestDirectoryPath, "base.js")).Text;
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
        public void ContextDomTarget()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', 
                function (e) { 
                    intellisense.logMessage('e.target: ' + e.target);
                    if(e.target)
                        intellisense.logMessage('>>>> OK');
                    else
                        intellisense.logMessage('>>>> NOT OK');
                });
                document.createElement('canvas').|",
            (context, offset, data, index) =>
            {
                var help = context.GetCompletionsAt(offset);
                EnsureMessageLogged(context, "e.target: [object Object]");
                EnsureMessageLogged(context, ">>>> OK");
            }, false, Paths.DomWindowsPath);
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
        public void LogMessageEscapeCharacter()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', 
                    function (event) { 
                        intellisense.logMessage('\r\n\t\b\f');
                        intellisense.logMessage('\052')                    // *
                        intellisense.logMessage('\x2a\x2A\u263a\u263A');   // **☺☺
                        intellisense.logMessage('\X2a\X2A\U263a\U263A');   // X2aX2AU263aU263A
                        intellisense.logMessage(""\"""");
                        intellisense.logMessage('\'');
                    });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, @"
	");
                EnsureMessageLogged(context, "*");
                EnsureMessageLogged(context, "**\u263a\u263a");
                EnsureMessageLogged(context, "X2aX2AU263aU263A");
                EnsureMessageLogged(context, "\"");
                EnsureMessageLogged(context, "'");
            });
        }

        [TestMethod]
        public void LogMessageInvalid()
        {
            string[] invalidMessages = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.Types,
                new string[] { ExtensibilityTestData.Exception });

            foreach (string invalidMessage in invalidMessages)
            {
                if (invalidMessage != "''")
                {
                    PerformRequests(string.Format(@"
                        intellisense.addEventListener('statementcompletion', function (event) {{
                            intellisense.logMessage({0});
                            intellisense.logMessage('should show');
                        }});|", invalidMessage),
                    (context, offset, data, index) =>
                    {
                        var completions = context.GetCompletionsAt(offset);
                        Assert.IsTrue(completions.Count > 0);
                        context.LoggedMessages.Count().Expect(1);
                        EnsureMessageLogged(context, "should show");
                    });
                }
            }
        }

        [TestMethod]
        public void LogMessageParameterCountIncorrect()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage();
                    intellisense.logMessage('should show');
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count().Expect(1);
                EnsureMessageLogged(context, "should show");
            });

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('should show', null);
                    intellisense.logMessage('should show again');
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, "should show");
                EnsureMessageLogged(context, "should show again");
            });
        }

        [TestMethod]
        public void LogMessageLongString()
        {
            PerformRequests(string.Format(@"
                intellisense.addEventListener('statementcompletion', function (event) {{
                    intellisense.logMessage('{0}');
                }});|", ExtensibilityTestData.Long),
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, ExtensibilityTestData.Long);
            });
        }

        [TestMethod]
        public void LogMessageMultiLineString()
        {
            PerformRequests(string.Format(@"
                intellisense.addEventListener('statementcompletion', function (event) {{
                    intellisense.logMessage('{0}');
                }});|", ExtensibilityTestData.MultiLine),
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, @" Multiple lined string  ");
            });
        }

        [TestMethod]
        public void LogMessageInternationalString()
        {
            string[] internationalStrings = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.International,
                ExtensibilityTestData.InternationalPartialSupported);

            foreach (string internationalString in internationalStrings)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        intellisense.logMessage('{0}');
                    }});|", internationalString),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsTrue(completions.Count > 0);
                    EnsureMessageLogged(context, internationalString);
                });
            }
        }

        [TestMethod]
        public void InfiniteLoop()
        {
            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  
                    function (e) { 
                        intellisense.logMessage('>>>> before infinite loop');
                        while(true) {}
                        intellisense.logMessage('>>>> after infinite loop');
                    });
                intellisense.logMessage('>>>> calling String.split');
                ''.split(|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            }, 500);
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
        public void EventCallBackInvalid()
        {
            string[] invalidRegistrations = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.Types,
                new string[] { 
                    ExtensibilityTestData.Exception, 
                    "function (event) { throw new Error(); }"});

            foreach (string invalidRegistration in invalidRegistrations)
            {
                if (invalidRegistration != "function(){}")
                {
                    PerformRequests(string.Format(@"
                    // invalid event
                    intellisense.addEventListener('statementcompletion', {0});

                    // valid event
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        intellisense.logMessage('should show');
                    }});|", invalidRegistration),
                    (context, offset, data, index) =>
                    {
                        var completions = context.GetCompletionsAt(offset);
                        Assert.IsTrue(completions.Count > 0);
                        EnsureMessageLogged(context, "should show");
                    });
                }
            }
        }

        [TestMethod]
        public void EventParameterCountIncorrect()
        {
            PerformRequests(@"
                // invalid event no param
                intellisense.addEventListener();

                // valid event 2 params
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('should show');
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, "should show");
            });

            PerformRequests(@"
                // invalid event only 1 param
                intellisense.addEventListener('statementcompletion');

                // valid event 2 params
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('should show');
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, "should show");
            });

            PerformRequests(@"
                // valid event 2 params + 1 ignored
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('should show');
                }, null);

                // valid event 2 params
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('should show again');
                });|",
           (context, offset, data, index) =>
           {
               var completions = context.GetCompletionsAt(offset);
               Assert.IsTrue(completions.Count > 0);
               EnsureMessageLogged(context, "should show");
               EnsureMessageLogged(context, "should show again");
           });
        }

        [TestMethod]
        public void EventNameInvalid()
        {
            string[] invalidNames = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.InternationalPartialSupported,
                ExtensibilityTestData.International,
                ExtensibilityTestData.Types,
                new string[] { "STATEMENTCOMPLETION", ExtensibilityTestData.Exception });

            foreach (string invalidName in invalidNames)
            {
                PerformRequests(string.Format(@"
                    // invalid event name
                    intellisense.addEventListener('{0}', function (event) {{
                        intellisense.logMessage('should not show');
                    }});

                    // valid event name
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        intellisense.logMessage('should show');
                    }});|", invalidName),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsTrue(completions.Count > 0);
                    context.LoggedMessages.Count(m => m == "should not show").Expect(0);
                    EnsureMessageLogged(context, "should show");
                });
            }
        }

        [TestMethod]
        public void UndefinedWithNestedCompletions()
        {
            PerformCompletionRequests(@"
                var x = intellisense.undefinedWithCompletionsOf(1);
                var y = intellisense.undefinedWithCompletionsOf(x);

                x.|Number|toString();
                y.|Number|toString();
            ");
        }

        [TestMethod]
        public void NullWithNestedCompletions()
        {
            PerformCompletionRequests(@"
                var x = intellisense.nullWithCompletionsOf(1);
                var y = intellisense.nullWithCompletionsOf(x);

                x.|Number|toString();
                y.|Number|toString();
            ");
        }

        [TestMethod]
        public void UndefinedWithDomCompletions()
        {
            PerformRequests(@"
                var x = intellisense.undefinedWithCompletionsOf(document.createElement('canvas'));

                x.|toString();",
            (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(new string[] { "getContext" });
            }, false, Paths.DomWindowsPath);
        }

        [TestMethod]
        public void NullWithDomCompletions()
        {
            PerformRequests(@"
                var x = intellisense.nullWithCompletionsOf(document.createElement('canvas'));

                x.|toString();",
            (context, offset, data, index) =>
            {
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(new string[] { "getContext" });
            }, false, Paths.DomWindowsPath);
        }

        [TestMethod]
        public void UndefinedWithValueOfUndefined()
        {
            PerformRequests(@"
                var x = intellisense.undefinedWithCompletionsOf(true);
                intellisense.addEventListener('statementcompletion', function (event) {
                    if (x) {
                        intellisense.logMessage('Not undefined');
                    }
                    else {
                        intellisense.logMessage('Undefined');
                    }
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "Not undefined").Expect(0);
                EnsureMessageLogged(context, "Undefined");
            });
        }

        [TestMethod]
        public void NullWithValueOfNull()
        {
            PerformRequests(@"
                var x = intellisense.nullWithCompletionsOf(true);
                intellisense.addEventListener('statementcompletion', function (event) {
                    if (x !== null) {
                        intellisense.logMessage('Not null');
                    }
                    else {
                        intellisense.logMessage('Null');
                    }
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "Not null").Expect(0);
                EnsureMessageLogged(context, "Null");
            });
        }
    }

    [TestClass]
    public class ParameterHelpExtensionsAdvanced : ParameterHelpTestsBase
    {
        [TestMethod]
        public void Extensions_ParameterHelp()
        {
            PerformRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                        intellisense.logMessage('>>>> e.parentObject: ' + JSON.stringify(e.parentObject));
                        if(e.parentObject) {
                            if(e.functionHelp.functionName.indexOf('set_') == 0) {
                                var getterName = 'get_' + e.functionHelp.functionName.substring(4, e.functionHelp.functionName.length);
                                intellisense.logMessage('>>>> getterName: ' + getterName);
                                var getter = e.parentObject[getterName];
                                if(getter) {
                                   var src = getter.toString(); 
                                   intellisense.logMessage('>>>> getter src: ' + src);
                                   var desc = src.substring(src.indexOf('<summary>') + 9, src.indexOf('</summary>')); 
                                   intellisense.logMessage('>>>> description: ' + desc);
                                   e.functionHelp.signatures[0].description = desc;
                                   return true;
                                }
                            }
                                                             
                        }
                        return false;
                    }
                );

                var x = {
                    set_X: function() {},
                    get_X: function() {
                        <summary>X desc</summary>
                    }
                };
                x.set_X(|);                    
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("set_X");
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().First();
                signature.Description.Expect("X desc");
            });

            PerformRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {});
                var x = undefined;
                x.set_X(|);                    
            ",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                Assert.IsNull(help.FunctionHelp);
            });

            PerformRequests(@"
                              function add(x, y) { } 
                              add(|);",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("add");
            }, // Extension defined in a context file
                new string[] {  @"
                    intellisense.addEventListener('signaturehelp', 
                        function (e) {
                            throw new Error('error from extension');
                        }
                    );" }
            );


            PerformRequests(@"
                              intellisense.addEventListener('signaturehelp',  function(e) {});
                              function add(x, y) { } 
                              add(|);",
            (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                help.FunctionHelp.FunctionName.Expect("x");
            }, // Extension defined in a context file
                new string[] {  @"
                    intellisense.addEventListener('signaturehelp', 
                        function (e) {
                            e.functionHelp.functionName = 'x';
                        }
                    );" }
            );

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [ 
                        { 
                            'description': 'adds two numbers',
                            'returnValue': {
                                'type':'String',
                                'description':'The sum of x, y'
                            },
                            'params':[ 
                                { 'description':'Value of x', 'name':'x', 'type':'Number' }, 
                                { 'description':'Value of y', 'name':'y', 'type':'Number' }
                            ]
                        },
                        {
                            'description': 'signature 2',
                            'returnValue': {
                                'type':'MyType'
                            },
                            'params':[ 
                                { 'description':'param1', 'name':'a', 'type':'String' }
                            ]
                        }
                    ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    var signatures = help.FunctionHelp.GetSignatures().ToEnumerable().ToArray();
                    signatures[0].Description.Expect("adds two numbers");
                    signatures[0].GetReturnValue().Type.Except("String");
                    signatures[0].GetReturnValue().Description.Except("String");
                    var params0 = signatures[0].GetParameters().ToEnumerable().ToArray();
                    params0[0].Name.Except("x");
                    params0[0].Description.Except("Value of x");
                    params0[0].Type.Except("Number");
                    params0[1].Name.Except("y");
                    params0[1].Description.Except("Value of y");
                    params0[1].Type.Except("Number");

                    signatures[1].Description.Expect("signature 2");
                    Assert.IsTrue(String.IsNullOrEmpty(signatures[1].GetReturnValue().Description));
                    signatures[1].GetReturnValue().Type.Except("MyType");
                    Assert.IsTrue(String.IsNullOrEmpty(signatures[1].GetReturnValue().Description));
                    var params1 = signatures[1].GetParameters().ToEnumerable().ToArray();
                    params1[0].Name.Except("a");
                    params1[0].Description.Except("param1");
                    params1[0].Type.Except("String");

                });


            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.functionName = 1;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp.FunctionName);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = 1;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [  ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = undefined;
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNull(help.FunctionHelp);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures = [ { } ];
                });
                function add(x, y) { return x+y; }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.GetParameters().ToEnumerable().Count().Expect(0);
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    function logMessage(msg) { intellisense.logMessage(msg + '\n'); }
                    logMessage('e.target: ' + e.target);
                    logMessage('e.functionHelp: ' + JSON.stringify(e.functionHelp));
                    var comments = intellisense.getFunctionComments(e.target);
                    logMessage('getComments result: ' + JSON.stringify(comments));
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.Description.Expect("adds two numbers");
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("Value of x");
                    p[1].Description.Except("Value of y");
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures[0].description = 'modified description';
                    e.functionHelp.signatures[0].params[0].description = 'modified x description';
                    e.functionHelp.signatures[0].params[1].description = 'modified x description';
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    signature.Description.Expect("modified description");
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("modified x description");
                    p[1].Description.Except("modified x description");
                });

            PerformParameterRequests(@"
                intellisense.addEventListener('signaturehelp',  function(e) {
                    e.functionHelp.signatures[0].params.splice(0, 1); // remove x parameter
                });
                function add(x, y) {
                    /// <summary>adds two numbers</summary>
                    /// <param name=""x"" type=""Number"">Value of x</param>
                    /// <param name=""y"" type=""Number"">Value of y</param>
                    /// <returns type=""Number"">The sum of x, y</returns>
                    return x+y;
                }
                add(|
                ",
                (help, data, index) =>
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    var p = signature.GetParameters().ToEnumerable().ToArray();
                    p[0].Description.Except("Value of y");
                });
        }
    }

    //
    // Summary: Runs all parameter help tests with an extension handler to ensure the tests
    //          are not affected.
    //
    [TestClass]
    public class ParameterHelpTestsWithExtensions : ParameterHelpTests
    {
        protected override string[] AdditionalContextFiles
        {
            get
            {
                return base.AdditionalContextFiles.Concat(new string[] {
                    @"
                     intellisense.addEventListener('signaturehelp',  function(e) {
                            intellisense.logMessage('base extension: e.functionHelp: ' + JSON.stringify(e.functionHelp) + '\n');
                        }
                    );"}).ToArray();
            }
        }
    }

    [TestClass]
    public class ShowPlainCommentsExtensionTests : DirectAuthorTest
    {
        private void Verify(string code, string desc, string itemName = "f", string[] paramComments = null)
        {
            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    var hint = completions.GetHintFor(itemName);
                    if (hint.Type == AuthorType.atFunction)
                    {
                        var funcHelp = hint.GetFunctionHelp();
                        var signature = funcHelp.GetSignatures().ToEnumerable().First();
                        signature.Description.Expect(desc);
                        if (paramComments != null)
                        {
                            var args = signature.GetParameters().ToEnumerable().ToArray();
                            for (int i = 0; i < args.Length; i++)
                            {
                                args[i].Description.Expect(paramComments[i]);
                            }
                        }
                    }
                    else
                    {
                        hint.Description.Expect(desc);
                    }
                }, false, Paths.ShowPlainCommentsPath);
            };

            VerifyImpl(code);
        }

        [TestMethod]
        public void CompletionHint()
        {
            Verify(@"/// <reference path='script1.js' />


                    /// hello there

                    function f(a, b) {
                    }
                    ;|
                ", null);

            Verify(@"
                // comment
                var f = 1;
                ;|
                ", "comment");

            Verify(@"
                // comment
                function f() {}
                ;|
                ", "comment");

            Verify(@"
                // comment
                function f() {
                    /// <summary>doc comment</summary>
                }
                ;|
                ", "doc comment");

            // Ignoring comments inside
            Verify(@"
                function f() {
                    // comment
                }
                ;|
                ", null);

            // Ignoring comments inside
            Verify(@"
                var x = {
                    f: function() {
                        // comment
                    }
                };
                x.|
                ", null);

            Verify(@"
                var x = {
                    // comment
                    f: function() {
                    }
                };
                x.|
                ", "comment");

            Verify(@"
                function f(/* a comment */ a, /* b comment */ b) {
                }
                ;|
                ", null, "f", new[] { "a comment", "b comment" });

            Verify(@"
                // comment
                function f(/* a comment */ a, /* b comment */ b) {
                }
                ;|
                ", "comment", "f", new[] { "a comment", "b comment" });

        }

        [TestMethod]
        public void Annotate()
        {
            Verify(@"
                var x = {
                    f: function() {}
                };
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                x.|;
                ", "comment");

            Verify(@"
                var x = {
                    f: 0
                };
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                x.|;
                ", "comment");

            Verify(@"
                var x = {
                    f: undefined
                };
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                x.|;
                ", "comment");
        }
    }

    [TestClass]
    public class AccessingCommentsInSignatureHelpEvent : DirectAuthorTest
    {
        #region Verify

        class ParamComment
        {
            public string Name { get; set; }
            public string Comment { get; set; }
        }

        private void Verify(string code, string commentAbove, string commentInside, ParamComment[] paramComments = null)
        {
            string extension = @"
                intellisense.addEventListener('signaturehelp', function (e) {
                        printComments(e.functionComments, 'e.functionComments.');
                        printComments(intellisense.getFunctionComments(e.target), 'intellisense.getFunctionComments(e.target).');
                        function printComments(comments, prefix) {
                            intellisense.logMessage(prefix + 'above: ' + comments.above);
                            intellisense.logMessage(prefix + 'inside: ' + comments.inside);
                            comments.paramComments.forEach(function(p) {
                                intellisense.logMessage(prefix + 'param: ' + p.name + ' comment: ' + p.comment);
                            });
                        }
                    });
                ";

            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    Action<string> EnsureMessages = (prefix) =>
                    {
                        EnsureMessageLogged(context, prefix + "above: " + commentAbove);
                        EnsureMessageLogged(context, prefix + "inside: " + commentInside);
                        if (paramComments != null)
                        {
                            foreach (var paramComment in paramComments)
                            {
                                EnsureMessageLogged(context, prefix + "param: " + paramComment.Name + " comment: " + paramComment.Comment);
                            }
                        }
                    };
                    var funcHelp = context.GetParameterHelpAt(offset);
                    EnsureMessages("e.functionComments.");
                    EnsureMessages("intellisense.getFunctionComments(e.target).");
                }, extension);
            };

            // Global scope
            VerifyImpl(code);

            // Function scope
            VerifyImpl(@"function g() {" + code + @"}");
        }
        #endregion

        [TestMethod]
        public void MemberFunction()
        {
            Verify(@"
                var x = {
                    /* comment above */
                    f: function() { /* comment inside */ }                        
                };
                x.f(|);
            ",
             "comment above", "comment inside");
        }

        [TestMethod]
        public void FuncDeclaration()
        {
            Verify(@"
                function f() {}
                f(|);
            ",
             "", "");

            Verify(@"
                /* func comment */
                function f() {}
                f(|);
            ",
             "func comment", "");

            Verify(@"
                // above
                function f() {
                    // inside
                }
                f(|);
            ",
             "above", "inside");

            Verify(@"
                // above1
                // above2
                function f() {
                    // inside1
                    // inside2
                }
                f(|);
            ",
             "above1\r\nabove2", "inside1\r\ninside2");

            Verify(@"
                function f(/*a*/ a) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a" } });

            Verify(@"
                function f(//a
                           a) {}
                f(|);
            ",
           "", "", new[] { new ParamComment { Name = "a", Comment = "a" } });

            Verify(@"
                function f(/* a comment */ a, /* abc comment */ abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a comment" }, new ParamComment { Name = "abc", Comment = "abc comment" } });

            Verify(@"
                function f(/* a comment */ a, abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a comment" }, new ParamComment { Name = "abc", Comment = "" } });

            Verify(@"
                function f(a, /* abc comment */ abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "" }, new ParamComment { Name = "abc", Comment = "abc comment" } });
        }

        [TestMethod]
        public void FuncVariable()
        {
            Verify(@"
                // above
                var f = function() {}
                f(|);
            ", "above", "");

            Verify(@"
                var f = function () {}
                f(|);
            ", "", "");

            Verify(@"
                // above
                var f = function(/*a*/ a) { /* inside */ }
                f(|);
            ",
             "above", "inside", new[] { new ParamComment { Name = "a", Comment = "a" } });
        }

        [TestMethod]
        public void Annotate()
        {
            // Make sure we get all the annotation comments in function help extension handler
            Verify(@"
                var x = { f: function() {} };
                intellisense.annotate(x, {
                    /* comment above */
                    f: function(/*a*/ a) { /* comment inside */ }
                });
                x.f(|);
            ",
             "comment above", "comment inside", new[] { new ParamComment { Name = "a", Comment = "a" } });
        }

        [TestMethod]
        [WorkItem(364466)]
        public void CommentAssociation()
        {
            Verify(@"
                (function () {
                    // Comment: This should not be associated with the declaration below
                })();

                var foo = function () {};
                foo(|);",
                "", "");

            Verify(@"
                var obj = {};
                (function () {
                    // Comment: This should not be associated with the assignment below
                })();

                obj.foo = function () {};
                obj.foo(|);",
                "", "");
        }
    }

    [TestClass]
    public class CompletionItemCommentsTests : DirectAuthorTest
    {
        #region Verify
        private void Verify(string code, string commentBefore, string funcComment, bool func = true, string itemName = "f", string[] contextFiles = null)
        {
            string extension = @"
                intellisense.addEventListener('statementcompletionhint', function (e) { 
                        intellisense.logMessage('e.completionItem.comments: ' + e.completionItem.name + ': ' + e.completionItem.comments);
                        if (typeof e.completionItem.value == 'function') {
                            var c = intellisense.getFunctionComments(e.completionItem.value);
                            var inside = c.inside || '';
                            intellisense.logMessage('getFunctionComments: ' + inside);
                        }
                    });
                ";

            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    var hint = completions.GetHintFor(itemName);
                    EnsureMessageLogged(context, "e.completionItem.comments: f: " + commentBefore);
                    if (func)
                    {
                        EnsureMessageLogged(context, "getFunctionComments: " + funcComment);
                    }
                }, contextFiles != null ? contextFiles.Concat(extension).ToArray() : new[] { extension });
            };

            // Global scope
            VerifyImpl(code);
            // Function scope
            VerifyImpl(@"
                    function g() {
                    " +
                code +
                @"
                    }
                    ");

            // With a comment before
            if (commentBefore != "")
            {
                VerifyImpl(@"
                    function g() {
                        // outer comment
                    " +
                    code +
                    @"
                }
                ");
            }

            // With additional extension before.
            // Verify that additional extension doesn't mess anything during data roundtripping.            
            var precedingExtension = @"
                intellisense.addEventListener('statementcompletion', function (e) { var items = e.items; });
                intellisense.addEventListener('statementcompletionhint', function (e) { var item = e.completionItem; });
            ";
            VerifyImpl(precedingExtension + code);
        }
        #endregion


        [TestMethod]
        public void Annotate()
        {
            Action<string, string, string, string, bool> VerifyAnnotate = (code, annotate, commentBefore, funcComment, func) =>
            {
                // Verify when annotate is in the same file
                Verify(code + annotate + @";x.|;", commentBefore, funcComment, func);
                // Verify when annotate is in a context file
                Verify(@";x.|;", commentBefore, funcComment, func, "f", new string[] { code, annotate });
            };

            // Primitive field value annotated, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: 0
                };",
                @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", false);

            // Object field value annotated, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: {}
                };",
                @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", false);

            // Primitive field annotation, use a function as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: 0
                };",
                   @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "", false);

            // Annotate a member function, use a function as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: function() {}
                };", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "comment inside", true);

            // Annotate a member function, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: function() {}
                };
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", true);

            // Annotate a this assignment function, use a function as annotation value.
            VerifyAnnotate(@"
                function X() {
                    /* func comment */
                    this.f = function() {
                    };
                }
                var x = new X();
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "comment inside", true);

            // Annotate a property assignment
            VerifyAnnotate(@"
                var x = {};
                // unwanted
                x.f = function() { /* unwanted */ };
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() { /* comment inside */ }
                });
                ", "comment", "comment inside", true);
        }

        [TestMethod]
        public void ThisAssignmentComments()
        {
            Verify(@"
                function X() {
                    /* func comment */
                    this.f = function() {
                    };
                }
                var x = new X();
                x.|;
            ",
             "func comment", "", true);
        }

        [TestMethod]
        public void Trimming()
        {
            // leading spaces after new lines
            Verify("/*\r\n \r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", "\r\n a \r\n a", "", true);

            // empty, 1 line
            Verify(@"
                //
                function f() {}
                ;|;
            ", "", "", true);

            // empty, 2 lines
            Verify(@"
                //
                //
                function f() {}
                ;|;
            ", "", "", true);

            // no space
            Verify(@"
                /**/
                function f() {}
                ;|;
            ", "", "", true);

            // 1 space
            Verify(@"
                /* */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 spaces
            Verify(@"
                /*  */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 spaces
            Verify(@"
                /*   */
                function f() {}
                ;|;
            ", "", "", true);

            // 1 tab
            Verify(@"
                /*  */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 tabs
            Verify(@"
                /*      */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 tabs
            Verify(@"
                /*          */
                function f() {}
                ;|;
            ", "", "", true);

            // 4 tabs
            Verify(@"
                /*              */
                function f() {}
                ;|;
            ", "", "", true);

            // 1 new line
            Verify(@"
                /*
                */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 new lines
            Verify(@"
                /*
    
                */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 new lines
            Verify(@"
                /*
    

                */
                function f() {}
                ;|;
            ", "", "", true);

            // no spaces, 1 char
            Verify(@"
                /*a*/
                function f() {}
                ;|;
            ", "a", "", true);

            // no spaces, 2 chars
            Verify(@"
                /*aa*/
                function f() {}
                ;|;
            ", "aa", "", true);

            // no spaces, 3 chars
            Verify(@"
                /*aaa*/
                function f() {}
                ;|;
            ", "aaa", "", true);

            // 1 leading space
            Verify(@"
                /* a*/
                function f() {}
                ;|;
            ", "a", "", true);

            // 2 leading spaces
            Verify(@"
                /*  a*/
                function f() {}
                ;|;
            ", " a", "", true);

            // 1 leading new line
            Verify("/*\r\na*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 leading new lines
            Verify("/*\r\n\r\na*/" +
                @"function f() {}
                ;|;
            ", "\r\na", "", true);

            // 3 leading new lines
            Verify("/*\r\n\r\n\r\na*/" +
                @"function f() {}
                ;|;
            ", "\r\n\r\na", "", true);

            // 1 trailing space
            Verify("/*a */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 trailing space
            Verify("/*a  */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 1 trailing new line
            Verify("/*a\r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 trailing new line
            Verify("/*a\r\n\r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // trailing new line and space
            Verify("/*a\r\n */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // trailing space and new line
            Verify("/*a \r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // leading spaces after new lines
            Verify("/*\r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", " a \r\n a", "", true);

            // leading spaces after new lines
            Verify("/*\r\n \r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", "\r\n a \r\n a", "", true);
        }

        [TestMethod]
        public void PropertyAssignmentComments()
        {
            Verify(@"
                var x = {
                    f: undefined
                };
                
                // f comment
                x.f = function() {};

                x.|;
            ",
             "f comment", "", true);

            Verify(@"
                var x = {
                    // field comment
                    f: undefined
                };
                
                // unwanted
                x.f = function() {};

                x.|;
            ",
             "field comment", "", true);

            Verify(@"
                var x = {};
                
                // line 1
                x.f = function() {};

                x.|;
            ",
             "line 1", "", true);

            Verify(@"
                function X() {
                    var x = {};
                
                    // line 1
                    x.f = function() {};

                    x.|;
                }
            ",
             "line 1", "", true);
        }

        [TestMethod]
        public void ParamComments()
        {
            Verify(@"
                function X(/* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(a, /* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(/* unwanted */ a, /* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(a,
                // comment 
                f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            // f is a func and has a comment above -  ?
        }

        [TestMethod]
        public void MixedFieldComments()
        {
            Verify(@"
                var x = {
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    // unwanted 
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /// <field>should be ignored</field> 
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // line 1
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // above
                    f: function() {
                        // line 1
                        // line 1 cont
                    }
                };
                x.|;
            ",
             "above",
             "line 1\r\nline 1 cont", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // above
                    f: function() {
                        ;
                        // should be ignored
                    }
                };
                x.|;
            ",
             "above",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted*/
                    a: function() {},
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* line 1*/
                    f: undefined
                };
                x.|;
            ", "line 1", "", false);

            Verify(@"
                var x = {
                    /// unwanted
                    /* line 1*/
                    f: undefined
                };
                x.|;
            ", "line 1", "", false);

            Verify(@"
                var x = {
                    /// unwanted
                    a: undefined,
                    /// unwanted
                    /* line 1*/
                    f: {
                        // unwanted
                    }
                };
                x.|;
            ", "line 1", "", false);
        }

        [TestMethod]
        public void FieldOfPrimitiveValue()
        {
            Verify(@"
                var x = {
                    /// unwanted
                    /* line 1*/
                    f: 0
                };
                x.|;
            ", "line 1", "", false);
        }

        [TestMethod]
        public void VarOfFunctionValue()
        {
            Verify(@"
                var x = {};
                /// a func comment
                function z() {
                    /// z func comment
                    var inner = function inner() {}
                }
            
                /// a func comment
                x.a = function a()  {
                    /// a func comment
                    var inner = function inner() {}
                };

                /// a func comment
                x.b = function b()  {
                    /// b func comment
                    var inner = function inner() {}
                };

                /// a func comment
                x.f = function f()  {
                    /// a func comment
                };

                x.f|
            ",
             "a func comment", "a func comment", true);

            Verify(@"
                // func comment
                function x() {};
                var f=x;
                ;|;
            ",
             "func comment",
             "", true);

            // Should use var comment
            Verify(@"
                // unwanted
                function x() {};
                // var comment
                var f=x;
                ;|;
            ",
             "var comment",
             "", true);
        }

        [TestMethod]
        public void VarOfPrimitiveValue()
        {
            Verify(@"
                // var comment
                var f=0;
                ;|;
            ",
             "var comment", "", false);

            Verify(@"
                /// unwanted
                // var comment
                var f='';
                ;|;
            ",
             "var comment", "", false);
        }

        [TestMethod]
        public void MixedVarComments()
        {
            // Empty line prevents comment blocks to be considered adjacent
            Verify(@"
                // unwanted
                /* unwanted block*/ 

                /* line 1*/ var f;
                /* unwanted*/ var x; /* unwanted*/  
                ;|;
            ",
             "line 1", "", false);

            Verify(@"
                // unwanted
                var x = { /* unwanted */ } /* unwanted */;
                /* line 1*/ var f = {
                    /* unwanted */ 
                };
                ;|;
            ",
            "line 1",
            "", false);

            Verify(@"
                // line 1
                var f = function() {};
                ;|;
            ",
             "line 1",
             "", true);

            Verify(@"
                // unwanted
                /* line 1*/ var f = function() {
                    /* inside */ 
                };
                ;|;
            ",
             "line 1",
             "inside", true);


            Verify(@"
                /* unwanted */
                // line 1
                // line 1 cont
                var f;
                ;|;
            ",
             "line 1\r\nline 1 cont",
             "", false);

            Verify(@"
                // unwanted
                /* line 1
                   line 1 cont */
                var f;
                ;|;
            ",
             "line 1\r\n                  line 1 cont",
             "", false);

            Verify(@"
                /* line 1*/ var f;
                ;|;
            ",
             "line 1",
             "", false);

            Verify(@"
                // unwanted
                /* line 1*/ var f;
                ;|;
            ",
             "line 1",
             "", false);
        }

        [TestMethod]
        public void NonAdjacentSimpleComments()
        {
            Verify(@"
                var x = {
                    // comment above
                    f:                     
                    function() {
                        // some comment inside
                    }
                };
                var f = x.f;
                ;|
            ",
             "comment above",
             "some comment inside");

            Verify(@"
                // some comment above

                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "",
             "some comment inside");

            Verify(@"
                /* some comment above */

                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "",
             "some comment inside");
        }

        [TestMethod]
        public void MixedFuncComments()
        {
            Verify(@"
/* before */
/* aaabbbb
*/
/* after */
            function f() {}
            ;|;
            ",
             "before \r\naaabbbb\r\n\r\nafter",
             "");

            Verify(@"
                function f() {
                    // a
                    /* */
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                }
                ;|;
            ",
             "",
             "a");

            Verify(@"
                function f() {
                    // a

                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                }
                ;|;
            ",
             "",
             "a");

            Verify(@"
                function f() {
                    // a
                    // a
                    // a
                    // a
                    /* unwanted 
                       unwanted
                       unwanted
                       unwanted
                    */
                    function f1() {}   
                }
                ;|;
            ",
             "",
             "a\r\na\r\na\r\na");

            Verify(@"
                /* unwanted */
                // line 1
                // line 1 cont
                function f() {
                }
                ;|;
            ",
             "line 1\r\nline 1 cont",
             "");

            Verify(@"
                // unwanted
                /// comment above
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                /* unwanted
                   unwanted 
                */
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                /// unwanted
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // line 1
                function f() {}
                ;|;
            ",
             "line 1",
             "");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                    /* line 3 */
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                // some comment
                function f() {
                    /// <signature/>
                }
                ;|;
            ",
             "some comment",
             "<signature/>");

            Verify(@"
                // some comment above
                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "some comment above",
             "some comment inside");
        }
    }

    [TestClass]
    public class JSDocTests : DirectAuthorTest
    {

        [TestMethod]
        public void FunctionHelp()
        {
            //optional parameters test
            PerformRequests(@"
                /**
                    @description some function
                    @param {String=} str some string
                    @param {Function} [someFunction] some other function
                    @returns {Something}
                 */
                var someFunction = function (str, someFunction) { }

                someFunction(|paramsAreOptional|);


                /**
                This function adds two numbers or more together
                *@param {number} A first number
                *@returns {number}
                */
                var add = function (a, A) { return a; }
                add(|paramsAreNotOptional|);
                ",
                (context, offset, data, index) =>
                {
                    var functionHelp = context.GetParameterHelpAt(offset);
                    Console.WriteLine(DumpObject(functionHelp));
                    Assert.IsTrue(functionHelp.FunctionHelp.GetSignatures().Count == 1);
                    var signature = functionHelp.FunctionHelp.GetSignatures().ToEnumerable().First();
                    signature.Description.Except("some function");
                    Assert.IsTrue(signature.GetParameters().Count == 2, "Param count do not match expected");
                    foreach (var param in signature.GetParameters().ToEnumerable())
                    {
                        switch (data)
                        {
                            case "paramsAreOptional":
                                Assert.IsTrue(param.Optional);
                                break;
                            case "paramsAreNotOptional":
                                Assert.IsFalse(param.Optional);
                                break;
                        }
                    }
                },
                false, Paths.JSDocExtensionPath);

            //function type parameters and variable number of arguments test
            PerformRequests(@"
                /**
                    @description some function
                    @param {String...} str some string
                    @param {Function(a,b)} [someFunction] some other function
                    @returns {Something}
                 */
                var someOtherFunction = function (str, someFunction) { }


                /** @deprecated
                    @description description
                */
                function testFunction2(){}
                ",
                (context, offset, data, index) =>
                {

                    var functionHelp = context.GetParameterHelpAt(offset);
                    Assert.IsTrue(functionHelp.FunctionHelp.GetSignatures().Count == 1);
                    var signature = functionHelp.FunctionHelp.GetSignatures().ToEnumerable().First();
                    signature.Description.Except("some function");
                    Assert.IsTrue(signature.GetParameters().Count == 2, "Param count do not match expected");
                    var param1 = signature.GetParameters().ToEnumerable().First();
                    param1.Type.Expect("String");
                    var param2 = signature.GetParameters().ToEnumerable().Last();
                    Assert.IsTrue(param2.Optional);
                    param2.Type.Expect("function");
                    Assert.IsTrue(param2.FunctionParamSignature.GetParameters().Count == 2);
                    var funcParas = param2.FunctionParamSignature.GetParameters().ToEnumerable();
                    funcParas.First().Name.Expect("a");
                    funcParas.Last().Name.Expect("b");

                },
                false, Paths.JSDocExtensionPath);
        }

        [TestMethod]
        public void varlenArgTest()
        {
            PerformRequests(@"
                 /** 
                    @param {Number} [varLenArg...] variable length argument
                  */
                function testFunction1(){}
"
                , (context, offset, data, index) =>
                {
                    var functionHelp = context.GetParameterHelpAt(offset);
                    var signature = functionHelp.FunctionHelp.GetSignatures().ToEnumerable().First();
                    var param = signature.GetParameters().ToEnumerable().First();
                    param.Name.Expect("...");
                    param.Type.Expect("Number");
                    param.Description.Expect("variable length argument");
                    Assert.IsTrue(param.Optional);

                },
                false, Paths.JSDocExtensionPath);
        }

        [TestMethod]
        public void CompletionHintTest()
        {
            PerformRequests(@"
                /**
                    @description some function
                    @param {String=} str some string
                    @returns {Something}
                 */
                var someFunction = function (str) { }

                /**@event
                @description
                    This method returns 
                        a^b @
                 *   @param      {number} [a] @ 
                 first number
                 *
                 *that has multiline description
                  **  
                    @returns {number} a.div.b
                */
                function test(a, b) { }

                ;|;",
                (context, offset, data, index) =>
                {
                    var completion = context.GetCompletionsAt(offset);
                    var completionHint = completion.GetHintFor("someFunction");
                    completionHint.GetFunctionHelp().FunctionName.Expect("someFunction");
                    var signature = completionHint.GetFunctionHelp().GetSignatures().ToEnumerable().First();
                    var param1 = signature.GetParameters().ToEnumerable().First();
                    param1.Description.Expect("some string");
                    param1.Type.Expect("String");
                    Assert.IsTrue(param1.Optional);
                    var param2 = signature.GetParameters().ToEnumerable().Last();
                    param2.Name.Except("...");
                    param2.Type.Except("Number | String");
                    param2.Description.Except("variable length argument");
                    Assert.IsTrue(param2.Optional);

                    signature.GetReturnValue().Type.Except("Something");
                    signature.Description.Except("some function");

                    completionHint = completion.GetHintFor("test");
                    completionHint.GetFunctionHelp().FunctionName.Expect("test");
                    signature = completionHint.GetFunctionHelp().GetSignatures().ToEnumerable().First();
                    param1 = signature.GetParameters().ToEnumerable().First();
                    param1.Description.Expect("@ first number that has multiline description");
                    param1.Type.Expect("number");
                    Assert.IsTrue(param1.Optional);
                    signature.GetReturnValue().Type.Except("number");
                    signature.Description.Except("this method returns a^b @");

                },
                false, Paths.JSDocExtensionPath);

            PerformRequests(@"
                    /** @deprecated */
                    function testFunction1(){}

                    /** @deprecated
                        @description description
                    */
                    function testFunction2(){}
                    ;|;",
            (context, offset, data, index) =>
            {
                var completion = context.GetCompletionsAt(offset);
                var completionHint1 = completion.GetHintFor("testFunction1");
                completionHint1.Description.Expect("[deprecated] ");
                var completionHint2 = completion.GetHintFor("testFunction2");
                completionHint2.Description.Expect("[deprecated]description");
            },
            false, Paths.JSDocExtensionPath);
        }

        [TestMethod]
        public void StatementCompetionTest()
        {

            PerformRequests(@"
                /** @enum */
                var CONFIG = {
                    /**Contains the default values for CONFIG
                        @event
                      */
                    defaults: {}
                };
                ;|;
                ",
                (context, offset, data, index) =>
                {
                    var completion = context.GetCompletionsAt(offset);
                    completion.Item("CONFIG").Glyph.Expect("vs:GlyphGroupEnum");
                }, false, Paths.JSDocExtensionPath);

            PerformRequests(@"
                /** @enum */
                var CONFIG = {
                    /**Contains the default values for CONFIG
                        @event
                      */
                    varevent: {},
                    /**Some random comment
                       @namespace */
                    varnamespace: {},
                    /** @constructor */
                    varconstructor : function(){},
                    /** @class */
                    varclass: function(){}
                };
                ;CONFIG.|;
                ",
                (context, offset, data, index) =>
                {
                    var completion = context.GetCompletionsAt(offset);
                    completion.Item("varevent").Glyph.Expect("vs:GlyphGroupEvent");
                    completion.Item("varnamespace").Glyph.Expect("vs:GlyphGroupNamespace");
                    completion.Item("varconstructor").Glyph.Expect("vs:GlyphGroupClass");
                    completion.Item("varclass").Glyph.Expect("vs:GlyphGroupClass");
                }, false, Paths.JSDocExtensionPath);
        }

        [TestMethod]
        public void TestPrivateIgnoreTags()
        {
            PerformCompletionRequests(@"
                /**@private */
                function pow(a, b) { }
                ;|!pow|;", File.ReadAllText(Paths.JSDocExtensionPath));


            PerformCompletionRequests(@"
                /**@ignore */
                function pow(a, b) { }
                ;|!pow|;", File.ReadAllText(Paths.JSDocExtensionPath));


            PerformCompletionRequests(@"
                /**@someTag */
                function pow(a, b) { }
                ;|pow|;", File.ReadAllText(Paths.JSDocExtensionPath));
        }
    }

    [TestClass]
    public class CompletionExtensionsAdvanced : DirectAuthorTest
    {
        [TestMethod]
        public void CompletionEventScope()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('scope: ' + event.scope);
                });
                f|unction foo() {
                    t|his.bar = w|indow.Math;
                }
                t|his.isNaN(27);
                d|ocument.getElementById('foo');
                new f|oo().bar.toString();
                with (|Math) {
                    c|os(90).toPrecision(2);
                }",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "scope: global").Expect(index + 1);
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    intellisense.logMessage('scope: ' + event.scope);
                });
                function foo() {
                    this.|bar = window.|Math;
                }
                this.i|sNaN(27);
                document.|getElementById('foo');
                new foo().|bar.|toString();
                with (Math) {
                    cos(90).t|oPrecision(2);
                }",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "scope: members").Expect(index + 1);
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    if(event.scope == 'member') {
                        event.scope = 'global';
                    }
                    else {
                        event.scope = 'member';
                    }
                });
                var foo = { bar: 'baz' };
                f|oo.|bar;
                }",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
            });

            string[] invalidValues = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.Types,
                new string[] { "gnotlobal", "m" });

            foreach (string invalidValue in invalidValues)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.scope = {0};
                        intellisense.logMessage('>>>> OK');
                    }});|", invalidValue),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsTrue(completions.Count > 0);
                    EnsureMessageLogged(context, ">>>> OK");
                });
            }
        }

        [TestMethod]
        public void CompletionItems()
        {
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (event) {
                    while (event.items.length > 0) {
                        event.items.pop();
                    }
                    event.items.push({ name: 'test', kind: 'property' });
                });
                var x = { f: function() {}, p: 'property' };
                x.|                    
            ",
            (completions, data, i) =>
            {
                completions.AsEnumerable().Count().Expect(1);
                completions.AsEnumerable().ExpectContains("test");
            });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (event) {
                    delete event.items;
                });
                var x = { f: function() {}, p: 'property' };
                x.|                    
            ",
            (completions, data, i) =>
            {
                completions.AsEnumerable().ExpectContains("f", "p", "toString");
            });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function (event) {
                    event.items = 'test';
                });
                var x = { f: function() {}, p: 'property' };
                x.|                    
            ",
            (completions, data, i) =>
            {
                completions.AsEnumerable().ExpectContains("f", "p", "toString");
            });

            foreach (string internationalString in ExtensibilityTestData.International)
            {
                PerformCompletionRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion',  function (event) {{
                            event.items.push({{ name: '{0}', kind: 'property' }});
                    }});
                    var x = {{ f: function() {{}}, p: 'property' }};
                    x.|                    
                ", internationalString),
                (completions, data, i) =>
                {
                    completions.AsEnumerable().ExpectContains("f", "p", internationalString);
                });
            }
        }

        [TestMethod]
        public void CompletionItemName()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (value) {
                        if (value.name == 'delete') {
                            value.name = 'test';
                        }
                    });
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                Assert.IsTrue(completions.Count > 0);
                completions.ToEnumerable().ExpectNotContains("delete");
                completions.ToEnumerable().ExpectContains("test");
            });

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (value) {
                        if (value.name == 'getElementById') {
                            intellisense.logMessage('>>>> OK');
                        }
                    });
                });
                document.|getElementById('foo');
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, ">>>> OK");
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (value) {
                        if (value.name == 'parentElement') {
                            intellisense.logMessage('>>>> OK');
                        }
                    });
                });
                document.getElementById('foo').|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, ">>>> OK");
            }, false, Paths.DomWindowsPath);

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (value) {
                        value.name == {};
                    });
                });|Math|
            ");
        }

        [TestMethod]
        public void CompletionItemGlyph()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (value) {
                        value.glyph = { foo: 'bar'};    
                    });
                    intellisense.logMessage('>>>> OK');
                });|",
           (context, offset, data, index) =>
           {
               var completions = context.GetCompletionsAt(offset);
               Assert.IsTrue(completions.Count > 0);
               EnsureMessageLogged(context, ">>>> OK");
               Assert.IsNull(completions.Item("Array").Glyph);
           });
        }

        [TestMethod]
        public void CompletionItemKind()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        switch (item.name) {
                            case 'bar':
                                intellisense.logMessage('bar is: ' + item.kind)
                                break;
                            case 'baz':
                                intellisense.logMessage('baz is: ' + item.kind)
                                break;
                            case 'case':
                                intellisense.logMessage('case is: ' + item.kind)
                                break;
                            case 'eval':
                                intellisense.logMessage('eval is: ' + item.kind)
                                break;
                            case 'Math':
                                intellisense.logMessage('Math is: ' + item.kind)
                                break;
                            case 'onload':
                                intellisense.logMessage('onload is: ' + item.kind)
                                break;
                            default:
                                break;
                        }
                    });
                });
                function foo(bar) {
                    var baz = 'baz';|
                }
                foo(1);
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, "bar is: parameter");
                EnsureMessageLogged(context, "baz is: variable");
                EnsureMessageLogged(context, "case is: reserved");
                EnsureMessageLogged(context, "eval is: method");
                EnsureMessageLogged(context, "Math is: field");
                EnsureMessageLogged(context, "onload is: property");
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        if (item.name == 'test') {
                            item.kind = 'method';
                        }
                    });
                });
                var x = { test: 1 };
                x.|test;
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                Assert.IsTrue(completions.Count > 0);
                completions.Item("test").Kind.Expect(AuthorCompletionKind.ackMethod);
            });

            string[] invalidValues = ExtensibilityTestData.CombineData(
               ExtensibilityTestData.Types,
               new string[] { "global", "f" });

            foreach (string value in invalidValues)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.items.forEach(function (item) {{
                            if (item.name == 'test') {{
                                item.kind = {0};
                            }}
                        }});
                    }});
                    var x = {{ test: 1 }};
                    x.|test;
                ", value),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                    Assert.IsTrue(completions.Count > 0);
                    completions.ToEnumerable().ExpectNotContains("test");
                });
            }
        }

        [TestMethod]
        public void CompletionItemParent()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    var doesNotHaveParent = false;
                    event.items.forEach(function (item) {
                        if (item.parentObject) {
                            if (item.parentObject != foo) {
                                intellisense.logMessage('Parent is not a foo');
                            }
                        }
                        else {
                            intellisense.logMessage('No parent was found');
                        }
                    });
                });

                var foo = {
                    bar: 'baz'
                }
                foo.|bar;
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "Parent is not a foo").Expect(0);
                context.LoggedMessages.Count(m => m == "No parent was found").Expect(0);
            });

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    var doesNotHaveParent = false;
                    event.items.forEach(function (item) {
                        if (item.parentObject && item.parentObject != this) {
                            intellisense.logMessage('Parent found');
                        }
                    });
                });

                var foo = {
                    bar: 'baz'
                }
                f|oo.bar;
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count(m => m == "Parent found").Expect(0);
            });

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        item.parent = intellisense.undefinedWithCompletionsOf(1);
                    });
                });

                var foo = {
                    string: 'string',
                    number: 1,
                    array: [1,2],
                    object: {},
                    func: function(){},
                    nul: null,
                    undef: undefined
                }

                foo.|string,number,array,object,func,nul,undef,toString|
            ");

            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        delete item.parent;
                    });
                });

                var foo = {
                    string: 'string',
                    number: 1,
                    array: [1,2],
                    object: {},
                    func: function(){},
                    nul: null,
                    undef: undefined
                }

                foo.|string,number,array,object,func,nul,undef,toString|
            ");

            string[] invalidValues = ExtensibilityTestData.CombineData(
               ExtensibilityTestData.Types,
               new string[] { "this", "document" });

            foreach (string value in invalidValues)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.items.forEach(function (item) {{
                            item.parentObject = {0};
                        }});
                        intellisense.logMessage('>>>> OK');
                    }});
                    var x = {{ test: 1 }};
                    x|.test;
                ", value),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                    Assert.IsTrue(completions.Count > 0);
                    EnsureMessageLogged(context, ">>>> OK");
                });

                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.items.forEach(function (item) {{
                            item.parentObject = {0};
                        }});
                        intellisense.logMessage('>>>> OK');
                    }});
                    var x = {{ test: 1 }};
                    x.|test;
                ", value),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfAny);
                    Assert.IsTrue(completions.Count > 0);
                    EnsureMessageLogged(context, ">>>> OK");
                }, false, Paths.DomWindowsPath);
            }
        }

        [TestMethod]
        public void CompletionItemValue()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        if (Object.keys(foo).indexOf(item.name) >= 0) {
                            intellisense.logMessage(item.name + ' has a value of ' + item.value);
                        }
                    });
                });

                var foo = {
                    string: 'string',
                    number: 1,
                    array: [1,2],
                    obj: {},
                    func: function(){},
                    nul: null,
                    undef: undefined
                }

                foo.|
            ",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                EnsureMessageLogged(context, "string has a value of string");
                EnsureMessageLogged(context, "number has a value of 1");
                EnsureMessageLogged(context, "array has a value of 1,2");
                EnsureMessageLogged(context, "obj has a value of [object Object]");
                EnsureMessageLogged(context, "func has a value of function(){}");
                EnsureMessageLogged(context, "nul has a value of null");
                EnsureMessageLogged(context, "undef has a value of undefined");
            });

            string[] Values = ExtensibilityTestData.CombineData(
              ExtensibilityTestData.Types,
              new string[] { "this", "document" });

            foreach (string value in Values)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.items.forEach(function (item) {{
                            item.value = {0};
                        }});
                        intellisense.logMessage('>>>> OK');
                    }});|", value),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsTrue(completions.Count > 0);
                    EnsureMessageLogged(context, ">>>> OK");
                }, false, Paths.DomWindowsPath);
            }
        }

        [TestMethod]
        public void CompletionItemScope()
        {
            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        if (item.scope != 'global') {
                            intellisense.logMessage(item.name + item.scope);
                        }
                    });
                    intellisense.logMessage('>>>> OK');
                });|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count().Expect(1);
                EnsureMessageLogged(context, ">>>> OK");
            }, false, Paths.DomWindowsPath);

            PerformRequests(@"
                intellisense.addEventListener('statementcompletion', function (event) {
                    event.items.forEach(function (item) {
                        if (item.scope != 'member') {
                            intellisense.logMessage(item.name + item.scope);
                        }
                    });
                    intellisense.logMessage('>>>> OK');
                });
                var x = {
                    test: '123'
                }
                x.|",
            (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                Assert.IsTrue(completions.Count > 0);
                context.LoggedMessages.Count().Expect(1);
                EnsureMessageLogged(context, ">>>> OK");
            });

            string[] values = ExtensibilityTestData.CombineData(
                ExtensibilityTestData.Types,
                new string[] { "gnotlobal", "m" });

            foreach (string value in values)
            {
                PerformRequests(string.Format(@"
                    intellisense.addEventListener('statementcompletion', function (event) {{
                        event.items.forEach(function (item) {{
                            item.scope = 'global';
                        }});
                    }});
                    v|ar x = {{
                        test: '123'
                    }}
                    function foo (bar) {{
                        b|ar.|charAt(0);
                    }}
                    foo(x.|test);
                ", value),
                (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsTrue(completions.Count > 0);
                }, false, Paths.DomWindowsPath);
            }
        }
    }

    #region Test data
    public static class ExtensibilityTestData
    {
        public static readonly string[] International = 
        {
            // double byte
            "啊齄丂狛狜隣郎隣兀﨩ˊ▇█〞〡￤℡㈱‐ー﹡﹢﹫、〓ⅰⅹ⒈€㈠㈩ⅠⅫ！￣ぁんァヶΑ︴АЯаяāɡㄅㄩ─╋︵﹄︻︱︳︴ⅰⅹɑɡ〇€",

            //Four byte: Ext-A
            "㐀㒣㕴㕵㙉㙊䵯䵰䶴䶵",

            //Four byte: Mongolian
            "ᠠᡷᢀᡨᡩᡪᡫ",

            //Four byte: Tibetan
            "ༀཇཉ",

            //Four byte: Yi
            "ꀀꒌꂋꂌꂍꂎꂔꂕ",

            //Four byte: Uighur
            "ئبتجدرشعەﭖﭙﯓﯿﺉﺒﻺﻼ",

            //Four byte: Tai Le
            "ᥐᥥᥦᥧᥨᥭᥰᥱᥲᥴ",

            //Four byte: Hangul        
            "ᄓᄕᇬᇌᇜᇱㄱㅣ가힝"
        };

        public static readonly string[] InternationalPartialSupported = 
        {
            //Four byte: Ext-B (Not supported in VS as function names, parameter names etc)
            "𠀀𠀁𠀂𠀃𪛑𪛒𪛓𪛔𪛕𪛖"
        };

        public static readonly string[] Types = 
        { 
            "null",
            "undefined",
            "[]",
            "{}",
            "/test/g",
            "''",
            "1",
            "true",
            "function(){}"
        };

        public static string MultiLine = @" \
Multiple \
lined \
string \
 ";

        public static string Long
        {
            get
            {
                // build a string 4 * 2 ^ 10 characters long
                StringBuilder builder = new StringBuilder("test");
                for (int count = 0; count < 10; count++)
                {
                    builder.Append(builder.ToString());
                }
                return builder.ToString();
            }
        }

        public static string Exception = "throw new Error()";

        public static string[] All
        {
            get
            {
                return ExtensibilityTestData.CombineData(
                    ExtensibilityTestData.International,
                    ExtensibilityTestData.InternationalPartialSupported,
                    ExtensibilityTestData.Types,
                    new string[] { 
                        ExtensibilityTestData.Long,
                        ExtensibilityTestData.MultiLine,
                        ExtensibilityTestData.Exception
                    });
            }
        }

        public static string[] CombineData(params string[][] dataSets)
        {
            List<string> combined = new List<string>();

            foreach (string[] data in dataSets)
            {
                combined.AddRange(data);
            }
            return combined.ToArray();
        }
    }
    #endregion
}
