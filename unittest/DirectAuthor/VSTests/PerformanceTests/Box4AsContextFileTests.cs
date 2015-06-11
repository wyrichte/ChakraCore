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
    public class Box4AsContextFileTests : PerformanceTests
    {
        protected static readonly string box4FileLocation = Path.Combine(Paths.FilesPath, @"Box4\box4.js");

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var requests = PerformGetParameterHelp("Common._redirectUtil._openRedirect$i(_|r|);", Paths.SiteTypesWebPath, Paths.DomWebPath, box4FileLocation);
            requests.ExpectFunction("_openRedirect$i");
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var result = PerformGetCompletions("Common._redirectUtil._|r|", Paths.SiteTypesWebPath, Paths.DomWebPath, box4FileLocation);
            result[0].ToEnumerable().ExpectContains("_openRedirect$i", "_openHelpWindow$i", "_openWindowToRedirectUrl$i", "_getLongAppName$i","_getShortAppName$i");
        }
    }
}