using System.Linq;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class DedicatedWorkerCompletion : CompletionsBase
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
        [WorkItem(194861)]
        public void NoPrivatesInGlobalScope()
        {
            var dedicatedWorker = _session.ReadFile(Paths.DedicatedWorkerPath).Text;

            PerformCompletionRequests(@";|", (completions, data, i) =>
            {
                completions.Count(c => c.Name.StartsWith("_")).Expect(0);
            }, AuthorCompletionFlags.acfAny, new[] { dedicatedWorker });
        }

        [TestMethod]
        [WorkItem(281057)]
        public void DedicatedWorkerCompletions()
        {
            // From WebWorker documentation at http://www.w3.org/TR/workers/

            var dedicatedWorker = _session.ReadFile(Paths.DedicatedWorkerPath).Text;

            // global scope
            PerformCompletionRequests(@";|", (completions, data, i) =>
            {
                completions.ExpectContains(new[] { 
                    // WorkerGlobalScope interface
                    "self", "location", "close", "toString", "onerror",
                    // WorkerUtils interface
                    "importScripts", "navigator", "msIndexedDB", 
                    //DedicatedWorkerGlobalScope interface
                    "postMessage","onmessage",
                    // WindowBase64 interface
                    "btoa", "atob",
                    // WindowConsole interface
                    "console",
                    // EventTarget interface
                    "addEventListener", "removeEventListener", "dispatchEvent",
                    //WorkerTimers interface
                    "setInterval", "clearInterval", "setTimeout", "clearTimeout"
                });
            }, AuthorCompletionFlags.acfMembersFilter, new[] { dedicatedWorker });

            // Worker object
            PerformCompletionRequests(@"var w = new Worker('test'); w.|", (completions, data, i) =>
            {
                completions.ExpectContains(new[] { 
                    // AbstractWorker  interface
                    "onerror",
                    // Worker interface
                    "terminate", "postMessage", "onmessage",
                    // EventTarget interface
                    "addEventListener", "removeEventListener", "dispatchEvent",
                });
            }, AuthorCompletionFlags.acfMembersFilter, new[] { dedicatedWorker });

            // WorkerLocation object
            PerformCompletionRequests(@"location.|", (completions, data, i) =>
            {
                completions.ExpectContains(new[] { 
                    "href", "protocol", "host", "hostname", "port", "pathname", "search", "hash"
                });
            }, AuthorCompletionFlags.acfMembersFilter, new[] { dedicatedWorker });

            // Console object
            PerformCompletionRequests(@"console.|", (completions, data, i) =>
            {
                completions.ExpectContains(new[] { 
                    "info", "assert", "warn", "error","log"
                });
            }, AuthorCompletionFlags.acfMembersFilter, new[] { dedicatedWorker });
        }


        [TestMethod]
        public void DedicatedWorkerEventArgs()
        {
            var dedicatedWorker = _session.ReadFile(Paths.DedicatedWorkerPath).Text;

            PerformCompletionRequests(
                @"
                    var w = new Worker('test'); 

                    w.onerror = function(e) {
                        e.|timeStamp,type,message,filename,lineno,initErrorEvent|
                    };
                    
                    w.onmessage = function(e) {
                        e.|timeStamp,type,source,origin,data,initMessageEvent|
                    };
                ", dedicatedWorker);
        }
    }
}
