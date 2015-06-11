//----------------------------------------------------------------------------------------------------------------------
// <copyright file="AccessingCommentsInSignatureHelpEvent.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the AccessingCommentsInSignatureHelpEvent type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class AccessingCommentsInSignatureHelpEvent : DirectAuthorTest
    {
        #region Verify

        class ParamComment
        {
            public string Name { get; set; }
            public string Comment { get; set; }
        }

        private void Verify(string code, string commentAbove, string commentInside, ParamComment[] paramComments = null)
        {
            string extension = @"
                intellisense.addEventListener('signaturehelp', function (e) {
                        printComments(e.functionComments, 'e.functionComments.');
                        printComments(intellisense.getFunctionComments(e.target), 'intellisense.getFunctionComments(e.target).');
                        function printComments(comments, prefix) {
                            intellisense.logMessage(prefix + 'above: ' + comments.above);
                            intellisense.logMessage(prefix + 'inside: ' + comments.inside);
                            comments.paramComments.forEach(function(p) {
                                intellisense.logMessage(prefix + 'param: ' + p.name + ' comment: ' + p.comment);
                            });
                        }
                    });
                ";

            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    Action<string> EnsureMessages = (prefix) =>
                    {
                        EnsureMessageLogged(context, prefix + "above: " + commentAbove);
                        EnsureMessageLogged(context, prefix + "inside: " + commentInside);
                        if (paramComments != null)
                        {
                            foreach (var paramComment in paramComments)
                            {
                                EnsureMessageLogged(context, prefix + "param: " + paramComment.Name + " comment: " + paramComment.Comment);
                            }
                        }
                    };
                    var funcHelp = context.GetParameterHelpAt(offset);
                    EnsureMessages("e.functionComments.");
                    EnsureMessages("intellisense.getFunctionComments(e.target).");
                }, extension);
            };

            // Global scope
            VerifyImpl(code);

            // Function scope
            VerifyImpl(@"function g() {" + code + @"}");
        }
        #endregion

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
        public void MemberFunction()
        {
            Verify(@"
                var x = {
                    /* comment above */
                    f: function() { /* comment inside */ }                        
                };
                x.f(|);
            ",
             "comment above", "comment inside");
        }

        [TestMethod]
        public void FuncDeclaration()
        {
            Verify(@"
                function f() {}
                f(|);
            ",
             "", "");

            Verify(@"
                /* func comment */
                function f() {}
                f(|);
            ",
             "func comment", "");

            Verify(@"
                // above
                function f() {
                    // inside
                }
                f(|);
            ",
             "above", "inside");

            Verify(@"
                // above1
                // above2
                function f() {
                    // inside1
                    // inside2
                }
                f(|);
            ",
             "above1\r\nabove2", "inside1\r\ninside2");

            Verify(@"
                function f(/*a*/ a) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a" } });

            Verify(@"
                function f(//a
                           a) {}
                f(|);
            ",
           "", "", new[] { new ParamComment { Name = "a", Comment = "a" } });

            Verify(@"
                function f(/* a comment */ a, /* abc comment */ abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a comment" }, new ParamComment { Name = "abc", Comment = "abc comment" } });

            Verify(@"
                function f(/* a comment */ a, abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "a comment" }, new ParamComment { Name = "abc", Comment = "" } });

            Verify(@"
                function f(a, /* abc comment */ abc) {}
                f(|);
            ",
             "", "", new[] { new ParamComment { Name = "a", Comment = "" }, new ParamComment { Name = "abc", Comment = "abc comment" } });
        }

        [TestMethod]
        public void FuncVariable()
        {
            Verify(@"
                // above
                var f = function() {}
                f(|);
            ", "above", "");

            Verify(@"
                var f = function () {}
                f(|);
            ", "", "");

            Verify(@"
                // above
                var f = function(/*a*/ a) { /* inside */ }
                f(|);
            ",
             "above", "inside", new[] { new ParamComment { Name = "a", Comment = "a" } });
        }

        [TestMethod]
        public void Annotate()
        {
            // Make sure we get all the annotation comments in function help extension handler
            Verify(@"
                var x = { f: function() {} };
                intellisense.annotate(x, {
                    /* comment above */
                    f: function(/*a*/ a) { /* comment inside */ }
                });
                x.f(|);
            ",
             "comment above", "comment inside", new[] { new ParamComment { Name = "a", Comment = "a" } });
        }

        [TestMethod]
        [WorkItem(364466)]
        public void CommentAssociation()
        {
            Verify(@"
                (function () {
                    // Comment: This should not be associated with the declaration below
                })();

                var foo = function () {};
                foo(|);",
                "", "");

            Verify(@"
                var obj = {};
                (function () {
                    // Comment: This should not be associated with the assignment below
                })();

                obj.foo = function () {};
                obj.foo(|);",
                "", "");
        }
    }
}