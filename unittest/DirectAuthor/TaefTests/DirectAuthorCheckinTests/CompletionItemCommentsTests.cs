//----------------------------------------------------------------------------------------------------------------------
// <copyright file="CompletionItemCommentsTests.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the CompletionItemCommentsTests type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class CompletionItemCommentsTests : DirectAuthorTest
    {
        #region Verify
        private void Verify(string code, string commentBefore, string funcComment, bool func = true, string itemName = "f", string[] contextFiles = null)
        {
            string extension = @"
                intellisense.addEventListener('statementcompletionhint', function (e) { 
                        intellisense.logMessage('e.completionItem.comments: ' + e.completionItem.name + ': ' + e.completionItem.comments);
                        if (typeof e.completionItem.value == 'function') {
                            var c = intellisense.getFunctionComments(e.completionItem.value);
                            var inside = c.inside || '';
                            intellisense.logMessage('getFunctionComments: ' + inside);
                        }
                    });
                ";

            Action<string> VerifyImpl = (text) =>
            {
                PerformRequests(text, (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    var hint = completions.GetHintFor(itemName);
                    EnsureMessageLogged(context, "e.completionItem.comments: f: " + commentBefore);
                    if (func)
                    {
                        EnsureMessageLogged(context, "getFunctionComments: " + funcComment);
                    }
                }, contextFiles != null ? contextFiles.Concat(extension).ToArray() : new[] { extension });
            };

            // Global scope
            VerifyImpl(code);
            // Function scope
            VerifyImpl(@"
                    function g() {
                    " +
                code +
                @"
                    }
                    ");

            // With a comment before
            if (commentBefore != "")
            {
                VerifyImpl(@"
                    function g() {
                        // outer comment
                    " +
                    code +
                    @"
                }
                ");
            }

            // With additional extension before.
            // Verify that additional extension doesn't mess anything during data roundtripping.            
            var precedingExtension = @"
                intellisense.addEventListener('statementcompletion', function (e) { var items = e.items; });
                intellisense.addEventListener('statementcompletionhint', function (e) { var item = e.completionItem; });
            ";
            VerifyImpl(precedingExtension + code);
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
        public void Annotate()
        {
            Action<string, string, string, string, bool> VerifyAnnotate = (code, annotate, commentBefore, funcComment, func) =>
            {
                // Verify when annotate is in the same file
                Verify(code + annotate + @";x.|;", commentBefore, funcComment, func);
                // Verify when annotate is in a context file
                Verify(@";x.|;", commentBefore, funcComment, func, "f", new string[] { code, annotate });
            };

            // Primitive field value annotated, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: 0
                };",
                @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", false);

            // Object field value annotated, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: {}
                };",
                @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", false);

            // Primitive field annotation, use a function as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: 0
                };",
                   @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "", false);

            // Annotate a member function, use a function as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: function() {}
                };", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "comment inside", true);

            // Annotate a member function, use undefined as annotation value.
            VerifyAnnotate(@"
                var x = {
                    f: function() {}
                };
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: undefined
                });
                ", "comment", "", true);

            // Annotate a this assignment function, use a function as annotation value.
            VerifyAnnotate(@"
                function X() {
                    /* func comment */
                    this.f = function() {
                    };
                }
                var x = new X();
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() {
                        // comment inside
                    }
                });
                ", "comment", "comment inside", true);

            // Annotate a property assignment
            VerifyAnnotate(@"
                var x = {};
                // unwanted
                x.f = function() { /* unwanted */ };
                ", @"
                intellisense.annotate(x, {
                    // comment
                    f: function() { /* comment inside */ }
                });
                ", "comment", "comment inside", true);
        }

        [TestMethod]
        public void ThisAssignmentComments()
        {
            Verify(@"
                function X() {
                    /* func comment */
                    this.f = function() {
                    };
                }
                var x = new X();
                x.|;
            ",
             "func comment", "", true);
        }

        [TestMethod]
        public void Trimming()
        {
            // leading spaces after new lines
            Verify("/*\r\n \r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", "\r\n a \r\n a", "", true);

            // empty, 1 line
            Verify(@"
                //
                function f() {}
                ;|;
            ", "", "", true);

            // empty, 2 lines
            Verify(@"
                //
                //
                function f() {}
                ;|;
            ", "", "", true);

            // no space
            Verify(@"
                /**/
                function f() {}
                ;|;
            ", "", "", true);

            // 1 space
            Verify(@"
                /* */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 spaces
            Verify(@"
                /*  */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 spaces
            Verify(@"
                /*   */
                function f() {}
                ;|;
            ", "", "", true);

            // 1 tab
            Verify(@"
                /*  */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 tabs
            Verify(@"
                /*      */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 tabs
            Verify(@"
                /*          */
                function f() {}
                ;|;
            ", "", "", true);

            // 4 tabs
            Verify(@"
                /*              */
                function f() {}
                ;|;
            ", "", "", true);

            // 1 new line
            Verify(@"
                /*
                */
                function f() {}
                ;|;
            ", "", "", true);

            // 2 new lines
            Verify(@"
                /*
    
                */
                function f() {}
                ;|;
            ", "", "", true);

            // 3 new lines
            Verify(@"
                /*
    

                */
                function f() {}
                ;|;
            ", "", "", true);

            // no spaces, 1 char
            Verify(@"
                /*a*/
                function f() {}
                ;|;
            ", "a", "", true);

            // no spaces, 2 chars
            Verify(@"
                /*aa*/
                function f() {}
                ;|;
            ", "aa", "", true);

            // no spaces, 3 chars
            Verify(@"
                /*aaa*/
                function f() {}
                ;|;
            ", "aaa", "", true);

            // 1 leading space
            Verify(@"
                /* a*/
                function f() {}
                ;|;
            ", "a", "", true);

            // 2 leading spaces
            Verify(@"
                /*  a*/
                function f() {}
                ;|;
            ", " a", "", true);

            // 1 leading new line
            Verify("/*\r\na*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 leading new lines
            Verify("/*\r\n\r\na*/" +
                @"function f() {}
                ;|;
            ", "\r\na", "", true);

            // 3 leading new lines
            Verify("/*\r\n\r\n\r\na*/" +
                @"function f() {}
                ;|;
            ", "\r\n\r\na", "", true);

            // 1 trailing space
            Verify("/*a */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 trailing space
            Verify("/*a  */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 1 trailing new line
            Verify("/*a\r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // 2 trailing new line
            Verify("/*a\r\n\r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // trailing new line and space
            Verify("/*a\r\n */" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // trailing space and new line
            Verify("/*a \r\n*/" +
                @"function f() {}
                ;|;
            ", "a", "", true);

            // leading spaces after new lines
            Verify("/*\r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", " a \r\n a", "", true);

            // leading spaces after new lines
            Verify("/*\r\n \r\n  a \r\n  a\r\n*/" +
                @"function f() {}
                ;|;
            ", "\r\n a \r\n a", "", true);
        }

        [TestMethod]
        public void PropertyAssignmentComments()
        {
            Verify(@"
                var x = {
                    f: undefined
                };
                
                // f comment
                x.f = function() {};

                x.|;
            ",
             "f comment", "", true);

            Verify(@"
                var x = {
                    // field comment
                    f: undefined
                };
                
                // unwanted
                x.f = function() {};

                x.|;
            ",
             "field comment", "", true);

            Verify(@"
                var x = {};
                
                // line 1
                x.f = function() {};

                x.|;
            ",
             "line 1", "", true);

            Verify(@"
                function X() {
                    var x = {};
                
                    // line 1
                    x.f = function() {};

                    x.|;
                }
            ",
             "line 1", "", true);
        }

        [TestMethod]
        public void ParamComments()
        {
            Verify(@"
                function X(/* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(a, /* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(/* unwanted */ a, /* comment */ f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            Verify(@"
                function X(a,
                // comment 
                f) {
                  ;|  
                }
            ",
             "comment",
             "", false);

            // f is a func and has a comment above -  ?
        }

        [TestMethod]
        public void MixedFieldComments()
        {
            Verify(@"
                var x = {
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    // unwanted 
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /// <field>should be ignored</field> 
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // line 1
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // above
                    f: function() {
                        // line 1
                        // line 1 cont
                    }
                };
                x.|;
            ",
             "above",
             "line 1\r\nline 1 cont", true);

            Verify(@"
                var x = {
                    /* unwanted */
                    // above
                    f: function() {
                        ;
                        // should be ignored
                    }
                };
                x.|;
            ",
             "above",
             "", true);

            Verify(@"
                var x = {
                    /* unwanted*/
                    a: function() {},
                    /* line 1*/
                    f: function() {}
                };
                x.|;
            ",
             "line 1",
             "", true);

            Verify(@"
                var x = {
                    /* line 1*/
                    f: undefined
                };
                x.|;
            ", "line 1", "", false);

            Verify(@"
                var x = {
                    /// unwanted
                    /* line 1*/
                    f: undefined
                };
                x.|;
            ", "line 1", "", false);

            Verify(@"
                var x = {
                    /// unwanted
                    a: undefined,
                    /// unwanted
                    /* line 1*/
                    f: {
                        // unwanted
                    }
                };
                x.|;
            ", "line 1", "", false);
        }

        [TestMethod]
        public void FieldOfPrimitiveValue()
        {
            Verify(@"
                var x = {
                    /// unwanted
                    /* line 1*/
                    f: 0
                };
                x.|;
            ", "line 1", "", false);
        }

        [TestMethod]
        public void VarOfFunctionValue()
        {
            Verify(@"
                var x = {};
                /// a func comment
                function z() {
                    /// z func comment
                    var inner = function inner() {}
                }
            
                /// a func comment
                x.a = function a()  {
                    /// a func comment
                    var inner = function inner() {}
                };

                /// a func comment
                x.b = function b()  {
                    /// b func comment
                    var inner = function inner() {}
                };

                /// a func comment
                x.f = function f()  {
                    /// a func comment
                };

                x.f|
            ",
             "a func comment", "a func comment", true);

            Verify(@"
                // func comment
                function x() {};
                var f=x;
                ;|;
            ",
             "func comment",
             "", true);

            // Should use var comment
            Verify(@"
                // unwanted
                function x() {};
                // var comment
                var f=x;
                ;|;
            ",
             "var comment",
             "", true);
        }

        [TestMethod]
        public void VarOfPrimitiveValue()
        {
            Verify(@"
                // var comment
                var f=0;
                ;|;
            ",
             "var comment", "", false);

            Verify(@"
                /// unwanted
                // var comment
                var f='';
                ;|;
            ",
             "var comment", "", false);
        }

        [TestMethod]
        public void MixedVarComments()
        {
            // Empty line prevents comment blocks to be considered adjacent
            Verify(@"
                // unwanted
                /* unwanted block*/ 

                /* line 1*/ var f;
                /* unwanted*/ var x; /* unwanted*/  
                ;|;
            ",
             "line 1", "", false);

            Verify(@"
                // unwanted
                var x = { /* unwanted */ } /* unwanted */;
                /* line 1*/ var f = {
                    /* unwanted */ 
                };
                ;|;
            ",
            "line 1",
            "", false);

            Verify(@"
                // line 1
                var f = function() {};
                ;|;
            ",
             "line 1",
             "", true);

            Verify(@"
                // unwanted
                /* line 1*/ var f = function() {
                    /* inside */ 
                };
                ;|;
            ",
             "line 1",
             "inside", true);


            Verify(@"
                /* unwanted */
                // line 1
                // line 1 cont
                var f;
                ;|;
            ",
             "line 1\r\nline 1 cont",
             "", false);

            Verify(@"
                // unwanted
                /* line 1
                   line 1 cont */
                var f;
                ;|;
            ",
             "line 1\r\n                  line 1 cont",
             "", false);

            Verify(@"
                /* line 1*/ var f;
                ;|;
            ",
             "line 1",
             "", false);

            Verify(@"
                // unwanted
                /* line 1*/ var f;
                ;|;
            ",
             "line 1",
             "", false);
        }

        [TestMethod]
        public void NonAdjacentSimpleComments()
        {
            Verify(@"
                var x = {
                    // comment above
                    f:                     
                    function() {
                        // some comment inside
                    }
                };
                var f = x.f;
                ;|
            ",
             "comment above",
             "some comment inside");

            Verify(@"
                // some comment above

                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "",
             "some comment inside");

            Verify(@"
                /* some comment above */

                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "",
             "some comment inside");
        }

        [TestMethod]
        public void MixedFuncComments()
        {
            Verify(@"
/* before */
/* aaabbbb
*/
/* after */
            function f() {}
            ;|;
            ",
             "before \r\naaabbbb\r\n\r\nafter",
             "");

            Verify(@"
                function f() {
                    // a
                    /* */
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                }
                ;|;
            ",
             "",
             "a");

            Verify(@"
                function f() {
                    // a

                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                    //  unwanted
                }
                ;|;
            ",
             "",
             "a");

            Verify(@"
                function f() {
                    // a
                    // a
                    // a
                    // a
                    /* unwanted 
                       unwanted
                       unwanted
                       unwanted
                    */
                    function f1() {}   
                }
                ;|;
            ",
             "",
             "a\r\na\r\na\r\na");

            Verify(@"
                /* unwanted */
                // line 1
                // line 1 cont
                function f() {
                }
                ;|;
            ",
             "line 1\r\nline 1 cont",
             "");

            Verify(@"
                // unwanted
                /// comment above
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                /* unwanted
                   unwanted 
                */
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                /// unwanted
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // comment above
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "comment above",
             "line 1\r\nline 1 cont");

            Verify(@"
                // line 1
                function f() {}
                ;|;
            ",
             "line 1",
             "");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                    /// line 2
                    /* line 3 */
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                    /* line 3 */
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                function f() {
                    // line 1
                    // line 1 cont
                }
                ;|;
            ",
             "",
             "line 1\r\nline 1 cont");

            Verify(@"
                // some comment
                function f() {
                    /// <signature/>
                }
                ;|;
            ",
             "some comment",
             "<signature/>");

            Verify(@"
                // some comment above
                function f() {
                    // some comment inside
                }
                ;|;
            ",
             "some comment above",
             "some comment inside");
        }
    }
}