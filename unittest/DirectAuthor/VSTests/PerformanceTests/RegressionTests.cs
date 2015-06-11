using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class RegressionTests : CompletionsBase
    {
        [TestMethod]
        [WorkItem(271685)]
        public void Bug271685()
        {
            int maxIterations = 250;
#if DEBUG
            double maxVariance = 20.0;
#else
            double maxVariance = 15.0;
#endif

            var contextFileNames = new[] {Paths.SiteTypesWebPath,
                Paths.LibHelpPath,
                Paths.DomWebPath,
                Path.Combine(Paths.FilesPath, @"Bug271685\02.MicrosoftAjax.debug.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\03.AjaxExtensions.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\04.MicrosoftAjaxWebForms.debug.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\05.MicrosoftAjax.debug.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\06.MicrosoftAjaxAdoNet.debug.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\07.jQueryv1.3.2.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\08.JQuery.UI.1.7.2.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\09. jQuery.UI.Accordion.1.7.2.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\10.jQuery.UI.Draggable.1.7.2.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\11.ui.resizeable.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\12.ui.dialog.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\13.jquery.bigframe.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\14.Jquery.treeview.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\15.Jquery.treeview.asyncway.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\16.vanadium.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\17.jquery.cookie.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\18.utils.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\19.mapcontrol.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\20.GeneratedElements.js"),
                Path.Combine(Paths.FilesPath, @"Bug271685\24.js")
            };

            List<IAuthorTestFile> contextFiles = new List<IAuthorTestFile>();
            for (int i = 0; i < contextFileNames.Length; i++)
            {
                contextFiles.Add(_session.ReadFile(contextFileNames[i]));
            }
            var primaryFile = _session.ReadFile(Path.Combine(Paths.FilesPath, @"Bug271685\primaryfile.js"));
            var context = _session.OpenContext(primaryFile, contextFiles.ToArray());

            // request first completion
            var completion = context.GetCompletionsAt(primaryFile.Text.Length);
            if (completion != null)
                Marshal.ReleaseComObject(completion);
            completion = null;

            // Invoke the GC
            _session.Cleanup();

            List<long> values = new List<long>();

            for (int i = 0; i < maxIterations; i++)
            {
                long callTime;

                primaryFile.Touch();
                completion = context.GetCompletionsAt(primaryFile.Text.Length, AuthorCompletionFlags.acfMembersFilter, out callTime);
                if (completion != null)
                    Marshal.ReleaseComObject(completion);
                completion = null;

                _session.Cleanup();

                values.Add(callTime);
            }

            var avarage = values.Average();
            var variance = Variance(values.ToArray());
            var relativeVariance = (variance / avarage) * 100;
            variance = Math.Sqrt(variance / maxIterations) / avarage;

            Console.WriteLine("\t Avarage Time: {0}, stdev {1}({2}%)", avarage, variance, Math.Round(relativeVariance, 2));
            Assert.IsTrue(relativeVariance < maxVariance);
        }

        private double Variance(long[] list)
        {
            if (list.Length == 0)
                return 0.0;

            double totalVariance = 0.0;
            double avarage = list.Average();

            foreach (var item in list)
            {
                totalVariance += Math.Pow((item - avarage), 2);
            }

            return Math.Sqrt(totalVariance / list.Length);
        }
    }
}
