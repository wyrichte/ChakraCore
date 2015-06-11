using System;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]   
    public class DeferredParsingTests: CompletionsBase
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
        public void CompletionsFromADeferredFunction() 
        {
            PerformCompletionRequests("var a = foo(); a.|bar,baz|; a.bar.|Number|; a.baz.|String|;", "function foo() { return { bar: 1, baz: '' }; }");
        }

        [TestMethod]
        public void CompletionsFromADeferredFunctionMultipleContexts()
        {
            PerformCompletionRequests("var a = foo(); a.|bar|; a.bar.|Number|; a = goo(); a.|zoo|; a.zoo.|Number|;", "function foo() { return { bar: 1 }; }", "function goo() { return { zoo: 1}; }");           
        }

        [TestMethod]
        public void CompletionsFromADeferredMethodMultipleContexts()
        {
            PerformCompletionRequests("var a = foo.bar(); a.|baz|; a.baz.|Number|; a = goo.boo(); a.|zoo|; a.zoo.|Number|;", "var foo = { bar: function () { return { baz: 1 }; } }", "var goo = { boo: function () { return { zoo: 1 }; } }");
        }

        [TestMethod]
        public void ErrorInContextFile()
        {
            PerformCompletionRequests("var a = foo(); a.|bar|; a.bar.|Number|;", "function hasError() { a a a a; } function foo() { return { bar: 1 } }");
        }

        [TestMethod]
        public void ModulePattern()
        {
            PerformCompletionRequests("init(); foo().|Number|; bar().|Number|; baz().|Number|;", ModulePattern_Module);
        }

        [TestMethod]
        public void DeferredParsingDocCommentsRewriting() 
        {
            PerformCompletionRequests("var a = foo(); a.|Number|",
                @"function foo() { 
                    /// <signature>
                    /// <returns type='Number'/>
                    /// </signature>
                    return null; }");
        }

        [TestMethod]
        public void DeferredParsingCallbackLifetime()
        {
            var file1 = _session.FileFromText(
                @"function test1() {
                    /// <Returns value='1' />
                  }");
            var file2 = _session.FileFromText(
                @"function test2() {
                    return _$value(11);
                  }");
            var file3 = _session.FileFromText("var b = test2()");
            var file4 = _session.FileFromText("b.");

            // Open a context, use it, and then force the leaf context to be discarded.
            var context1 = _session.OpenContext(file1);
            for (var i = 0; i < 2; i++)
            {
                file1.Touch();
                context1.GetCompletionsAt(file1.Text.Length);
            }
            context1.Close();

            // Open a second file and try to use it.
            var context2 = _session.OpenContext(file4, file2, file3);
            var result = context2.GetCompletionsAt(file4.Text.Length);
            Assert.IsNotNull(result);
        }


        #region Test data
        const string ModulePattern_Module = @"
(function(global) {
    function init() {
        function foo() {
           return 1;
        }
        function bar() {
           return 1;
        }
        function baz() {
           return 1;
        }
        global.foo = foo;
        global.bar = bar;
        global.baz = baz;
   }

   global.init = init;
})(this);";
        #endregion

        #region Helpers
        new internal void PerformCompletionRequests(string text, Action<IEnumerable<AuthorCompletion>, string, int> action, AuthorCompletionFlags flags = AuthorCompletionFlags.acfMembersFilter, params string[] contextFiles)
        {
            if (contextFiles != null && contextFiles.Length > 0)
            {
                var forceDeferredParsing = TestFiles.DeferredParsing;
                contextFiles = contextFiles.Select(f => f + "\r\n" + forceDeferredParsing).ToArray();
            }
            PerformRequests(text, (context, offset, data, index) =>
            {
                var completions = context.GetCompletionsAt(offset, flags);
                Assert.IsNotNull(completions);
                action(completions.ToEnumerable(), data, index);
            }, contextFiles);
        }

        new internal void PerformCompletionRequests(string text, params string[] contextFiles)
        {
            PerformCompletionRequests(text,  (completions, data, index) => 
            {
                if (!String.IsNullOrEmpty(data))
                {
                    switch (data)
                    {
                        case "Number":
                            completions.ExpectContains(NumberMethods);
                            break;
                        case "String":
                            completions.ExpectContains(StringMethods);
                            break;
                        default:
                            completions.ExpectContains(data.Split(',').ToArray());
                            break;
                    }
                }
            }, AuthorCompletionFlags.acfMembersFilter, contextFiles);
        }
        #endregion
    }
}
