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
    public class SolitaireTests : PerformanceTests
    {
        protected static readonly string mainFileLocation = Path.Combine(Paths.FilesPath, @"SolitaireApp\default.js");
        protected static readonly string[] contextFiles = new[] { 
            Paths.DomWindowsPath, 
            Paths.SiteTypesWindowsPath, 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\winrt\winrt.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\js\base.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\js\ui.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\js\wwaapp.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\js\xhr.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\cards.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\cardsui.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\cardTests.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\gameController.js"), 
            Path.Combine(Paths.FilesPath, @"SolitaireApp\simpleAnim.js") 
        };

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var mainFile = _session.ReadFile(mainFileLocation);
            var requests = PerformGetParameterHelp(mainFile.Text + " app.run(_|r|); ", AuthorHostType.ahtApplication, contextFiles);
            requests.ExpectFunction("run");
        }

        [TestMethod]
        public void GetFunctionHelpSecondRequest()
        {
            var mainFile = _session.ReadFile(mainFileLocation);
            var requests = PerformGetParameterHelp(mainFile.Text + " app.run(_|s|); app.dispatch(_|r|); ", AuthorHostType.ahtApplication, contextFiles);
            requests.ExpectFunction("dispatch");
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var mainFile = _session.ReadFile(mainFileLocation);

            var result = PerformGetCompletions(mainFile.Text + "; app._|r|; ", AuthorHostType.ahtApplication, contextFiles);
            result[0].ToEnumerable().ExpectContains(GetCompletionsExpected);
        }
        #region Test data
        static readonly string[] GetCompletionsExpected = new[] { "run", "dispatch", "recycle", "localStorage" };
        #endregion

        [TestMethod]
        public void GetCompletionsSecondRequest()
        {
            var mainFile = _session.ReadFile(mainFileLocation);

            var result = PerformGetCompletions(mainFile.Text + "; app._|s|; app._|r|;", AuthorHostType.ahtApplication, contextFiles);
            result[0].ToEnumerable().ExpectContains(GetCompletionsExpected);
        }
    }
}