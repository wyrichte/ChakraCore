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
    public class OpenMailTests : PerformanceTests
    {
        protected static readonly string openMailFileLocation = Path.Combine(Paths.FilesPath, @"OpenMail\openmail_init.js");

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var openMailFile = _session.ReadFile(openMailFileLocation);

            PerformVSPerformanceTest(openMailFile.Text + "; YAHOO.log(__|r|);",
                (context, offset, data, index) =>
                {
                    Int64 callTime;
                    var help = context.GetParameterHelpAt(offset, out callTime);
                    Assert.AreEqual(help.FunctionHelp.FunctionName, "log");

                    if (String.Compare(data, "r", StringComparison.InvariantCultureIgnoreCase) == 0)
                    {
                        RecordTestTime(callTime);
                    }
                }, Paths.SiteTypesWebPath, Paths.DomWebPath);
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var openMailFile = _session.ReadFile(openMailFileLocation);

            PerformVSPerformanceTest(openMailFile.Text + ".__|r|",
                (context, offset, data, index) =>
                {
                    Int64 callTime;
                    var completions = context.GetCompletionsAt(offset, AuthorCompletionFlags.acfMembersFilter, out callTime);
                    completions.ToEnumerable().ExpectContains("CellRenderer", "CheckboxRenderer", "Recordset", "Scrollbar");
                    if (String.Compare(data, "r", StringComparison.InvariantCultureIgnoreCase) == 0)
                    {
                        RecordTestTime(callTime);
                    }
                }, Paths.SiteTypesWebPath, Paths.DomWebPath);
        }

        #region Helper Methods
        /// <summary>
        /// Override the default parse mechanism to use a different marker for code completion "__|ch|" instead of "|ch|" or simply "|"
        /// </summary>
        protected override ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();

            int start = 0;
            int markerBeginningIndex = 0;
            string markerBeginning = "__|";
            string markerEnd = "|";

            while ((markerBeginningIndex = text.IndexOf(markerBeginning, start)) >= 0)
            {
                builder.Append(text, start, markerBeginningIndex - start);

                int markerEndIndex = text.IndexOf(markerEnd, markerBeginningIndex + markerBeginning.Length);
                if (markerEndIndex <= 0)
                {
                    throw new ArgumentException(string.Format("Could not find the end of the marker. Expected '{0}' after {1}", markerEnd, markerBeginningIndex));
                }
                string data = text.Substring(markerBeginningIndex + markerBeginning.Length, markerEndIndex - (markerBeginningIndex + markerBeginning.Length));
                requests.Add(new Request() { Offset = builder.Length, Data = data });

                start = markerEndIndex + markerEnd.Length;
            }
            builder.Append(text, start, text.Length - start);

            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }

        /// <summary>
        /// Simulate a VS Language service request by first building a context cache for all context files, then performing the actual call
        /// </summary>
        internal void PerformVSPerformanceTest(string text, Action<IAuthorTestContext, int, string, int> action, params string[] contextFileNames)
        {

            var primaryFile = _session.FileFromText("");

            // Build the context cache
            List<IAuthorTestFile> contextFiles = new List<IAuthorTestFile>();
            foreach (var contextFileName in contextFileNames)
            {
                contextFiles.Add(_session.ReadFile(contextFileName));
            }
            var context = _session.OpenContext(primaryFile, contextFiles.ToArray());
            context.GetCompletionsAt(primaryFile.Text.Length);


            // Update the primaryFile equest completion on the file
            var offsets = ParseRequests(text);
            primaryFile.InsertText(0, offsets.Text);

            // Start the code markers listener
            InitializeCodeMarkers();

            // Perform the requests
            var i = 0;
            foreach (var request in offsets.Requests)
            {
                action(context, request.Offset, request.Data, i++);
            }
        }
        #endregion
    }
}