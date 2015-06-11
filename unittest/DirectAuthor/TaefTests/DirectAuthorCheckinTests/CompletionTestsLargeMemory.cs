using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class CompletionTestsLargeMemory : CompletionsBase
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
        public void LargeAllocations()
        {
            // Array via ctor
            PerformCompletionRequests(@"
                x = new Array(0x80000000);
                x & x;
                x.|length|
            ");

            // Setting array size via length property
            PerformCompletionRequests(@"
                x = [];
                x.length = 0x80000000;
                x & x;
                x.|length|
            ");

            // Typed array
            PerformCompletionRequests(@"
                x = new Int8Array(0x50000000);
                x & x;
                this.|x|
            ");

            // Very long string
            PerformCompletionRequests(@"
                intellisense.addEventListener('statementcompletion',  function() { });
                var x = '                                                                                                   ';
                for(var i=0; i<30; i++) {
                    intellisense.logMessage('>>> x.length: ' + x.length);
                    x = (x + x + x + x + x + x).toLowerCase();
                }
                x.|length|;
            ");

            // Function.toString for a very large function 
            // (make sure that recycler allocation limit doesn't affect the ability to execute a large function)   
            string text = "function f() { return 1;" + new string(' ', 50 * 1024 * 1024) + "}; var s=f.toString().toLower();f().|toFixed|";
            PerformCompletionRequests(text);
        }
    }
}
