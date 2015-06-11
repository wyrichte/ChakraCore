using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace DirectAuthorTests
{
    [TestClass]
    public class DomAsContextFileTests : PerformanceTests
    {
        protected static readonly string dom_jsFileLocation = Paths.DomWebPath;

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var requests = PerformGetParameterHelp("window.alert(_|r|);", dom_jsFileLocation);
            requests.ExpectFunction("alert");
        }

        [TestMethod]
        public void GetFunctionHelpSecondRequest()
        {
            var requests = PerformGetParameterHelp("window.alert(_|s|); \nwindow.confirm(_|r|);", dom_jsFileLocation);
            requests.ExpectFunction("confirm");
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var result = PerformGetCompletions("window._|r|", dom_jsFileLocation);
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }
        #region Test data
        static readonly string[] DomCompletionsExpected = new[] { "document", "alert", "history", "ondblclick" };
        #endregion

        [TestMethod]
        public void GetCompletionsSecondRequest()
        {
            var result = PerformGetCompletions("window._|s|; window._|r|", dom_jsFileLocation);
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }
    }
}