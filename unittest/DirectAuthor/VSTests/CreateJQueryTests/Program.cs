using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Xml;

namespace CreateJQueryTests
{
    class CreateJQueryTests
    {
        private const string SourceFile = @"http://api.jquery.com/api";
        private const string OutputFile = @"%SDXROOT%\inetcore\jscript\unittest\DirectAuthor\VSTests\DirectAuthorTests\JQueryGeneratedTests.cs";
        private const string FileHeader = @"using System;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorTests
{
    [TestClass]
    public class JQueryGeneratedTests : JQueryBase
    {
        /*
         *     Note: This file is auto-generated. If you need to change any of the tests, or their attributes
         *           please update the script that generates it at CreateJQueryTests\Program.cs
         */";
        private static string FileFooter = @"
    }
}";

        private StringBuilder outputText;
        private Dictionary<string, string[]> ignoredSamples;

        private static XmlElement GetFirstChildByName(XmlElement element, string childName)
        {
            if (element.HasChildNodes)
            {
                foreach (XmlElement child in element.ChildNodes)
                {
                    if (child.Name == childName)
                        return child;
                }
            }
            return null;
        }

        private void DumpFile(string sourceFileName, string outputFileName)
        {
            XmlDocument xml = new XmlDocument();
            xml.Load(sourceFileName);

            outputText.AppendLine(FileHeader);

            DumpEntries(xml);

            outputText.AppendLine(FileFooter);

            File.WriteAllText(Environment.ExpandEnvironmentVariables(outputFileName), outputText.ToString());
        }

        private void DumpEntries(XmlDocument document)
        {
            Dictionary<string, int> testCaseNames = new Dictionary <string, int>();
            XmlNodeList entries = document.SelectNodes("/api/entries/entry");
            int blockIndex = 0;
            int entryIndex = 0;
            foreach (XmlElement entry in entries)
            {
                entryIndex++;
                var entryName = entry.GetAttribute("name");
                var entryDescriptionNode = GetFirstChildByName(entry, "desc");
                int exampleIndex = 0;

                if (testCaseNames.ContainsKey(entryName))
                {
                    var index = testCaseNames[entryName]++;
                    entryName = entryName + '_' + index;
                }
                else
                {
                    testCaseNames.Add(entryName, 1);
                }

                foreach (XmlElement child in entry.ChildNodes)
                {
                    if (child.Name == "example")
                    {
                        exampleIndex++;
                        var testName = String.Format("{1}_Example{2}", entryIndex, entryName.Replace('.', '_').Replace('-', '_').Replace(' ', '_'), exampleIndex);
                        var exmapleDescriptionNode = GetFirstChildByName(child, "desc");
                        var code = GetFirstChildByName(child, "code").InnerText;
                        code = code.Replace("\"", "\"\"");

                        outputText.AppendLine();
                        outputText.AppendLine("        [TestMethod]");
                        outputText.AppendFormat("        public void {0}()", testName);
                        outputText.AppendLine();
                        outputText.AppendLine("        {");

                        outputText.AppendFormat("            // Test Block {0}.", ++blockIndex);
                        outputText.AppendLine();

                        if (entryDescriptionNode != null)
                        {
                            var description = entryDescriptionNode.InnerText.Replace('\n', ' ').Replace('\r', ' ');
                            outputText.AppendFormat("            // Entry {0}: {1}", entryName, description);
                            outputText.AppendLine();
                        }

                        if (exmapleDescriptionNode != null)
                        {
                            var description = exmapleDescriptionNode.InnerText.Replace('\n', ' ').Replace('\r', ' ');
                            outputText.AppendFormat("            // Example {0}: {1}", exampleIndex, description);
                            outputText.AppendLine();
                        }

                        var codeLines = code.Split('\n', '\r');
                        outputText.AppendLine("            PerformJQueryTest(@\"");
                        foreach (var line in codeLines)
                        {
                            if (line.Trim().Length == 0)
                                continue;
                            outputText.AppendFormat("                {0}", line);
                            outputText.AppendLine();
                        }
                        outputText.Append("            \"");
                        if (ignoredSamples.ContainsKey(testName))
                        {
                            foreach (var missingCompletion in ignoredSamples[testName])
                            {
                                outputText.AppendFormat(", \"{0}\"", missingCompletion);
                            }
                        }
                        outputText.AppendLine(");");
                        outputText.AppendLine("        }");
                    }
                }
            }
        }

        private CreateJQueryTests()
        {
            outputText = new StringBuilder();
            ignoredSamples = new Dictionary<string, string[]>() { 
                {"ajaxError_Example1", new[] {"url"}},                          // a send event was never triggered, this should be handeled in the extension
                {"ajaxSend_Example1", new[] {"url"}},                           // a send event was never triggered, this should be handeled in the extension
                {"contents_Example1", new[] {"nodeType"}},                      // contents return empty list as selected nodes have no children. This requiers having a complete dom simulation.
                {"data_Example1", new[] {"first","last"}},                      // stores data on a query and expects it to be there on another. This is not possible without having a complete dom simulation.
                {"deferred_pipe_Example3", new[] {"url","url2", "userId"}},     // undefined values
                {"deferred_promise_Example2", new[] {"defer"}},                 // defer is on the left hand side of an assigment statement
                {"end_Example1", new[] {"showTags"}},                           // showTags is on the left hand side of an assigment statement
                {"event_result_Example1", new[] {"result"}},                    // requires maintiang a valid dom returing the same nodes for the same queries
                {"event_namespace_Example1", new[] {"namespace"}},              // requires maintiang a valid dom returing the same nodes for the same queries
                {"jQuery_ajax_Example3", new[] {"create", "xml", "handleResponse"}}, // undefined values
                {"jQuery_ajaxSetup_Example1", new[] {"myData"}},                // undefined value
                {"jQuery_browser_Example3", new[] {"webkit"}},                  // webkit specific
                {"jQuery_browser_Example4", new[] {"mozilla"}},                 // mozilla specific
                {"jquery_Example1", new[] {"jquery"}},                          // undefined value
                {"jQuery_Example3", new[] {"xml","responseXML"}},               // undefined values
                {"jQuery_Example5", new[] {"myForm","elements"}},               // undefined values
                {"jQuery_fx_off_Example1", new[] {"off", "off"}},               // undefined value
                {"jQuery_get_Example6", new[] {"name","time"}},                 // undefined values
                {"jQuery_getJSON_Example2", new[] {"name","users"}},            // undefined values
                {"jQuery_getJSON_Example3", new[] {"name","users"}},            // undefined values
                {"keydown_Example1", new[] {"print", "print"}},                 // undefined function print
                {"keypress_Example1", new[] {"print", "print"}},                // undefined function print
                {"keyup_Example1", new[] {"print", "print"}},                   // undefined function print
                {"map_Example3", new[] {"equalizeHeights"}},                    // undefined value
                {"jQuery_getJSON_Example1", new[] {"items", "media", "m" , "appendTo"}}, // undefined values
                {"jQuery_noConflict_Example1", new[] {"style", "display"}},     // $ is not used by JQuery. 
                {"jQuery_noConflict_Example4", new[] {"style", "display"}},     // $ is not used by JQuery. 
                {"jQuery_noConflict_Example5", new[] {"query"}},                // query is on the left hand side of an assigment statement
                {"jQuery_post_Example7", new[] {"process"}},                    // undefined value
                {"jQuery_post_Example8", new[] {"name","time"}},                // undefined values
                {"jQuery_sub_Example1", new[] {"myCustomMethod", "myCustomMethod"}}, // undefined values
                {"jQuery_sub_Example3", new[] {"myplugin", "hide"}},            // close function never called, so this is never set correctelly
                {"jQuery_when_Example1", new[] {"responseText"}},               // requires async request script
                {"jQuery_when_Example2", new[] {"myFunc", "myFailure"}},        // undefined values
                {"serializeArray_Example1", new[] {"value"}},                   // No element with class "input" is returned in getElementsByTagName('*'). This requiers having a complete dom simulation.
                {"submit_Example1", new[] {"nodeName"}},                        // Children() returns an empty array as the node has no children
                {"submit_1_Example2", new[] {"some_flag_variable"}},            // undefined value
                {"toggleClass_Example3", new[] {"className", "className"}},     // Children() returns an empty array as the node has no children
                {"trigger_Example5", new[] {"user","pass"}},

                // unknown (originally blamed on Bug 317326)
                {"live_Example3", new[] {"preventDefault"}},
                {"on_Example10", new[] {"preventDefault"}},
                {"delegate_Example4", new[] {"preventDefault"}},
                {"event_delegateTarget_Example1", new[] {"delegateTarget"}},
                {"jQuery_queue_Example1", new[] {"length"}},
            };
        }

        static void Main(string[] args)
        {
            CreateJQueryTests createJQueryTests = new CreateJQueryTests();
            createJQueryTests.DumpFile(SourceFile, OutputFile);
        }
    }
}