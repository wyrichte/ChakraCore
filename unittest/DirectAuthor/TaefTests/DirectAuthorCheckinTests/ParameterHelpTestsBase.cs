//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ParameterHelpTestsBase.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the ParameterHelpTestsBase type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System.Collections.Generic;
    using System.Linq;
    using Microsoft.BPT.Tests.DirectAuthor;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class ParameterHelpTestsBase : DirectAuthorTest
    {
        protected IEnumerable<AuthorParameterHelp> CollectParameterRequests(string text)
        {
            var results = new List<AuthorParameterHelp>();
            PerformRequests(text, (context, offset, data, index) =>
            {
                var help = context.GetParameterHelpAt(offset);
                results.Add(help);
            }, AdditionalContextFiles);
            return results.ToArray();
        }

        protected void ValidateParameterHelpAvailable(string code, params string[] expectedParams)
        {
            PerformParameterRequests(code, (help, data, index) =>
            {
                Assert.IsNotNull(help.FunctionHelp);
                if (expectedParams.Length > 0)
                {
                    var signature = help.FunctionHelp.GetSignatures().ToEnumerable().Single();
                    AssertAreStructurallyEqual(expectedParams, signature.GetParameters().ToEnumerable().Select(p => p.Name).ToArray());
                }
            });
        }

        protected void ValidateCurrentParameterIndex(string code, bool expectFunctionHelp)
        {
            PerformParameterRequests(code,
            (help, data, index) =>
            {
                if (expectFunctionHelp)
                {
                    Assert.IsNotNull(help.FunctionHelp);
                    if (data == "x")
                    {
                        help.ParameterIndex.Expect(-1);
                    }
                    else
                    {
                        int expectedIndex = int.Parse(data);
                        help.ParameterIndex.Expect(expectedIndex);
                    }
                }
                else
                {
                    Assert.IsNull(help.FunctionHelp);
                    help.ParameterIndex.Expect(0);
                }
            });
        }

        protected void VerifyFunctionDocComments(string text, string expectedDocComments)
        {
            var requests = CollectParameterRequests(text);
            requests.Expect(expectedDocComments);
        }

        protected string[] CombinedContextFiles(string[] contextFiles)
        {
            var allContextFiles = new List<string>(contextFiles);
            allContextFiles.AddRange(AdditionalContextFiles);
            return allContextFiles.ToArray();
        }
    }

    public static class ParameterHelpHelpers
    {
        public static string DumpParameterHelp(this IEnumerable<AuthorParameterHelp> requests)
        {
            return DirectAuthorTest.DumpObject(requests, (name) => { return name != "SourceFileHandle"; });
        }

        public static void Expect(this IEnumerable<AuthorParameterHelp> parameterHelp, string expect)
        {
            var actual = parameterHelp.DumpParameterHelp();
            Assert.AreEqual(expect, actual);
        }

        public static void ExpectFunction(this IEnumerable<AuthorParameterHelp> parameterHelp, string expectFunctionName)
        {
            bool found = false;
            foreach (var request in parameterHelp)
            {
                if (expectFunctionName == request.FunctionHelp.FunctionName)
                {
                    found = true;
                    break;
                }
            }
            Assert.IsTrue(found, string.Format("Parameter help for function: '{0}' was not found.", expectFunctionName));
        }
    }

}