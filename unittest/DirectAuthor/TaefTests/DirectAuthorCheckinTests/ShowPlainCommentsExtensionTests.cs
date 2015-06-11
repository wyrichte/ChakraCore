//----------------------------------------------------------------------------------------------------------------------
// <copyright file="ShowPlainCommentsExtensionTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the ShowPlainCommentsExtensionTests type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class ShowPlainCommentsExtensionTests : DirectAuthorTest
    {
        private void Verify(string code, string desc, string itemName = "f", string[] paramComments = null)
        {
            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    var hint = completions.GetHintFor(itemName);
                    if (hint.Type == AuthorType.atFunction)
                    {
                        var funcHelp = hint.GetFunctionHelp();
                        var signature = funcHelp.GetSignatures().ToEnumerable().First();
                        signature.Description.Expect(desc);
                        if (paramComments != null)
                        {
                            var args = signature.GetParameters().ToEnumerable().ToArray();
                            for (int i = 0; i < args.Length; i++)
                            {
                                args[i].Description.Expect(paramComments[i]);
                            }
                        }
                    }
                    else
                    {
                        hint.Description.Expect(desc);
                    }
                }, false, Paths.ShowPlainCommentsPath);
            };

            VerifyImpl(code);
        }

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
        public void CompletionHint()
        {
            Verify(@"/// <reference path='script1.js' />


                    /// hello there

                    function f(a, b) {
                    }
                    ;|
                ", null);

            Verify(@"
                // comment
                var f = 1;
                ;|
                ", "comment");

            Verify(@"s
                // comment
                function f() {}
                ;|
                ", "comment");

            Verify(@"
                // comment
                function f() {
                    /// <summary>doc comment</summary>
                }
                ;|
                ", "doc comment");

            // Ignoring comments inside
            Verify(@"
                function f() {
                    // comment
                }
                ;|
                ", null);

            // Ignoring comments inside
            Verify(@"
                var x = {
                    f: function() {
                        // comment
                    }
                };
                x.|
                ", null);

            Verify(@"
                var x = {
                    // comment
                    f: function() {
                    }
                };
                x.|
                ", "comment");

            Verify(@"
                function f(/* a comment */ a, /* b comment */ b) {
                }
                ;|
                ", null, "f", new[] { "a comment", "b comment" });

            Verify(@"
                // comment
                function f(/* a comment */ a, /* b comment */ b) {
                }
                ;|
                ", "comment", "f", new[] { "a comment", "b comment" });

        }

        [TestMethod]
        public void Annotate()
        {
            Verify(@"
                var x = {
                    f: function() {}
                };
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                x.|;
                ", "comment");

            Verify(@"
                var x = {
                    f: 0
                };
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                x.|;
                ", "comment");

            Verify(@"
                var x = {
                    f: undefined
                };
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                x.|;
                ", "comment");
        }
    }
}