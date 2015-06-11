using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.BPT.Tests.DirectAuthor;

namespace DirectAuthorTests
{
    [TestClass]
    public class ParameterHelpTestsBase : DirectAuthorTest
    {
        protected IEnumerable<AuthorParameterHelp> CollectParameterRequests(string text)
        {
            var results = new List<AuthorParameterHelp>();
            PerformRequests(text, (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                results.Add(help);
            }, AdditionalContextFiles);
            return results.ToArray();
        }

        protected void ValidateParameterHelpAvailable(string code, params string[] expectedParams)
        {
            PerformParameterRequests(code, (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                if (expectedParams.Length > 0)
                {
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    AssertAreStructurallyEqual(expectedParams, signature.GetParameters().ToEnumerable().Select(p => p.Name).ToArray());
                }
            });
        }

        protected void ValidateCurrentParameterIndex(string code, bool expectFunctionHelp)
        {
            PerformParameterRequests(code,
            (help, data, index) =>
            {
                if (expectFunctionHelp)
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    if (data == "x")
                    {
                        help.ParameterIndex.Expect(-1);
                    }
                    else
                    {
                        int expectedIndex = int.Parse(data);
                        help.ParameterIndex.Expect(expectedIndex);
                    }
                }
                else
                {
                    Assert.IsNull(help.FunctionHelp);
                    help.ParameterIndex.Expect(0);
                }
            });
        }

        protected void VerifyFunctionDocComments(string text, string expectedDocComments)
        {
            var requests = CollectParameterRequests(text);
            requests.Expect(expectedDocComments);
        }

        protected string[] CombinedContextFiles(string[] contextFiles)
        {
            var allContextFiles = new List<string>(contextFiles);
            allContextFiles.AddRange(AdditionalContextFiles);
            return allContextFiles.ToArray();
        }
    }

    public static class ParameterHelpHelpers
    {
        public static string DumpParameterHelp(this IEnumerable<AuthorParameterHelp> requests)
        {
            return DirectAuthorTest.DumpObject(requests, (name) => { return name != "SourceFileHandle"; });
        }

        public static void Expect(this IEnumerable<AuthorParameterHelp> parameterHelp, string expect)
        {
            var actual = parameterHelp.DumpParameterHelp();
            Assert.AreEqual(expect, actual);
        }

        public static void ExpectFunction(this IEnumerable<AuthorParameterHelp> parameterHelp, string expectFunctionName)
        {
            bool found = false;
            foreach (var request in parameterHelp)
            {
                if (expectFunctionName == request.FunctionHelp.FunctionName)
                {
                    found = true;
                    break;
                }
            }
            Assert.IsTrue(found, string.Format("Parameter help for function: '{0}' was not found.", expectFunctionName));
        }
    }

    [TestClass]
    public class ParameterHelpTests : ParameterHelpTestsBase
    {
        [TestMethod]
        [WorkItem(426016)]
        public void BoundFunction()
        {
            PerformParameterRequests(
            @"
                function changeGlobal(n1, n2, n3, n4) {
                    this.test = "" + n1 + n2 + n3 + n4;
                }
                var g = changeGlobal.bind(null, 1, 2);
                var o = new g(|3, 4);
            ",
             (help, data, offset) =>
             {
                 Assert.IsNotNull(help.FunctionHelp);
             });
        }

        [TestMethod]
        [WorkItem(510260)]
        public void BindHelp()
        {
            PerformParameterRequests(@"
                function test(a, b) { 
                    /// <param name=""a"" type=""String"">Some parameter</param>
                    /// <param name=""b"" type=""Number"">Some parameter</param>
                    return this.bar + a + b; 
                }
 
                var foo = { bar: 1 };
                var prisec = test.bind(foo, 2);
                prisec(|3);
                ", (help, data, offset) =>
                 {
                     Assert.IsNotNull(help.FunctionHelp);
                 });
        }

        [TestMethod]
        [WorkItem(385192)]
        public void CallingUndefinedFunctionAfterInfiniteLoop()
        {
            PerformParameterRequests(
            @"
                function foo(a, b) { }
                function f() {
                    while(true) {}
                    alert2(); 
                    foo(|);
                }
            ",
             (help, data, offset) =>
             {
                 Assert.IsNotNull(help.FunctionHelp);
             }, 200);
        }

        [TestMethod]
        public void TaggedUndefined()
        {
            ValidateParameterHelpAvailable(
            @"
                function f2(a) { return intellisense.undefinedWithCompletionsOf(''); }
                f2().split(|);
            ");
        }

        [TestMethod]
        public void DomConstructorMethods()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;

            PerformRequests("new Worker(|);", (context, offset, data, index) =>
            {
                new[] { context.GetParameterHelpAt(offset) }.Expect(@"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 12 }, FunctionHelp = 
        new { FunctionName = ""Worker"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""stringUrl"", Type = ""String"", Optional = False }
                    }
                }
            }
        }
    }
}");
            }, domjs);

            PerformRequests("new WebSocket(|);", (context, offset, data, index) =>
            {
                new[] { context.GetParameterHelpAt(offset) }.Expect(@"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 15 }, FunctionHelp = 
        new { FunctionName = ""WebSocket"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""url"", Type = ""String"", Optional = False }, 
                        new { Name = ""protocols"", Type = ""String"", Optional = True }
                    }
                }, 
                new { Parameters = new [] { 
                        new { Name = ""url"", Type = ""String"", Optional = False }, 
                        new { Name = ""protocols"", Type = ""Array"", ElementType = ""String"", Optional = True }
                    }
                }
            }
        }
    }
}");
            }, domjs);

            PerformRequests("new MSCSSMatrix(|);", (context, offset, data, index) =>
            {
                new[] { context.GetParameterHelpAt(offset) }.Expect(@"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 17 }, FunctionHelp = 
        new { FunctionName = ""MSCSSMatrix"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""text"", Type = ""String"", Optional = True }
                    }
                }
            }
        }
    }
}");
            }, domjs);

            // Validate objects created via special constructors have the required members.
            PerformCompletionRequests(@"
                var worker = new Worker('');
                worker.|postMessage,terminate,onmessage,onerror|;
                var webSocket = new WebSocket('');
                webSocket.|send,close,onerror,onmessage,onopen,url|;
                WebSocket.|OPEN,CLOSING,CONNECTING,CLOSED,!send|;
                var matrix = new MSCSSMatrix('');
                matrix.|a,b,c,d,e|;
            ", domjs);
        }

        [TestMethod]
        [WorkItem(230115)]
        public void Extent()
        {
            string text = @"
                function b() {
                      document.valueOf().valueOf().valueOf().valueOf().valueOf().valueOf(|a)
                }
            ";
            PerformParameterRequests(text,
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var expectedExtentStart = text.LastIndexOf('.') + 1;
                var expectedExtentEnd = text.LastIndexOf(')');
                help.Region.Offset.Expect(expectedExtentStart);
                help.Region.Length.Expect(expectedExtentEnd - expectedExtentStart);
            });

            text = @"
                function abcd(a,b) {}
                abcd(|);
            ";
            PerformParameterRequests(text,
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var expectedExtentStart = text.LastIndexOf("abcd");
                var expectedExtentEnd = text.LastIndexOf(')');
                help.Region.Offset.Expect(expectedExtentStart);
                help.Region.Length.Expect(expectedExtentEnd - expectedExtentStart);
            });

        }

        [TestMethod]
        [WorkItem(212846)]
        public void CJKCharsInDocComments()
        {
            PerformParameterRequests(@"
                function GB18030_CJKUnifiedExtensionB(x, y) {
                    /// <summary>𢪾𥚄𨆞𤢲𩬆𨷊𧳓</summary>
                    /// <param name='x' type='String'>𣚿𦒚𣐗𡷀</param>
                    /// <param name='y' type='String'>𠿣𣫏𢀎𩉂</param>
                    return '';
                GB18030_CJKUnifiedExtensionB(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                var args = signature.GetParameters().ToEnumerable().ToArray();
                args[0].Description.Expect("𣚿𦒚𣐗𡷀");
            });
        }

        [TestMethod]
        public void UseMemberDocComment()
        {
            PerformParameterRequests(@"
                var x = {
                    /// <field locid='locid' externalFile='externalFile' externalid='externalid' helpKeyword='helpkeyword'>f desc</field>
                    f: function(a) {}
                };
                x.f(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                signature.Description.Expect("f desc");
                signature.Locid.Expect("locid");
                signature.ExternalFile.Expect("externalFile");
                signature.Externalid.Expect("externalid");
                signature.HelpKeyword.Expect("helpkeyword");
            });

            PerformParameterRequests(@"
                var x = {
                    /// <field>f desc</field>
                    f: function(a) {
                        /// <signature>
                        /// </signature>
                    }
                };
                x.f(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                signature.Description.Expect(null);
            });

            PerformParameterRequests(@"
                var x = {
                    /// <field>field desc</field>
                    f: function(a) {
                        /// <summary>func desc</summary>
                    }
                };
                x.f(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                signature.Description.Expect("func desc");
            });

            PerformParameterRequests(@"
                var x = {
                    /// <field>f desc</field>
                    f: 0
                };
                x.f = function abc() { }
                x.f(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                signature.Description.Expect("f desc");
            });

        }

        [TestMethod]
        [WorkItem(192364)]
        public void ErrorCorrectionNoLCurly()
        {
            PerformParameterRequests(@"

function Invalid content() {
    function invalidContent(a, b) {
        /// some text
    }
    invalidContent();
}

function MissingSignatureClosingTag() {
    function missingSignatureClosingTag(a, b) {
        /// <signature>
        ///     <summary>missing signature closing tag</summary>
        ///     <param name='a' type='String'>parameter a</param>
        ///     <param name='b' type='Number'>parameter b</param>
        ///     <returns type='String'>return description</returns>
    }
    missingSignatureClosingTag(|);
}


            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        [WorkItem(192364)]
        public void ErrorCorrectionNoParamName()
        {
            PerformParameterRequests(@"
                function MissingParamName() {
                    function missingParamName(a, b) {
                        /// <param type='String'>parameter a</param>
                    }
                    missingParamName();
                }

                function Invalid content() {
                    function invalidContent(a, b) {
                        /// some text
                    }
                    invalidContent(|);
                }
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        [WorkItem(214842)]
        public void BuiltInFunctionAsAnonymous()
        {
            ValidateParameterHelpAvailable(
            @" (function() { return 'hi'.toLower; })(|
            ");
        }

        [TestMethod]
        [WorkItem(90468)]
        [WorkItem(245387)]
        public void SubsumedFunction()
        {
            // Inside if clause
            ValidateParameterHelpAvailable(
            @"
                if(f2(|
                function f2() { }
            ");

            // Inside if body
            ValidateParameterHelpAvailable(
            @"
                if(1) {
                 f2() + f2(| 
                 function f2() { }
            ");

            // After else
            ValidateParameterHelpAvailable(
            @"
                var x;
                if(false) 
                    x = 0; 
                else f2(|
                function f2() { }
            ");

            // Inside for
            ValidateParameterHelpAvailable(
            @"
                var x;
                for(var i=0; i<3; i++) {
                    f2(|
                    function f2() { }
                }
            ");

            // After for
            ValidateParameterHelpAvailable(
            @"
                for(var i = f2(| 
                function f2() { }
            ");

            // After for in
            ValidateParameterHelpAvailable(
            @"
                var x;
                for(var i in f2(|
                    function f2() { }
            ");

            // After while
            ValidateParameterHelpAvailable(
            @"
                var x;
                while (x < f2(|
                function f2() { }
            ");

            // After do
            ValidateParameterHelpAvailable(
            @"
                do { 
                    f2(|
                function f2() { }
            ");


            // After do while
            ValidateParameterHelpAvailable(
            @"
                do { } while (f2(|
                function f2() { }
            ");

            // After with
            ValidateParameterHelpAvailable(
            @"
                with (f2(|
                function f2() { }
            ");

            // After case
            ValidateParameterHelpAvailable(
            @"
                var x = 0;
                switch (x) {
                    case f2(|
                function f2() { }
            ");

            // After switch
            ValidateParameterHelpAvailable(
            @"
                switch (f2(|
                function f2() { }
            ");


            // Inside try / catch
            ValidateParameterHelpAvailable(
            @"
                var x;
                try {
                    f2(|
                    function f2() { }
                }
                catch(e) { }
            ");

            // Inside catch
            ValidateParameterHelpAvailable(
            @"
                var x;
                try {
                    
                }
                catch(e) { f2(|
                function f2() { }
            ");

            // After catch
            ValidateParameterHelpAvailable(
            @"
                var x;
                try {
                    
                }
                catch(e f2(|
                function f2() { }
            ");

            // After assignment
            ValidateParameterHelpAvailable(
            @"
                var x;
                x = f2(|
                function f2() { }

            ");

            ValidateParameterHelpAvailable(
            @"
                f3(|
                function f2() { }
                function f3() { }

            ");

            // With nested function
            ValidateParameterHelpAvailable(
            @"
                var b = whatever1(|
                function whatever1(a)
                {
                    /// <summary>Creates Pet class</summary>
                    /// <param name='a' type='Number'>just some input param...</param>
                    /// <returns type='String' />
                    var b;
                    function whatever2(a) {
                        /// <summary>Creates Pet class</summary>
                        /// <param name='a' type='Number'>just some input param...</param>
                        /// <returns type='String' />
                        var b;
                        b =a;
                        return b;
                    }
                    
                    b =a;
                    return b;
                }

            ");

            PerformParameterRequests(
            @"
                var b = whatever1(|
                function whatever1(a)
                {
                    /// <summary>Creates Pet class</summary>
                    /// <param name='a' type='Number'>just some input param...</param>
                    /// <returns type='String' />
                    var b;
                    b =a;
                    return b;
                }

            ", (help, data, index) =>
             {
                 Assert.IsNotNull(help.FunctionHelp);
                 help.FunctionHelp.GetSignatures().ToEnumerable().Single().Description.Expect("Creates Pet class");
             });

            // Hoist inside a function
            ValidateParameterHelpAvailable(
            @"
                function foo (x) {
                    f2(|
                    function f2() { }
                }

            ");

            // Hoist inside catch statement
            ValidateParameterHelpAvailable(
            @"
                var x;
                try {
                }
                catch (e) {
                    f2(|
                    function f2() { }
                }

            ");

            // Hoist inside with statement
            ValidateParameterHelpAvailable(
            @"
                var x = {};
                with (x) {
                    f2(|
                    function f2() { }
                }

            ");

            // Hoist inside while statement
            ValidateParameterHelpAvailable(
            @"
                while (false) {
                    f2(|
                    function f2() { }
                }

            ");

            // Hoist inside for statement
            ValidateParameterHelpAvailable(
            @"
                for (var i = 0; i < 10; i++) {
                    f2(|
                    function f2() { }
                }

            ");

            // Multiple nested subsumed scopes
            ValidateParameterHelpAvailable(
            @"
                function f2() { 
                    f3(
                    function f3(){
                        f4(|
                        function f4(){  }
                        try {
                        }
                        catch(e) {
                            function foo(){
                                var x = 0;
                                with (x){
                                    f5(|
                                    function f5(){  }
                                }
                            }
                        }
                    }
                }

            ");

            // Hoist inside var decl list
            ValidateParameterHelpAvailable(
            @"
                var a = 0, b = 1, d =  f2(|
                function f2() { }
            ");
       }

        [TestMethod]
        [WorkItem(196251)]
        [WorkItem(183346)]
        [WorkItem(196279)]
        public void ArrayMethodsReturnType()
        {
            Action<AuthorParameterHelp, string> CheckReturnType = (help, type) => 
            {
                var returns = help.FunctionHelp.GetSignatures().ToEnumerable().First().GetReturnValue();
                if (type == null && returns == null) return;
                Assert.AreEqual(returns.Type, type);
            };

            PerformRequests("[].forEach(|)", (context, offset, data, index) =>
            {
                CheckReturnType(context.GetParameterHelpAt(offset), null);
            });


            PerformRequests(
            @"
                [].map(|);
                [].filter(|);
            ", (context, offset, data, index) =>
            {
                CheckReturnType(context.GetParameterHelpAt(offset), "Array");
            });

        }

        [TestMethod]
        [WorkItem(182494)]
        public void JQuerySignaturesDescription()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            var jQuery = _session.ReadFile(Paths.JQuery.JQuery_1_4_1VSDocFilePath).Text;
            PerformRequests("jQuery.prototype.|", (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset);
                var hint = completions.GetHintFor("toArray");
                Assert.IsNotNull(hint);
                var help = hint.GetFunctionHelp();
                var signature = help.GetSignatures().ToEnumerable().Single();
                signature.Description.Trim().Expect("Retrieve all the DOM elements contained in the jQuery set, as an array.");
                Assert.IsNotNull(help);
            }, domjs, jQuery);
        }

        [TestMethod]
        [WorkItem(197171)]
        public void DomOptionalArguments()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests("document.body.createTextRange().scrollIntoView(|)", (context, offset, data, index) =>
            {
                var param = context.GetParameterHelpAt(offset).FunctionHelp.GetSignatures().ToEnumerable().Single().GetParameters().ToEnumerable().First();
                param.Name.Expect("fStart");
                Assert.IsTrue(param.Optional);
            }, domjs);
        }

        [TestMethod]
        public void DomOverloads()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests("document.createElement('canvas').getContext('2d').drawImage(|", (context, offset, data, index) =>
            {
                // Verify that drawImage has the overloads specified in AdditionalDomDefinitions.pl
                context.GetParameterHelpAt(offset).FunctionHelp.GetSignatures().ToEnumerable().Count().Expect(3);
            }, domjs);

            var items = new[] { "alpha", "antialias", "depth", "premultipliedAlpha", "preserveDrawingBuffer", "stencil" };
            PerformRequests("document.createElement('canvas').getContext('experimental-webgl').getContextAttributes().|", (context, offset, data, index) =>
            {
                // Verify the context attributes returned from the WebGLRenderingContext has the expected completions.
                context.GetCompletionsAt(offset).ToEnumerable().ExpectContains(items);
            }, domjs);

            items = new[] { "drawImage", "getContextAttributes" };
            PerformRequests("document.createElement('canvas').getContext('').|", (context, offset, data, index) =>
            {
                // Verify get context does not return WebGLRenderingContext or CanvasRenderingContext2D completions for an unsupported context id.
                context.GetCompletionsAt(offset).ToEnumerable().ExpectNotContains(items);
            }, domjs);
        }

        [TestMethod]
        [WorkItem(183885)]
        public void MultipleJQueryIncluded()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            var jQuery = _session.ReadFile(Paths.JQuery.JQuery_1_4_1VSDocFilePath).Text;
            PerformRequests("$(|", (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                Assert.IsNotNull(help);
            }, domjs, jQuery, jQuery, jQuery, jQuery, jQuery, jQuery, jQuery, jQuery, jQuery, jQuery);
        }

        [TestMethod]
        public void Bug180443()
        {
            PerformParameterRequests(
            @"
                if(VSIntellisenseExtensions)
                {
                     intellisense.addEventListener('statementcompletion',  function (e) { });
                     intellisense.addEventListener('signaturehelp',  function (e) { }); 
                }
                set_foo(|
                function set_foo(param)
                {
                /// <signature>
                /// <summary></summary>
                /// </signature>
                function set_bar(x) {}
                }
            ", (help, data, index) =>
             {
                 Assert.IsNotNull(help.FunctionHelp);
             });

            PerformParameterRequests(
            @"
                if(VSIntellisenseExtensions)
                {
                     intellisense.addEventListener('statementcompletion',  function (e) { });
                     intellisense.addEventListener('signaturehelp',  function (e) { 
                        e.functionHelp.functionName = 'set_foo';   
                        e.functionHelp.signatures = [ 
                            { 
                                'description': 'set',
                                'returnValue': {
                                    'type':'Number',
                                    'description':'Some number'
                                 },
                                'params':[ 
                                    { 'description':'Value of x', 'name':'x', 'type':'Number' }
                                ]
                            }
                        ];
                     }); 
                }
                set_foo(|
                function set_foo(param)
                {
                /// <signature>
                /// <summary></summary>
                /// </signature>
                function set_bar(x) {}
                }
            ", (help, data, index) =>
             {
                 Assert.IsNotNull(help.FunctionHelp);
                 help.FunctionHelp.FunctionName.Expect("set_foo");
                 var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                 signature.Description.Expect("set");
                 signature.GetReturnValue().Type.Expect("Number");
                 signature.GetParameters().ToEnumerable().Count().Expect(1);
             });
        }

        [TestMethod]
        public void Bug150800()
        {
            string code = @"
                function add(x, y){
                /// <summary>adds two numbers</summary>
                /// <param name=""x"" type=""Number"">Value of x</param>
                /// <param name=""y"" type=""Number"">Value of y</param>
                /// <returns type=""Number""></returns>
                return x+y;
                }
            ";

            PerformRequests(@"function foo() { add(|", (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                signature.Description.Expect("adds two numbers");
                var p = signature.GetParameters().ToEnumerable().ToArray();
                p[0].Description.Except("Value of x");
                p[1].Description.Except("Value of y");
            }, code);
        }

        [TestMethod]
        public void DefineViaFunctionConstructor()
        {
            ValidateParameterHelpAvailable(@"
                    var f = new Function('a', 'b', 'return a + b;');
                    f(|
            ", "a", "b");

            // Check SourceFileHandle is null
            PerformParameterRequests(@"
                    var f = new Function(""a"", ""a"");
                    f(|
            ", (help, data, index) =>
             {
                 Assert.IsNull(help.FunctionHelp.SourceFileHandle);
             });
        }

        [TestMethod]
        [Ignore]
        public void Bug135826()
        {
            string code = @"
			return function() {
				self;
			};
            encodeURI(|
            ";

            PerformParameterRequests(code, (help, data, index) => {});
        }

        [TestMethod]
        public void Bug90468()
        {
            ValidateParameterHelpAvailable(@"
                    myfunc(|);
                    function myfunc(a) { }
            ", "a");
        }

        [TestMethod]
        public void Bug123984()
        {
            ValidateParameterHelpAvailable(@"
                    function myfunc(a) { }

                    for(var i in x)
                        myfunc(|);

                    for(var i in { a: 1 })
                        myfunc(|);

                    for(var i in [ 1 ])
                        myfunc(|);

                    for(var i in [])
                        myfunc(|);

                    for(var i in {})
                        myfunc(|);
            ", "a");
        }

        [TestMethod]
        public void Bug111891()
        {
            PerformParameterRequests(@"
                function f(a) {}
                f(""x"", { 
                    function(|
            ",
            (help, data, index) =>
            {
                Assert.IsNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        public void Bug124704()
        {
            PerformParameterRequests(@"
                String(|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.ParameterIndex.Expect(0);
            });

        }

        [TestMethod]
        public void Bug125633()
        {
            PerformParameterRequests(@"
                Math.abs(new N|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.FunctionHelp.FunctionName.Expect("abs");
            });

            PerformParameterRequests(@"
                function N(a) {}    
                Math.abs(new N(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            });

            PerformParameterRequests(@"
                function f(a) {}
                var x = new f(|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            });

        }

        [TestMethod]
        public void Bug176776()
        {
            PerformParameterRequests(@"
                var n = 1;
                n.toFixed(|);
                n.toExponential(|);
                n.toPrecision(|);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.FunctionHelp.GetSignatures().ToEnumerable().First().GetReturnValue().Type.Expect("String");
            });
        }

        [TestMethod]
        public void Bug148407()
        {
            PerformParameterRequests(@"
                function test(a, b) {
                   /// <summary>
                   /// Some sample function description
                   /// </summary>
                   /// <signature>
                   ///    <invalid> 
                   ///    <param name='a' type='Number'>The indentification <b>of</b> something</param>
                   ///    <param name='b' type='String'>
                   ///    Some long description for name parameter IVector'1<Number>        
                   /// </param>        
                   /// <returns type='String' />
                   /// </signature>
                   this.aa = new String();
                   this.bb = 10;
                   this.cc = new Object();
                 }
                 do
                 {
                  var d = new test(3, 2|
                 } while (1);
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.ParameterIndex.Expect(1);
            });
        }

        [TestMethod]
        public void BuiltInObjects()
        {
            PerformParameterRequests(@"
                function globalObjectMethods() {
                    eval(|String|);
                    parseInt(|String,Number|);
                    parseFloat(|String|);
                    isNaN(|Number|);
                    isFinite(|Number|);
                    decodeURI(|String|);
                    decodeURIComponent(|String|);
                    encodeURI(|String|);
                    encodeURIComponent(|String|);
                    escape(|String|);
                    unescape(|String|);
                    new Object(|);
                    new Function(|);
                    new Array(|);
                    new String(|);
                    new Boolean(|);
                    new Number(|);
                    new Date(|);
                    new RegExp(|RegExp,String|);
                    new Error(|String|);
                    new EvalError(|String|);
                    new RangeError(|String|);
                    new ReferenceError(|String|);
                    new SyntaxError(|String|);
                    new TypeError(|String|);
                    new URIError(|String|);
                    new DataView(|ArrayBuffer,Number,Number|);
                    new ArrayBuffer(|Number|);
                    new Int8Array(|Number|);
                    new Uint8Array(|Number|);
                    new Int16Array(|Number|);
                    new Uint16Array(|Number|);
                    new Int32Array(|Number|);
                    new Uint32Array(|Number|);
                    new Float32Array(|Number|);
                    new Float64Array(|Number|);
                    ScriptEngine(|);
                    ScriptEngineMajorVersion(|);
                    ScriptEngineMinorVersion(|);
                    ScriptEngineBuildVersion(|);
                    CollectGarbage(|);
                    CanvasPixelArray(|);
                }

                function jsonMethods() {
                    JSON.parse(|String,Function|);
                    JSON.stringify(|Any,Any,Any|);    
                }

                function regexpMethods() {
                    new RegExp().exec(|String|);
                    new RegExp().test(|String|);
                    new RegExp().compile(|RegExp,String|);
                    new RegExp().toString(|);
                }

                function errorMethods() {
                    new Error().toString(|);
                }

                function objectMethods() {
                    Object.getPrototypeOf(|Object|);
                    Object.getOwnPropertyDescriptor(|Object,String|);
                    Object.getOwnPropertyNames(|Object|);
                    Object.create(|Object,Any|);
                    Object.defineProperty(|Object,String,Object|);
                    Object.defineProperties(|Object,Object|);
                    Object.seal(|Object|);
                    Object.freeze(|Object|);
                    Object.preventExtensions(|Object|);
                    Object.isSealed(|Object|);
                    Object.isFrozen(|Object|);
                    Object.isExtensible(|Object|);
                    Object.keys(|Object|);

                    new Object().toString(|);
                    new Object().toLocaleString(|);
                    new Object().valueOf(|);
                    new Object().hasOwnProperty(|String|);
                    new Object().isPrototypeOf(|Any|);
                    new Object().propertyIsEnumerable(|String|);
                }

                function booleanMethods() {
                    new Boolean().toString(|);
                    new Boolean().valueOf(|);
                }

                function functionMethods() {
                    new Function().toString(|);
                    new Function().apply(|Any,Array|);
                    new Function().call(|Any,Any|);
                    new Function().bind(|Any,Any|);
                }

                function arrayMethods() {
                    Array.isArray(|);

                    new Array().toString(|);
                    new Array().toLocaleString(|);
                    new Array().concat(|);
                    new Array().join(|String|);
                    new Array().pop(|);
                    new Array().push(|);
                    new Array().reverse(|);
                    new Array().shift(|);
                    new Array().slice(|Number,Number|);
                    new Array().sort(|);
                    new Array().splice(|);
                    new Array().unshift(|);
                    new Array().indexOf(|);
                    new Array().lastIndexOf(|);
                    new Array().every(|Function,Any|);
                    new Array().some(|Function,Any|);
                    new Array().forEach(|Function,Any|);
                    new Array().map(|Function,Any|);
                    new Array().filter(|Function,Any|);
                    new Array().reduce(|Function,Any|);
                    new Array().reduceRight(|Function,Any|);
                }

                function stringMethods() {
                    String.fromCharCode(|);
                    new String().toString(|);
                    new String().valueOf(|);
                    new String().charAt(|Number|);
                    new String().charCodeAt(|Number|);
                    new String().concat(|);
                    new String().indexOf(|);
                    new String().lastIndexOf(|);
                    new String().localeCompare(|);
                    new String().match(|);
                    new String().replace(|);
                    new String().search(|RegExp|);
                    new String().slice(|);
                    new String().split(|String,Number|);
                    new String().substring(|);
                    new String().toLowerCase(|);
                    new String().toLocaleLowerCase(|);
                    new String().toUpperCase(|);
                    new String().toLocaleUpperCase(|);
                    new String().trim(|);
                    new String().substr(|);
                    new String().anchor(|String|);
                    new String().big(|);
                    new String().blink(|);
                    new String().bold(|);
                    new String().fixed(|);
                    new String().fontcolor(|String|);
                    new String().fontsize(|Number|);
                    new String().italics(|);
                    new String().link(|String|);
                    new String().small(|);
                    new String().strike(|);
                    new String().sub(|);
                    new String().sup(|);
                }

                function numberMethods() {
                    new Number().toString(|);
                    new Number().toLocaleString(|);
                    new Number().valueOf(|);
                    new Number().toFixed(|);
                    new Number().toExponential(|);
                    new Number().toPrecision(|);
                }

                function mathMethods() {
                    Math.abs(|);
                    Math.acos(|);
                    Math.asin(|);
                    Math.atan(|);
                    Math.atan2(|Number,Number|);
                    Math.ceil(|);
                    Math.cos(|);
                    Math.exp(|);
                    Math.floor(|);
                    Math.log(|);
                    Math.max(|);
                    Math.min(|);
                    Math.pow(|);
                    Math.random(|);
                    Math.round(|);
                    Math.sin(|);
                    Math.sqrt(|);
                    Math.tan(|);
                }

                function dateMethods() {
                    Date.parse(|);
                    Date.UTC(|);
                    Date.now(|);
                    new Date().toString(|);
                    new Date().toDateString(|);
                    new Date().toTimeString(|);
                    new Date().toLocaleString(|);
                    new Date().toLocaleDateString(|);
                    new Date().toLocaleTimeString(|);
                    new Date().valueOf(|);
                    new Date().getTime(|);
                    new Date().getFullYear(|);
                    new Date().getUTCFullYear(|);
                    new Date().getMonth(|);
                    new Date().getUTCMonth(|);
                    new Date().getDate(|);
                    new Date().getUTCDate(|);
                    new Date().getDay(|);
                    new Date().getUTCDay(|);
                    new Date().getHours(|);
                    new Date().getUTCHours(|);
                    new Date().getMinutes(|);
                    new Date().getUTCMinutes(|);
                    new Date().getSeconds(|);
                    new Date().getUTCSeconds(|);
                    new Date().getMilliseconds(|);
                    new Date().getUTCMilliseconds(|);
                    new Date().getTimezoneOffset(|);
                    new Date().setTime(|);
                    new Date().setMilliseconds(|);
                    new Date().setUTCMilliseconds(|);
                    new Date().setSeconds(|);
                    new Date().setUTCSeconds(|);
                    new Date().setMinutes(|);
                    new Date().setUTCMinutes(|);
                    new Date().setHours(|);
                    new Date().setUTCHours(|);
                    new Date().setDate(|);
                    new Date().setUTCDate(|);
                    new Date().setMonth(|);
                    new Date().setUTCMonth(|);
                    new Date().setFullYear(|);
                    new Date().setUTCFullYear(|);
                    new Date().toUTCString(|);
                    new Date().toISOString(|);
                    new Date().toJSON(|);
                    new Date().getYear(|);
                    new Date().setYear(|);
                    new Date().toGMTString(|);
                    new Date().getVarDate(|);
                }

                function typedArrayMethods() {
                    new Int8Array(1).set(|Number,Number|);
                    new Int8Array(1).subarray(|Number,Number|);

                    new Uint8Array(1).set(|Number,Number|);
                    new Uint8Array(1).subarray(|Number,Number|);

                    new Int16Array(1).set(|Number,Number|);
                    new Int16Array(1).subarray(|Number,Number|);

                    new Uint16Array(1).set(|Number,Number|);
                    new Uint16Array(1).subarray(|Number,Number|);

                    new Int32Array(1).set(|Number,Number|);
                    new Int32Array(1).subarray(|Number,Number|);

                    new Uint32Array(1).set(|Number,Number|);
                    new Uint32Array(1).subarray(|Number,Number|);

                    new Float32Array(1).set(|Number,Number|);
                    new Float32Array(1).subarray(|Number,Number|);

                    new Float64Array(1).set(|Number,Number|);
                    new Float64Array(1).subarray(|Number,Number|);
                }
                
                function dataViewMethods() {
                    var dataView = new DataView(new ArrayBuffer(1), 0, 1);
                    dataView.getInt8(|Number|);
                    dataView.getUint8(|Number|);
                    dataView.getInt16(|Number,Boolean|);
                    dataView.getUint16(|Number,Boolean|);
                    dataView.getInt32(|Number,Boolean|);
                }
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                Assert.IsNotNull(help.FunctionHelp.FunctionName);
                Assert.IsTrue(help.FunctionHelp.FunctionName != "");
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().First();
                Assert.IsNotNull(signature);

                if (!String.IsNullOrEmpty(data))
                {
                    var expectedParamTypes = data.Split(',');
                    var actualParamTypes = signature.GetParameters().ToEnumerable().Select(param => param.Type != null ? param.Type : "Any").ToArray();
                    AssertAreStructurallyEqual(expectedParamTypes, actualParamTypes);
                }
            });
        }

        [TestMethod]
        public void Bug123032()
        {
            PerformParameterRequests(@"
                var f = function foo(a) {
                    while (true) ;
                    return f;
                }
                f(1)(|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            },
            250);
        }

        [TestMethod]
        public void DocCommentsInInnerFunction()
        {
            PerformParameterRequests(@"
                function hypotenuse(a, b) {
                function square(x) { 
                ///<param name='x' type='Number'> Some random number </param>
                ///<returns type='Number' />
                return x*x; }
                hypotenuse(|
                return Math.sqrt(square(a) + square(b));
                }
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().First();
                help.FunctionHelp.FunctionName.Except("hypotenuse");
                // Make sure that the doc comments of the inner function do 
                // not affect parameter help for the outer function.
                Assert.IsNull(signature.GetReturnValue());
            });
        }

        [TestMethod]
        public void DocCommentsInMiddleOfAFunction()
        {
            PerformParameterRequests(@"
                function hypotenuse(x) {
                    var x = 1;
                    ///<param name='x' type='Number'> Some random number </param>
                    ///<returns type='Number' />
                    return x*x; 
                }
                hypotenuse(|
                return Math.sqrt(square(a) + square(b));
                }
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().First();
                help.FunctionHelp.FunctionName.Except("hypotenuse");
                // Make sure that the doc comments of the inner function do 
                // not affect parameter help for the outer function.
                Assert.IsNull(signature.GetReturnValue());
            });
        }

        [TestMethod]
        public void ComplexArgument()
        {
            PerformParameterRequests(@"
                function f(a, b) { }
                f(function () {|});
                f({ f1:function(){|} });
                f(2, { f1:function(){|;
            ",
            (help, data, index) =>
            {
                Assert.IsNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        public void AnonymousFunction()
        {
            PerformParameterRequests(@"
                var a = function(de, soft)
                 {
                   return function(x)
                   {
                     return de + soft + x;
                   }
                 }
                 a(2, 3)(|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                Assert.IsNull(help.FunctionHelp.FunctionName);
            });
        }

        [TestMethod]
        public void CommentAfterFuncName()
        {
            PerformParameterRequests(@"
                abc = function foo(id, name, str) {
                    this.a = new String();
                    this.b = 10;
                    this.c = new Object();
                }
                var abcd = new abc /* this is my comment */ ("",|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.FunctionHelp.FunctionName.Except("abc");
            });
        }

        [TestMethod]
        public void RecursiveFunc()
        {
            PerformParameterRequests(@"
                var f = function fact(x) { 
                    if (x <= 1) return 1; 
                        else return x*fact(|); 
                };
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.FunctionHelp.FunctionName.Except("fact");
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().FirstOrDefault();
                Assert.IsNotNull(signature);
                var parameters = signature.GetParameters().ToEnumerable().ToArray();
                parameters[0].Name.Except("x");
            });
        }

        [TestMethod]
        public void Bug91434()
        {
            PerformParameterRequests(@" foo(|
            ",
            (help, data, index) =>
            {
                Assert.IsNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        public void Bug75760()
        {
            PerformParameterRequests(@"
                function f(a, b) {}
                for(var i in [1,2]) {
                    f(|);
                }
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
            });
        }

        [TestMethod]
        public void Bug111872()
        {
            PerformParameterRequests(@"
                function f(p1, p2) { }
                f(Windows.|
            ",
            (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                help.FunctionHelp.FunctionName.Except("f");
                help.ParameterIndex.Expect(0);
            });
        }

        [TestMethod]
        public void CurrentParameterIndex()
        {
            PerformRequests(@"abs|    (", (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(5);
                Assert.IsNull(help.FunctionHelp);
            },
            true);

            ValidateCurrentParameterIndex(@"
                    function fnc(a){ }
                    fnc|x|();
                    ", true);

            ValidateCurrentParameterIndex(@"
                function ParamIndexTest() {
	                function fnc(aa, bb, cc, dd) {}
                    fnc(|0|{}); 
	                fnc(fnc(),|1| fnc(a,b|1|)|1|); 
	                fnc(|0|
		                ""aaa""|0|,
		                 ""bbb""|1|
		                 ,|2|); 
 	                fnc(0 /* , */|0|); 
	                fnc(|0| 0|0| ,|1| 1|1|,|2| 2|2|,|3| 3|3|); 
	                fnc(|0|0|0|,|1|1|1|,|2|2|2|,|3|3|3|); 
	                fnc(|0|  0|0| /* , */|0| ,|1|  1|1| ,|2|  2|2| ,|3|3|3| /* aa */ ); 
	                fnc(  0 /* , */ ,  1 ,  2 ,3 /* aa */|3| ); 
	                fnc(  0  ,  1 ,  2 ,|3|3); 
	                fnc(  0  ,  1 ,  2|2|,3); 
	                fnc( 1 ,|1| ); 
	                fnc(|0|); 
	                fnc(1|0|); 
	                fnc(1|0|,);
	                fnc(1,|1|);
	                fnc( 1 , 2,|2|);
	                fnc(0 ,  1|1| ,  2 ,|3|);
	                fnc(0 ,  1 ,  2 ,|3|); 
	                fnc(  0  ,  1 ,  2 ,|3|)
	                fnc(  0  ,  1 ,  2 ,|3| ); 
	                fnc(fnc(),|1| fnc(a,b)); 
                }
                ", true);

            // Before and after object literal parameter
            ValidateCurrentParameterIndex(@"
                function abc1(a,b,c) { }
                abc1(|0|{});
                abc1(|0| {});
                abc1(1,|1|{});
                abc1(1,|1| {});
                abc1(1,{}|1|);
            ", true);

            // Before object literal parameter - missing right curly
            ValidateCurrentParameterIndex(@"
                function abc1(a,b,c) { }
                abc1(|0|{);
                abc1(|0| {);
                abc1(1,|1|{);
                abc1(1,|1| {);
                abc1(1,{}|1|);
                abc1(|0|{
            ", true);

            // Inside object literal parameter
            ValidateCurrentParameterIndex(@"
                function abc1(a,b,c) { }
                abc1(1,{|x|});
                abc1(1,{|x| });
                abc1(1,{ a:|x|{} });
            ", false);

            // Inside object literal parameter - missing right curly
            ValidateCurrentParameterIndex(@"
                function abc1(a,b,c) { }
                abc1(1,{|x|);
                abc1(1,{ a:|x|{} );
            ", false);
            
            // Before and after function expression parameter
            ValidateCurrentParameterIndex(
            @"
                function abc2(a,b,c) { }
                abc2('',|1|function x(a,b)|1|{}|1|,{});
            ", true);

            // Function expression parameter - after missing right curly
            ValidateCurrentParameterIndex(
            @"
                function abc2(a,b,c) { }
                abc2('',function x(a,b){|x|,{});
                abc2('',function x(a,b){|x|,{}
            ", false);

            // Function expression parameter - missing left curly
            ValidateCurrentParameterIndex(
            @"
                function abc2(a,b,c) { }
                abc2('',function x(a,b)|x|},{});
            ", false);
        }

        [TestMethod]
        public void FunctionParamWithSignature()
        {
            VerifyFunctionDocComments(FunctionParamWithSignatureFile, FunctionParamWithSignatureExpected);
        }
        #region Test data
        const string FunctionParamWithSignatureFile = @"
function FunctionParamWithSignature() {
    function funcParam(a) {
        /// <param name=""a"" type=""Function"">
        ///     <summary>this is a function param</summary>
        ///     <signature>
        ///         <summary>func param signature 2</summary>
        ///         <param name=""a2"">func param - parameter 1</param>
        ///         <param name=""b2"">func param - parameter 2</param>
        ///         <returns>func param - return description</returns>
        /// </signature>
        /// </param>
    }
    funcParam(|);
}";
        const string FunctionParamWithSignatureExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 544, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""funcParam"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"", Type = ""Function"", Description = ""this is a function param"", Optional = False, FunctionParamSignature = 
                            new { Description = ""func param signature 2"", Parameters = new [] { 
                                    new { Name = ""a2"", Description = ""func param - parameter 1"", Optional = False }, 
                                    new { Name = ""b2"", Description = ""func param - parameter 2"", Optional = False }
                                }, ReturnValue = 
                                new { Description = ""func param - return description"" }
                            }
                        }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ImplicitSignature()
        {
            VerifyFunctionDocComments(ImplicitSignatureFile, ImplicitSignatureExpected);
        }
        #region Test data
        const string ImplicitSignatureFile = @"
function ImplicitSignature() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name=""a"" type=""String"">parameter a</param>
        /// <param name=""b"" type=""Number"">parameter b</param>
        /// <returns type=""String"">return description</returns>
    }
    func(|);
}";
        const string ImplicitSignatureExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 314, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ExplicitSingleSignatureSameAsDecl()
        {
            VerifyFunctionDocComments(ExplicitSingleSignatureSameAsDeclFile, ExplicitSingleSignatureSameAsDeclExpected);
        }
        #region Test data
        const string ExplicitSingleSignatureSameAsDeclFile = @"
function ExplicitSingleSignatureSameAsDecl() {
    function func(a, b) {
        /// <signature>
        ///     <summary>explicit signature</summary>
        ///     <param name=""a"" type=""String"">parameter a</param>
        ///     <param name=""b"" type=""Number"">parameter b</param>
        ///     <returns type=""String"">return description</returns>
        /// </signature>
    }
    func(|);
}";
        const string ExplicitSingleSignatureSameAsDeclExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 397, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""explicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ExplicitSignatureNotSameAsDecl()
        {
            VerifyFunctionDocComments(ExplicitSignatureNotSameAsDeclFile, ExplicitSignatureNotSameAsDeclExpected);
        }
        #region Test data
        const string ExplicitSignatureNotSameAsDeclFile = @"
function ExplicitSignatureNotSameAsDecl() {
    function func(a, b, c) {
        /// <signature>
        ///     <summary>single signature, different from declaration</summary>
        ///     <param name=""a"" type=""String"">parameter a</param>
        ///     <returns type=""String"">return description</returns>
        /// </signature>
    }
    func(|);
}";
        const string ExplicitSignatureNotSameAsDeclExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 356, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""single signature, different from declaration"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MultipleSignatures()
        {
            VerifyFunctionDocComments(MultipleSignaturesFile, MultipleSignaturesExpected);
        }
        #region Test data
        const string MultipleSignaturesFile = @"
function MultipleSignatures() {
    function func(p1, p2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name=""a1"" type=""String"">parameter 1</param>
        ///     <returns type=""String"">return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name=""a2"" type=""String"">parameter 1</param>
        ///     <param name=""b2"" type=""String"">parameter 2</param>
        ///     <returns type=""String"">return description 2</returns>
        /// </signature>
    }
    func(|);
}";
        const string MultipleSignaturesExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 619, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""signature 1"", Parameters = new [] { 
                        new { Name = ""a1"", Type = ""String"", Description = ""parameter 1"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description 1"" }
                }, 
                new { Description = ""signature 2"", Parameters = new [] { 
                        new { Name = ""a2"", Type = ""String"", Description = ""parameter 1"", Optional = False }, 
                        new { Name = ""b2"", Type = ""String"", Description = ""parameter 2"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description 2"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ImplicitAndMultipleSignatures()
        {
            VerifyFunctionDocComments(ImplicitAndMultipleSignaturesFile, ImplicitAndMultipleSignaturesExpected);
        }
        #region Test data
        const string ImplicitAndMultipleSignaturesFile = @"
function ImplicitAndMultipleSignatures() {
    function func(p1, p2) {
        /// <summary>implicit signature - should not be seen</summary>
        /// <param name=""a"" type=""String"">parameter a</param>
        /// <param name=""b"" type=""Number"">parameter b</param>
        /// <returns type=""String"">return description</returns>
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name=""a1"" type=""String"">parameter 1</param>
        ///     <returns type=""String"">return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name=""a2"" type=""String"">parameter 1</param>
        ///     <param name=""b2"" type=""String"">parameter 2</param>
        ///     <returns type=""String"">return description 2</returns>
        /// </signature>
    }
    func(|);
}";
        const string ImplicitAndMultipleSignaturesExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 893, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""signature 1"", Parameters = new [] { 
                        new { Name = ""a1"", Type = ""String"", Description = ""parameter 1"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description 1"" }
                }, 
                new { Description = ""signature 2"", Parameters = new [] { 
                        new { Name = ""a2"", Type = ""String"", Description = ""parameter 1"", Optional = False }, 
                        new { Name = ""b2"", Type = ""String"", Description = ""parameter 2"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description 2"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MissingReturns()
        {
            VerifyFunctionDocComments(MissingReturnsFile, MissingReturnsExpected);
        }
        #region Test data
        const string MissingReturnsFile = @"
function MissingReturns() {
    function missingReturns(p1, p2) {
        /// <signature>
        ///     <summary>missing returns 1</summary>
        ///     <param name=""a1"" type=""String"">parameter 1</param>
        /// </signature>
        /// <signature>
        ///     <summary>missing returns 2</summary>
        ///     <param name=""a2"" type=""String"">parameter 1</param>
        ///     <param name=""b2"" type=""String"">parameter 2</param>
        /// </signature>
    }
    missingReturns(|);
}";
        const string MissingReturnsExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 495, Length = 16 }, FunctionHelp = 
        new { FunctionName = ""missingReturns"", Signatures = new [] { 
                new { Description = ""missing returns 1"", Parameters = new [] { 
                        new { Name = ""a1"", Type = ""String"", Description = ""parameter 1"", Optional = False }
                    }
                }, 
                new { Description = ""missing returns 2"", Parameters = new [] { 
                        new { Name = ""a2"", Type = ""String"", Description = ""parameter 1"", Optional = False }, 
                        new { Name = ""b2"", Type = ""String"", Description = ""parameter 2"", Optional = False }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MissingType()
        {
            VerifyFunctionDocComments(MissingTypeFile, MissingTypeExpected);
        }
        #region Test data
        const string MissingTypeFile = @"
function MissingType() {
    function missingType(p1, p2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name=""a1"">parameter 1</param>
        ///     <returns>return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name=""a2"">parameter 1</param>
        ///     <param name=""b2"">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// </signature>
    }
    missingType(|);
}";
        const string MissingTypeExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 549, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""missingType"", Signatures = new [] { 
                new { Description = ""signature 1"", Parameters = new [] { 
                        new { Name = ""a1"", Description = ""parameter 1"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return description 1"" }
                }, 
                new { Description = ""signature 2"", Parameters = new [] { 
                        new { Name = ""a2"", Description = ""parameter 1"", Optional = False }, 
                        new { Name = ""b2"", Description = ""parameter 2"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return description 2"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MissingParamName()
        {
            VerifyFunctionDocComments(MissingParamNameFile, MissingParamNameExpected);
        }
        #region Test data
        const string MissingParamNameFile = @"
function MissingParamName() {
    function missingParamName(a, b) {
        /// <param type=""String"">parameter a</param>
    }
    missingParamName(|);
}";
        const string MissingParamNameExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 137, Length = 18 }, FunctionHelp = 
        new { FunctionName = ""missingParamName"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"", Optional = False }, 
                        new { Name = ""b"", Optional = False }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MissingReturnsDescr()
        {
            VerifyFunctionDocComments(MissingReturnsDescrFile, MissingReturnsDescrExpected);
        }
        #region Test data
        const string MissingReturnsDescrFile = @"
function MissingReturnsDescr() {
    function missingReturnDescr(a, b) {
        /// <returns type=""Number"" />
    }
    missingReturnDescr(|);
}";
        const string MissingReturnsDescrExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 127, Length = 20 }, FunctionHelp = 
        new { FunctionName = ""missingReturnDescr"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"", Optional = False }, 
                        new { Name = ""b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""Number"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void invalidContent()
        {
            VerifyFunctionDocComments(invalidContentFile, invalidContentExpected);
        }
        #region Test data
        const string invalidContentFile = @"
function Invalid content() {
    function invalidContent(a, b) {
        /// some text
    }
    invalidContent(|);
}";
        const string invalidContentExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 103, Length = 16 }, FunctionHelp = 
        new { FunctionName = ""invalidContent"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"", Optional = False }, 
                        new { Name = ""b"", Optional = False }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void MissingSignatureClosingTag()
        {
            VerifyFunctionDocComments(MissingSignatureClosingTagFile, MissingSignatureClosingTagExpected);
        }
        #region Test data
        const string MissingSignatureClosingTagFile = @"
function MissingSignatureClosingTag() {
    function missingSignatureClosingTag(a, b) {
        /// <signature>
        ///     <summary>missing signature closing tag</summary>
        ///     <param name=""a"" type=""String"">parameter a</param>
        ///     <param name=""b"" type=""Number"">parameter b</param>
        ///     <returns type=""String"">return description</returns>
    }
    missingSignatureClosingTag(|);
}";
        const string MissingSignatureClosingTagExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 397, Length = 28 }, FunctionHelp = 
        new { FunctionName = ""missingSignatureClosingTag"", Signatures = new [] { 
                new { Description = ""missing signature closing tag"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void InvalidTags()
        {
            VerifyFunctionDocComments(InvalidTagsFile, InvalidTagsExpected);
        }
        #region Test data
        const string InvalidTagsFile = @"
function InvalidTags() {
    function invalidTag(a, b) {
        /// <summary>implicit signature</summary>
        /// <sometag />
        /// <param name=""a"" type=""String"">parameter a</param>
        /// <param name=""b"" type=""Number"">parameter b</param>
        /// <returns type=""String"">return description</returns>
    }
    invalidTag(|);
}";
        const string InvalidTagsExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 339, Length = 12 }, FunctionHelp = 
        new { FunctionName = ""invalidTag"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void EmptyParamTags()
        {
            VerifyFunctionDocComments(EmptyParamTagsFile, EmptyParamTagsExpected);
        }
        #region Test data
        const string EmptyParamTagsFile = @"
function EmptyParamTags() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name=""a"" type=""String"" />
        /// <param name=""b"" type=""Number"" />
        /// <returns type=""String"">return description</returns>
    }
    func(|);
}";
        const string EmptyParamTagsExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 277, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void IgnoreComments()
        {
            VerifyFunctionDocComments(IgnoreCommentsFile, IgnoreCommentsExpected);
        }
        #region Test data
        const string IgnoreCommentsFile = @"
function IgnoreComments() {
    function func(a) {
        /// <signature>
        ///     <!-- XML comments are silently skipped outside root elements-->
        ///     <summary>summary text </summary>
        ///     <param name=""a"" type=""Number""><!-- XML comments are ignored --> param description</param>
        ///     <returns>return <!-- XML comments are still ignored --> description</returns>
        /// </signature>
    }
    func(|);
}";
        const string IgnoreCommentsExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 450, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""summary text "", Parameters = new [] { 
                        new { Name = ""a"", Type = ""Number"", Description = "" param description"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return  description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void IgnoreProcessingInstructions()
        {
            VerifyFunctionDocComments(IgnoreProcessingInstructionsFile, IgnoreProcessingInstructionsExpected);
        }
        #region Test data
        const string IgnoreProcessingInstructionsFile = @"
function IgnoreProcessingInstructions() {
    function func(a) {
        /// <signature>
        ///     <summary>summary <?processing instructions Ignored?> text </summary>
        ///     <param name=""a"" type=""Number""><?this processing instruction too?> param description</param>
        /// </signature>
    }
    func(|);
}";
        const string IgnoreProcessingInstructionsExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 326, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""summary  text "", Parameters = new [] { 
                        new { Name = ""a"", Type = ""Number"", Description = "" param description"", Optional = False }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void PreserveCDATA()
        {
            VerifyFunctionDocComments(PreserveCDATAFile, PreserveCDATAExpected);
        }
        #region Test data
        const string PreserveCDATAFile = @"
function PreserveCDATA() {
    function func(a) {
        /// <signature>
        ///     <![CDATA[CDATA tags and thier content skipped]]>
        ///     <summary>summary text <![CDATA[CDATA tags preserved]]> </summary>
        ///     <param name=""a"" type=""Number""><![CDATA[CDATA tags are still preserved]]> param description</param>
        /// </signature>
    }
    func(|);
}";
        const string PreserveCDATAExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 381, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = @""summary text <![CDATA[CDATA tags preserved]]> "", Parameters = new [] { 
                        new { Name = ""a"", Type = ""Number"", Description = @""<![CDATA[CDATA tags are still preserved]]> param description"", Optional = False }
                    }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void PreserveFormating()
        {
            VerifyFunctionDocComments(PreserveFormatingFile, PreserveFormatingExpected);
        }
        #region Test data
        const string PreserveFormatingFile = @"
function PreserveFormating() {
    function func(a) {
        /// <signature>
        ///     <summary> summary \n text \n on multiple lines \t \n \t</summary>
        ///     <param name=""a"" type=""Number"">\n\n\nParam           Description              </param>
        ///     <returns>return                description </returns>
        /// </signature>
    }
    func(|);
}";
        const string PreserveFormatingExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 377, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = "" summary \n text \n on multiple lines \t \n \t"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""Number"", Description = ""\n\n\nParam           Description              "", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return                description "" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void UnknownContent()
        {
            VerifyFunctionDocComments(UnknownContentFile, UnknownContentExpected);
        }
        #region Test data
        const string UnknownContentFile = @"function UnknownContent() {
    function func(a) {
        /// <signature xmlns:foo=""http://w"">
        ///     <summary xml:space=""preserve"">summary text:<doccomment>content inside an unknown element<para>with a paragraph</para> and some more <d><e><f>unknown</f></e></d></doccomment>tags</summary>
        ///     <param name=""a"" type=""Number""><foo:bold>param description</foo:bold></param>
        ///     <returns unknownAttribute=""some value""><AnotherTag name=""value""/>return description</returns>
        /// </signature>
    }
    func(|);
}";
        const string UnknownContentExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 546, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = @""summary text:<doccomment>content inside an unknown element<para>with a paragraph</para> and some more <d><e><f>unknown</f></e></d></doccomment>tags"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""Number"", Description = @""<foo:bold>param description</foo:bold>"", Optional = False }
                    }, ReturnValue = 
                    new { Description = @""<AnotherTag name=\""value\""/>return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ParameterNameMatching()
        {
            VerifyFunctionDocComments(ParameterNameMatchingFile, ParameterNameMatchingExpected);
        }
        #region Test data
        const string ParameterNameMatchingFile = @"
function ParameterNameMatching() {
    function func(a1, b2) {
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name=""  a1   "">parameter 1</param>
        ///     <returns>return description 1</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name="" a2        "">parameter 1</param>
        ///     <param name=""                                    b2"">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// </signature>
        /// <signature>
        ///     <summary>signature 3</summary>
        ///     <param name=""b2"">parameter 2</param>
        ///     <returns>return description 2</returns>
        /// </signature>
    }
    func(|);
}
function ParameterMatching2() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name=""a d"" type=""String"">param name does not match and should not show in output</param>
        /// <param name="" b        c "" type=""Number"">param name does not match and should not show in output</param>
        /// <returns type=""String"">return description</returns>
    }
    func(|);
}";
        const string ParameterNameMatchingExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 812, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""signature 1"", Parameters = new [] { 
                        new { Name = ""a1"", Description = ""parameter 1"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return description 1"" }
                }, 
                new { Description = ""signature 2"", Parameters = new [] { 
                        new { Name = ""a2"", Description = ""parameter 1"", Optional = False }, 
                        new { Name = ""b2"", Description = ""parameter 2"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return description 2"" }
                }, 
                new { Description = ""signature 3"", Parameters = new [] { 
                        new { Name = ""b2"", Description = ""parameter 2"", Optional = False }
                    }, ReturnValue = 
                    new { Description = ""return description 2"" }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1238, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Optional = False }, 
                        new { Name = ""b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void LongFunctionComment()
        {
            var requests = CollectParameterRequests(LongFunctionCommentFile);
            var signature = requests.First().FunctionHelp.GetSignatures().ToEnumerable().First();
            Assert.IsTrue(signature.Description.StartsWith("begin long string --") && signature.Description.EndsWith("-- end long string"));

            var param1 = signature.GetParameters().ToEnumerable().First();
            Assert.IsTrue(param1.Description.StartsWith("begin param a description -- ") && param1.Description.EndsWith("-- end param a description"));

            var returnValue = signature.GetReturnValue();
            Assert.IsTrue(returnValue.Description.StartsWith("begin return description --") && returnValue.Description.EndsWith("-- end return description"));

        }
        #region Test data
        const string LongFunctionCommentFile = @"
function LongFunctionComment() {
    function func(a, b) {
        /// <summary>begin long string -- string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string string string string string
        /// string string string string string string string string string string string string string -- end long string</summary>
        /// <param name=""a"" type=""String"">begin param a description -- string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string -- end param a description</param>
        /// <param name=""b"" type=""Number"">
        ///         <summary>begin param b description -- string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string -- end param b description</summary>
        /// </param>
        /// <returns type=""String"">begin return description -- string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string string string string string
        ///         string string string string string string string string string string string string string -- end return description</returns>
    }
    func(|);
}";
        #endregion

        //[TestMethod]
        public void FunctionHelp()
        {
            var requests = CollectParameterRequests(FunctionHelpFile);

            var result = DumpObject(requests);

            Assert.AreEqual(FunctionHelpExpected, result);
        }
        #region Test data
        const string FunctionHelpFile = @"//
// Nested method calls - 3 levels
//
function ParamIndexTest() {
	function fnc(aa, bb, cc, dd) {}
	fnc(0, 1, 2,|
}

//
// Nested method calls - 3 levels
//
function NestedMethod3() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(testobj.testobjmethod1(testobj.testobjmethod1(|
}

//
// Calling a method, one of the arguments is a method call result
//
function MethodCallWithMethodCallArg() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(|, testobj.testobjmethod1())
}

//
// Completion in nested method call param
//
function NestedMethodCallParam() {
	var testobj = { testobjmethod1: function(param1) { } }
	testobj.testobjmethod1(2, testobj.testobjmethod1(|
}

//
// Nested call expressions - methods
//
function NestedCallExpressionsTest_Methods()
{
	var obj = { m1:function(a) { }, m2: function(b) { } };
	function f1(a) {}
	function f2(a) {}
	obj.m1(obj.m2(|));
	obj.m2(obj.m1(),|);
}

//
// Calling a method via identifier
//

var objid = { method10: function(a, b, c) { } };
var methodid = objid.method10;
methodid(|);

//
// DoWhileBlock
//
function DoWhileBlock() {
	function doWork(w) { }
	var c = 0;
	do 
	{
		doWork(|);
	}
	while(c);
}

//
// Return anonymous function
//
function ReturnAnonymousFunction() {
	function a() {
		return function(bb,cc) { }
	}
	a()(|
}

//
// BeforeComment
//
function BeforeComment() {
	function doWork(w) { }
	doWork(| /* comment */
}

//
// InsideIf
//
function InsideIf() {
	function doWork(w) { }
	if(doWork(|) {
	}
}

//
// Anonymous function
//
function AnonymousTest() {
	var f;
	f = function(a, b) { }
	f(|
}

//
// InWhileBlock
//
function InWhileBlock() {
	function doWork(w) { }
	var c = 1;
	while(c) {
		doWork(|
	}
}

//
// Constructor (new)
//
function ctorTest() {
	function viaCtor(cparam) { }
	var newVal = new viaCtor(|;
}

//
// Calling outer function from a nested function (2 levels)	
//
function outer(c, d) {
	function inner1(a1, b1) {
		outer(|
	}
}

//
// Calling a nested function	
//
function NestedFunctionTest() {
	function nested1(param1, param2) { }
	nested1(|
}

//
// Nested function via identifier	
//
function NestedFunctionsTest() {
	function outer(c, d) {
		function inner1(a1, b1) { }
		var inner1Var = inner1;
		inner1Var(|
	}
}

//
// Nested call expressions - functions
//
function NestedCallExpressionsTest_Funcs()
{
	function f1(a) { }
	function f2(a) { }
	f1(f2(|
	f1(f2(|)
	f1(f2(|))
	f1(f2(),|;
}

//
// A global function definition
//
function f(a, b) { }
f(|);

//
// Missing closing parenthesis
//
function missingClosingParenthesis() { 
	function func(parameterName) { }
	func(|
}

//
// Object methods
//
function f1(a, b) { }
var obj = {
	method1: f1,
	method2: function(x, y) { },
	method3: function() { },
	method4: function(e) { return 1/0; } 
};

// Calling methods
obj.method1(|);
obj.method2(|);
obj.method3(|);
obj.method4(|);

//
// Calling methods inside a function
//
var globalObj1 = { method1: function(a, b, c, d)  {} };
function callingMethodsTest() {
	globalObj1.method1(|);
	var localObj = globalObj1;
	localObj.method1(|);
}

//
//	Errors in code path 
//
function f12()
{
	var localobj = { m: function(a, b) { } };
	localobj.NonExistentMethod();
	localobj.m(|);
}

function doWork(w) { }
function LastStatementInBlockTest() {
    if(true)
    {
        do
        { 
            doWork(|); 
        } while(false);
    }
}

//
//  Calling a field, the resulting function name should be the field name.
//
function CallFieldTest() {
    function global23(x)
    {
        return x;
    }
    var el = {
    field1: global23                
    };
    el.field1(|
}

//
// Calling via identifier, the resulting function name should be the identifier name.
//
function CallVarTest() {
    function shouldNotAppear(a) {}
    var shouldAppear = shouldNotAppear;
    shouldAppear(|
}


//
//	Infinite loop
//
function InfiniteLoopTest() {
	while(1) {}
	f = function(a) { }
	f(|
}

//
// A nested function calling another nested function on the same level	
//
function SameLevelNestedFunctionsTest() {
	function nested1(param1, param2) { }
	function nested2() { 
		nested1(|
	}
}

//
// Calling outer function from a nested function (3 levels)	
//
function outer1(o11, o12) {
	function outer2(o21, o22) {
		function outer3(o31, o32) {
			outer2(|
		}
	}
}

//
//  new inside for
//
function newInForTest() {
    function foo(id, name, str) { }
    for(var o = 1; o< 10; o++) {
	    var abc = new foo /* this is my comment */ (500,|);
    }
}

//
//  Prototype access in constructor method
//
function prototypeTest() {
    function o(a, b, c) {
	    this.superclass(|
    }
    o.prototype.superclass = function p(p1, p2) { };
}

//
// KNOWN TO FAIL, BUT NEED TO PASS EVENTUALLY
//

//
// Call immediately following declaration - inside function
//
function DeclareAndCallTest() {
	(function(a, b) { })(|)
}

//
// Call immediately following declaration - global
//
(function(a, b) { })(|)

//
// Calling library functions
//
function LibraryCallTest() {
	var n = new Number(1);
	var s = n.toString(|
}

//
// NEGATIVE TESTS - EMPTY RESULT EXPECTED FOR THE TESTS BELOW
//

//
// Calling this method
//
var objThis = {
	method1: function(p1, p2) {
	},
	method3: function() { 
		this.method1(|); 
	}
};

//
// Non existent object methods
//
function nonExistentMethodTest() {
	var obj = { };
	obj.method0(|);
	obj.method1(|);
}

//
// Invalid location
//
function InvalidLocation() {
	if(|
}

//
//  Int result call
//
function intCallTest() {
    function test(a, b, c) {
        return a + b + c;
    }
    test(5, 6, 7)(|
}";
        const string FunctionHelpExpected = @"new [] { 
    new { ParameterIndex = 3, Region = 
        new { Offset = 107, Length = 15 }, FunctionHelp = 
        new { FunctionName = ""fnc"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""aa"" }, 
                        new { Name = ""bb"" }, 
                        new { Name = ""cc"" }, 
                        new { Name = ""dd"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 301, Length = 25 }, FunctionHelp = 
        new { FunctionName = ""testobjmethod1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""param1"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 506, Length = 50 }, FunctionHelp = 
        new { FunctionName = ""testobjmethod1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""param1"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 733, Length = 26 }, FunctionHelp = 
        new { FunctionName = ""testobjmethod1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""param1"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 964, Length = 8 }, FunctionHelp = 
        new { FunctionName = ""m2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 1, Region = 
        new { Offset = 977, Length = 17 }, FunctionHelp = 
        new { FunctionName = ""m2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1130, Length = 10 }, FunctionHelp = 
        new { FunctionName = ""methodid"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }, 
                        new { Name = ""c"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1247, Length = 8 }, FunctionHelp = 
        new { FunctionName = ""doWork"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""w"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1407, Length = 7 }, FunctionHelp = 
        new { FunctionName = ""Anonymous Function"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""bb"" }, 
                        new { Name = ""cc"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1498, Length = 24 }, FunctionHelp = 
        new { FunctionName = ""doWork"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""w"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1599, Length = 8 }, FunctionHelp = 
        new { FunctionName = ""doWork"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""w"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1714, Length = 5 }, FunctionHelp = 
        new { FunctionName = ""f"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1828, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""doWork"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""w"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1944, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""viaCtor"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""cparam"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2088, Length = 10 }, FunctionHelp = 
        new { FunctionName = ""outer"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""c"" }, 
                        new { Name = ""d"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2217, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""nested1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""param1"" }, 
                        new { Name = ""param2"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2395, Length = 14 }, FunctionHelp = 
        new { FunctionName = ""inner1Var"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a1"" }, 
                        new { Name = ""b1"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2557, Length = 23 }, FunctionHelp = 
        new { FunctionName = ""f2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2566, Length = 7 }, FunctionHelp = 
        new { FunctionName = ""f2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 1, Region = 
        new { Offset = 2560, Length = 19 }, FunctionHelp = 
        new { FunctionName = ""f1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 1, Region = 
        new { Offset = 2584, Length = 9 }, FunctionHelp = 
        new { FunctionName = ""f1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2663, Length = 3 }, FunctionHelp = 
        new { FunctionName = ""f"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 2788, Length = 8 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""parameterName"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3002, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""method1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3018, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""method2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""x"" }, 
                        new { Name = ""y"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3034, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""method3"", Signatures = new [] { 
                new { Parameters = new [] { } }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3050, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""method4"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""e"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3205, Length = 20 }, FunctionHelp = 
        new { FunctionName = ""method1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }, 
                        new { Name = ""c"" }, 
                        new { Name = ""d"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3258, Length = 18 }, FunctionHelp = 
        new { FunctionName = ""method1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }, 
                        new { Name = ""c"" }, 
                        new { Name = ""d"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3413, Length = 12 }, FunctionHelp = 
        new { FunctionName = ""m"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3553, Length = 8 }, FunctionHelp = 
        new { FunctionName = ""doWork"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""w"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 3839, Length = 13 }, FunctionHelp = 
        new { FunctionName = ""field1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""x"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 4058, Length = 16 }, FunctionHelp = 
        new { FunctionName = ""shouldAppear"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 4174, Length = 5 }, FunctionHelp = 
        new { FunctionName = ""f"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 4372, Length = 12 }, FunctionHelp = 
        new { FunctionName = ""nested1"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""param1"" }, 
                        new { Name = ""param2"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 4554, Length = 12 }, FunctionHelp = 
        new { FunctionName = ""outer2"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""o21"" }, 
                        new { Name = ""o22"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 1, Region = 
        new { Offset = 4718, Length = 39 }, FunctionHelp = 
        new { FunctionName = ""foo"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""id"" }, 
                        new { Name = ""name"" }, 
                        new { Name = ""str"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 4884, Length = 23 }, FunctionHelp = 
        new { FunctionName = ""superclass"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""p1"" }, 
                        new { Name = ""p2"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 5129, Length = 21 }, FunctionHelp = 
        new { FunctionName = ""Anonymous Function"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 5218, Length = 21 }, FunctionHelp = 
        new { FunctionName = ""Anonymous Function"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"" }, 
                        new { Name = ""b"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 5345, Length = 14 }, FunctionHelp = 
        new { FunctionName = ""toString"", Signatures = new [] { 
                new { ReturnType = ""String"", Parameters = new [] { } }, 
                new { ReturnType = ""String"", Parameters = new [] { 
                        new { Name = ""radix"", Type = ""Number"" }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 0 }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 0 }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 0 }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 0 }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 0, Length = 0 }
    }
}";
        #endregion

        [TestMethod]
        public void Bug160093()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var jquery = _session.ReadFile(Paths.JQuery.JQuery_1_2_6VSDocFilePath).Text;
            PerformRequests(@"
                function MakeSudoku() {
                    var defaultNumbers = null;
                   var usersNumbers = null;
                    var cellLocations = null;
                    var scratchNumbers = null;
                    var scratchMode = false;
                    var mouseMenu = null;
                    var errorCells = null;
                    var win = false;
 
                    var currentCell = { Row: 0, Col: 0 };
                    var borderThickness = 7;
                    var squareThickness = 5;
                    var cellThickness = 3;
 
                    var canvas = document.getElementById('Sudoku');
                    var width = canvas.width;
                    var height = canvas.height;
                    var winLoop = null;
 
                    var fireworksOnly = false;
                    var fireworks = new Array();
                    var fireworksStage = { Exploding: 0, Exploded: 1 };
                    var maxFireworks = 20;
 
                    Setup();
 
                    function Setup() {
                        SetupPuzzle();
                        SetUpMouseMenu();
 
                        Reset();
 
                        $(canvas).click(Click);
                        $(canvas).keydown(KeyDown);
                        $(canvas).keyup(KeyUp);
 
                        $('#ResetButton').click(|
                ", (context, offset, data, index) =>
                 {
                     for (int i = 0; i < 2; i++)
                     {
                         var help = context.GetParameterHelpAt(offset);
                         Assert.IsNotNull(help.FunctionHelp);
                     }
                 }, dom, jquery);
        }


        [TestMethod]
        [WorkItem(210832)]
        public void ExtraFunctionHelpAttributes()
        {
            var requests = CollectParameterRequests(ExtraFunctionHelpAttributesFile);
            requests.Expect(ExtraFunctionHelpAttributesExpected);
        }
        #region Test data
        const string ExtraFunctionHelpAttributesFile = @"
function FunctionParam() {
    function funcParam(a) {
        /// <param name=""a"" type=""Function"" domElement=""true"" elementType=""Empty"" elementDomElement=""true"" locid=""paramLocid"" optional=""true"" helpKeyword=""paramHelpKeyword"">
        ///     <summary locid=""paramSummaryLocid"">this is a function param</summary>
        ///     <signature externalFile=""signatureExternalFile"" externalid=""signatureExternalid"" helpKeyword=""signatureHelpKeyword"">
        ///         <summary locid=""signatureSummaryLocid"">func param signature 2</summary>
        ///         <param name=""a2"" optional=""false"" locid=""param_a2_locid"" helpKeyword=""param_a2_HelpKeyword"">func param - parameter 1</param>
        ///         <param name=""b2"" optional=""true""><summary locid=""param_b2_summary_locid"" helpKeyword=""param_b2_HelpKeyword"">func param - parameter 2</summary></param>
        ///         <returns domElement=""true"" elementType=""ReturnType"" elementDomElement=""true"" locid=""returnLocid"" helpKeyword=""returnHelpKeyword"" externalFile=""returnsExternalFile"" externalid=""returnsExternalid"">func param - return description</returns>
        /// </signature>
        /// </param>
    }
    funcParam(|);
}
        
function ImplicitSignature() {
    function func(a, b) {
        /// <summary locid=""summaryLocid"">implicit signature</summary>
        /// <param name=""a"" type=""String"" domElement=""false"" elementType=""NULL"" elementDomElement=""false"" locid=""param_a_Locid"" helpKeyword=""param_a_HelpKeyword"">parameter a</param>
        /// <param name=""b"" type=""Array"" domElement=""true"" elementType=""String"" elementDomElement=""true"" locid=""param_b_Locid"" helpKeyword=""param_b_HelpKeyword"">parameter b</param>
        /// <returns type=""String"" domElement=""INVALID"" elementType="""" elementDomElement=""false"" locid=""ReturnLocid"" helpKeyword=""ReturnHelpKeyword"" externalFile=""returnsExternalFile"" externalid=""returnsExternalid"">return description</returns>
    }
    func(|);
}";
        const string ExtraFunctionHelpAttributesExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1183, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""funcParam"", Signatures = new [] { 
                new { Parameters = new [] { 
                        new { Name = ""a"", Type = ""Function"", Description = ""this is a function param"", Locid = ""paramSummaryLocid"", ElementType = ""Empty"", Optional = True, FunctionParamSignature = 
                            new { Description = ""func param signature 2"", Locid = ""signatureSummaryLocid"", ExternalFile = ""signatureExternalFile"", Externalid = ""signatureExternalid"", HelpKeyword = ""signatureHelpKeyword"", Parameters = new [] { 
                                    new { Name = ""a2"", Description = ""func param - parameter 1"", Locid = ""param_a2_locid"", Optional = False }, 
                                    new { Name = ""b2"", Description = ""func param - parameter 2"", Locid = ""param_b2_summary_locid"", Optional = True }
                                }, ReturnValue = 
                                new { Type = ""HTMLElement"", Description = ""func param - return description"", Locid = ""returnLocid"", ElementType = ""ReturnType"", HelpKeyword = ""returnHelpKeyword"", ExternalFile = ""returnsExternalFile"", Externalid = ""returnsExternalid"" }
                            }
                        }
                    }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1962, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Locid = ""summaryLocid"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Locid = ""param_a_Locid"", ElementType = ""NULL"", Optional = False }, 
                        new { Name = ""b"", Type = ""Array"", Description = ""parameter b"", Locid = ""param_b_Locid"", ElementType = ""String"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"", Locid = ""ReturnLocid"", HelpKeyword = ""ReturnHelpKeyword"", ExternalFile = ""returnsExternalFile"", Externalid = ""returnsExternalid"" }
                }
            }
        }
    }
}";
        #endregion


        [TestMethod]
        public void FunctionHelpSourceFileHandle()
        {
            var contextFile1 = _session.FileFromText(@"
                function context1(a) {
                    /// <summary> context1 summary </summary>
                }");
            var contextFile2 = _session.FileFromText(@"
                function context2(b) {
                    /// <summary> context2 summary </summary>
                }");
            var contextFile3 = _session.FileFromText(@"
                function context3(c) {
                    /// <summary> context3 summary </summary>
                }");

            var primaryFileText = @"
                context1(|1|);
                context2(|2|);
                context3(|3|);";


            var offsets = ParseRequests(primaryFileText);
            var primaryFile = _session.FileFromText(offsets.Text);
            var context = _session.OpenContext(primaryFile, contextFile1, contextFile2, contextFile3);
            foreach (var request in offsets.Requests)
            {
                var paramHelp = context.GetParameterHelpAt(request.Offset);

                    var sourceFileHandle = paramHelp.FunctionHelp.SourceFileHandle;
                    Assert.IsNotNull(sourceFileHandle, "sourceFileHandle is null.");
                
                    switch (request.Data)
                    { 
                        case "1":
                            Assert.AreEqual(sourceFileHandle, contextFile1.GetHandle());
                            break;
                        case "2":
                            Assert.AreEqual(sourceFileHandle, contextFile2.GetHandle());
                            break;
                        case "3":
                            Assert.AreEqual(sourceFileHandle, contextFile3.GetHandle());
                            break;
                        default:
                            Assert.Fail("Unknown file index");
                            break;
                    }
            }
        }

         [TestMethod]
        public void EscapedXML()
        {
            var requests = CollectParameterRequests(EscapedXMLFile);
            requests.Expect(EscapedXMLExpectedString);
        }
        #region Test data
         const string EscapedXMLFile = @"
function ValueElement() {
    function get_value() {
        /// <summary><x doubleQuotes='""' doubleQuotesEscaped='&quot;' singleQuotes=""'"" singleQuotesEscaped=""&apos;"" greaterThanEscaped=""&gt;"" lessThanEscaped=""&lt;"" mixedText=""text with --&quot;--, --&apos;--, --&amp;--, --&gt;-- and --&lt;--""/></summary>
        /// <returns>text with unescaped characters:  --'--, --""-- ; and escaped characters: --&quot;--, --&apos;--, --&amp;--, --&gt;-- and --&lt;--</returns>
    }
    get_value(|);
}
";

         const string EscapedXMLExpectedString = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 486, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""get_value"", Signatures = new [] { 
                new { Description = @""<x doubleQuotes=\""&quot;\"" doubleQuotesEscaped=\""&quot;\"" singleQuotes=\""&apos;\"" singleQuotesEscaped=\""&apos;\"" greaterThanEscaped=\""&gt;\"" lessThanEscaped=\""&lt;\"" mixedText=\""text with --&quot;--, --&apos;--, --&amp;--, --&gt;-- and --&lt;--\""/>"", Parameters = new [] { }, ReturnValue = 
                    new { Description = ""text with unescaped characters:  --&apos;--, --&quot;-- ; and escaped characters: --&quot;--, --&apos;--, --&amp;--, --&gt;-- and --&lt;--"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void ValueElement()
        {
            var requests = CollectParameterRequests(ValueElementFile);
            requests.Expect(ValueElementExpected);
        }
        #region Test data
        const string ValueElementFile = @"
function ValueElement() {
    function get_value() {
        /// <value type=""ValueType"" domElement=""true"" elementType=""ArrayElementType"" elementDomElement=""true"" locid=""valueLocid"" helpKeyword=""valueHelpKeyword"">value Description</value>
    }
    get_value(|);
}

function ValueAndMultipleSignatures() {
    function func(p1, p2) {
        /// <summary>implicit signature - should not be seen</summary>
        /// <param name=""a"" type=""String"">parameter a</param>
        /// <param name=""b"" type=""Number"">parameter b</param>
        /// <returns type=""String"">return description</returns>
        /// <signature>
        ///     <summary>signature 1</summary>
        ///     <param name=""a1"" type=""String"">parameter 1</param>
        ///     <returns type=""String"">return description 1</returns>
        /// </signature>
        /// <value locid=""valueLocid"" type=""ValueType"">Value Description</value>
        /// <signature>
        ///     <summary>signature 2</summary>
        ///     <param name=""a2"" type=""String"">parameter 1</param>
        ///     <param name=""b2"" type=""String"">parameter 2</param>
        ///     <returns type=""String"">return description 2</returns>
        /// </signature>
    }
    func(|);
}";
        const string ValueElementExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 255, Length = 11 }, FunctionHelp = 
        new { FunctionName = ""get_value"", Signatures = new [] { 
                new { Description = ""value Description"", Locid = ""valueLocid"", HelpKeyword = ""valueHelpKeyword"", Parameters = new [] { }, ReturnValue = 
                    new { Type = ""ValueType"", ElementType = ""HTMLElement"" }
                }
            }
        }
    }, 
    new { ParameterIndex = 0, Region = 
        new { Offset = 1244, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""Value Description"", Locid = ""valueLocid"", Parameters = new [] { 
                        new { Name = ""p1"", Optional = False }, 
                        new { Name = ""p2"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""ValueType"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void DomElement()
        {
            var requests = CollectParameterRequests(DomElementFile);
            requests.Expect(DomElementExpected);
        }
        #region Test data
        const string DomElementFile = @"
function DomElement() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name=""a"" type=""CorrectType"" domElement=""true"" elementType=""CorrectElementType"" elementDomElement=""true"">parameter a</param>
        /// <param name=""b"" domElement=""true"" elementDomElement=""true"">parameter b</param>
        /// <returns type=""Array"" elementDomElement=""true"">return description</returns>
    }
    func(|);
}";
        const string DomElementExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 441, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""CorrectType"", Description = ""parameter a"", ElementType = ""CorrectElementType"", Optional = False }, 
                        new { Name = ""b"", Type = ""HTMLElement"", Description = ""parameter b"", ElementType = ""HTMLElement"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""Array"", Description = ""return description"", ElementType = ""HTMLElement"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        [WorkItem(199843)]
        [WorkItem(205878)]
        public void InternalFileHandles()
        {
            var contextFile = "function foo(a,b,c) {}";

            PerformRequests(@"
                function bar (a) {}

                // function in primary file
                bar(|c|);

                // function in a context file
                foo(|c|); 

                // function in LibHelp
                foo.hasOwnProperty(|i|);", (context, offset, data, index) =>
            {
                var functionHelp = context.GetParameterHelpAt(offset);
                Assert.IsNotNull(functionHelp);
                switch (data)
                {
                    case "c":
                        Assert.IsNotNull(functionHelp.FunctionHelp.SourceFileHandle);
                        break;
                    case "i":
                        Assert.IsNotNull(functionHelp.FunctionHelp.SourceFileHandle);
                        break;
                };
            }, CombinedContextFiles(new string[] {contextFile}));
        }

        [TestMethod]
        public void EngineHelperFunctions()
        {
            PerformRequests(@"_$createDomObject(|); 
                              _$return(|); 
                              _$callLss(|)", (context, offset, data, index) =>
                            {
                                var help = context.GetParameterHelpAt(offset);
                                Assert.IsNull(help.FunctionHelp);
                             });
        }

        [TestMethod]
        public void RedirectFunctionDefinition()
        {
            PerformRequests(
                @"
                    function foo(a) { return a; }
                    function bar(a) {
                       /// <summary>Some description</summary>
                       /// <param name='a' type='string' />
                       /// <returns type='string' />
                    }
                    intellisense.redirectDefinition(foo, bar);

                    foo(|
                ", (context, offset, data, index) =>
                 {
                     var help = context.GetParameterHelpAt(offset);
                     var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                     Assert.AreEqual("Some description", signature.Description);
                     Assert.AreEqual("string", signature.GetReturnValue().Type);
                     var parameter = signature.GetParameters().ToEnumerable().Single();
                     Assert.AreEqual("a", parameter.Name);
                     Assert.AreEqual("string", parameter.Type);
                 });
        }

        [TestMethod]
        public void AnnotateFunction()
        {
            PerformRequests(
                @"
                    function foo(a) { return a; }

                    intellisense.annotate(foo, function (a) {
                       /// <summary>Some description</summary>
                       /// <param name='a' type='string' />
                       /// <returns type='string' />
                    });

                    foo(|
                ", (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                Assert.AreEqual("Some description", signature.Description);
                Assert.AreEqual("string", signature.GetReturnValue().Type);
                var parameter = signature.GetParameters().ToEnumerable().Single();
                Assert.AreEqual("a", parameter.Name);
                Assert.AreEqual("string", parameter.Type);
            });
        }

        [TestMethod]
        public void IndirectAnnotatedFunction()
        {
            PerformRequests(
                @"
                    function foo(a) { return a; }
                    function bar(a) { return a; }

                    intellisense.annotate(foo, function (a) {
                       /// <summary>Some description</summary>
                       /// <param name='a' type='string' />
                       /// <returns type='string' />
                    });
                    intellisense.annotate(bar, foo);

                    bar(|
                ", (context, offset, data, index) =>
                 {
                     var help = context.GetParameterHelpAt(offset);
                     var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                     Assert.AreEqual("Some description", signature.Description);
                     Assert.AreEqual("string", signature.GetReturnValue().Type);
                     var parameter = signature.GetParameters().ToEnumerable().Single();
                     Assert.AreEqual("a", parameter.Name);
                     Assert.AreEqual("string", parameter.Type);
                 });
        }

        [TestMethod]
        public void AnnotateFunctions()
        {
            PerformRequests(
                @"
                    var someObject = {
                       foo: function (a) { return a; },
                       bar: function (a) { return a; }
                    };

                    intellisense.annotate(someObject, {
                       foo: function (a) {
                         /// <summary>Some description</summary>
                         /// <param name='a' type='string' />
                         /// <returns type='string' />
                       },
                       bar: function (a) { 
                         /// <summary>Some description</summary>
                         /// <param name='a' type='string' />
                         /// <returns type='string' />
                       }
                    });

                    someObject.foo(|);
                    someObject.bar(|);
                ", (context, offset, data, index) =>
                 {
                     var help = context.GetParameterHelpAt(offset);
                     var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                     Assert.AreEqual("Some description", signature.Description);
                     Assert.AreEqual("string", signature.GetReturnValue().Type);
                     var parameter = signature.GetParameters().ToEnumerable().Single();
                     Assert.AreEqual("a", parameter.Name);
                     Assert.AreEqual("string", parameter.Type);
                 });
        }

        [TestMethod]
        public void IndirectAnnotateFunctions()
        {
            PerformRequests(
                @"
                    var someObject = {
                       foo: function (a) { return a; },
                       bar: function (a) { return a; }
                    };

                    intellisense.annotate(someObject, {
                       foo: function (a) {
                         /// <summary>Some description</summary>
                         /// <param name='a' type='string' />
                         /// <returns type='string' />
                       },
                       bar: function (a) { 
                         /// <summary>Some description</summary>
                         /// <param name='a' type='string' />
                         /// <returns type='string' />
                       }
                    });

                    var someOtherObject = {
                       foo: function (a) { return a; },
                       bar: function (a) { return a; }
                    };
                    intellisense.annotate(someOtherObject, someObject);

                    someOtherObject.foo(|);
                    someOtherObject.bar(|);
                ", (context, offset, data, index) =>
                 {
                     var help = context.GetParameterHelpAt(offset);
                     var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                     Assert.AreEqual("Some description", signature.Description);
                     Assert.AreEqual("string", signature.GetReturnValue().Type);
                     var parameter = signature.GetParameters().ToEnumerable().Single();
                     Assert.AreEqual("a", parameter.Name);
                     Assert.AreEqual("string", parameter.Type);
                 });
        }

        [TestMethod]
        public void CircularAnnotations()
        {
            PerformRequests(
                @"
                    function foo() { }
                    function bar() { }
                    intellisense.annotate(foo, bar);
                    intellisense.annotate(bar, foo);
                    foo(|", (context, offset, data, index) =>
                 {
                     var help = context.GetParameterHelpAt(offset);
                     Assert.IsNotNull(help);
                 });
        }


        [TestMethod]
        public void AnnotateFields()
        {
            PerformRequests(
                @"
                    var someObject = {
                       foo: 1,
                       bar: 2
                    };

                    intellisense.annotate(someObject, {
                       /// <field name='foo' type='number'>Some field</field>
                       foo: 1,

                       /// <field name='bar' type='number'>Some field</field>
                       bar: 2
                    });

                    someObject.|;
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset);
                     var hint = completions.GetHintFor("foo");
                     Assert.AreEqual(AuthorType.atNumber, hint.Type);
                     Assert.AreEqual("number", hint.TypeName);
                     Assert.AreEqual("Some field", hint.Description);
                     hint = completions.GetHintFor("bar");
                     Assert.AreEqual(AuthorType.atNumber, hint.Type);
                     Assert.AreEqual("number", hint.TypeName);
                     Assert.AreEqual("Some field", hint.Description);
                 });
        }

        [TestMethod]
        public void ParameterWithErrors()
        {
            PerformRequests(@"
                function test(a, ...) {
                    ///<summary> summary </summary>
                    ///<param name=""a"" type=""Number""> A Para </param>
                }
                test(|
            ", (context, offset, data, index) => {
                 var help = context.GetParameterHelpAt(offset);
                 var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                 var parameter = signature.GetParameters().ToEnumerable().Where(p => p.Name == "?").FirstOrDefault();
                 Assert.IsNotNull(parameter, "A parameter with an error is not called '?'");
            });
        }

        [TestMethod]
        public void CrashTest()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath);
            var text = _session.FileFromText(CrashTestText);
            var context = _session.OpenContext(text, domjs);
            context.GetParameterHelpAt(CrashTestText.Length);
        }
        #region Test data
        const string CrashTestText = @"/* Copyright 2007 Google. All Rights Reserved. */ (function() { function e(a){throw a;}var i=true,j=null,l=false;function aa(){return function(){}}function ba(a){return function(b){this[a]=b}}function o(a){return function(){return this[a]}}function ca(a){return function(){return a}}var s,da=da||{},t=this;function ea(a,b,c){a=a.split(""."");c=c||t;!(a[0]in c)&&c.execScript&&c.execScript(""var ""+a[0]);for(var d;a.length&&(d=a.shift());)if(!a.length&&b!==undefined)c[d]=b;else c=c[d]?c[d]:c[d]={}}function v(){}var fa=""number"",x=""function"";
function ga(a){var b=typeof a;if(b==""object"")if(a){if(a instanceof Array||!(a instanceof Object)&&Object.prototype.toString.call(a)==""[object Array]""||typeof a.length==fa&&typeof a.splice!=""undefined""&&typeof a.propertyIsEnumerable!=""undefined""&&!a.propertyIsEnumerable(""splice""))return""array"";if(!(a instanceof Object)&&(Object.prototype.toString.call(a)==""[object Function]""||typeof a.call!=""undefined""&&typeof a.propertyIsEnumerable!=""undefined""&&!a.propertyIsEnumerable(""call"")))return x}else return""null"";
else if(b==x&&typeof a.call==""undefined"")return""object"";return b}function y(a){return ga(a)==""array""}function ha(a){var b=ga(a);return b==""array""||b==""object""&&typeof a.length==fa}function z(a){return typeof a==""string""}function ia(a){return typeof a==fa}function ja(a){return ga(a)==x}function ka(a){a=ga(a);return a==""object""||a==""array""||a==x}function la(a){return a[ma]||(a[ma]=++na)}var ma=""closure_uid_""+Math.floor(Math.random()*2147483648).toString(36),na=0;
function oa(a){var b=ga(a);if(b==""object""||b==""array""){if(a.B)return a.B();b=b==""array""?[]:{};for(var c in a)b[c]=oa(a[c]);return b}return a}function A(a,b){var c=b||t;if(arguments.length>2){var d=Array.prototype.slice.call(arguments,2);return function(){var f=Array.prototype.slice.call(arguments);Array.prototype.unshift.apply(f,d);return a.apply(c,f)}}else return function(){return a.apply(c,arguments)}}
function pa(a){var b=Array.prototype.slice.call(arguments,1);return function(){var c=Array.prototype.slice.call(arguments);c.unshift.apply(c,b);return a.apply(this,c)}}var qa=Date.now||function(){return+new Date};function B(a,b){function c(){}c.prototype=b.prototype;a.e=b.prototype;a.prototype=new c;a.prototype.constructor=a};function ra(a){return a.replace(/^[\s\xa0]+|[\s\xa0]+$/g,"""")}var sa=/^[a-zA-Z0-9\-_.!~*'()]*$/;function ta(a){a=String(a);if(!sa.test(a))return encodeURIComponent(a);return a}
function ua(a,b){if(b)return a.replace(va,""&amp;"").replace(wa,""&lt;"").replace(xa,""&gt;"").replace(ya,""&quot;"");else{if(!za.test(a))return a;if(a.indexOf(""&"")!=-1)a=a.replace(va,""&amp;"");if(a.indexOf(""<"")!=-1)a=a.replace(wa,""&lt;"");if(a.indexOf("">"")!=-1)a=a.replace(xa,""&gt;"");if(a.indexOf('""')!=-1)a=a.replace(ya,""&quot;"");return a}}var va=/&/g,wa=/</g,xa=/>/g,ya=/\""/g,za=/[&<>\""]/;function Ba(){return Array.prototype.join.call(arguments,"""")}
function Ca(a,b){for(var c=0,d=ra(String(a)).split("".""),f=ra(String(b)).split("".""),g=Math.max(d.length,f.length),h=0;c==0&&h<g;h++){var k=d[h]||"""",m=f[h]||"""",n=RegExp(""(\\d*)(\\D*)"",""g""),p=RegExp(""(\\d*)(\\D*)"",""g"");do{var q=n.exec(k)||["""","""",""""],r=p.exec(m)||["""","""",""""];if(q[0].length==0&&r[0].length==0)break;c=Da(q[1].length==0?0:parseInt(q[1],10),r[1].length==0?0:parseInt(r[1],10))||Da(q[2].length==0,r[2].length==0)||Da(q[2],r[2])}while(c==0)}return c}
function Da(a,b){if(a<b)return-1;else if(a>b)return 1;return 0}var Ea=Math.random()*2147483648|0;var C=Array.prototype,D=C.indexOf?function(a,b,c){return C.indexOf.call(a,b,c)}:function(a,b,c){c=c==j?0:c<0?Math.max(0,a.length+c):c;if(z(a)){if(!z(b)||b.length!=1)return-1;return a.indexOf(b,c)}for(c=c;c<a.length;c++)if(c in a&&a[c]===b)return c;return-1},Fa=C.forEach?function(a,b,c){C.forEach.call(a,b,c)}:function(a,b,c){for(var d=a.length,f=z(a)?a.split(""""):a,g=0;g<d;g++)g in f&&b.call(c,f[g],g,a)},Ga=C.filter?function(a,b,c){return C.filter.call(a,b,c)}:function(a,b,c){for(var d=a.length,f=[],
g=0,h=z(a)?a.split(""""):a,k=0;k<d;k++)if(k in h){var m=h[k];if(b.call(c,m,k,a))f[g++]=m}return f},Ha=C.map?function(a,b,c){return C.map.call(a,b,c)}:function(a,b,c){for(var d=a.length,f=Array(d),g=z(a)?a.split(""""):a,h=0;h<d;h++)if(h in g)f[h]=b.call(c,g[h],h,a);return f};function Ia(a,b,c){a:{for(var d=a.length,f=z(a)?a.split(""""):a,g=0;g<d;g++)if(g in f&&b.call(c,f[g],g,a)){b=g;break a}b=-1}return b<0?j:z(a)?a.charAt(b):a[b]}
function Ja(a){if(!y(a))for(var b=a.length-1;b>=0;b--)delete a[b];a.length=0}function Ka(a,b){var c=D(a,b),d;if(d=c>=0)C.splice.call(a,c,1);return d}function La(){return C.concat.apply(C,arguments)}function Ma(a){if(y(a))return La(a);else{for(var b=[],c=0,d=a.length;c<d;c++)b[c]=a[c];return b}}
function Na(a){for(var b=1;b<arguments.length;b++){var c=arguments[b],d;if(y(c)||(d=ha(c))&&c.hasOwnProperty(""callee""))a.push.apply(a,c);else if(d)for(var f=a.length,g=c.length,h=0;h<g;h++)a[f+h]=c[h];else a.push(c)}}function Oa(a){return C.splice.apply(a,Pa(arguments,1))}function Pa(a,b,c){return arguments.length<=2?C.slice.call(a,b):C.slice.call(a,b,c)}
function Qa(a,b,c){if(!ha(a)||!ha(b)||a.length!=b.length)return l;var d=a.length;c=c||Ra;for(var f=0;f<d;f++)if(!c(a[f],b[f]))return l;return i}function Sa(a,b){return a>b?1:a<b?-1:0}function Ra(a,b){return a===b};var Ua;function Va(a){return(a=a.className)&&typeof a.split==x?a.split(/\s+/):[]}function Wa(a){var b=Va(a),c;c=Pa(arguments,1);for(var d=0,f=0;f<c.length;f++)if(!(D(b,c[f])>=0)){b.push(c[f]);d++}c=d==c.length;a.className=b.join("" "");return c}function Xa(a){var b=Va(a),c;c=Pa(arguments,1);for(var d=0,f=0;f<b.length;f++)if(D(c,b[f])>=0){Oa(b,f--,1);d++}c=d==c.length;a.className=b.join("" "");return c};function E(a,b){this.width=a;this.height=b}s=E.prototype;s.B=function(){return new E(this.width,this.height)};s.xc=function(){return!(this.width*this.height)};s.ceil=function(){this.width=Math.ceil(this.width);this.height=Math.ceil(this.height);return this};s.floor=function(){this.width=Math.floor(this.width);this.height=Math.floor(this.height);return this};s.round=function(){this.width=Math.round(this.width);this.height=Math.round(this.height);return this};
s.scale=function(a){this.width*=a;this.height*=a;return this};function Ya(a,b,c){for(var d in a)b.call(c,a[d],d,a)}function Za(a){var b=[],c=0,d;for(d in a)b[c++]=a[d];return b}function $a(a){var b=[],c=0,d;for(d in a)b[c++]=d;return b}function ab(a){for(var b in a)return l;return i}function bb(a,b){var c;if(c=b in a)delete a[b];return c}function cb(a,b,c){if(b in a)e(Error('The object already contains the key ""'+b+'""'));a[b]=c}function db(a,b,c){if(b in a)return a[b];return c}function eb(a){var b={},c;for(c in a)b[c]=a[c];return b}
var fb=[""constructor"",""hasOwnProperty"",""isPrototypeOf"",""propertyIsEnumerable"",""toLocaleString"",""toString"",""valueOf""];function gb(a){for(var b,c,d=1;d<arguments.length;d++){c=arguments[d];for(b in c)a[b]=c[b];for(var f=0;f<fb.length;f++){b=fb[f];if(Object.prototype.hasOwnProperty.call(c,b))a[b]=c[b]}}};var hb,ib,jb,kb,mb,nb;function ob(){return t.navigator?t.navigator.userAgent:j}mb=kb=jb=ib=hb=l;var pb;if(pb=ob()){var qb=t.navigator;hb=pb.indexOf(""Opera"")==0;ib=!hb&&pb.indexOf(""MSIE"")!=-1;kb=(jb=!hb&&pb.indexOf(""WebKit"")!=-1)&&pb.indexOf(""Mobile"")!=-1;mb=!hb&&!jb&&qb.product==""Gecko""}var rb=hb,F=ib,G=mb,sb=jb,tb=kb,ub=t.navigator,vb=ub&&ub.platform||"""";nb=vb.indexOf(""Mac"")!=-1;var wb=vb.indexOf(""Linux"")!=-1,xb;
a:{var yb="""",zb;if(rb&&t.opera){var Ab=t.opera.version;yb=typeof Ab==x?Ab():Ab}else{if(G)zb=/rv\:([^\);]+)(\)|;)/;else if(F)zb=/MSIE\s+([^\);]+)(\)|;)/;else if(sb)zb=/WebKit\/(\S+)/;if(zb){var Bb=zb.exec(ob());yb=Bb?Bb[1]:""""}}if(F){var Cb,Db=t.document;Cb=Db?Db.documentMode:undefined;if(Cb>parseFloat(yb)){xb=String(Cb);break a}}xb=yb}var Eb=xb,Fb={};function Gb(a){return Fb[a]||(Fb[a]=Ca(Eb,a)>=0)};var Hb=!F||Gb(""9""),Ib=F&&!Gb(""9"");function H(a){return z(a)?document.getElementById(a):a}
function Jb(a,b,c,d){a=d||a;b=b&&b!=""*""?b.toUpperCase():"""";if(a.querySelectorAll&&a.querySelector&&(!sb||document.compatMode==""CSS1Compat""||Gb(""528""))&&(b||c))return a.querySelectorAll(b+(c?"".""+c:""""));if(c&&a.getElementsByClassName){a=a.getElementsByClassName(c);if(b){d={};for(var f=0,g=0,h;h=a[g];g++)if(b==h.nodeName)d[f++]=h;d.length=f;return d}else return a}a=a.getElementsByTagName(b||""*"");if(c){d={};for(g=f=0;h=a[g];g++){b=h.className;if(typeof b.split==x&&D(b.split(/\s+/),c)>=0)d[f++]=h}d.length=
f;return d}else return a}function Kb(a,b){Ya(b,function(c,d){if(d==""style"")a.style.cssText=c;else if(d==""class"")a.className=c;else if(d==""for"")a.htmlFor=c;else if(d in Lb)a.setAttribute(Lb[d],c);else a[d]=c})}var Mb=""height"",Lb={cellpadding:""cellPadding"",cellspacing:""cellSpacing"",colspan:""colSpan"",rowspan:""rowSpan"",valign:""vAlign"",height:Mb,width:""width"",usemap:""useMap"",frameborder:""frameBorder"",type:""type""};function I(){return Nb(document,arguments)}
function Nb(a,b){var c=b[0],d=b[1];if(!Hb&&d&&(d.name||d.type)){c=[""<"",c];d.name&&c.push(' name=""',ua(d.name),'""');if(d.type){c.push(' type=""',ua(d.type),'""');var f={};gb(f,d);d=f;delete d.type}c.push("">"");c=c.join("""")}c=a.createElement(c);if(d)if(z(d))c.className=d;else y(d)?Wa.apply(j,[c].concat(d)):Kb(c,d);b.length>2&&Ob(a,c,b,2);return c}
function Ob(a,b,c,d){function f(h){if(h)b.appendChild(z(h)?a.createTextNode(h):h)}for(d=d;d<c.length;d++){var g=c[d];ha(g)&&!(ka(g)&&g.nodeType>0)?Fa(Pb(g)?Ma(g):g,f):f(g)}}function Qb(a){for(var b;b=a.firstChild;)a.removeChild(b)}function Rb(a){return a&&a.parentNode?a.parentNode.removeChild(a):j}function Sb(a){return a.nodeType==9?a:a.ownerDocument||a.document}
function Tb(a,b){if(""textContent""in a)a.textContent=b;else if(a.firstChild&&a.firstChild.nodeType==3){for(;a.lastChild!=a.firstChild;)a.removeChild(a.lastChild);a.firstChild.data=b}else{Qb(a);a.appendChild(Sb(a).createTextNode(b))}}var Ub={SCRIPT:1,STYLE:1,HEAD:1,IFRAME:1,OBJECT:1},Vb={IMG:"" "",BR:""\n""};
function Wb(a,b,c){if(!(a.nodeName in Ub))if(a.nodeType==3)c?b.push(String(a.nodeValue).replace(/(\r\n|\r|\n)/g,"""")):b.push(a.nodeValue);else if(a.nodeName in Vb)b.push(Vb[a.nodeName]);else for(a=a.firstChild;a;){Wb(a,b,c);a=a.nextSibling}}function Pb(a){if(a&&typeof a.length==fa)if(ka(a))return typeof a.item==x||typeof a.item==""string"";else if(ja(a))return typeof a.item==x;return l}function Xb(a){this.a=a||t.document||document}s=Xb.prototype;s.c=function(a){return z(a)?this.a.getElementById(a):a};
s.u=function(){return Nb(this.a,arguments)};s.createElement=function(a){return this.a.createElement(a)};s.createTextNode=function(a){return this.a.createTextNode(a)};s.appendChild=function(a,b){a.appendChild(b)};s.append=function(a){Ob(Sb(a),a,arguments,1)};var Yb;!F||Gb(""9"");var Zb=F&&!Gb(""8"");function $b(){}$b.prototype.gd=l;$b.prototype.ea=function(){if(!this.gd){this.gd=i;this.o()}};$b.prototype.o=aa();function J(a,b){this.type=a;this.currentTarget=this.target=b}B(J,$b);s=J.prototype;s.o=function(){delete this.type;delete this.target;delete this.currentTarget};s.eb=l;s.Vb=i;s.stopPropagation=function(){this.eb=i};s.preventDefault=function(){this.Vb=l};function ac(a,b){a&&this.uc(a,b)}B(ac,J);s=ac.prototype;s.target=j;s.relatedTarget=j;s.offsetX=0;s.offsetY=0;s.clientX=0;s.clientY=0;s.screenX=0;s.screenY=0;s.button=0;s.keyCode=0;s.charCode=0;s.ctrlKey=l;s.altKey=l;s.shiftKey=l;s.metaKey=l;s.$e=l;s.rb=j;
s.uc=function(a,b){var c=this.type=a.type;this.target=a.target||a.srcElement;this.currentTarget=b;var d=a.relatedTarget;if(d){if(G)try{d=d.nodeName&&d}catch(f){d=j}}else if(c==""mouseover"")d=a.fromElement;else if(c==""mouseout"")d=a.toElement;this.relatedTarget=d;this.offsetX=a.offsetX!==undefined?a.offsetX:a.layerX;this.offsetY=a.offsetY!==undefined?a.offsetY:a.layerY;this.clientX=a.clientX!==undefined?a.clientX:a.pageX;this.clientY=a.clientY!==undefined?a.clientY:a.pageY;this.screenX=a.screenX||0;
this.screenY=a.screenY||0;this.button=a.button;this.keyCode=a.keyCode||0;this.charCode=a.charCode||(c==""keypress""?a.keyCode:0);this.ctrlKey=a.ctrlKey;this.altKey=a.altKey;this.shiftKey=a.shiftKey;this.metaKey=a.metaKey;this.$e=nb?a.metaKey:a.ctrlKey;this.rb=a;delete this.Vb;delete this.eb};s.stopPropagation=function(){ac.e.stopPropagation.call(this);if(this.rb.stopPropagation)this.rb.stopPropagation();else this.rb.cancelBubble=i};
s.preventDefault=function(){ac.e.preventDefault.call(this);var a=this.rb;if(a.preventDefault)a.preventDefault();else{a.returnValue=l;if(Zb)try{if(a.ctrlKey||a.keyCode>=112&&a.keyCode<=123)a.keyCode=-1}catch(b){}}};s.o=function(){ac.e.o.call(this);this.relatedTarget=this.currentTarget=this.target=this.rb=j};function bc(a,b){this.i=b;this.b=[];if(a>this.i)e(Error(""[goog.structs.SimplePool] Initial cannot be greater than max""));for(var c=0;c<a;c++)this.b.push(this.a?this.a():{})}B(bc,$b);bc.prototype.a=j;bc.prototype.f=j;function cc(a){if(a.b.length)return a.b.pop();return a.a?a.a():{}}function dc(a,b){a.b.length<a.i?a.b.push(b):ec(a,b)}function ec(a,b){if(a.f)a.f(b);else if(ka(b))if(ja(b.ea))b.ea();else for(var c in b)delete b[c]}
bc.prototype.o=function(){bc.e.o.call(this);for(var a=this.b;a.length;)ec(this,a.pop());delete this.b};var fc;var gc=(fc=""ScriptEngine""in t&&t.ScriptEngine()==""JScript"")?t.ScriptEngineMajorVersion()+"".""+t.ScriptEngineMinorVersion()+"".""+t.ScriptEngineBuildVersion():""0"";function hc(){}var ic=0;s=hc.prototype;s.key=0;s.Ab=l;s.Zc=l;s.uc=function(a,b,c,d,f,g){if(ja(a))this.a=i;else if(a&&a.handleEvent&&ja(a.handleEvent))this.a=l;else e(Error(""Invalid listener argument""));this.ub=a;this.b=b;this.src=c;this.type=d;this.capture=!!f;this.sc=g;this.Zc=l;this.key=++ic;this.Ab=l};s.handleEvent=function(a){if(this.a)return this.ub.call(this.sc||this.src,a);return this.ub.handleEvent.call(this.ub,a)};var jc,kc,lc,mc,nc,oc,pc,qc,rc,sc,tc;
(function(){function a(){return{k:0,ua:0}}function b(){return[]}function c(){function r(u){return h.call(r.src,r.key,u)}return r}function d(){return new hc}function f(){return new ac}var g=fc&&!(Ca(gc,""5.7"")>=0),h;oc=function(r){h=r};if(g){jc=function(){return cc(k)};kc=function(r){dc(k,r)};lc=function(){return cc(m)};mc=function(r){dc(m,r)};nc=function(){return cc(n)};pc=function(){dc(n,c())};qc=function(){return cc(p)};rc=function(r){dc(p,r)};sc=function(){return cc(q)};tc=function(r){dc(q,r)};
var k=new bc(0,600);k.a=a;var m=new bc(0,600);m.a=b;var n=new bc(0,600);n.a=c;var p=new bc(0,600);p.a=d;var q=new bc(0,600);q.a=f}else{jc=a;kc=v;lc=b;mc=v;nc=c;pc=v;qc=d;rc=v;sc=f;tc=v}})();var uc={},K={},vc={},wc={};
function L(a,b,c,d,f){if(b)if(y(b)){for(var g=0;g<b.length;g++)L(a,b[g],c,d,f);return j}else{d=!!d;var h=K;b in h||(h[b]=jc());h=h[b];if(!(d in h)){h[d]=jc();h.k++}h=h[d];var k=la(a),m;h.ua++;if(h[k]){m=h[k];for(g=0;g<m.length;g++){h=m[g];if(h.ub==c&&h.sc==f){if(h.Ab)break;return m[g].key}}}else{m=h[k]=lc();h.k++}g=nc();g.src=a;h=qc();h.uc(c,g,a,b,d,f);c=h.key;g.key=c;m.push(h);uc[c]=h;vc[k]||(vc[k]=lc());vc[k].push(h);if(a.addEventListener){if(a==t||!a.ed)a.addEventListener(b,g,d)}else a.attachEvent(xc(b),
g);return c}else e(Error(""Invalid event type""))}function yc(a,b,c,d,f){if(y(b)){for(var g=0;g<b.length;g++)yc(a,b[g],c,d,f);return j}a=L(a,b,c,d,f);uc[a].Zc=i;return a}function zc(a,b,c,d,f){if(y(b)){for(var g=0;g<b.length;g++)zc(a,b[g],c,d,f);return j}d=!!d;a=Ac(a,b,d);if(!a)return l;for(g=0;g<a.length;g++)if(a[g].ub==c&&a[g].capture==d&&a[g].sc==f)return Bc(a[g].key);return l}
function Bc(a){if(!uc[a])return l;var b=uc[a];if(b.Ab)return l;var c=b.src,d=b.type,f=b.b,g=b.capture;if(c.removeEventListener){if(c==t||!c.ed)c.removeEventListener(d,f,g)}else c.detachEvent&&c.detachEvent(xc(d),f);c=la(c);f=K[d][g][c];if(vc[c]){var h=vc[c];Ka(h,b);h.length==0&&delete vc[c]}b.Ab=i;f.ie=i;Cc(d,g,c,f);delete uc[a];return i}
function Cc(a,b,c,d){if(!d.Ac)if(d.ie){for(var f=0,g=0;f<d.length;f++)if(d[f].Ab){var h=d[f].b;h.src=j;pc(h);rc(d[f])}else{if(f!=g)d[g]=d[f];g++}d.length=g;d.ie=l;if(g==0){mc(d);delete K[a][b][c];K[a][b].k--;if(K[a][b].k==0){kc(K[a][b]);delete K[a][b];K[a].k--}if(K[a].k==0){kc(K[a]);delete K[a]}}}}
function Dc(a,b,c){var d=0,f=a==j,g=b==j,h=c==j;c=!!c;if(f)Ya(vc,function(m){for(var n=m.length-1;n>=0;n--){var p=m[n];if((g||b==p.type)&&(h||c==p.capture)){Bc(p.key);d++}}});else{a=la(a);if(vc[a]){a=vc[a];for(f=a.length-1;f>=0;f--){var k=a[f];if((g||b==k.type)&&(h||c==k.capture)){Bc(k.key);d++}}}}return d}function Ac(a,b,c){var d=K;if(b in d){d=d[b];if(c in d){d=d[c];a=la(a);if(d[a])return d[a]}}return j}function xc(a){if(a in wc)return wc[a];return wc[a]=""on""+a}
function Ec(a,b,c,d,f){var g=1;b=la(b);if(a[b]){a.ua--;a=a[b];if(a.Ac)a.Ac++;else a.Ac=1;try{for(var h=a.length,k=0;k<h;k++){var m=a[k];if(m&&!m.Ab)g&=Fc(m,f)!==l}}finally{a.Ac--;Cc(c,d,b,a)}}return Boolean(g)}function Fc(a,b){var c=a.handleEvent(b);a.Zc&&Bc(a.key);return c}
oc(function(a,b){if(!uc[a])return i;var c=uc[a],d=c.type,f=K;if(!(d in f))return i;f=f[d];var g,h;if(Yb===undefined)Yb=F&&!t.addEventListener;if(Yb){var k;if(!(k=b))a:{k=""window.event"".split(""."");for(var m=t;g=k.shift();)if(m[g])m=m[g];else{k=j;break a}k=m}g=k;k=i in f;m=l in f;if(k){if(g.keyCode<0||g.returnValue!=undefined)return i;a:{var n=l;if(g.keyCode==0)try{g.keyCode=-1;break a}catch(p){n=i}if(n||g.returnValue==undefined)g.returnValue=i}}n=sc();n.uc(g,this);g=i;try{if(k){for(var q=lc(),r=n.currentTarget;r;r=
r.parentNode)q.push(r);h=f[i];h.ua=h.k;for(var u=q.length-1;!n.eb&&u>=0&&h.ua;u--){n.currentTarget=q[u];g&=Ec(h,q[u],d,i,n)}if(m){h=f[l];h.ua=h.k;for(u=0;!n.eb&&u<q.length&&h.ua;u++){n.currentTarget=q[u];g&=Ec(h,q[u],d,l,n)}}}else g=Fc(c,n)}finally{if(q){q.length=0;mc(q)}n.ea();tc(n)}return g}d=new ac(b,this);try{g=Fc(c,d)}finally{d.ea()}return g});function Gc(a){a=String(a);var b;b=/^\s*$/.test(a)?l:/^[\],:{}\s\u2028\u2029]*$/.test(a.replace(/\\[""\\\/bfnrtu]/g,""@"").replace(/""[^""\\\n\r\u2028\u2029\x00-\x08\x10-\x1f\x80-\x9f]*""|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,""]"").replace(/(?:^|:|,)(?:[\s\u2028\u2029]*\[)+/g,""""));if(b)try{return eval(""(""+a+"")"")}catch(c){}e(Error(""Invalid JSON string: ""+a))}function Hc(){}Hc.prototype.pe=function(a){var b=[];Ic(this,a,b);return b.join("""")};
function Ic(a,b,c){switch(typeof b){case ""string"":Jc(a,b,c);break;case fa:c.push(isFinite(b)&&!isNaN(b)?b:""null"");break;case ""boolean"":c.push(b);break;case ""undefined"":c.push(""null"");break;case ""object"":if(b==j){c.push(""null"");break}if(y(b)){var d=b.length;c.push(""["");for(var f="""",g=0;g<d;g++){c.push(f);Ic(a,b[g],c);f="",""}c.push(""]"");break}c.push(""{"");d="""";for(f in b)if(b.hasOwnProperty(f)){g=b[f];if(typeof g!=x){c.push(d);Jc(a,f,c);c.push("":"");Ic(a,g,c);d="",""}}c.push(""}"");break;case x:break;default:e(Error(""Unknown type: ""+
typeof b))}}var Kc={'""':'\\""',""\\"":""\\\\"",""/"":""\\/"",""\u0008"":""\\b"",""\u000c"":""\\f"",""\n"":""\\n"",""\r"":""\\r"",""\t"":""\\t"",""\u000b"":""\\u000b""},Lc=/\uffff/.test(""\uffff"")?/[\\\""\x00-\x1f\x7f-\uffff]/g:/[\\\""\x00-\x1f\x7f-\xff]/g;function Jc(a,b,c){c.push('""',b.replace(Lc,function(d){if(d in Kc)return Kc[d];var f=d.charCodeAt(0),g=""\\u"";if(f<16)g+=""000"";else if(f<256)g+=""00"";else if(f<4096)g+=""0"";return Kc[d]=g+f.toString(16)}),'""')};var Mc=""StopIteration""in t?t.StopIteration:Error(""StopIteration"");function Nc(){}Nc.prototype.next=function(){e(Mc)};Nc.prototype.ve=function(){return this};function Oc(a){if(typeof a.$a==x)return a.$a();if(z(a))return a.split("""");if(ha(a)){for(var b=[],c=a.length,d=0;d<c;d++)b.push(a[d]);return b}return Za(a)}function Pc(a,b,c){if(typeof a.forEach==x)a.forEach(b,c);else if(ha(a)||z(a))Fa(a,b,c);else{var d;if(typeof a.sb==x)d=a.sb();else if(typeof a.$a!=x)if(ha(a)||z(a)){d=[];for(var f=a.length,g=0;g<f;g++)d.push(g);d=d}else d=$a(a);else d=void 0;f=Oc(a);g=f.length;for(var h=0;h<g;h++)b.call(c,f[h],d&&d[h],a)}};function Qc(a){this.b={};this.a=[];var b=arguments.length;if(b>1){if(b%2)e(Error(""Uneven number of arguments""));for(var c=0;c<b;c+=2)this.D(arguments[c],arguments[c+1])}else if(a){if(a instanceof Qc){b=a.sb();c=a.$a()}else{b=$a(a);c=Za(a)}for(var d=0;d<b.length;d++)this.D(b[d],c[d])}}s=Qc.prototype;s.k=0;s.ec=0;s.Xd=o(""k"");s.$a=function(){Rc(this);for(var a=[],b=0;b<this.a.length;b++)a.push(this.b[this.a[b]]);return a};s.sb=function(){Rc(this);return this.a.concat()};
s.Ja=function(a){return Sc(this.b,a)};s.Hb=function(a,b){if(this===a)return i;if(this.k!=a.Xd())return l;var c=b||Tc;Rc(this);for(var d,f=0;d=this.a[f];f++)if(!c(this.g(d),a.g(d)))return l;return i};function Tc(a,b){return a===b}s.xc=function(){return this.k==0};s.clear=function(){this.b={};this.ec=this.k=this.a.length=0};s.remove=function(a){if(Sc(this.b,a)){delete this.b[a];this.k--;this.ec++;this.a.length>2*this.k&&Rc(this);return i}return l};
function Rc(a){if(a.k!=a.a.length){for(var b=0,c=0;b<a.a.length;){var d=a.a[b];if(Sc(a.b,d))a.a[c++]=d;b++}a.a.length=c}if(a.k!=a.a.length){var f={};for(c=b=0;b<a.a.length;){d=a.a[b];if(!Sc(f,d)){a.a[c++]=d;f[d]=1}b++}a.a.length=c}}s.g=function(a,b){if(Sc(this.b,a))return this.b[a];return b};s.D=function(a,b){if(!Sc(this.b,a)){this.k++;this.a.push(a);this.ec++}this.b[a]=b};s.B=function(){return new Qc(this)};
s.ve=function(a){Rc(this);var b=0,c=this.a,d=this.b,f=this.ec,g=this,h=new Nc;h.next=function(){for(;;){if(f!=g.ec)e(Error(""The map has changed since the iterator was created""));if(b>=c.length)e(Mc);var k=c[b++];return a?k:d[k]}};return h};function Sc(a,b){return Object.prototype.hasOwnProperty.call(a,b)};function Uc(){}B(Uc,$b);s=Uc.prototype;s.ed=i;s.Fc=j;s.Kc=ba(""Fc"");s.addEventListener=function(a,b,c,d){L(this,a,b,c,d)};s.removeEventListener=function(a,b,c,d){zc(this,a,b,c,d)};
s.dispatchEvent=function(a){a=a;if(z(a))a=new J(a,this);else if(a instanceof J)a.target=a.target||this;else{var b=a;a=new J(a.type,this);gb(a,b)}b=1;var c,d=a.type,f=K;if(d in f){f=f[d];d=i in f;var g;if(d){c=[];for(g=this;g;g=g.Fc)c.push(g);g=f[i];g.ua=g.k;for(var h=c.length-1;!a.eb&&h>=0&&g.ua;h--){a.currentTarget=c[h];b&=Ec(g,c[h],a.type,i,a)&&a.Vb!=l}}if(l in f){g=f[l];g.ua=g.k;if(d)for(h=0;!a.eb&&h<c.length&&g.ua;h++){a.currentTarget=c[h];b&=Ec(g,c[h],a.type,l,a)&&a.Vb!=l}else for(c=this;!a.eb&&
c&&g.ua;c=c.Fc){a.currentTarget=c;b&=Ec(g,c,a.type,l,a)&&a.Vb!=l}}a=Boolean(b)}else a=i;return a};s.o=function(){Uc.e.o.call(this);Dc(this);this.Fc=j};function Vc(a,b){this.a=a||1;this.b=b||Wc;this.f=A(this.jf,this);this.i=qa()}B(Vc,Uc);Vc.prototype.enabled=l;var Wc=t.window;s=Vc.prototype;s.Ha=j;s.setInterval=function(a){this.a=a;if(this.Ha&&this.enabled){this.stop();this.start()}else this.Ha&&this.stop()};s.jf=function(){if(this.enabled){var a=qa()-this.i;if(a>0&&a<this.a*0.8)this.Ha=this.b.setTimeout(this.f,this.a-a);else{this.dispatchEvent(""tick"");if(this.enabled){this.Ha=this.b.setTimeout(this.f,this.a);this.i=qa()}}}};
s.start=function(){this.enabled=i;if(!this.Ha){this.Ha=this.b.setTimeout(this.f,this.a);this.i=qa()}};s.stop=function(){this.enabled=l;if(this.Ha){this.b.clearTimeout(this.Ha);this.Ha=j}};s.o=function(){Vc.e.o.call(this);this.stop();delete this.b};function Xc(){if(G){this.Wa={};this.Tc={};this.Db=[]}}Xc.prototype.qb=G;function Yc(a,b){if(a.qb)for(var c=la(b),d=0;d<a.Db.length;d++){var f=a.Db[d];Zc(a,a.Wa,f,c);Zc(a,a.Tc,c,f)}}function $c(a,b){var c=a.Tc[b],d=a.Wa[b];c&&d&&Fa(c,function(f){Fa(d,function(g){Zc(this,this.Wa,f,g);Zc(this,this.Tc,g,f)},this)},a)}function Zc(a,b,c,d){b[c]||(b[c]=[]);D(b[c],d)>=0||b[c].push(d)}var M=new Xc;function ad(){}ad.prototype.ic=j;function bd(){return cd(dd)}var dd;function ed(){}B(ed,ad);function cd(a){return(a=fd(a))?new ActiveXObject(a):new XMLHttpRequest}function gd(a){var b={};if(fd(a)){b[0]=i;b[1]=i}return b}ed.prototype.a=j;
function fd(a){if(!a.a&&typeof XMLHttpRequest==""undefined""&&typeof ActiveXObject!=""undefined""){for(var b=[""MSXML2.XMLHTTP.6.0"",""MSXML2.XMLHTTP.3.0"",""MSXML2.XMLHTTP"",""Microsoft.XMLHTTP""],c=0;c<b.length;c++){var d=b[c];try{new ActiveXObject(d);return a.a=d}catch(f){}}e(Error(""Could not create ActiveXObject. ActiveX might be disabled, or MSXML might not be installed""))}return a.a}dd=new ed;function hd(a){this.headers=new Qc;this.a=a||j}B(hd,Uc);var id=[];function jd(a,b,c,d,f,g){var h=new hd;id.push(h);b&&L(h,""complete"",b);L(h,""ready"",pa(kd,h));if(g)h.Oc=Math.max(0,g);h.send(a,c,d,f)}function kd(a){a.ea();Ka(id,a)}s=hd.prototype;s.Ia=l;s.A=j;s.Sc=j;s.Te="""";s.Se="""";s.tb=0;s.db="""";s.nd=l;s.tc=l;s.sd=l;s.ab=l;s.Oc=0;s.gb=j;
s.send=function(a,b,c,d){if(this.A)e(Error(""[goog.net.XhrIo] Object is active with another request""));b=b||""GET"";this.Te=a;this.db="""";this.tb=0;this.Se=b;this.nd=l;this.Ia=i;this.A=this.a?cd(this.a):new bd;this.Sc=this.a?this.a.ic||(this.a.ic=gd(this.a)):dd.ic||(dd.ic=gd(dd));Yc(M,this.A);this.A.onreadystatechange=A(this.ke,this);try{this.sd=i;this.A.open(b,a,i);this.sd=l}catch(f){ld(this,5,f);return}a=c||"""";var g=this.headers.B();d&&Pc(d,function(k,m){g.D(m,k)});b==""POST""&&!g.Ja(""Content-Type"")&&
g.D(""Content-Type"",""application/x-www-form-urlencoded;charset=utf-8"");Pc(g,function(k,m){this.A.setRequestHeader(m,k)},this);try{if(this.gb){Wc.clearTimeout(this.gb);this.gb=j}if(this.Oc>0)this.gb=Wc.setTimeout(A(this.kf,this),this.Oc);this.tc=i;this.A.send(a);this.tc=l}catch(h){ld(this,5,h)}};
s.dispatchEvent=function(a){if(this.A){M.qb&&M.Db.push(z(this.A)?this.A:ka(this.A)?la(this.A):"""");try{return hd.e.dispatchEvent.call(this,a)}finally{M.qb&&$c(M,M.Db.pop())}}else return hd.e.dispatchEvent.call(this,a)};s.kf=function(){if(typeof da!=""undefined"")if(this.A){this.db=""Timed out after ""+this.Oc+""ms, aborting"";this.tb=8;this.dispatchEvent(""timeout"");this.abort(8)}};function ld(a,b,c){a.Ia=l;if(a.A){a.ab=i;a.A.abort();a.ab=l}a.db=c;a.tb=b;md(a);nd(a)}
function md(a){if(!a.nd){a.nd=i;a.dispatchEvent(""complete"");a.dispatchEvent(""error"")}}s.abort=function(a){if(this.A&&this.Ia){this.Ia=l;this.ab=i;this.A.abort();this.ab=l;this.tb=a||7;this.dispatchEvent(""complete"");this.dispatchEvent(""abort"");nd(this)}};s.o=function(){if(this.A){if(this.Ia){this.Ia=l;this.ab=i;this.A.abort();this.ab=l}nd(this,i)}hd.e.o.call(this)};s.ke=function(){!this.sd&&!this.tc&&!this.ab?this.Xe():od(this)};s.Xe=function(){od(this)};
function od(a){if(a.Ia)if(typeof da!=""undefined"")if(!(a.Sc[1]&&pd(a)==4&&qd(a)==2))if(a.tc&&pd(a)==4)Wc.setTimeout(A(a.ke,a),0);else{a.dispatchEvent(""readystatechange"");if(pd(a)==4){a.Ia=l;if(rd(a)){a.dispatchEvent(""complete"");a.dispatchEvent(""success"")}else{a.tb=6;var b;try{b=pd(a)>2?a.A.statusText:""""}catch(c){b=""""}a.db=b+"" [""+qd(a)+""]"";md(a)}nd(a)}}}
function nd(a,b){if(a.A){var c=a.A,d=a.Sc[0]?v:j;a.A=j;a.Sc=j;if(a.gb){Wc.clearTimeout(a.gb);a.gb=j}if(!b){M.qb&&M.Db.push(z(c)?c:ka(c)?la(c):"""");a.dispatchEvent(""ready"");M.qb&&$c(M,M.Db.pop())}if(M.qb){var f=la(c);delete M.Tc[f];for(var g in M.Wa){Ka(M.Wa[g],f);M.Wa[g].length==0&&delete M.Wa[g]}}try{c.onreadystatechange=d}catch(h){}}}function rd(a){switch(qd(a)){case 0:case 200:case 204:case 304:return i;default:return l}}function pd(a){return a.A?a.A.readyState:0}
function qd(a){try{return pd(a)>2?a.A.status:-1}catch(b){return-1}};function sd(a){this.a=fc?[]:"""";a!=j&&this.append.apply(this,arguments)}sd.prototype.D=function(a){this.clear();this.append(a)};if(fc){sd.prototype.b=0;sd.prototype.append=function(a,b){if(b==j)this.a[this.b++]=a;else{this.a.push.apply(this.a,arguments);this.b=this.a.length}return this}}else sd.prototype.append=function(a,b){this.a+=a;if(b!=j)for(var c=1;c<arguments.length;c++)this.a+=arguments[c];return this};sd.prototype.clear=function(){if(fc)this.b=this.a.length=0;else this.a=""""};
sd.prototype.toString=function(){if(fc){var a=this.a.join("""");this.clear();a&&this.append(a);return a}else return this.a};var td=RegExp(""^(?:([^:/?#.]+):)?(?://(?:([^/?#]*)@)?([\\w\\d\\-\\u0100-\\uffff.%]*)(?::([0-9]+))?)?([^?#]+)?(?:\\?([^#]*))?(?:#(.*))?$""),ud=/#|$/;function vd(a,b){var c=a.search(ud),d;a:{d=0;for(var f=b.length;(d=a.indexOf(b,d))>=0&&d<c;){var g=a.charCodeAt(d-1);if(g==38||g==63){g=a.charCodeAt(d+f);if(!g||g==61||g==38||g==35){d=d;break a}}d+=f+1}d=-1}if(d<0)return j;else{f=a.indexOf(""&"",d);if(f<0||f>c)f=c;d+=b.length+1;return decodeURIComponent(a.substr(d,f-d).replace(/\+/g,"" ""))}};function wd(a,b){var c;if(a instanceof wd){this.Cb(b==j?a.xa:b);xd(this,a.Oa);yd(this,a.dc);zd(this,a.ob);Ad(this,a.yb);this.aa(a.Lb());Bd(this,a.a.B());Cd(this,a.Ib)}else if(a&&(c=String(a).match(td))){this.Cb(!!b);xd(this,c[1]||"""",i);yd(this,c[2]||"""",i);zd(this,c[3]||"""",i);Ad(this,c[4]);this.aa(c[5]||"""",i);Bd(this,c[6]||"""",i);Cd(this,c[7]||"""",i)}else{this.Cb(!!b);this.a=new Dd(j,this,this.xa)}}s=wd.prototype;s.Oa="""";s.dc="""";s.ob="""";s.yb=j;s.Q="""";s.Ib="""";s.Re=l;s.xa=l;
s.toString=function(){if(this.oa)return this.oa;var a=[];this.Oa&&a.push(Ed(this.Oa,Fd),"":"");if(this.ob){a.push(""//"");this.dc&&a.push(Ed(this.dc,Fd),""@"");var b;b=this.ob;b=z(b)?encodeURIComponent(b):j;a.push(b);this.yb!=j&&a.push("":"",String(this.yb))}if(this.Q){this.ob&&this.Q.charAt(0)!=""/""&&a.push(""/"");a.push(Ed(this.Q,Gd))}(b=String(this.a))&&a.push(""?"",b);this.Ib&&a.push(""#"",Ed(this.Ib,Hd));return this.oa=a.join("""")};
s.B=function(){var a=this.Oa,b=this.dc,c=this.ob,d=this.yb,f=this.Q,g=this.a.B(),h=this.Ib,k=new wd(j,this.xa);a&&xd(k,a);b&&yd(k,b);c&&zd(k,c);d&&Ad(k,d);f&&k.aa(f);g&&Bd(k,g);h&&Cd(k,h);return k};function xd(a,b,c){Id(a);delete a.oa;a.Oa=c?b?decodeURIComponent(b):"""":b;if(a.Oa)a.Oa=a.Oa.replace(/:$/,"""");return a}function yd(a,b,c){Id(a);delete a.oa;a.dc=c?b?decodeURIComponent(b):"""":b;return a}function zd(a,b,c){Id(a);delete a.oa;a.ob=c?b?decodeURIComponent(b):"""":b;return a}
function Ad(a,b){Id(a);delete a.oa;if(b){b=Number(b);if(isNaN(b)||b<0)e(Error(""Bad port number ""+b));a.yb=b}else a.yb=j;return a}s.Lb=o(""Q"");s.aa=function(a,b){Id(this);delete this.oa;this.Q=b?a?decodeURIComponent(a):"""":a;return this};function Bd(a,b,c){Id(a);delete a.oa;if(b instanceof Dd){a.a=b;a.a.f=a;a.a.Cb(a.xa)}else{c||(b=Ed(b,Jd));a.a=new Dd(b,a,a.xa)}return a}function Cd(a,b,c){Id(a);delete a.oa;a.Ib=c?b?decodeURIComponent(b):"""":b;return a}
function Id(a){if(a.Re)e(Error(""Tried to modify a read-only Uri""))}s.Cb=function(a){this.xa=a;this.a&&this.a.Cb(a)};var Kd=/^[a-zA-Z0-9\-_.!~*'():\/;?]*$/;function Ed(a,b){var c=j;if(z(a)){c=a;Kd.test(c)||(c=encodeURI(a));if(c.search(b)>=0)c=c.replace(b,Ld)}return c}function Ld(a){a=a.charCodeAt(0);return""%""+(a>>4&15).toString(16)+(a&15).toString(16)}var Fd=/[#\/\?@]/g,Gd=/[\#\?]/g,Jd=/[\#\?@]/g,Hd=/#/g;function Dd(a,b,c){this.a=a||j;this.f=b||j;this.xa=!!c}
function Md(a){if(!a.I){a.I=new Qc;if(a.a)for(var b=a.a.split(""&""),c=0;c<b.length;c++){var d=b[c].indexOf(""=""),f=j,g=j;if(d>=0){f=b[c].substring(0,d);g=b[c].substring(d+1)}else f=b[c];f=decodeURIComponent(f.replace(/\+/g,"" ""));f=Nd(a,f);a.add(f,g?decodeURIComponent(g.replace(/\+/g,"" "")):"""")}}}s=Dd.prototype;s.I=j;s.k=j;s.Xd=function(){Md(this);return this.k};
s.add=function(a,b){Md(this);Od(this);a=Nd(this,a);if(this.Ja(a)){var c=this.I.g(a);y(c)?c.push(b):this.I.D(a,[c,b])}else this.I.D(a,b);this.k++;return this};s.remove=function(a){Md(this);a=Nd(this,a);if(this.I.Ja(a)){Od(this);var b=this.I.g(a);if(y(b))this.k-=b.length;else this.k--;return this.I.remove(a)}return l};s.clear=function(){Od(this);this.I&&this.I.clear();this.k=0};s.xc=function(){Md(this);return this.k==0};s.Ja=function(a){Md(this);a=Nd(this,a);return this.I.Ja(a)};
s.sb=function(){Md(this);for(var a=this.I.$a(),b=this.I.sb(),c=[],d=0;d<b.length;d++){var f=a[d];if(y(f))for(var g=0;g<f.length;g++)c.push(b[d]);else c.push(b[d])}return c};s.$a=function(a){Md(this);if(a){a=Nd(this,a);if(this.Ja(a)){var b=this.I.g(a);if(y(b))return b;else{a=[];a.push(b)}}else a=[]}else{b=this.I.$a();a=[];for(var c=0;c<b.length;c++){var d=b[c];y(d)?Na(a,d):a.push(d)}}return a};
s.D=function(a,b){Md(this);Od(this);a=Nd(this,a);if(this.Ja(a)){var c=this.I.g(a);if(y(c))this.k-=c.length;else this.k--}this.I.D(a,b);this.k++;return this};s.g=function(a,b){Md(this);a=Nd(this,a);if(this.Ja(a)){var c=this.I.g(a);return y(c)?c[0]:c}else return b};
s.toString=function(){if(this.a)return this.a;if(!this.I)return"""";for(var a=[],b=0,c=this.I.sb(),d=0;d<c.length;d++){var f=c[d],g=ta(f);f=this.I.g(f);if(y(f))for(var h=0;h<f.length;h++){b>0&&a.push(""&"");a.push(g);f[h]!==""""&&a.push(""="",ta(f[h]));b++}else{b>0&&a.push(""&"");a.push(g);f!==""""&&a.push(""="",ta(f));b++}}return this.a=a.join("""")};function Od(a){delete a.b;delete a.a;a.f&&delete a.f.oa}s.B=function(){var a=new Dd;if(this.b)a.b=this.b;if(this.a)a.a=this.a;if(this.I)a.I=this.I.B();return a};
function Nd(a,b){var c=String(b);if(a.xa)c=c.toLowerCase();return c}s.Cb=function(a){if(a&&!this.xa){Md(this);Od(this);Pc(this.I,function(b,c){var d=c.toLowerCase();if(c!=d){this.remove(c);this.add(d,b)}},this)}this.xa=a};var Pd={},Qd={};ea(""_setFlag"",function(a,b){Qd[a]=b},void 0);ea(""_initConsole"",Pd.Ce?Pd.Ce.zf:v,void 0);var Rd=F&&!Gb(""7""),Sd=j;function Td(a,b,c){var d={};if(a)d.id=a;if(b)d[""class""]=b;if(c)d.style=c;return d};function Ud(a,b){a.prototype.o=function(){a.e.o.call(this);for(var c in b)if(this[c]){if(b[c]!=0){var d=this[c];d&&typeof d.ea==x&&d.ea()}delete this[c]}}};function Vd(){this.ya=[]}B(Vd,$b);var Wd=j;function Xd(){return Wd||(Wd=new Vd)}Vd.prototype.ya=j;function Yd(a,b,c,d){if(b&&c&&a.ya.length<100){var f={};f.uC=b+2E3;f.cT=c;if(typeof d!=""undefined"")f.eS=d;a.ya.push(f)}}Vd.prototype.start=function(a,b){if(a&&this.ya.length<100){var c={};c.uC=a;c.s=qa();if(typeof b!=""undefined"")c.eS=b;this.ya.push(c)}};
Vd.prototype.end=function(a,b){if(a){var c=Ia(this.ya,function(f){return""s""in f&&f.uC==a},this);if(c&&c.s>0){c.uC=a+2E3;var d=qa()-c.s;delete c.s;c.cT=d;if(typeof b!=""undefined"")c.eS=b}}};function Zd(a,b){a.ya=b?[]:Ga(a.ya,function(c){return c&&!ia(c.cT)})}Ud(Vd,{ya:1});function $d(a,b){Xd().start(a,b)};var ae,be={};function ce(a,b,c){this.a=a;this.message=b;this.status=c}function de(a){var b=[],c=[],d;for(d in a){var f=a[d];if(f!==undefined&&f!==j){z(f)||(f=(new Hc).pe(f));b.push(d);c.push(f)}}a=[];d=0;for(f=Math.min(b.length,c.length);d<f;d++){var g=c[d];g!==undefined&&g!==j&&a.push(encodeURIComponent(b[d])+""=""+encodeURIComponent(g))}return a.join(""&"")}
function ee(a,b,c,d,f,g,h){var k=j;if(f)k=function(m){var n;try{var p;if(m.indexOf(""&&&START&&&"")==0)p=eval(""(""+m.substring(11)+"")"");else{var q=m.replace(/^\s*while\s*\(true\);/,"""");p=eval(""(""+q+"")"")}n=p;var r=n.debug;if(r){h&&h(r);delete n.debug}}catch(u){g&&g(new ce(6,u.toString(),500));return}f(n)};fe(a,b,c,d,k,g)}
function fe(a,b,c,d,f,g){b.POST_TOKEN=c;gb(b,d);b=de(b)+""&finis=true"";jd(a,function(h){h=h.target;if(rd(h)){if(f){var k;try{k=h.A?h.A.responseText:""""}catch(m){k=""""}f(k)}}else g&&g(new ce(h.tb,z(h.db)?h.db:String(h.db),qd(h)))},""POST"",b,j,2E4)}function ge(a,b,c){a=new sd(a);b&&a.append(""?"").append(de(b));c&&a.append(""#"").append(c);return a.toString()}function he(){if(!ae){var a=parseInt,b;b=(new wd(window.location)).a.g(""cS"");ae=a(b||10,10)}return ae}
function ie(a,b,c,d,f,g,h,k,m){if(a.length>0){$d(71,Math.ceil(a.length/he()));var n=je(b,a[0])?""update"":""create"",p=ke(b);if(n==""update"")p-=a.length;var q=a.slice(0,he()),r=1,u=typeof k==fa?k:3,w=typeof m==fa?m:300,Aa=function(lb){if(u==-1||r<u){r++;window.setTimeout(pa(le,q,b,c,d,g,Ta,Aa),w)}else{Xd().end(70,void 0);f(lb)}},Ta=function(lb){r=1;delete lb.protocol;lb.debug&&delete lb.debug;for(var Ie=0,xi=q.length;Ie<xi;Ie++){var Je=lb[q[Ie]];if(Je){Je.action=n;Je.index=p++}else{f(new ce(6,""Unable to fetch all components from the server."",
500));return}}a.splice(0,q.length);b.update({children:lb},i);Xd().end(70,void 0);if(a.length>0){q=a.slice(0,he());window.setTimeout(pa(le,q,b,c,d,g,Ta,Aa),300)}else{h&&h();Xd().end(71,void 0)}};le(q,b,c,d,g,Ta,Aa)}else h&&h()}function le(a,b,c,d,f,g,h){$d(70);a={action:""fetchcomponents"",tok:j,fetchBy:""slide_id"",id:b.z(),revision:b.Sd,components:a};f||(a.gu=i);ee(""components"",a,c,d,g,h)}ea(""_setSitePrefix"",aa(),void 0);ea(""_setNewDispatcherPage"",aa(),void 0);function me(a,b,c,d){a=[a];if(b&&b!=3){a.push(""&interval="");a.push(b)}c&&a.push(""&autoStart=true"");d&&a.push(""&loop=true"");return a.join("""")}ea(""_getPublishedUrl"",me,void 0);function ne(a,b,c,d,f,g){a=me(a,d,f,g);d=oe[c||""s""];return""<iframe ""+(b?'id=""'+b+'"" ':"""")+'src=""'+encodeURI(c&&c!=""s""?a+""&size=""+c:a)+'"" frameborder=""0"" width=""'+d[0]+'"" height=""'+d[1]+'""></iframe>'}var oe={l:[700,559],m:[555,451],s:[410,342]};ea(""_getEmbeddedHtml"",ne,void 0);function pe(){Vc.call(this);this.setInterval(3E3)}B(pe,Vc);s=pe.prototype;s.Xc=l;s.loop=l;s.start=function(){this.dispatchEvent(""play"");pe.e.start.call(this)};s.stop=function(){this.dispatchEvent(""pause"");pe.e.stop.call(this)};function qe(a,b){var c=b||window.location.href;a.ee(parseFloat(vd(c,""interval"")),vd(c,""autoStart"")==""true"",vd(c,""loop"")==""true"")}s.ee=function(a,b,c){this.setInterval(isNaN(a)||a<0?3E3:a*1E3);this.Xc=b;this.loop=c;this.Xc&&this.start()};pe.prototype.initialize=pe.prototype.ee;var re={},se=1,te=(document.location.protocol==""https:""?""https"":""http"")+""://gg.google.com/csi?v=2&s=presently&"";function ue(a){delete re[a]};var N=""DIV"";function ve(a){if(a){var b=H(a);if(b){this.ga=a;this.a={};a=Jb(document,N,""slide"",b);for(b=0;b<a.length;b++){var c=a[b].id;if(c)this.a[c]=i}}}}ve.prototype.toString=function(){return'<Gallery name=""'+this.ga+'"" />'};ve.prototype.getName=o(""ga"");function we(a){this.wa=a}B(we,$b);var xe=new bc(0,100);function ye(a,b,c,d,f,g){if(y(c))for(var h=0;h<c.length;h++)ye(a,b,c[h],d,f,g);else{b=L(b,c,d||a,f||l,g||a.wa||a);if(a.a)a.a[b]=i;else if(a.b){a.a=cc(xe);a.a[a.b]=i;a.b=j;a.a[b]=i}else a.b=b}return a}
function ze(a,b,c,d,f,g){if(a.b||a.a)if(y(c))for(var h=0;h<c.length;h++)ze(a,b,c[h],d,f,g);else{a:{d=d||a;g=g||a.wa||a;f=!!(f||l);if(b=Ac(b,c,f))for(c=0;c<b.length;c++)if(b[c].ub==d&&b[c].capture==f&&b[c].sc==g){b=b[c];break a}b=j}if(b){b=b.key;Bc(b);if(a.a)bb(a.a,b);else if(a.b==b)a.b=j}}return a}function Ae(a){if(a.a){for(var b in a.a){Bc(b);delete a.a[b]}dc(xe,a.a);a.a=j}else a.b&&Bc(a.b)}we.prototype.o=function(){we.e.o.call(this);Ae(this)};we.prototype.handleEvent=function(){e(Error(""EventHandler.handleEvent not implemented""))};function Be(a){return function(){return a}}var Ce=Be(l),De=Be(i);function Ee(a,b){this.f=a||"""";this.oc=b||"""";this.a=new Fe(16);this.b=new we(this)}B(Ee,Uc);s=Ee.prototype;s.na="""";s.ud="""";s.td=j;s.vd=j;s.Ea=o(""na"");s.$b=function(a,b){if(this.na!=a){this.na=a;this.dispatchEvent(new Ge(""g"",!!b))}};s.Na=function(){return!!(this.a.a&16)};s.W=function(a){a=!!(this.a.a&16)==a?this.a:new Fe(this.a.a^16);var b=this.a;if(!b.Hb(a)){this.a=a;this.dispatchEvent(new He(b))}};s.o=function(){Ee.e.o.call(this);this.b.ea()};function Ge(a,b){J.call(this,a);this.a=b||l}B(Ge,J);
function He(a){J.call(this,""f"");this.a=a}B(He,J);function Fe(a){this.a=ka(a)?a.a:a}Fe.prototype.$d=o(""a"");Fe.prototype.Hb=function(a){return this.a==a.a};function Ke(a,b,c,d){this.left=a;this.top=b;this.width=c;this.height=d}Ke.prototype.B=function(){return new Ke(this.left,this.top,this.width,this.height)};function Le(a,b){if(a==b)return i;if(!a||!b)return l;return a.left==b.left&&a.width==b.width&&a.top==b.top&&a.height==b.height}Ke.prototype.Ya=function(){return new E(this.width,this.height)};function Me(a,b,c){z(b)?Ne(a,c,b):Ya(b,pa(Ne,a))}function Ne(a,b,c){a.style[Oe(c)]=b}function Pe(a,b){var c=Sb(a);if(c.defaultView&&c.defaultView.getComputedStyle)if(c=c.defaultView.getComputedStyle(a,""""))return c[b];return j}function Qe(a,b,c){if(b instanceof E){c=b.height;b=b.width}else{if(c==undefined)e(Error(""missing height argument""));c=c}a.style.width=Re(b,i);a.style.height=Re(c,i)}function Re(a,b){if(typeof a==fa)a=(b?Math.round(a):a)+""px"";return a}var Se=""none"",Te=""hidden"",Ue=""absolute"";
function Ve(a){var b=rb&&!Gb(""10"");if((Pe(a,""display"")||(a.currentStyle?a.currentStyle.display:j)||a.style.display)!=Se)return b?new E(a.offsetWidth||a.clientWidth,a.offsetHeight||a.clientHeight):new E(a.offsetWidth,a.offsetHeight);var c=a.style,d=c.display,f=c.visibility,g=c.position;c.visibility=Te;c.position=Ue;c.display=""inline"";if(b){b=a.offsetWidth||a.clientWidth;a=a.offsetHeight||a.clientHeight}else{b=a.offsetWidth;a=a.offsetHeight}c.display=d;c.position=g;c.visibility=f;return new E(b,a)}
var We={};function Oe(a){return We[a]||(We[a]=String(a).replace(/\-([a-z])/g,function(b,c){return c.toUpperCase()}))}var Xe={};function Ye(a,b){var c=a.style;if(""opacity""in c)c.opacity=b;else if(""MozOpacity""in c)c.MozOpacity=b;else if(""filter""in c)c.filter=b===""""?"""":""alpha(opacity=""+b*100+"")""}function Ze(a,b){a.style.display=b?"""":Se}function $e(a,b){if(F)a.cssText=b;else a[sb?""innerText"":""innerHTML""]=b}var af=G?""MozUserSelect"":sb?""WebkitUserSelect"":j;
function bf(a){var b={};Fa(a.split(/\s*;\s*/),function(c){c=c.split(/\s*:\s*/);if(c.length==2)b[Oe(c[0].toLowerCase())]=c[1]});return b}function cf(a){var b=[];Ya(a,function(c,d){b.push(Xe[d]||(Xe[d]=d.replace(/([A-Z])/g,""-$1"").toLowerCase()),"":"",c,"";"")});return b.join("""")};function df(){}(function(a){a.Yd=function(){return a.Qe||(a.Qe=new a)}})(df);df.prototype.a=0;df.Yd();function ef(a){this.L=a||Ua||(Ua=new Xb);this.df=ff}B(ef,Uc);ef.prototype.fa=df.Yd();var ff=j;s=ef.prototype;s.R=j;s.p=l;s.n=j;s.df=j;s.Ue=j;s.V=j;s.t=j;s.pa=j;s.pf=l;s.z=function(){return this.R||(this.R="":""+(this.fa.a++).toString(36))};function gf(a,b){if(a.V&&a.V.pa){bb(a.V.pa,a.R);cb(a.V.pa,b,a)}a.R=b}s.c=o(""n"");s.Da=function(){return this.i||(this.i=new we(this))};
function hf(a,b){if(a==b)e(Error(""Unable to set parent component""));if(b&&a.V&&a.R&&je(a.V,a.R)&&a.V!=b)e(Error(""Unable to set parent component""));a.V=b;ef.e.Kc.call(a,b)}s.K=o(""V"");s.Kc=function(a){if(this.V&&this.V!=a)e(Error(""Method not supported""));ef.e.Kc.call(this,a)};s.u=function(){this.n=this.L.createElement(""div"")};s.Bb=function(a){jf(this,a)};
function jf(a,b,c){if(a.p)e(Error(""Component already rendered""));a.n||a.u();b?b.insertBefore(a.n,c||j):a.L.a.body.appendChild(a.n);if(!a.V||a.V.p)a.H()}s.H=function(){this.p=i;this.X(function(a){!a.p&&a.c()&&a.H()})};s.Ca=function(){this.X(function(a){a.p&&a.Ca()});this.i&&Ae(this.i);this.p=l};s.o=function(){ef.e.o.call(this);this.p&&this.Ca();if(this.i){this.i.ea();delete this.i}this.X(function(a){a.ea()});!this.pf&&this.n&&Rb(this.n);this.V=this.Ue=this.n=this.pa=this.t=j};
function kf(a,b){if(!a.p)e(Error(""Operation not supported while component is not in document""));return a.L.c(a.z()+"".""+b)}s.Pd=function(a,b){this.Ua(a,ke(this),b)};
s.Ua=function(a,b,c){if(a.p&&(c||!this.p))e(Error(""Component already rendered""));if(b<0||b>ke(this))e(Error(""Child component index out of bounds""));if(!this.pa||!this.t){this.pa={};this.t=[]}if(a.K()==this){this.pa[a.z()]=a;Ka(this.t,a)}else cb(this.pa,a.z(),a);hf(a,this);Oa(this.t,b,0,a);if(a.p&&this.p&&a.K()==this){c=this.Jb();c.insertBefore(a.c(),c.childNodes[b]||j)}else if(c){this.n||this.u();b=O(this,b+1);jf(a,this.Jb(),b?b.n:j)}else this.p&&!a.p&&a.n&&a.H()};s.Jb=o(""n"");
function ke(a){return a.t?a.t.length:0}function je(a,b){return a.pa&&b?db(a.pa,b)||j:j}function O(a,b){return a.t?a.t[b]||j:j}s.X=function(a,b){this.t&&Fa(this.t,a,b)};function lf(a,b){return a.t&&b?D(a.t,b):-1}s.removeChild=function(a,b){if(a){var c=z(a)?a:a.z();a=je(this,c);if(c&&a){bb(this.pa,c);Ka(this.t,a);if(b){a.Ca();a.n&&Rb(a.n)}hf(a,j)}}if(!a)e(Error(""Child is not in parent component""));return a};var mf=j;function nf(a,b){this.i=a;var c=z(b);this.f=c?j:b;this.a=c?b:j;this.b=[]}B(nf,$b);nf.prototype.o=function(){delete this.i;delete this.f;delete this.a;delete this.b};function of(a){this.a=a}B(of,$b);of.prototype.addRule=function(a){var b=this.a.rules||this.a.cssRules,c=b.length;b=a.a;if(!b)a.a=b=cf(a.f);var d=a.i;b=b;if(F){d=d.split("","");for(var f=0,g=d.length;f<g;f++)this.a.addRule(d[f],b)}else this.a.insertRule(d+"" {""+b+""}"",(this.a.rules||this.a.cssRules).length);b=this.a.rules||this.a.cssRules;c=c;for(d=b.length;c<d;c++)a.b.push(b[c])};of.prototype.o=function(){delete this.a};function pf(a){this.ka=a||[]}pf.prototype.B=function(){var a=oa(this.ka);return new pf(a)};function qf(a,b,c,d){J.call(this,b,a);this.name=c;this.a=d}B(qf,J);function rf(a,b){this.ga=a;if(b){this.fa=b.defaultValue;this.i=b.r;this.a=b.action;this.f=b.pc;this.b=b.Ae;this.j=b.zb;this.v=b.yd;this.Za=b.$d;this.Ve=b.B===l;this.Fa=b.Vd}}rf.prototype.getName=o(""ga"");rf.prototype.g=function(a){return a.hasAttribute(this)?this.Za?this.Za.call(a):a.getAttribute(this.ga):this.fa};
rf.prototype.D=function(a,b,c){c=c&&!(this.b&&this.b.call(a,b));b=this.j?this.j.call(a,b):b;var d=this.g(a);if(this.f?this.f(d,b):d==b)return l;a.Va[this.ga]=b;this.a&&this.a.call(a,b,d,c);this.i&&a.dispatchEvent(new qf(a,this.i,this.ga,!!c));c||P(a,this.ga,this.v?this.v.call(a,b):b);return i};function sf(a){if(y(a))return new Ke(a[0],a[1],a[2],a[3]);else if(a)return new Ke(a.left,a.top,a.width,a.height);return j}function tf(a){return a&&[a.left,a.top,a.width,a.height]};function Q(a){ef.call(this,a);this.Gd={};this.Va={}}B(Q,ef);s=Q.prototype;s.B=function(a){var b=new this.constructor;P(b,""action"",""create"");P(b,""index"",-1);gf(b,a?this.z():uf());b.M=this.M;for(var c in this.Gd)vf(b,c,this.Va[c]);return b};s.oe=0;s.J=j;s.pb=l;s.Sb=j;s.Ka=j;s.ha=0;s.Na=o(""pb"");s.W=function(a){if(this.pb&&!a||!this.pb&&a){if(!this.p)e(Error(""Operation not supported while component is not in document""));this.X(function(b){b.W(a)});this.pb=a}};
s.Da=function(){return this.wa||(this.wa=new we(this))};s.Tb=function(a,b,c){if(b!=lf(this,a)){var d=a.Na();d&&a.W(l);a.Yc();this.removeChild(a,i);this.Ua(a,b,i);d&&a.W(i);a.Uc();c||P(a,""index"",-1);return i}return l};s.Yc=v;s.Uc=v;s.bb=function(a,b,c,d){this.Ua(a,b);c||a.J&&!ab(a.J)&&wf(this,a);this.p&&xf(a,O(this,b+1));d||a.W(this.Na());(b=this.ha)&&a.Ra(b)};
function xf(a,b){if(!a.p){var c=a.K();if(b&&b.c()){c=b.c();jf(a,c.parentNode,c)}else a.Bb(c?c.Jb():j)}c=0;for(var d=ke(a);c<d;c++)xf(O(a,c),O(a,c+1))}s.Aa=function(a,b){var c=a.J?db(a.J,""action""):j;c!=""delete""&&yf(this,a);!b&&c!=""create""&&P(a,""action"",""delete"");this.removeChild(a.z());a.ea()};
s.update=function(a,b){if(a){var c=a.children;if(c!=j){var d={};Ya(c,function(n,p){var q=n.action;if(q==""delete"")this.Aa(je(this,p),b);else{var r=je(this,p),u=n.index;if(u!=j){if(q==""create"")r=this.wb(p,n.attributes,b);d[u]=r}r.update(n,b)}},this);c=Ha($a(d),Number);C.sort.call(c,Sa);for(var f=0;f<c.length;f++){for(var g=c[f],h=0,k=f+1;k<c.length;k++){var m=lf(this,d[c[k]]);0<=m&&m<g&&h++}k=d[g];je(this,k.z())?this.Tb(k,g+h,b):this.bb(k,g+h,b)}}(c=a.references)&&this.of(c);(c=a.attributes)&&zf(this,
c,b)}};s.of=v;function zf(a,b,c){for(var d in b)a.cc(d,b[d],c)}s.fd=Ce;s.cc=function(a,b,c){this.fd(a,b,c)||vf(this,a,b,c)};s.toString=function(){return(new sd(""<"")).append(' id=""').append(this.z()).append('"" revision=""').append(this.oe).append('""/>').toString()};s.u=function(a){this.n=this.L.u(N,{className:a||"""",id:this.z()})};s.Ca=function(){this.W(l);Q.e.Ca.call(this);this.Ka=j;Ae(this.Da())};Ud(Q,{wa:1,Va:0,Gd:0});
function P(a,b,c){if(!a.J)a.J={};switch(b){case ""children"":case ""action"":case ""index"":a.J[b]=c;break;default:var d=a.J.attributes;d||(d=a.J.attributes={});d[b]=c}""action""in a.J||(a.J.action=""update"");(b=a.K())&&wf(b,a)}function wf(a,b){var c=a.J?db(a.J,""children""):j;if(!c){c={};P(a,""children"",c)}var d=b.z(),f=b.J||(b.J={});c[d]=f}
function yf(a,b){var c=l,d=a.J?db(a.J,""children""):j;if(d&&b){c=bb(d,b.z());if(ab(d))if(a.J&&!ab(a.J)){switch(""children""){case ""children"":case ""action"":case ""index"":delete a.J.children;break;default:if(d=a.J.attributes){delete d.children;ab(d)&&delete a.J.attributes}}if(!a.J||ab(a.J))(d=a.K())&&yf(d,a)}}return c}
Q.prototype.jc=function(a){var b=new Ke(0,0,100,100),c=l;this.X(function(d){if(!c){if(!d.Ka||a!=d.Sb){var f=a||d.Sb;if(f){d.Ka=d.jc(f);d.Sb=d.Ka&&isNaN(d.Ka.width)?j:f}}if(d=d.Ka){f=Math.max(b.left+b.width,d.left+d.width);var g=Math.max(b.top+b.height,d.top+d.height);b.left=Math.min(b.left,d.left);b.top=Math.min(b.top,d.top);b.width=f-b.left;b.height=g-b.top}else c=i}});return c?j:b};function Af(a){if(a.Sb){var b=a.jc(a.Sb);if(!Le(b,a.Ka)){a.Ka=b;a.dispatchEvent(a.pd())}}}
Q.prototype.Ra=function(a){if(a!=this.ha){this.ha==0&&this.Dc();this.ha=a;this.Id()}};Q.prototype.Dc=v;Q.prototype.Id=function(){this.X(function(a){a.Ra(this.ha)},this)};var Bf=0;function uf(){return""null_""+Bf++}function vf(a,b,c,d){if(a.Va[b]==c)return l;a.Gd[b]=a.Va[b]=c;d||P(a,b,c);return i}Q.prototype.hasAttribute=function(a){return a.getName()in this.Va};Q.prototype.getAttribute=function(a){return this.Va[a]};Q.prototype.mb=v;
function R(a,b,c){if(!a.jb){a.jb={};a.prototype.B=function(d){var f=a.e.B.call(this,d),g;for(g in a.jb){var h=a.jb[g];!h.Ve&&h.D(f,h.g(this),l)}this.mb(f,d);return f};a.prototype.cc=function(d,f,g){var h=a.jb[d];h?h.D(this,f,g):a.e.cc.call(this,d,f,g)}}a.jb[b]=new rf(b,c);return a.jb[b]};function Cf(a,b,c){J.call(this,b,a);this.data=c}B(Cf,J);function Df(a,b){if(b.width){var c=b.width;a.style.width=(c>0?c:0)+""px""}if(b.height){c=b.height;a.style.height=(c>0?c:0)+""px""}}function Ef(a){this.n=a?H(a):document.body;this.Td=[];this.he=[];for(a=this.n.firstChild;a;){if(a.nodeType==1&&a.tagName!=""SCRIPT"")if(!this.Wc&&D(Va(a),""autolayout"")>=0)this.Wc=a;else this.he.push(a);a=a.nextSibling}}B(Ef,$b);Ud(Ef,{n:0,Td:0,he:0,Wc:0});Ef.prototype.update=function(){Ze(this.Wc,l);Fa(this.Td,function(a){a.update()})};function Ff(a,b){this.j=b||j;this.b=a;this.a=this.F=j}B(Ff,$b);Ff.prototype.v=o(""j"");Ff.prototype.je=ba(""j"");Ff.prototype.Ob=o(""F"");Ud(Ff,{F:0});function Gf(a){Q.call(this,a)}B(Gf,Q);var Hf=R(Gf,""bounds"",{defaultValue:j,r:""M"",action:function(){this.Eb()},pc:Le,zb:sf,yd:tf});F&&Gb(7);s=Gf.prototype;s.cd="""";s.contentType=""unknown"";s.lb=j;s.Gc=j;s.ge=l;s.H=function(){Gf.e.H.call(this);this.lb&&If(this)};function Jf(a){return(a=Hf.g(a))&&a.B()}s.Wb=function(a,b){return Hf.D(this,a,b)};function If(a){a.p&&a.lb&&a.c().appendChild(a.lb.c())}function Kf(a){if(a.p&&a.lb&&!a.lb.gd){var b=a.lb.c();b.parentNode==a.c()&&a.c().removeChild(b)}}
function Lf(a){if(a.Gc)for(var b=a.Gc.sa,c=0,d=b.length;c<d;c++){var f=b[c];f.parentNode==a.c()&&a.c().removeChild(f)}}function Mf(a){if(a.Gc)for(var b=a.Gc.sa,c=0,d=b.length;c<d;c++)a.c().appendChild(b[c])}s.ib=function(){this.za();this.dispatchEvent(""M"");Af(this)};s.Eb=function(){this.za();Af(this);this.p&&this.xb()};
s.za=function(){if(this.hasAttribute(Hf)){var a=this.Pb();a=new sd("";left: "",a.left,""%; top: "",a.top,""%; width: "",a.width,""%; height: "",Rd?a.height*0.225+""em"":a.height+""%"");if(mf==j)mf=""rtl""==(Pe(document.body,""direction"")||(document.body.currentStyle?document.body.currentStyle.direction:j)||document.body.style.direction);mf&&a.append(""; right: auto"");this.cd=a.toString()}else this.cd="""";if(a=this.c())a.style.cssText+=this.cd};
s.W=function(a){if(this.pb&&!a||!this.pb&&a){Gf.e.W.call(this,a);var b=this.Da();if(a){ye(b,this.c(),[""mousedown"",""click"",""mouseup""],this.ta,i);sb&&ye(b,this.c(),""contextmenu"",this.ta,i);if(F&&!this.ze()&&!this.ge){var c=this.c();a=c.getElementsByTagName(""*"");if(af){b=Se;c.style[af]=b;if(a){c=0;for(var d;d=a[c];c++)d.style[af]=b}}else if(F||rb){b=""on"";c.setAttribute(""unselectable"",b);if(a)for(c=0;d=a[c];c++)d.setAttribute(""unselectable"",b)}this.ge=i}}else{ze(b,this.c(),[""mousedown"",""click"",""mouseup""],
this.ta,i);sb&&ze(b,this.c(),""contextmenu"",this.ta,i)}}Nf(this)};s.u=function(){Gf.e.u.call(this,this.od());this.za()};s.Pb=function(){var a=Jf(this);if(a)return a;a={};var b=this.c();if(b){var c=Of(this,b.offsetLeft,b.offsetTop),d=b.style[Oe(""padding"")];Me(b,""padding"",""0"");var f=Ve(b);Me(b,""padding"",d);b=Of(this,f.width,f.height);a.left=c[0]+""%"";a.top=c[1]+""%"";a.width=b[0]+""%"";a.height=b[1]+""%""}return new Ke(parseFloat(a.left),parseFloat(a.top),parseFloat(a.width),parseFloat(a.height))};
s.T=function(a,b){var c=this.K(),d=j;if(c){c=c.c();if(a&&b){c=Ve(c);d=new E(a*c.width/100,b*c.height/100)}else d=Ve(this.c())}else d=new E(0,0);return d};s.xb=aa();s.jc=function(a){return a&&this.p&&this.c()?this.Pb():j};s.pd=ca(""O"");s.od=function(){return""placeholder ""+this.contentType};function Of(a,b,c){a=Ve(a.c().parentNode);return[parseFloat(b)/a.width*100,parseFloat(c)/a.height*100]}
s.ta=function(a){var b;switch(a.type){case ""mousedown"":case ""contextmenu"":b=""P"";break;case ""click"":b=""R"";break;case ""mouseup"":b=""Q"";break;default:e(Error(""Shape.onMouseEvent: Invalid event type""))}this.dispatchEvent({sf:a,type:b})};s.ze=Ce;s.Rb=Ce;s.wc=Ce;
function Nf(a){var b=a.z(),c=H(""placeholder_target_""+b);if(a.Rb()&&a.Na()){var d=H(""placeholder_""+b);if(!c){c=I(N,{id:""placeholder_arrow_""+b,src:""presently/images/arrow_overlay.png"",""class"":""editor-icon-sprite placeholder-arrow""});d=I(N,{id:""placeholder_""+b,""class"":""editor-icon-sprite placeholder-icon-inactive""},c);c=I(N,{id:""placeholder_target_""+b,""class"":""placeholder-target placeholder-transparent""},d,c);Ze(c,a.wc());L(c,""click"",A(a.We,a));a.c().appendChild(c);yc(a.c(),""mouseover"",A(a.de,a,i))}if(a=
a.v){b=a.width/2;c.style.top=a.height/2-20.5+""px"";c.style.left=b-20.5+""px""}}else c&&Rb(c)}s.de=function(a,b){var c=this.z(),d=H(""placeholder_""+c);if(d){a?Wa(d,""placeholder-icon""):Xa(d,""placeholder-icon"");!a?Wa(d,""placeholder-icon-inactive""):Xa(d,""placeholder-icon-inactive"");c=H(""placeholder_target_""+c);!(D(Va(b.target),""editor-icon-sprite"")>=0)&&a?Wa(c,""placeholder-transparent""):Xa(c,""placeholder-transparent"");this.wc()||Ze(c,a)}yc(this.c(),a?""mouseout"":""mouseover"",A(this.de,this,!a))};s.We=function(){this.dispatchEvent(new Pf)};
function Pf(){J.call(this,""CLICK"")}B(Pf,J);function Qf(a,b,c){this.R=a;this.ga=b;this.oc=c}Qf.prototype.Hb=function(a){return this===a||!!a&&this.R==a.R&&this.ga==a.ga&&this.oc==a.oc};Qf.prototype.toString=function(){return(new sd('<User id=""')).append(this.R).append('"" name=""').append(this.ga).append('"" email=""').append(this.oc).append('""/>').toString()};Qf.prototype.z=o(""R"");Qf.prototype.getName=o(""ga"");function S(a,b){Q.call(this);this.se=a;this.kc=[];this.lc=[];this.M=b||new Ff(l);this.M.F=this;this.bc={};this.nb=new Ee;this.nb.Kc(this)}B(S,Q);R(S,""templateId"",{defaultValue:""""});
var Sf=R(S,""theme"",{action:function(){if(this.p){var a;if(this.Z&&G){a=this.Z.Na();this.Z.W(l)}var b=this.c(),c=Ga(Va(b),Rf);pa(Xa,b).apply(j,c);Wa(b,""goog-presently-theme-""+Sf.g(this));c=j;if(F)(c=kf(this,""ie-hack""))&&Rb(c);var d=Tf(this);d&&d.parentNode.removeChild(d);if(this.se){d=Sf.g(this);var f=j;if(d in this.se.a&&H(d)){f=H(d).cloneNode(i);f.id=j}d=f}else d=j;if(d=d){this.ma&&Df(d,this.ma);Wa(d,""goog-presently-background"");b.parentNode.insertBefore(d,b)}c!=j&&Uf(this,c);this.Z&&G&&this.Z.W(a)}},
r:""p""});s=S.prototype;s.Z=j;s.ma=j;s.ff=j;s.yc=j;s.gc=j;s.Wd="""";s.qe=l;s.Ea=function(){return this.nb.Ea()};s.Pe=function(a){a=a.target;if(this.yc&&lf(this,a)==0){a=ra(a.Ea()).replace(/\s+/g,"" "");a.length&&this.$b(a,l,i)}};s.$b=function(a,b,c){if(a!=this.Ea()){if((this.yc=this.yc!=l&&(""Untitled Presentation""==a&&b||!!c))&&!c)if((c=O(this,0))&&!c.Ea()||!c)ye(this.Da(),this,""B"",this.Pe);this.nb.$b(a);b||P(this,""title"",this.Ea());return i}return l};
function Rf(a){return a.lastIndexOf(""goog-presently-theme-"",0)==0}function Tf(a){if(a.p)if((a=a.c().parentNode.firstChild)&&D(Va(a),""goog-presently-background"")>=0)return a;return j}function Vf(a,b){if(a.Z){a.Z.W(l);Ze(a.Z.c(),l)}if(a.Z=b){var c=b.c();Wa(c,""goog-presently-foreground"");a.ma&&Df(c,a.ma);Ze(c,i);b.W(a.Na());F&&window.setTimeout(A(a.Qd,a),0);a.Id()}c=b?""inherit"":Te;a.c().style.visibility=c;var d=Tf(a);if(d)d.style.visibility=c;a.dispatchEvent(""l"")}
function Uf(a,b){var c=a.Z;if(c){c=(c=O(c,0))&&c.contentType==""background""?c.c():Tf(a);c!=j&&c.parentNode&&c.parentNode.insertBefore(b,c.nextSibling)}}s.Qd=function(){var a=kf(this,""ie-hack"");a&&Uf(this,a)};
s.Vc=function(a){if(this.p){this.ye();var b=this.c();this.ma=a;var c=Tf(this);c&&Df(c,a);Df(b,a);this.Z&&Df(this.Z.c(),a);if(Rd)b=H(""slideEditor"")||b;Ze(b,l);Me(b,""fontSize"",j);var d=Math.min(a.width/800,a.height/600),f=20*d+""pt"";this.gc&&window.clearTimeout(this.gc);this.gc=window.setTimeout(A(function(){this.gc=j;Me(b,""fontSize"",f);Ze(b,i);this.Ra(d);this.we();this.Hd()},this),1);F&&this.Qd()}};s.ye=v;s.we=v;s.pd=ca(""y"");s.Ye=function(){Af(this)};s.Id=function(){var a=this.Z;a&&a.Ra(this.ha)};
s.Hd=function(){var a=this.Z;a&&a.Hd()};s.qc=function(){return ke(this)};s.bb=function(a,b,c){S.e.bb.call(this,a,b,c,i);Ze(a.c(),l);this.dispatchEvent(new Cf(this,""u"",a.z()));Af(this)};s.Tb=function(a,b,c){(b=S.e.Tb.call(this,a,b,c))&&this.dispatchEvent(new Cf(this,""v"",a.z()));return b};s.Aa=function(a,b){if(!a)e(Error(""Attempted to delete a non-existent slide""));var c=a.z();this.dispatchEvent(new Cf(this,""w"",c));S.e.Aa.call(this,a,b);this.dispatchEvent(new Cf(this,""x"",c));Af(this)};
s.wb=function(a,b,c){b=new Wf.hf(void 0);gf(b,a||uf());b.M=this.M;if(!c){P(b,""action"",""create"");P(b,""index"",-1)}return b};s.H=function(){S.e.H.call(this);ye(this.Da(),this,""H"",this.Ye)};
s.fd=function(a,b,c){if(a.lastIndexOf(""styles."",0)==0){var d=a.split("".""),f=d[1];f=this.bc[f]=this.bc[f]||new pf;var g=Number(d[2]);d=d[3];f.ka[g]=f.ka[g]||{};f.ka[g][d]=b}switch(a){case ""revision"":if(b&&b!=this.ne){this.ne=b;b=b.split("":"");(c=b[0])&&gf(this,c);if(b=b[1])this.oe=b}break;case ""cancelRevision"":if(b&&b!=this.Sd)this.Sd=b;break;case ""title"":this.$b(b,c);break;case ""lastEdited"":case ""timestamp"":c=""newtimestamp"";a=0;if(c.charAt(0)==""!""){c=c.substring(1);a=1}if(a^(Qd[c]||window.location.search&&
window.location.search.match(c+""=true"")!=j?1:0)){d=b.indexOf("":"");c=this.nb;a=d==-1||d+1==b.length?j:b.substring(d+1);b=d==-1?parseInt(b):parseInt(b.substring(0,d));if(c.td!=a||c.vd!=b){c.td=a;c.vd=b;c.ud="""";c.dispatchEvent(new Ge(""e"",i))}}else{c=this.nb;if(c.ud!=b){c.ud=b;c.td=j;c.vd=j;c.dispatchEvent(new Ge(""e"",l))}}break;case ""shared"":b=b==""true"";c=this.qe!=b;this.qe=b;c&&this.dispatchEvent(""q"");break;case ""owner"":b=new Qf(b.id,b.name,b.email);if(!b.Hb(this.Ze)){this.Ze=b;this.dispatchEvent(""r"")}break;
case ""viewers"":if(b){if(this.Nd)Ja(this.Nd);else this.Nd=[];c=0;for(a=b.length;c<a;c++){d=b[c];this.Nd.push(new Qf(d.id,d.name,d.email))}this.dispatchEvent(""s"")}break;case ""writers"":if(b){if(this.Od)Ja(this.Od);else this.Od=[];c=0;for(a=b.length;c<a;c++){d=b[c];this.Od.push(new Qf(d.id,d.name,d.email))}this.dispatchEvent(""t"")}break;case ""shortId"":if(b)this.ff=b;break;case ""fromTemplateId"":if(b||/^[\s\xa0]*$/.test(b)){this.Wd=b;this.M.Wd=b}break;case ""customColors"":b=b?b.split("",""):j;if(!this.kc||
!Qa(b,this.kc,void 0)){this.kc=b||[];c?this.dispatchEvent(""z""):P(this,""customColors"",this.kc.join("",""))}break;case ""customFontSizes"":b=b?b.split("",""):j;if(!this.lc||!Qa(b,this.lc,void 0)){this.lc=b||[];c?this.dispatchEvent(""A""):P(this,""customFontSizes"",this.lc.join("",""))}break;default:return S.e.fd.call(this,a,b,c)}return i};Ud(S,{M:1,nb:1});S.prototype.rc=function(a){return this.bc[a]};
S.prototype.Ra=function(a){S.e.Ra.call(this,a);a=a*20;for(var b in this.bc){var c=this.bc[b];if(c.Rd){c=c;var d=a;if(c.ka.length>1)for(var f=1,g=c.ka.length;f<g;f++){var h=c.Rd[f-1],k=c.ka[f];if(k&&k.fontSize){h=h;k={fontSize:parseFloat(k.fontSize)*d+""pt""};var m=h.f;if(!m)h.f=m=bf(h.a);gb(m,k);h.a=j;m=0;for(var n=h.b.length;m<n;m++)gb(h.b[m].style,k)}}}else{c=c;d=b;f=a;g=[];if(c.ka.length>1){h=(h=void 0)||document;k=(k=h.body)?new Xb(Sb(k)):Ua||(Ua=new Xb);n=j;if(F){n=k.a.createStyleSheet();$e(n,
"""")}else{m=Jb(k.a,""head"",void 0,void 0)[0];if(!m){n=Jb(k.a,""body"",void 0,void 0)[0];m=k.u(""head"");n.parentNode.insertBefore(m,n)}n=k.u(""style"");$e(n,"""");k.appendChild(m,n)}h=new of(h.styleSheets[h.styleSheets.length-1]);k=1;for(m=c.ka.length;k<m;k++)if((n=c.ka[k])&&!ab(n)){var p;p=k;for(var q=d,r=[],u=0,w=Math.pow(2,p);u<w;u++){var Aa=u.toString(2);Aa=Array(p-Aa.length+1).join(""0"")+Aa;r.push("".""+q+Aa.replace(/0/g,"" ul"").replace(/1/g,"" ol"")+"" li"")}p=r.join("","");q=eb(n);if(n.fontSize)q.fontSize=parseFloat(n.fontSize)*
f+""pt"";n=new nf(p,q);h.addRule(n);g[k-1]=n}}c.Rd=g}}};function Xf(){this.a={}}Xf.prototype.Ud=function(a,b){var c=this.a[a];return c?new c(b):j};var Yf={arrowE:{elements:[{type:""path"",data:""arrowE""}],width:110,height:110},arrowNE:{elements:[{type:""path"",data:""arrowNE""}],width:100,height:100},arrowN:{elements:[{type:""path"",data:""arrowN""}],width:110,height:110},speech:{elements:[{type:""path"",data:""speech""}],width:200,height:150},starburst:{elements:[{type:""path"",data:""starburst""}],width:200,height:200},ellipse:{elements:[{type:""ellipse"",data:[10800,10800,10800,10800]}],width:200,height:200},rectangle:{elements:[{type:""rect"",data:[21600,21600,
0,0]}],width:200,height:200}};function Zf(a){return a*Math.PI/180};function T(){this.N=[];this.k=[];this.ba=[]}T.prototype.ia=j;T.prototype.O=j;T.prototype.la=i;var $f=[];$f[0]=2;$f[1]=2;$f[2]=6;$f[3]=6;$f[4]=0;s=T.prototype;s.clear=function(){this.N.length=0;this.k.length=0;this.ba.length=0;delete this.ia;delete this.O;delete this.la;return this};s.moveTo=function(a,b){if(this.N[this.N.length-1]==0)this.ba.length-=2;else{this.N.push(0);this.k.push(1)}this.ba.push(a,b);this.O=this.ia=[a,b];return this};
s.lineTo=function(){var a=this.N[this.N.length-1];if(a==j)e(Error(""Path cannot start with lineTo""));if(a!=1){this.N.push(1);this.k.push(0)}for(a=0;a<arguments.length;a+=2){var b=arguments[a],c=arguments[a+1];this.ba.push(b,c)}this.k[this.k.length-1]+=a/2;this.O=[b,c];return this};
s.dd=function(){var a=this.N[this.N.length-1];if(a==j)e(Error(""Path cannot start with curve""));if(a!=2){this.N.push(2);this.k.push(0)}for(a=0;a<arguments.length;a+=6){var b=arguments[a+4],c=arguments[a+5];this.ba.push(arguments[a],arguments[a+1],arguments[a+2],arguments[a+3],b,c)}this.k[this.k.length-1]+=a/6;this.O=[b,c];return this};s.close=function(){var a=this.N[this.N.length-1];if(a==j)e(Error(""Path cannot start with close""));if(a!=4){this.N.push(4);this.k.push(1);this.O=this.ia}return this};
s.arcTo=function(a,b,c,d){var f=this.O[0]-a*Math.cos(Zf(c))+a*Math.cos(Zf(c+d)),g=this.O[1]-b*Math.sin(Zf(c))+b*Math.sin(Zf(c+d));this.N.push(3);this.k.push(1);this.ba.push(a,b,c,d,f,g);this.la=l;this.O=[f,g];return this};
s.xe=function(a,b,c,d){var f=this.O[0]-a*Math.cos(Zf(c)),g=this.O[1]-b*Math.sin(Zf(c)),h=Zf(d);d=Math.ceil(Math.abs(h)/Math.PI*2);h=h/d;c=Zf(c);for(var k=0;k<d;k++){var m=Math.cos(c),n=Math.sin(c),p=4/3*Math.sin(h/2)/(1+Math.cos(h/2)),q=f+(m-p*n)*a,r=g+(n+p*m)*b;c+=h;m=Math.cos(c);n=Math.sin(c);this.dd(q,r,f+(m+p*n)*a,g+(n-p*m)*b,f+m*a,g+n*b)}return this};function ag(a,b){for(var c=a.ba,d=0,f=0,g=a.N.length;f<g;f++){var h=a.N[f],k=$f[h]*a.k[f];b(h,c.slice(d,d+k));d+=k}}
s.B=function(){var a=new this.constructor;a.N=this.N.concat();a.k=this.k.concat();a.ba=this.ba.concat();a.ia=this.ia&&this.ia.concat();a.O=this.O&&this.O.concat();a.la=this.la;return a};var bg={};bg[0]=T.prototype.moveTo;bg[1]=T.prototype.lineTo;bg[4]=T.prototype.close;bg[2]=T.prototype.dd;bg[3]=T.prototype.xe;function cg(a){if(a.la)return a.B();var b=new T;ag(a,function(c,d){bg[c].apply(b,d)});return b}function dg(a,b){var c=cg(a);c.transform(b);return c}
T.prototype.transform=function(a){if(!this.la)e(Error(""Non-simple path""));a.transform(this.ba,0,this.ba,0,this.ba.length/2);this.ia&&a.transform(this.ia,0,this.ia,0,1);this.O&&this.ia!=this.O&&a.transform(this.O,0,this.O,0,1);return this};T.prototype.xc=function(){return this.N.length==0};function eg(a,b,c,d,f){ef.call(this,f);this.width=a;this.height=b;this.a=c||j;this.f=d||j}B(eg,ef);s=eg.prototype;s.G=j;s.ja=0;s.qa=0;s.Ga=function(a,b){this.a=a;this.f=b};s.Xa=function(){return this.a?new E(this.a,this.f):this.T()};s.Ya=function(){return this.T()};s.T=function(){if(this.p)return Ve(this.c());if(ia(this.width)&&ia(this.height))return new E(this.width,this.height);return j};s.La=function(){var a=this.T();return a?a.width/this.Xa().width:0};
s.Mb=function(){var a=this.T();return a?a.height/this.Xa().height:0};function fg(a,b,c,d,f,g){if(arguments.length==6)this.setTransform(a,b,c,d,f,g);else if(arguments.length!=0)e(Error(""Insufficient matrix parameters""));else{this.b=this.i=1;this.a=this.f=this.j=this.v=0}}s=fg.prototype;s.B=function(){return new fg(this.b,this.a,this.f,this.i,this.j,this.v)};s.setTransform=function(a,b,c,d,f,g){if(!ia(a)||!ia(b)||!ia(c)||!ia(d)||!ia(f)||!ia(g))e(Error(""Invalid transform parameters""));this.b=a;this.a=b;this.f=c;this.i=d;this.j=f;this.v=g;return this};
s.scale=function(a,b){this.b*=a;this.a*=a;this.f*=b;this.i*=b;return this};s.translate=function(a,b){this.j+=a*this.b+b*this.f;this.v+=a*this.a+b*this.i;return this};s.rotate=function(a,b,c){a=gg(a,b,c);b=this.b;c=this.f;this.b=a.b*b+a.a*c;this.f=a.f*b+a.i*c;this.j+=a.j*b+a.v*c;b=this.a;c=this.i;this.a=a.b*b+a.a*c;this.i=a.f*b+a.i*c;this.v+=a.j*b+a.v*c;return this};s.toString=function(){return""matrix(""+[this.b,this.a,this.f,this.i,this.j,this.v].join("","")+"")""};
s.transform=function(a,b,c,d,f){var g=b;d=d;for(b=b+2*f;g<b;){f=a[g++];var h=a[g++];c[d++]=f*this.b+h*this.f+this.j;c[d++]=f*this.a+h*this.i+this.v}};function gg(a,b,c){var d=new fg,f=Math.cos(a);a=Math.sin(a);return d.setTransform(f,a,-a,f,b-b*f+c*a,c-b*a-c*f)}function hg(a,b,c){return a.setTransform(b,0,0,c,0,0)}s.Hb=function(a){if(this==a)return i;if(!a)return l;return this.b==a.b&&this.f==a.f&&this.j==a.j&&this.a==a.a&&this.i==a.i&&this.v==a.v};function U(a,b){this.n=a;this.Ma=b;this.ed=l}B(U,Uc);s=U.prototype;s.Ma=j;s.n=j;s.Fd=j;s.c=o(""n"");s.C=o(""Ma"");s.addEventListener=function(a,b,c,d){L(this.n,a,b,c,d)};s.removeEventListener=function(a,b,c,d){zc(this.n,a,b,c,d)};s.o=function(){U.e.o.call(this);Dc(this.n)};function V(a,b,c,d){U.call(this,a,b);this.fb(c);this.Yb(d)}B(V,U);s=V.prototype;s.fill=j;s.ac=j;s.Yb=function(a){this.fill=a;this.C().zd(this,a)};s.Ee=o(""fill"");s.fb=function(a){this.ac=a;this.C().Ad(this,a)};s.Ie=o(""ac"");function ig(a,b,c,d){V.call(this,a,b,c,d)}B(ig,V);function jg(a,b){U.call(this,a,b)}B(jg,U);function kg(a,b){U.call(this,a,b)}B(kg,U);function lg(a,b,c,d){V.call(this,a,b,c,d)}B(lg,V);function mg(a,b,c,d){V.call(this,a,b,c,d)}B(mg,V);function ng(a){U.call(this,j,a);this.t=[]}B(ng,jg);ng.prototype.clear=function(){if(this.t.length){this.t.length=0;this.C().q()}};ng.prototype.S=aa();ng.prototype.appendChild=function(a){this.t.push(a)};ng.prototype.P=function(){for(var a=0,b=this.t.length;a<b;a++)og(this.C(),this.t[a])};function pg(a,b,c,d,f,g,h,k){V.call(this,a,b,h,k);this.j=c;this.fa=d;this.f=f;this.i=g;this.Q=new T;qg(this);this.a=new rg(j,b,this.Q,h,k)}B(pg,ig);
function qg(a){a.Q.clear();a.Q.moveTo(a.j+a.f*Math.cos(Zf(0)),a.fa+a.i*Math.sin(Zf(0)));a.Q.arcTo(a.f,a.i,0,360);a.Q.close()}pg.prototype.Ic=function(a,b){this.j=a;this.fa=b;qg(this);this.a.aa(this.Q)};pg.prototype.v=function(a,b){this.f=a;this.i=b;qg(this);this.a.aa(this.Q)};pg.prototype.P=function(a){this.a.P(a)};function sg(a,b,c,d,f,g,h,k){V.call(this,a,b,h,k);this.a=c;this.f=d;this.j=f;this.i=g}B(sg,mg);sg.prototype.Pa=function(a,b){this.a=a;this.f=b;this.ra&&this.C().q()};sg.prototype.ra=l;
sg.prototype.S=function(a,b){this.j=a;this.i=b;this.ra&&this.C().q()};sg.prototype.P=function(a){this.ra=i;a.beginPath();a.moveTo(this.a,this.f);a.lineTo(this.a,this.f+this.i);a.lineTo(this.a+this.j,this.f+this.i);a.lineTo(this.a+this.j,this.f);a.closePath()};function rg(a,b,c,d,f){V.call(this,a,b,d,f);this.aa(c)}B(rg,lg);rg.prototype.ra=l;rg.prototype.aa=function(a){this.Q=a.la?a:cg(a);this.ra&&this.C().q()};
rg.prototype.P=function(a){this.ra=i;a.beginPath();ag(this.Q,function(b,c){switch(b){case 0:a.moveTo(c[0],c[1]);break;case 1:for(var d=0;d<c.length;d+=2)a.lineTo(c[d],c[d+1]);break;case 2:for(d=0;d<c.length;d+=6)a.bezierCurveTo(c[d],c[d+1],c[d+2],c[d+3],c[d+4],c[d+5]);break;case 3:e(Error(""Canvas paths cannot contain arcs""));case 4:a.closePath()}})};function tg(a,b,c,d,f,g,h){U.call(this,a,b);this.a=c;this.f=d;this.j=f;this.i=g;this.Fa=h}B(tg,kg);s=tg.prototype;s.ra=l;
s.Pa=function(a,b){this.a=a;this.f=b;this.ra&&this.C().q()};s.S=function(a,b){this.j=a;this.i=b;this.ra&&this.C().q()};s.Lc=function(a){this.Fa=a;this.ra&&this.C().q()};s.P=function(a){if(this.fa){this.j&&this.i&&a.drawImage(this.fa,this.a,this.f,this.j,this.i);this.ra=i}else{a=new Image;a.onload=A(this.Le,this,a);a.src=this.Fa}};s.Le=function(a){this.fa=a;this.C().q()};function ug(){};function vg(a,b){this.b=a;this.a=b||1}B(vg,ug);vg.prototype.va=o(""b"");function wg(a,b){this.a=a;this.b=b}wg.prototype.U=o(""a"");wg.prototype.va=o(""b"");function xg(a,b,c,d,f){eg.call(this,a,b,c,d,f)}B(xg,eg);s=xg.prototype;s.zd=function(){this.q()};s.Ad=function(){this.q()};s.Bd=function(){this.q()};function yg(a,b){var c=a.getContext();c.save();var d=b.Fd?b.Fd.B():new fg,f=d.j,g=d.v;if(f||g)c.translate(f,g);(d=d.a)&&c.rotate(Math.asin(d))}s.u=function(){var a=this.L.u(""div"",{style:""position:relative;overflow:hidden""});this.n=a;this.j=this.L.u(""canvas"");a.appendChild(this.j);this.v=this.G=new ng(this);this.Fa=0;this.ib()};
s.getContext=function(){this.c()||this.u();if(!this.b){this.b=this.j.getContext(""2d"");this.b.save()}return this.b};s.Xb=function(a,b){this.ja=a;this.qa=b;this.q()};s.Ga=function(){xg.e.Ga.apply(this,arguments);this.q()};s.S=function(a,b){this.width=a;this.height=b;this.ib();this.q()};
s.T=function(){var a=this.width,b=this.height,c=z(a)&&a.indexOf(""%"")!=-1,d=z(b)&&b.indexOf(""%"")!=-1;if(!this.p&&(c||d))return j;var f,g;if(c){f=this.c().parentNode;g=Ve(f);a=parseFloat(a)*g.width/100}if(d){f=f||this.c().parentNode;g=g||Ve(f);b=parseFloat(b)*g.height/100}return new E(a,b)};s.ib=function(){Qe(this.c(),this.width,this.height);var a=this.T();if(a){Qe(this.j,a.width,a.height);this.j.width=a.width;this.j.height=a.height;this.b=j}};
s.reset=function(){var a=this.getContext();a.restore();var b=this.T();b.width&&b.height&&a.clearRect(0,0,b.width,b.height);a.save()};s.clear=function(){this.reset();this.G.clear();for(var a=this.c();a.childNodes.length>1;)a.removeChild(a.lastChild)};s.q=function(){if(this.wd)this.zc=i;else if(this.p){this.reset();if(this.a){var a=this.T();this.getContext().scale(a.width/this.a,a.height/this.f)}if(this.ja||this.qa)this.getContext().translate(-this.ja,-this.qa);yg(this,this.G);this.G.P(this.b);this.getContext().restore()}};
function og(a,b){var c=a.getContext();yg(a,b);if(!b.Ee||!b.Ie)b.P(c);else{var d=b.fill;if(d)if(d instanceof vg){if(d.a!=0){c.globalAlpha=d.a;c.fillStyle=d.va();b.P(c);c.fill();c.globalAlpha=1}}else{var f=c.createLinearGradient(d.vf(),d.xf(),d.wf(),d.yf());f.addColorStop(0,d.tf());f.addColorStop(1,d.uf());c.fillStyle=f;b.P(c);c.fill()}if(d=b.ac){b.P(c);c.strokeStyle=d.va();d=d.U();if(z(d)&&d.indexOf(""px"")!=-1)d=parseFloat(d)/a.La();c.lineWidth=d;c.stroke()}}a.getContext().restore()}
s.Y=function(a,b){b=b||this.G;b.appendChild(a);this.p&&!this.Fa&&!(b!=this.G&&b!=this.v)&&og(this,a)};s.hd=function(a,b,c,d,f,g,h){a=new pg(j,this,a,b,c,d,f,g);this.Y(a,h);return a};s.ld=function(a,b,c,d,f,g,h){a=new sg(j,this,a,b,c,d,f,g);this.Y(a,h);return a};s.drawImage=function(a,b,c,d,f,g){a=new tg(j,this,a,b,c,d,f);this.Y(a,g);return a};s.kd=function(a,b,c,d){a=new rg(j,this,a,b,c);this.Y(a,d);return a};
s.bd=function(a){var b=new ng(this);a=a||this.G;if(a==this.G||a==this.v)this.v=b;this.Y(b,a);return b};s.o=function(){this.b=j;xg.e.o.call(this)};s.H=function(){var a=this.T();xg.e.H.call(this);if(!a){this.ib();this.dispatchEvent(""resize"")}this.q()};function zg(a,b){U.call(this,a,b)}B(zg,jg);zg.prototype.clear=function(){Qb(this.c())};zg.prototype.S=function(a,b){Ag(this.C(),this.c(),{width:a,height:b})};function Bg(a,b,c,d){V.call(this,a,b,c,d)}B(Bg,ig);Bg.prototype.Ic=function(a,b){Ag(this.C(),this.c(),{cx:a,cy:b})};Bg.prototype.v=function(a,b){Ag(this.C(),this.c(),{rx:a,ry:b})};function Cg(a,b,c,d){V.call(this,a,b,c,d)}B(Cg,mg);Cg.prototype.Pa=function(a,b){Ag(this.C(),this.c(),{x:a,y:b})};
Cg.prototype.S=function(a,b){Ag(this.C(),this.c(),{width:a,height:b})};function Dg(a,b,c,d){V.call(this,a,b,c,d)}B(Dg,lg);Dg.prototype.aa=function(a){Ag(this.C(),this.c(),{d:Eg(a)})};function Fg(a,b){U.call(this,a,b)}B(Fg,kg);Fg.prototype.Pa=function(a,b){Ag(this.C(),this.c(),{x:a,y:b})};Fg.prototype.S=function(a,b){Ag(this.C(),this.c(),{width:a,height:b})};Fg.prototype.Lc=function(a){Ag(this.C(),this.c(),{""xlink:href"":a})};function Gg(a,b,c,d,f){eg.call(this,a,b,c,d,f);this.v={};this.j=sb&&!Gb(526);this.wa=new we(this)}var Hg;B(Gg,eg);function Ig(a,b,c){b=a.L.a.createElementNS(""http://www.w3.org/2000/svg"",b);c&&Ag(a,b,c);return b}function Ag(a,b,c){for(var d in c)b.setAttribute(d,c[d])}s=Gg.prototype;s.Y=function(a,b){(b||this.G).c().appendChild(a.c())};s.zd=function(a,b){var c=a.c();if(b instanceof vg){c.setAttribute(""fill"",b.va());c.setAttribute(""fill-opacity"",b.a)}else c.setAttribute(""fill"",Se)};
s.Ad=function(a,b){var c=a.c();if(b){c.setAttribute(""stroke"",b.va());var d=b.U();z(d)&&d.indexOf(""px"")!=-1?c.setAttribute(""stroke-width"",parseFloat(d)/this.La()):c.setAttribute(""stroke-width"",d)}else c.setAttribute(""stroke"",Se)};s.Bd=function(a,b,c,d,f,g){a.c().setAttribute(""transform"",""translate(""+b+"",""+c+"") rotate(""+d+"" ""+f+"" ""+g+"")"")};
s.u=function(){var a=Ig(this,""svg"",{width:this.width,height:this.height,overflow:Te}),b=Ig(this,""g"");this.b=Ig(this,""defs"");this.G=new zg(b,this);a.appendChild(this.b);a.appendChild(b);this.n=a;Jg(this)};s.Xb=function(a,b){this.ja=a;this.qa=b;Jg(this)};s.Ga=function(){Gg.e.Ga.apply(this,arguments);Jg(this)};function Jg(a){if(a.a||a.ja||a.qa){a.c().setAttribute(""preserveAspectRatio"",Se);a.j?a.Rc():a.c().setAttribute(""viewBox"",a.ja+"" ""+a.qa+"" ""+(a.a?a.a+"" ""+a.f:""""))}}
s.Rc=function(){if(this.p&&(this.a||this.ja||!this.qa)){var a=this.T();if(a.width==0)this.c().style.visibility=Te;else{this.c().style.visibility="""";var b=-this.ja,c=-this.qa,d=a.width/this.a;a=a.height/this.f;this.G.c().setAttribute(""transform"",""scale(""+d+"" ""+a+"") translate(""+b+"" ""+c+"")"")}}};s.S=aa();
s.T=function(){if(!G)return this.p?Ve(this.c()):Gg.e.T.call(this);var a=this.width,b=this.height,c=z(a)&&a.indexOf(""%"")!=-1,d=z(b)&&b.indexOf(""%"")!=-1;if(!this.p&&(c||d))return j;var f,g;if(c){f=this.c().parentNode;g=Ve(f);a=parseFloat(a)*g.width/100}if(d){f=f||this.c().parentNode;g=g||Ve(f);b=parseFloat(b)*g.height/100}return new E(a,b)};s.clear=function(){this.G.clear();Qb(this.b);this.v={}};
s.hd=function(a,b,c,d,f,g,h){a=Ig(this,""ellipse"",{cx:a,cy:b,rx:c,ry:d});f=new Bg(a,this,f,g);this.Y(f,h);return f};s.ld=function(a,b,c,d,f,g,h){a=Ig(this,""rect"",{x:a,y:b,width:c,height:d});f=new Cg(a,this,f,g);this.Y(f,h);return f};s.drawImage=function(a,b,c,d,f,g){a=Ig(this,""image"",{x:a,y:b,width:c,height:d,""image-rendering"":""optimizeQuality"",preserveAspectRatio:Se});a.setAttributeNS(""http://www.w3.org/1999/xlink"",""href"",f);f=new Fg(a,this);this.Y(f,g);return f};
s.kd=function(a,b,c,d){a=Ig(this,""path"",{d:Eg(a)});b=new Dg(a,this,b,c);this.Y(b,d);return b};function Eg(a){var b=[];ag(a,function(c,d){switch(c){case 0:b.push(""M"");Array.prototype.push.apply(b,d);break;case 1:b.push(""L"");Array.prototype.push.apply(b,d);break;case 2:b.push(""C"");Array.prototype.push.apply(b,d);break;case 3:var f=d[3];b.push(""A"",d[0],d[1],0,Math.abs(f)>180?1:0,f>0?1:0,d[4],d[5]);break;case 4:b.push(""Z"")}});return b.join("" "")}
s.bd=function(a){var b=Ig(this,""g"");(a||this.G).c().appendChild(b);return new zg(b,this)};s.H=function(){var a=this.T();Gg.e.H.call(this);a||this.dispatchEvent(""resize"");if(this.j){a=this.width;var b=this.height;typeof a==""string""&&a.indexOf(""%"")!=-1&&typeof b==""string""&&b.indexOf(""%"")!=-1&&ye(this.wa,Kg(),""tick"",this.Rc);this.Rc()}};s.Ca=function(){Gg.e.Ca.call(this);this.j&&ze(this.wa,Kg(),""tick"",this.Rc)};s.o=function(){delete this.v;delete this.b;delete this.G;Gg.e.o.call(this)};
function Kg(){if(!Hg){Hg=new Vc(400);Hg.start()}return Hg};function Lg(){return this.n=this.C().L.c(this.R)||this.n}function Mg(a,b){this.R=a.id;U.call(this,a,b)}B(Mg,jg);Mg.prototype.c=Lg;Mg.prototype.clear=function(){Qb(this.c())};Mg.prototype.S=function(a,b){var c=this.c(),d=c.style;d.width=W(a)+""px"";d.height=W(b)+""px"";c.coordsize=W(a)+"" ""+W(b);if(this.C().G!=this)c.coordorigin=""0 0""};function Ng(a,b,c,d,f,g,h,k){this.R=a.id;V.call(this,a,b,h,k);this.i=c;this.j=d;this.a=f;this.f=g}B(Ng,ig);Ng.prototype.c=Lg;
Ng.prototype.Ic=function(a,b){this.i=a;this.j=b;Og(this.c(),a-this.a,b-this.f,this.a*2,this.f*2)};Ng.prototype.v=function(a,b){this.a=a;this.f=b;Og(this.c(),this.i-a,this.j-b,a*2,b*2)};function Pg(a,b,c,d){this.R=a.id;V.call(this,a,b,c,d)}B(Pg,mg);Pg.prototype.c=Lg;Pg.prototype.Pa=function(a,b){var c=this.c().style;c.left=Qg(a);c.top=Qg(b)};Pg.prototype.S=function(a,b){var c=this.c().style;c.width=W(a)+""px"";c.height=W(b)+""px""};function Rg(a,b,c,d){this.R=a.id;V.call(this,a,b,c,d)}B(Rg,lg);
Rg.prototype.c=Lg;Rg.prototype.aa=function(a){Sg(this.c(),""path"",Tg(a))};function Ug(a,b){this.R=a.id;U.call(this,a,b)}B(Ug,kg);Ug.prototype.c=Lg;Ug.prototype.Pa=function(a,b){var c=this.c().style;c.left=Qg(a);c.top=Qg(b)};Ug.prototype.S=function(a,b){var c=this.c().style;c.width=Qg(a);c.height=Qg(b)};Ug.prototype.Lc=function(a){Sg(this.c(),""src"",a)};function Vg(a,b,c,d,f){eg.call(this,a,b,c,d,f);this.wa=new we(this)}B(Vg,eg);var Wg=document.documentMode&&document.documentMode>=8;function Xg(a){var b;if(b=z(a)){b=a.length-1;b=b>=0&&a.indexOf(""%"",b)==b}return b?a:parseFloat(a.toString())+""px""}function Qg(a){return Math.round((parseFloat(a.toString())-0.5)*100)+""px""}function W(a){return Math.round(parseFloat(a.toString())*100)}function Sg(a,b,c){if(Wg)a[b]=c;else a.setAttribute(b,c)}
function Yg(a,b){var c=a.L.createElement(""g_vml_:""+b);c.id=""goog_""+Ea++;return c}function Zg(a){if(Wg&&a.p)a.c().innerHTML=a.c().innerHTML}Vg.prototype.Y=function(a,b){(b||this.G).c().appendChild(a.c());Zg(this)};
Vg.prototype.zd=function(a,b){var c=a.c();c.fillcolor="""";for(var d=0;d<c.childNodes.length;d++){var f=c.childNodes[d];f.tagName==""fill""&&c.removeChild(f)}if(b instanceof vg)if(b.va()==""transparent"")c.filled=l;else if(b.a!=1){c.filled=i;d=Yg(this,""fill"");d.opacity=Math.round(b.a*100)+""%"";d.color=b.va();c.appendChild(d)}else{c.filled=i;c.fillcolor=b.va()}else c.filled=l;Zg(this)};
Vg.prototype.Ad=function(a,b){var c=a.c();if(b){c.stroked=i;var d=b.U();if(z(d)&&d.indexOf(""px"")==-1)d=parseFloat(d);else d*=this.La();var f=c.getElementsByTagName(""stroke"")[0];if(d<1){f=f||Yg(this,""stroke"");f.opacity=d;f.Af=""1px"";f.color=b.va();c.appendChild(f)}else{f&&c.removeChild(f);c.strokecolor=b.va();c.strokeweight=d+""px""}}else c.stroked=l;Zg(this)};Vg.prototype.Bd=function(a,b,c,d,f,g){a=a.c();a.style.left=Qg(b);a.style.top=Qg(c);if(d||a.rotation){a.rotation=d;a.coordsize=W(f*2)+"" ""+W(g*2)}};
function Og(a,b,c,d,f){var g=a.style;g.position=Ue;g.left=Qg(b);g.top=Qg(c);g.width=W(d)+""px"";g.height=W(f)+""px"";if(a.tagName==""shape"")a.coordsize=W(d)+"" ""+W(f)}function $g(a,b){var c=Yg(a,b),d=a.Xa();Og(c,0,0,d.width,d.height);return c}try{eval(""document.namespaces"")}catch(ah){}s=Vg.prototype;
s.u=function(){var a=this.L.a;if(!a.namespaces.g_vml_){Wg?a.namespaces.add(""g_vml_"",""urn:schemas-microsoft-com:vml"",""#default#VML""):a.namespaces.add(""g_vml_"",""urn:schemas-microsoft-com:vml"");a.createStyleSheet().cssText=""g_vml_\\:*{behavior:url(#default#VML)}""}a=this.width;var b=this.height,c=this.L.u(""div"",{style:""overflow:hidden;position:relative;width:""+Xg(a)+"";height:""+Xg(b)});this.n=c;var d=Yg(this,""group""),f=d.style;f.position=Ue;f.left=f.top=0;f.width=this.width;f.height=this.height;d.coordsize=
this.a?W(this.a)+"" ""+W(this.f):W(a)+"" ""+W(b);d.coordorigin=this.ja!==undefined?W(this.ja)+"" ""+W(this.qa):""0 0"";c.appendChild(d);this.G=new Mg(d,this);L(c,""resize"",A(this.rd,this))};s.rd=function(){var a=Ve(this.c()),b=this.G.c().style;if(a.width){b.width=a.width+""px"";b.height=a.height+""px""}else{for(a=this.c();a&&a.currentStyle&&a.currentStyle.display!=Se;)a=a.parentNode;a&&a.currentStyle&&ye(this.wa,a,""propertychange"",this.rd)}this.dispatchEvent(""resize"")};
s.Xb=function(a,b){this.ja=a;this.qa=b;this.G.c().coordorigin=W(this.ja)+"" ""+W(this.qa)};s.Ga=function(a,b){Vg.e.Ga.apply(this,arguments);this.G.c().coordsize=W(a)+"" ""+W(b)};s.S=aa();s.T=function(){var a=this.c();return new E(a.style.pixelWidth||a.offsetWidth||1,a.style.pixelHeight||a.offsetHeight||1)};s.clear=function(){this.G.clear()};s.hd=function(a,b,c,d,f,g,h){var k=Yg(this,""oval"");Og(k,a-c,b-d,c*2,d*2);a=new Ng(k,this,a,b,c,d,f,g);this.Y(a,h);return a};
s.ld=function(a,b,c,d,f,g,h){var k=Yg(this,""rect"");Og(k,a,b,c,d);a=new Pg(k,this,f,g);this.Y(a,h);return a};s.drawImage=function(a,b,c,d,f,g){var h=Yg(this,""image"");Og(h,a,b,c,d);Sg(h,""src"",f);a=new Ug(h,this);this.Y(a,g);return a};s.kd=function(a,b,c,d){var f=$g(this,""shape"");Sg(f,""path"",Tg(a));a=new Rg(f,this,b,c);this.Y(a,d);return a};
function Tg(a){var b=[];ag(a,function(c,d){switch(c){case 0:b.push(""m"");Array.prototype.push.apply(b,Ha(d,W));break;case 1:b.push(""l"");Array.prototype.push.apply(b,Ha(d,W));break;case 2:b.push(""c"");Array.prototype.push.apply(b,Ha(d,W));break;case 4:b.push(""x"");break;case 3:var f=d[2]+d[3],g=W(d[4]-d[0]*Math.cos(Zf(f)));f=W(d[5]-d[1]*Math.sin(Zf(f)));var h=W(d[0]),k=W(d[1]);b.push(""ae"",g,f,h,k,Math.round(d[2]*-65536),Math.round(d[3]*-65536))}});return b.join("" "")}
s.bd=function(a){var b=$g(this,""group"");(a||this.G).c().appendChild(b);return new Mg(b,this)};s.H=function(){Vg.e.H.call(this);this.rd();Zg(this)};s.o=function(){this.G=j;Vg.e.o.call(this)};function bh(a,b,c,d,f){a=F?new Vg(a,b,c,d,f):sb&&(!Gb(""420"")||tb)?new xg(a,b,c,d,f):new Gg(a,b,c,d,f);a.u();return a};var ch={};function dh(a){a in ch||(ch[a]=z(a)&&(a.indexOf(""%"")!=-1||a.indexOf(""px"")!=-1));return ch[a]}function eh(a,b,c){var d=parseFloat(String(a));if(z(a))if(a.indexOf(""%"")!=-1)return d*b/100;else if(a.indexOf(""px"")!=-1)return d/c;return d};function fh(a,b){this.b=b;this.Ma=a?a.C():this;this.f=new gh(this,i);this.i=new gh(this,l);if(a){this.V=a;this.V.Pd(this)}}B(fh,Uc);s=fh.prototype;s.Ec=l;s.ca=l;s.ef=0;s.C=o(""Ma"");s.K=o(""V"");function hh(a,b,c,d,f){b.Pa(c,d);ih(a,b);a.ca=i;f||a.transform()}function jh(a,b,c,d){if(b.S(c)){a.ca=i;ih(a,b);d||a.reset()}else!d&&a.ca&&a.reset()}s.Ic=function(a,b){hh(this,this.f,a,1,b)};s.U=function(){return this.f.Ya()};s.Mc=function(a,b){jh(this,this.f,a,b)};s.$=function(){return this.i.Ya()};
s.Jc=function(a,b){jh(this,this.i,a,b)};s.Dd=function(a){var b=this.i;b.Bc=a;kh(b);this.ca=i;ih(this,b)};s.Pa=function(a,b,c){hh(this,this.f,a,0,i);hh(this,this.i,b,0,c)};s.S=function(a,b,c){this.Mc(a,i);this.Jc(b,c)};s.Wb=function(a,b,c,d,f){hh(this,this.f,a,0,i);hh(this,this.i,b,0,i);this.Mc(c,i);this.Jc(d,f)};s.reset=function(){kh(this.f);kh(this.i);this.q();this.ca=i;this.transform()};s.q=v;function ih(a,b){a.Ec=b.cb()||a.f.cb()||a.i.cb()||a.$c()}s.cb=o(""Ec"");s.$c=Ce;
s.transform=function(){if(this.ca){this.ca=l;var a=this.b,b=lh(this.f),c=lh(this.i),d=this.ef,f=(this.U()||1)/2,g=(this.$()||1)/2;a.Fd=gg(Zf(d),f,g).translate(b,c);a.C().Bd(a,b,c,d,f,g)}};s.La=function(){return this.C().La()};s.Mb=function(){return this.C().Mb()};s.o=function(){fh.e.o.call();this.b.ea()};function gh(a,b){this.n=a;this.a=b}function mh(a){var b=a.n.K();return a.a?b.U():b.$()}s=gh.prototype;s.Ya=function(){return Math.max(this.Za(this.ma),this.Za(this.Bc))};
s.S=function(a){if(a!=this.ma){this.ma=a;kh(this);return i}return l};s.Za=function(a,b){if(!dh(a))return parseFloat(String(a));var c=this.ad||(this.ad={}),d=this.a?this.n.La():this.n.Mb(),f;if(b)f=eh(this.ma||0,0,d);else{var g=this.n.K();f=this.a?g.U():g.$()}g=a;f=f;if(!ia(g)){var h=c&&(b?""X"":"""")+g;if(c&&h in c)g=c[h];else{g=dh(g)?eh(g,f,d):parseFloat(g);if(c)c[h]=g}}return g};function lh(a){if(a.Gb==j){var b=a.Za(a.nc);a.Gb=a.mc==0?b:a.mc==1?b+(mh(a)-a.Ya())/2:mh(a)-b-a.Ya()}return a.Gb}
s.Pa=function(a,b){this.nc=a;this.mc=b;this.Gb=j};function nh(a){return a.Za(a.nc||0)+(dh(a.ma)?0:a.Ya())}function kh(a){a.ad=j;a.Gb=j}s.cb=function(){return this.mc!=0||dh(this.ma)||dh(this.Bc)||dh(this.nc)};s.Gb=j;s.ad=j;s.Bc=0;s.ma=0;s.nc=0;s.mc=0;function oh(a,b){b=b||a.Ma.a.bd(a.b);fh.call(this,a,b);this.t=[]}B(oh,fh);s=oh.prototype;s.Pd=function(a,b){D(this.t,a)>=0||this.t.push(a);var c;c=l;var d=nh(a.f);if(d>this.U()){c=this.f;c.Bc=d;kh(c);this.ca=i;ih(this,c);c=i}d=nh(a.i);if(d>this.$()){this.Dd(d);c=i}c=c;if(a.cb())a.ca=a.ca||a.Ec;!b&&a.ca&&a.reset();c&&this.reset()};s.removeChild=function(a){Ka(this.t,a);Rb(a.b.c())};s.X=function(a,b){this.t&&Fa(this.t,a,b)};s.reset=function(){oh.e.reset.call(this);this.Qc()};
s.q=function(){this.b.S(this.U(),this.$());ph(this)};function ph(a){a.X(function(b){if(b.cb())b.ca=b.ca||b.Ec})}s.Qc=function(){this.X(function(a){if(a.cb()||a.ca)a.reset();else a.Qc&&a.Qc()})};s.clear=function(){for(;this.t.length;)this.removeChild(this.t[0])};function qh(a,b,c,d,f,g){if(g)if(nb&&G&&!Gb(""1.9a"")){a=new xg(a,b,c,d,f);a.u();a=a}else a=bh(a,b,c,d,f);else a=bh(a,b,c,d,f);this.a=a=a;oh.call(this,j,a.G);L(a,""resize"",this.Qc,l,this)}B(qh,oh);s=qh.prototype;s.Ga=function(a,b){this.a.Ga(a,b);qh.e.S.call(this,a,b)};s.Xa=function(){return this.a.Xa()};s.Xb=function(a,b){this.a.Xb(a,b)};s.T=function(){return this.a.T()};s.U=function(){return this.a.Xa().width};s.$=function(){return this.a.Xa().height};s.La=function(){return this.a.La()};s.Mb=function(){return this.a.Mb()};
s.c=function(){return this.a.c()};s.Bb=function(a){this.a.Bb(a)};s.transform=v;s.q=function(){ph(this)};function rh(){T.call(this)}B(rh,T);rh.prototype.a=j;rh.prototype.B=function(){var a=rh.e.B.call(this);a.a=this.a&&this.a.B();return a};rh.prototype.transform=function(a){rh.e.transform.call(this,a);this.a=j;return this};function sh(a){if(!a.a&&!a.xc()){var b,c=b=Number.POSITIVE_INFINITY,d,f=d=Number.NEGATIVE_INFINITY;ag(a.la?a:cg(a),function(g,h){for(var k=0,m=h.length;k<m;k+=2){c=Math.min(c,h[k]);f=Math.max(f,h[k]);b=Math.min(b,h[k+1]);d=Math.max(d,h[k+1])}});a.a=new Ke(c,b,f-c,d-b)}return a.a};function th(a,b){fh.call(this,a,b)}B(th,fh);th.prototype.Yb=function(a){this.b.Yb(a)};th.prototype.fb=function(a){this.b.fb(a)};th.prototype.q=function(){var a=this.b;a.ac&&a.fb(a.ac)};function uh(a,b,c){this.hc=!!c;c=a.Ma.a.kd(b,j,j,a.b);fh.call(this,a,c);this.aa(b)}B(uh,th);s=uh.prototype;s.hc=l;s.kb=j;s.Lb=o(""Q"");s.aa=function(a){this.Q=a;if(this.hc)this.kb=sh(a);vh(this)};
function vh(a){var b;if(a.kb){b=a.Q.B();var c=-a.kb.left,d=-a.kb.top,f=a.U()/(a.kb.width||1),g=a.$()/(a.kb.height||1);if(!b.la){var h=cg(b);b.clear();if(h.O){Array.prototype.push.apply(b.N,h.N);Array.prototype.push.apply(b.k,h.k);Array.prototype.push.apply(b.ba,h.ba);b.O=h.O.concat();b.ia=h.ia.concat();b.la=b.la&&h.la}}b=b.transform(hg(new fg,f,g).translate(c,d))}else b=a.Q;a.j=b;(b=a.b)&&b.aa(a.j)}s.q=function(){uh.e.q.call(this);this.hc&&vh(this)};s.$c=function(){return this.hc||uh.e.$c.call(this)};function wh(a){var b=a.Ma.a.hd(1,1,2,2,j,j,a.b);fh.call(this,a,b)}B(wh,th);wh.prototype.q=function(){wh.e.q.call(this);var a=this.U()/2,b=this.$()/2,c=this.b;c.Ic(a,b);c.v(a,b)};function xh(a){var b=a.Ma.a.ld(0,0,1,1,j,j,a.b);fh.call(this,a,b)}B(xh,th);xh.prototype.q=function(){xh.e.q.call(this);this.b.S(this.U(),this.$())};var yh={arrowE:""M0,5615;L10427,5615;C10427,5615,10412,481,10427,435;C10427,-550,11410,445,11410,445;L21600,10795;C21600,10795,11413,21119,11413,21141;C10263,22165,10427,21155,10427,21155;L10427,15976;L0,15976;L0,5615;Z"",arrowNE:""M0,13287;L8352,4934;C8352,4934,4221,828,4196,779;C3405,-10,4992,0,4992,0;L21460,141;C21460,141,21582,16584,21600,16601;C21501,18344,20821,17402,20821,17402;L16665,13247;L8312,21600;L0,13287;Z"",arrowN:""M5615,21600;L5615,11173;C5615,11173,481,11188,435,11173;C-550,11173,445,10190,445,10190;L10795,0;C10795,0,21119,10187,21141,10187;C22165,11337,21155,11173,21155,11173;L15976,11173;L15976,21600;L5615,21600;Z"",
speech:""M21600,8628;C21600,3863,16763,0,10799,0;C4833,0,0,3863,0,8628;C0,12311,2883,15453,6947,16693;C7371,16821,7356,19211,5803,21600;C8571,20513,9126,17161,9574,17203;C9976,17238,10384,17257,10799,17257;C16763,17257,21600,13394,21600,8628;Z"",starburst:""M10800,0;C10968,0,11963,1517,12076,1533;C12193,1550,13559,355,13720,399;C13882,445,14431,2177,14537,2224;C14644,2270,16281,1490,16426,1578;C16568,1665,16631,3481,16720,3553;C16810,3627,18596,3321,18711,3442;C18825,3565,18398,5332,18465,5425;C18530,5521,20335,5708,20412,5857;C20490,6006,19601,7591,19639,7699;C19678,7809,21366,8476,21399,8642;C21433,8807,20151,10091,20159,10208;C20167,10323,21611,11421,21600,11588;C21588,11757,20009,12648,19986,12762;C19961,12877,21057,14323,21001,14482;C20945,14641,19182,15072,19129,15175;C19077,15279,19739,16970,19644,17107;C19546,17244,17735,17182,17655,17267;C17576,17352,17761,19160,17631,19265;C17500,19372,15773,18823,15674,18883;C15575,18945,15267,20733,15113,20801;C14957,20868,13442,19873,13331,19904;C13219,19935,12439,21576,12273,21599;C12109,21622,10915,20252,10800,20252;C10684,20252,9492,21622,9326,21599;C9159,21576,8380,19935,8268,19904;C8157,19873,6641,20868,6486,20801;C6332,20733,6024,18945,5925,18883;C5825,18823,4099,19372,3968,19265;C3837,19160,4023,17352,3944,17267;C3863,17182,2053,17244,1954,17107;C1859,16970,2522,15279,2469,15175;C2416,15072,654,14641,598,14482;C541,14323,1638,12877,1615,12762;C1590,12648,11,11757,0,11588;C-10,11421,1431,10323,1439,10208;C1447,10091,166,8807,200,8642;C235,8476,1920,7809,1959,7699;C1999,7591,1109,6006,1187,5857;C1264,5708,3069,5521,3134,5425;C3201,5332,2773,3565,2887,3442;C3003,3321,4789,3627,4878,3553;C4970,3481,5033,1665,5173,1578;C5317,1490,6956,2270,7062,2224;C7167,2177,7716,445,7879,399;C8040,355,9406,1550,9522,1533;C9637,1517,10630,0,10800,0;Z""};function zh(a,b){this.b=a;this.a=b}var Ah=new Ke(0,0,21600,21600);
zh.prototype.P=function(a,b,c){var d,f=this.a;switch(this.b){case ""path"":var g=new rh;f=yh[f].split("";"");for(var h=0;d=f[h];h++){var k=d.substr(0,1);if(k==""Z"")g.close();else{d=d.substr(1).split("","");for(var m=0,n=d.length;m<n;m++)d[m]=parseFloat(d[m]);d=d;var p;switch(k){case ""M"":p=g.moveTo;break;case ""L"":p=g.lineTo;break;case ""C"":p=g.dd}p.apply(g,d)}}g.a=Ah&&Ah.B();a=new uh(a,g,i);break;case ""ellipse"":a=new wh(a);break;case ""rect"":a=new xh(a);break;default:return j}a.S(""100%"",""100%"");a.fb(c);a.Yb(b);
return a};function X(a){Q.call(this,a)}B(X,Gf);
var Dh=R(X,""fillColor"",{defaultValue:""#ffffff"",r:""N"",action:function(){var a=Bh(this);Ch(this,function(b){b.Yb(a)})}}),Gh=R(X,""strokeColor"",{defaultValue:""#000000"",r:""N"",action:function(a,b){if(!a||!b){this.ib();Eh(this)}var c=Fh(this);Ch(this,function(d){d.fb(c)})}}),Hh=R(X,""strokeWidth"",{defaultValue:1,r:""N"",action:function(){if(Gh.g(this)!=j){this.ib();Eh(this)}var a=Fh(this);Ch(this,function(b){b.fb(a)})}}),Ih=R(X,""flipH"",{defaultValue:l,r:""N"",action:function(){Ch(this,function(a){a instanceof
uh&&a.aa(dg(a.Lb(),hg(new fg,-1,1)))})}}),Jh=R(X,""flipV"",{defaultValue:l,r:""N"",action:function(){Ch(this,function(a){a instanceof uh&&a.aa(dg(a.Lb(),hg(new fg,1,-1)))})}});s=X.prototype;s.sa=j;s.md=j;s.Qa=j;s.mb=function(a,b){X.e.mb.call(this,a,b);if(this.constructor==X){var c=this.sa;if(a.sa!=c){a.sa=c;Kh(a)}}};s.Eb=function(){X.e.Eb.call(this);Eh(this)};
s.Pb=function(){var a=X.e.Pb.call(this),b=Gh.g(this)!=j?Hh.g(this)||0:0;if(b){a=a.B();var c=b/2;a.left-=c/8;a.top-=c/6;a.width+=b/8;a.height+=b/6}return a};function Eh(a,b,c){if(b=b||a.Qa){c=c||a.b;var d=Jf(a);a=Gh.g(a)!=j?Hh.g(a)||0:0;var f=a/2;b.Xb(d.left*8-f,d.top*6-f);b.Ga(d.width*8+a,d.height*6+a);c.Wb(d.left*8,d.top*6,d.width*8,d.height*6)}}function Ch(a,b){a.md?Fa(a.md,b):Kh(a)}function Bh(a){return(a=a.Kb())?new vg(a):j}function Fh(a){var b=Gh.g(a);return b?new wg(Hh.g(a),b):j}s.Kb=function(){return Dh.g(this)};
s.Cd=function(a,b){return Dh.D(this,a,b)};function Kh(a){var b=a.c();if(b&&a.sa){if(a.Qa)a.b.clear();else{a.Qa=new qh(""100%"",""100%"",1,1,a.L);a.Qa.Bb(b);a.Qa.c().style.display=""block"";a.b=new oh(a.Qa);Eh(a)}a.md=a.Ba(a.Qa,a.b)}}s.Ba=function(a,b){if(this.sa){for(var c=[],d=Ih.g(this),f=Jh.g(this),g=0,h=this.sa.length;g<h;g++){var k=this.sa[g];k=new zh(k.type,k.data);var m=Bh(this),n=Fh(this);k=k.P(b,m,n);if(k instanceof uh&&(d||f))k.aa(dg(k.Lb(),hg(new fg,d?-1:1,f?-1:1)));c.push(k)}return c}return j};
s.Yc=function(){X.e.Yc.apply(this,arguments);G&&Ze(this.c(),l)};s.Uc=function(){X.e.Uc.apply(this,arguments";
        #endregion

        [TestMethod]
        [WorkItem(323062)]
        public void ImplicitRequestTest()
        {
            var primary = _session.FileFromText("function test(a) { } test('some content(')");
            var context = _session.OpenContext(primary);
            var offset = primary.OffsetAfter("content(");
            var result = context.GetParameterHelpAt(offset);
            Assert.IsNotNull(result.FunctionHelp);

            result = context.GetImplicitParameterHelpAt(offset);
            Assert.IsNull(result.FunctionHelp);
        }

        [TestMethod]
        public void ImplicitRequestWithArgs()
        {
            PerformRequests(
            @"
                function foo(a,b,c) { }
                foo(|1,2,3);
                foo(|'',2,3);
                foo(|'hi',2,3);
                foo(|0.1,2,3);
                foo(|x);
                foo(|,);
            ",
             (context, offset, data, index) =>
             {
                 var help = context.GetImplicitParameterHelpAt(offset);
                 Assert.IsNotNull(help.FunctionHelp);
             });

            PerformRequests(
            @"
                function foo(a,b,c) { }
                foo(1,'(|',2,3);
                foo(1,(|1),2,3);
                foo(1,(|),2,3);
                foo(1,2(|,2,3);
            ",
             (context, offset, data, index) =>
             {
                 var help = context.GetImplicitParameterHelpAt(offset);
                 Assert.IsNull(help.FunctionHelp);
             });

        }

        [TestMethod]
        public void DeprecatedImplicitSignature()
        {
            VerifyFunctionDocComments(DeprecatedImplicitSignatureFile, DeprecatedImplicitSignatureExpected);
        }
        #region Test data
        const string DeprecatedImplicitSignatureFile = @"
function ImplicitSignature() {
    function func(a, b) {
        /// <summary>implicit signature</summary>
        /// <param name=""a"" type=""String"">parameter a</param>
        /// <param name=""b"" type=""Number"">parameter b</param>
        /// <returns type=""String"">return description</returns>
        /// <deprecated type=""deprecate"">deprecated message</deprecated>
    }
    func(|);
}";
        const string DeprecatedImplicitSignatureExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 388, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""implicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }, Deprecated = 
                    new { Type = ""deprecate"", Message = ""deprecated message"" }
                }
            }
        }
    }
}";
        #endregion

        [TestMethod]
        public void DeprecatedExplicitSingleSignature()
        {
            VerifyFunctionDocComments(DeprecatedExplicitSingleSignatureFile, DeprecatedExplicitSingleSignatureExpected);
        }
        #region Test data
        const string DeprecatedExplicitSingleSignatureFile = @"
function ExplicitSingleSignatureSameAsDecl() {
    function func(a, b) {
        /// <signature>
        ///     <summary>explicit signature</summary>
        ///     <param name=""a"" type=""String"">parameter a</param>
        ///     <param name=""b"" type=""Number"">parameter b</param>
        ///     <returns type=""String"">return description</returns>
        ///     <deprecated type=""remove"">deprecated message</deprecated>
        /// </signature>
    }
    func(|);
}";
        const string DeprecatedExplicitSingleSignatureExpected = @"new [] { 
    new { ParameterIndex = 0, Region = 
        new { Offset = 472, Length = 6 }, FunctionHelp = 
        new { FunctionName = ""func"", Signatures = new [] { 
                new { Description = ""explicit signature"", Parameters = new [] { 
                        new { Name = ""a"", Type = ""String"", Description = ""parameter a"", Optional = False }, 
                        new { Name = ""b"", Type = ""Number"", Description = ""parameter b"", Optional = False }
                    }, ReturnValue = 
                    new { Type = ""String"", Description = ""return description"" }, Deprecated = 
                    new { Type = ""remove"", Message = ""deprecated message"" }
                }
            }
        }
    }
}";
        #endregion

    }
}