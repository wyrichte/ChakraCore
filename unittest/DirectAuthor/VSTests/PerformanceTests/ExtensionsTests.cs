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
    public class ExtensionsTests : PerformanceTests
    {
        [TestMethod]
        public void ManyItemsInGlobalScope()
        {
            string extension = @"
                intellisense.addEventListener('statementcompletion', function (e) {
                    intellisense.logMessage('>>> in extension');
                    var items = e.items;
                    var len = items.length;
                    for (var i = 0; i < len; i++) {
                        var item = items[i];
                    }
                });";

            StringBuilder primary = new StringBuilder();
            for (int i = 0; i < 1000; i++)
            {
                primary.AppendLine("this.i" + i.ToString() + "=" + i.ToString() + ";");
            }

            PerformGetCompletions(extension + "\n" + primary.ToString() + ";_|s|_|r|", Paths.SiteTypesWebPath, Paths.DomWebPath);
        }
    }
}