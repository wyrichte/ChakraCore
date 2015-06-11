using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace DirectAuthorTests
{
    [TestClass]
    public class JQueryTests : PerformanceTests
    {
        protected static readonly string dom_jsFileLocation = Paths.DomWebPath;
        protected static readonly string jQueryFileLocation = Paths.JQuery.JQuery_1_2_6VSDocFilePath;
        protected static readonly string jQueryErrorsFileLocation = Path.Combine(Paths.FilesPath, @"JQuery\jquery-1.2.6-vsdoc_error.js");

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var requests = PerformGetParameterHelp("window.$.apply(_|r|);", dom_jsFileLocation, jQueryFileLocation);
            requests.ExpectFunction("apply");
        }

        [TestMethod]
        public void GetFunctionHelpSecondRequest()
        {
            var requests = this.PerformGetParameterHelp("window.$.each(_|s|); \nwindow.$.apply(_|r|);", dom_jsFileLocation, jQueryFileLocation);
            requests.ExpectFunction("apply");
        }

        [TestMethod]
        public void GetRegionsFirstRequest()
        {
            var result = PerformGetRegions(jQueryFileLocation, 1, 1);
            result.Contains(JQueryGetRegionsExpected);
        }

        [TestMethod]
        public void GetRegionsSecondRequest()
        {
            var result = PerformGetRegions(jQueryFileLocation, 2, 2);
            result.Contains(JQueryGetRegionsExpected);
        }
        #region Test data
        const string JQueryGetRegionsExpected = @"
    new { Offset = 185076, Length = 458 }, 
    new { Offset = 185598, Length = 1002 }, 
    new { Offset = 186466, Length = 115 }, 
    new { Offset = 186718, Length = 518 }, 
    new { Offset = 187143, Length = 80 }, 
    new { Offset = 187286, Length = 724 }, 
    new { Offset = 188058, Length = 724 }, 
    new { Offset = 188834, Length = 745 }, 
    new { Offset = 189626, Length = 728 }, 
    new { Offset = 190402, Length = 729 }, 
    new { Offset = 191182, Length = 732 }";
        #endregion

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var result = PerformGetCompletions("window.$._|r|", dom_jsFileLocation, jQueryFileLocation);
            result[0].ToEnumerable().ExpectContains(JQueryCompletionsExpected);
        }
        #region Test data
        static readonly string[] JQueryCompletionsExpected = new[] { "ajax", "each", "isXMLDoc", "makeArray", "find" };
        #endregion

        [TestMethod]
        public void GetCompletionsSecondRequest()
        {
            var result = PerformGetCompletions("window.$._|s|; window.$._|r|", dom_jsFileLocation, jQueryFileLocation);
            result[0].ToEnumerable().ExpectContains(JQueryCompletionsExpected);
        }

        [TestMethod]
        public void GetASTFirstRequest()
        {
            var result = PerformGetAST(jQueryFileLocation, 1, 1);
            result.ToString().CompactJSON().Contains(JQueryASTExpected.CompactJSON());
        }
        #region Test data
        const string JQueryASTExpected = @"{
    ""__type"": ""FuncDecl"",
    ""offset"": 30597,
    ""length"": 378,
    ""name"": ""evalScript"",
    ""arguments"": [
        ""i"",
        ""elem"" 
    ],
    ""body"": [
        {
            ""__type"": ""IfStmt"",
            ""offset"": 30718,
            ""length"": 186,
            ""condition"": {
                ""__type"": ""DotOper"",
                ""offset"": 30723,
                ""length"": 8,
                ""left"": {
                    ""__type"": ""NameExpr"",
                    ""offset"": 30723,
                    ""length"": 4,
                    ""value"": ""elem"" 
                },
                ""right"": {
                    ""__type"": ""NameExpr"",
                    ""offset"": 30728,
                    ""length"": 3,
                    ""value"": ""src"" 
                } 
            },
            ""then"": [
                {
                    ""__type"": ""CallExpr"",
                    ""offset"": 30737,
                    ""length"": 79,
                    ""target"": {
                        ""__type"": ""DotOper"",
                        ""offset"": 30737,
                        ""length"": 11,
                        ""left"": {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30737,
                            ""length"": 6,
                            ""value"": ""jQuery"" 
                        },
                        ""right"": {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30744,
                            ""length"": 4,
                            ""value"": ""ajax"" 
                        } 
                    },
                    ""args"": [
                        {
                            ""__type"": ""ObjectExpr"",
                            ""offset"": 30749,
                            ""length"": 66,
                            ""members"": [
                                {
                                    ""__type"": ""MemberOper"",
                                    ""offset"": 30755,
                                    ""length"": 13,
                                    ""target"": {
                                        ""__type"": ""StringLit"",
                                        ""offset"": 30755,
                                        ""length"": 3,
                                        ""value"": ""url"" 
                                    },
                                    ""member"": {
                                        ""__type"": ""DotOper"",
                                        ""offset"": 30760,
                                        ""length"": 8,
                                        ""left"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30760,
                                            ""length"": 4,
                                            ""value"": ""elem"" 
                                        },
                                        ""right"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30765,
                                            ""length"": 3,
                                            ""value"": ""src"" 
                                        } 
                                    } 
                                },
                                {
                                    ""__type"": ""MemberOper"",
                                    ""offset"": 30774,
                                    ""length"": 12,
                                    ""target"": {
                                        ""__type"": ""StringLit"",
                                        ""offset"": 30774,
                                        ""length"": 5,
                                        ""value"": ""async"" 
                                    },
                                    ""member"": {
                                        ""__type"": ""FalseLit"",
                                        ""offset"": 30781,
                                        ""length"": 5 
                                    } 
                                },
                                {
                                    ""__type"": ""MemberOper"",
                                    ""offset"": 30792,
                                    ""length"": 18,
                                    ""target"": {
                                        ""__type"": ""StringLit"",
                                        ""offset"": 30792,
                                        ""length"": 8,
                                        ""value"": ""dataType"" 
                                    },
                                    ""member"": {
                                        ""__type"": ""StringLit"",
                                        ""offset"": 30802,
                                        ""length"": 8,
                                        ""value"": ""script"" 
                                    } 
                                } 
                            ] 
                        } 
                    ] 
                } 
            ],
            ""else"": [
                {
                    ""__type"": ""CallExpr"",
                    ""offset"": 30830,
                    ""length"": 74,
                    ""target"": {
                        ""__type"": ""DotOper"",
                        ""offset"": 30830,
                        ""length"": 17,
                        ""left"": {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30830,
                            ""length"": 6,
                            ""value"": ""jQuery"" 
                        },
                        ""right"": {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30837,
                            ""length"": 10,
                            ""value"": ""globalEval"" 
                        } 
                    },
                    ""args"": [
                        {
                            ""__type"": ""LogOrOper"",
                            ""offset"": 30848,
                            ""length"": 55,
                            ""left"": {
                                ""__type"": ""LogOrOper"",
                                ""offset"": 30849,
                                ""length"": 47,
                                ""left"": {
                                    ""__type"": ""LogOrOper"",
                                    ""offset"": 30849,
                                    ""length"": 29,
                                    ""left"": {
                                        ""__type"": ""DotOper"",
                                        ""offset"": 30849,
                                        ""length"": 9,
                                        ""left"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30849,
                                            ""length"": 4,
                                            ""value"": ""elem"" 
                                        },
                                        ""right"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30854,
                                            ""length"": 4,
                                            ""value"": ""text"" 
                                        } 
                                    },
                                    ""right"": {
                                        ""__type"": ""DotOper"",
                                        ""offset"": 30862,
                                        ""length"": 16,
                                        ""left"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30862,
                                            ""length"": 4,
                                            ""value"": ""elem"" 
                                        },
                                        ""right"": {
                                            ""__type"": ""NameExpr"",
                                            ""offset"": 30867,
                                            ""length"": 11,
                                            ""value"": ""textContent"" 
                                        } 
                                    } 
                                },
                                ""right"": {
                                    ""__type"": ""DotOper"",
                                    ""offset"": 30882,
                                    ""length"": 14,
                                    ""left"": {
                                        ""__type"": ""NameExpr"",
                                        ""offset"": 30882,
                                        ""length"": 4,
                                        ""value"": ""elem"" 
                                    },
                                    ""right"": {
                                        ""__type"": ""NameExpr"",
                                        ""offset"": 30887,
                                        ""length"": 9,
                                        ""value"": ""innerHTML"" 
                                    } 
                                } 
                            },
                            ""right"": {
                                ""__type"": ""StringLit"",
                                ""offset"": 30900,
                                ""length"": 2,
                                ""value"": """" 
                            } 
                        } 
                    ] 
                } 
            ] 
        },
        {
            ""__type"": ""IfStmt"",
            ""offset"": 30910,
            ""length"": 61,
            ""condition"": {
                ""__type"": ""DotOper"",
                ""offset"": 30915,
                ""length"": 15,
                ""left"": {
                    ""__type"": ""NameExpr"",
                    ""offset"": 30915,
                    ""length"": 4,
                    ""value"": ""elem"" 
                },
                ""right"": {
                    ""__type"": ""NameExpr"",
                    ""offset"": 30920,
                    ""length"": 10,
                    ""value"": ""parentNode"" 
                } 
            },
            ""then"": [
                {
                    ""__type"": ""CallExpr"",
                    ""offset"": 30936,
                    ""length"": 35,
                    ""target"": {
                        ""__type"": ""DotOper"",
                        ""offset"": 30936,
                        ""length"": 27,
                        ""left"": {
                            ""__type"": ""DotOper"",
                            ""offset"": 30936,
                            ""length"": 15,
                            ""left"": {
                                ""__type"": ""NameExpr"",
                                ""offset"": 30936,
                                ""length"": 4,
                                ""value"": ""elem"" 
                            },
                            ""right"": {
                                ""__type"": ""NameExpr"",
                                ""offset"": 30941,
                                ""length"": 10,
                                ""value"": ""parentNode"" 
                            } 
                        },
                        ""right"": {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30952,
                            ""length"": 11,
                            ""value"": ""removeChild"" 
                        } 
                    },
                    ""args"": [
                        {
                            ""__type"": ""NameExpr"",
                            ""offset"": 30964,
                            ""length"": 6,
                            ""value"": ""elem"" 
                        } 
                    ] 
                } 
            ],
            ""else"": [
                
            ] 
        } 
    ]
}";
        #endregion

        [TestMethod]
        public void GetASTSecondRequest()
        {
            var result = PerformGetAST(jQueryFileLocation, 2, 2);
            result.ToString().CompactJSON().Contains(JQueryASTExpected.CompactJSON());
        }

        [TestMethod]
        public void GetASTCursorFirstRequest()
        {
            PerformGetASTCursor(jQueryFileLocation, 1, 1);
        }

        [TestMethod]
        public void GetASTCursorSecondRequest()
        {
            PerformGetASTCursor(jQueryFileLocation, 2, 2);
        }

        [TestMethod]
        public void GetASTSubTree()
        {
            var subtree = PerformGetASTSubTree(jQueryFileLocation, 1, 1);
            subtree[0].ExpectContains(GetASTSubTreeExpected);
        }
        #region Test data
        static readonly AuthorParseNodeDetails[] GetASTSubTreeExpected = new AuthorParseNodeDetails[] {
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkFncDecl, StartOffset = 1880, EndOffset = 3055},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkVarDecl, StartOffset = 1890, EndOffset = 1898},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkVarDecl, StartOffset = 1900, EndOffset = 1907},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkList, StartOffset = 3003, EndOffset = 3055},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkReturn, StartOffset = 3003, EndOffset = 3047},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkNew, StartOffset = 3010, EndOffset = 3047},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkDot, StartOffset = 3014, EndOffset = 3028},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkDot, StartOffset = 3014, EndOffset = 3023},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkName, StartOffset = 3014, EndOffset = 3020},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkName, StartOffset = 3021, EndOffset = 3023},
            new AuthorParseNodeDetails { Kind = AuthorParseNodeKind.apnkName, StartOffset = 3024, EndOffset = 3028},
        };
        #endregion

        [TestMethod]
        public void GetMessagesFirstRequest()
        {
            var result = PerformGetMessages(jQueryErrorsFileLocation, 1, 1);
            result.Contains(JQueryMessagesExpected);
        }
        #region Test data
        const string JQueryMessagesExpected = @"new [] { 
    new { Kind = AuthorMessageKind.amkError, Position = 329, Length = 4, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 337, Length = 1, Message = ""Expected '/'"", MessageID = 1012 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1690, Length = 4, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1698, Length = 1, Message = ""Expected '/'"", MessageID = 1012 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 3110, Length = 3, Message = ""The use of a keyword for an identifier is invalid"", MessageID = 1048 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 3123, Length = 5, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 3329, Length = 5, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 3470, Length = 5, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 4647, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 4744, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 6377, Length = 3, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 6396, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 6416, Length = 1, Message = ""Expected '}'"", MessageID = 1009 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 6635, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 6645, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 7091, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 7219, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 7727, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 7959, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 8565, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 8743, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 9405, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 9508, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 10091, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 10111, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 11165, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 11191, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 11253, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 11885, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 11980, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 12000, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 12835, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 12858, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 13993, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 14018, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 14447, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 14467, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 15352, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 15374, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 15909, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 15932, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 16467, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 16489, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 16880, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 16901, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 17301, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 17320, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 17792, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 17812, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 18612, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 18633, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 20687, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 20709, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 21349, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 21368, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 22317, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 22336, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 22375, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 22864, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 22882, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 23513, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 23537, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 23942, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 23961, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26037, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26057, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26517, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26536, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26605, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26973, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 26991, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 27423, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 27444, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 27468, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28088, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28107, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28362, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28385, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28566, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 28586, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 29436, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 29462, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 29770, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 29795, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 31360, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 31387, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 31562, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 31658, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 32083, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 33008, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 33013, Length = 3, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 33029, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 34402, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 34516, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 38930, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 40236, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 45914, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 52992, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 54943, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 55673, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 55680, Length = 1, Message = ""Expected '}'"", MessageID = 1009 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 55749, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 56303, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 56326, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 56903, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 56924, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 57911, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 57933, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 58514, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 58589, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 58609, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 59627, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 59702, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 59721, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 60792, Length = 1, Message = ""Syntax error"", MessageID = 1002 }
}";
        #endregion

        [TestMethod]
        public void GetMessagesSecondRequest()
        {
            var result = PerformGetMessages(jQueryErrorsFileLocation, 2, 2);
            result.Contains(JQueryMessagesExpected);
        }

        [TestMethod]
        public void ConcatenatedJQueryAfterDot()
        {
            // How many concatenated jQuery does it take to exceed 1 second?
            var jqueryFile = _session.ReadFile(jQueryFileLocation);
            var jqueryText = jqueryFile.Text;

            var dom = _session.ReadFile(dom_jsFileLocation);
            var file = _session.FileFromText("");
            var length = 0;
            var context = _session.OpenContext(file);

            long time = 0;
            int i = 0;
            for (; i < 100; i++)
            {
                file.InsertText(length, jqueryText);
                length += jqueryText.Length;

                file.InsertText(length, "\n$.");
                {
                    var completions = context.GetCompletionsAt(length + 3, AuthorCompletionFlags.acfMembersFilter, out time);
                    Marshal.ReleaseComObject(completions);
                    completions = null;
                }
                Console.WriteLine("### Copies: {0}, Time: {1}, Bytes: {2}, Bytes per second {3}",
                    i, time, length, length / (time / 1000.0));
                if (time > 2000) break;
            }

        }

        [TestMethod]
        [Ignore]
        public void ConcatenatedJQueryGlobalScope()
        {
            // How many concatenated jQuery does it take to exceed 1 second?
            var jqueryFile = _session.ReadFile(jQueryFileLocation);
            var jqueryText = jqueryFile.Text;

            var dom = _session.ReadFile(dom_jsFileLocation);
            var file = _session.FileFromText("");
            var length = 0;
            var context = _session.OpenContext(file);

            long time = 0;
            int i = 0;
            for (; i < 100; i++)
            {
                file.InsertText(length, jqueryText);
                length += jqueryText.Length;

                file.InsertText(length, "\n");
                var completions = context.GetCompletionsAt(length + 1, AuthorCompletionFlags.acfMembersFilter, out time);
                Console.WriteLine("### Copies: {0}, Time: {1}, Bytes: {2}, Bytes per second {3}",
                    i, time, length, length / (time / 1000.0));
                if (time > 2000) break;
            }
        }

    }

    [TestClass]
    public class Dev10JQueryTests : Dev10PerformanceTests
    {
        protected static readonly string jQueryFileLocation = Paths.JQuery.JQuery_1_2_6VSDocFilePath;
        protected static readonly string jQueryErrorsFileLocation = Path.Combine(Paths.FilesPath, @"JQuery\jquery-1.2.6-vsdoc_error.js");

        [TestMethod]
        [Ignore]
        public void GetCompletions()
        {
            var file = _session.ReadFile(jQueryFileLocation);
            file.InsertText(file.Text.Length, "window.$.");
            var result = PerformDev10GetCompletions(file.Text);

            result.ToString().Expect(Dev10JQueryCompletionsExpected);
        }
        #region Test data
        const string Dev10JQueryCompletionsExpected = @"";
        #endregion

        [TestMethod]
        public void GetMessages()
        {
            var file = _session.ReadFile(jQueryErrorsFileLocation);
            var result = PerformDev10GetMessages(file.Text);

            result.ToString().Expect(JQueryMessagesExpected);
        }
        #region Test data
        const string JQueryMessagesExpected = @"
new { lpszMessage = ""Expected expression"", uStartLineNumber = 10, uStartColNumber = 13, uEndLineNumber = 10, uEndColNumber = 17, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 41, uStartColNumber = 13, uEndLineNumber = 41, uEndColNumber = 17, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 73, uStartColNumber = 1, uEndLineNumber = 73, uEndColNumber = 4, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 73, uStartColNumber = 14, uEndLineNumber = 73, uEndColNumber = 19, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 79, uStartColNumber = 14, uEndLineNumber = 79, uEndColNumber = 19, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""'t02' is already defined"", uStartLineNumber = 79, uStartColNumber = 5, uEndLineNumber = 79, uEndColNumber = 8, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 83, uStartColNumber = 18, uEndLineNumber = 83, uEndColNumber = 23, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 108, uStartColNumber = 36, uEndLineNumber = 108, uEndColNumber = 37, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected '}'"", uStartLineNumber = 111, uStartColNumber = 9, uEndLineNumber = 111, uEndColNumber = 12, uErrorNumber = 2148140017, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 111, uStartColNumber = 32, uEndLineNumber = 111, uEndColNumber = 33, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 111, uStartColNumber = 39, uEndLineNumber = 111, uEndColNumber = 45, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 111, uStartColNumber = 51, uEndLineNumber = 111, uEndColNumber = 52, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 133, uStartColNumber = 8, uEndLineNumber = 136, uEndColNumber = 13, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 136, uStartColNumber = 7, uEndLineNumber = 136, uEndColNumber = 13, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 144, uStartColNumber = 5, uEndLineNumber = 144, uEndColNumber = 11, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 149, uStartColNumber = 4, uEndLineNumber = 151, uEndColNumber = 9, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 151, uStartColNumber = 3, uEndLineNumber = 151, uEndColNumber = 9, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 152, uStartColNumber = 2, uEndLineNumber = 152, uEndColNumber = 3, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 166, uStartColNumber = 3, uEndLineNumber = 166, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 168, uStartColNumber = 28, uEndLineNumber = 168, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'t03' is already defined"", uStartLineNumber = 168, uStartColNumber = 9, uEndLineNumber = 168, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 168, uStartColNumber = 35, uEndLineNumber = 168, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 168, uStartColNumber = 47, uEndLineNumber = 168, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 192, uStartColNumber = 3, uEndLineNumber = 192, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 215, uStartColNumber = 3, uEndLineNumber = 215, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 238, uStartColNumber = 3, uEndLineNumber = 238, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 260, uStartColNumber = 3, uEndLineNumber = 260, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 283, uStartColNumber = 3, uEndLineNumber = 283, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 322, uStartColNumber = 3, uEndLineNumber = 322, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 323, uStartColNumber = 24, uEndLineNumber = 323, uEndColNumber = 25, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 341, uStartColNumber = 32, uEndLineNumber = 341, uEndColNumber = 33, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 343, uStartColNumber = 3, uEndLineNumber = 343, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 372, uStartColNumber = 3, uEndLineNumber = 372, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 408, uStartColNumber = 3, uEndLineNumber = 408, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 422, uStartColNumber = 3, uEndLineNumber = 422, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 445, uStartColNumber = 3, uEndLineNumber = 445, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 463, uStartColNumber = 3, uEndLineNumber = 463, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 481, uStartColNumber = 3, uEndLineNumber = 481, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 496, uStartColNumber = 3, uEndLineNumber = 496, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 511, uStartColNumber = 3, uEndLineNumber = 511, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 524, uStartColNumber = 3, uEndLineNumber = 524, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 548, uStartColNumber = 3, uEndLineNumber = 548, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 603, uStartColNumber = 3, uEndLineNumber = 603, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 626, uStartColNumber = 3, uEndLineNumber = 626, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 651, uStartColNumber = 3, uEndLineNumber = 651, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 654, uStartColNumber = 24, uEndLineNumber = 654, uEndColNumber = 25, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 670, uStartColNumber = 3, uEndLineNumber = 670, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 687, uStartColNumber = 3, uEndLineNumber = 687, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 697, uStartColNumber = 3, uEndLineNumber = 697, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 778, uStartColNumber = 3, uEndLineNumber = 778, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 796, uStartColNumber = 3, uEndLineNumber = 796, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 798, uStartColNumber = 15, uEndLineNumber = 798, uEndColNumber = 17, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 809, uStartColNumber = 3, uEndLineNumber = 809, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 824, uStartColNumber = 3, uEndLineNumber = 824, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 827, uStartColNumber = 19, uEndLineNumber = 827, uEndColNumber = 21, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 837, uStartColNumber = 3, uEndLineNumber = 837, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 849, uStartColNumber = 3, uEndLineNumber = 849, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 858, uStartColNumber = 3, uEndLineNumber = 858, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 884, uStartColNumber = 3, uEndLineNumber = 884, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 895, uStartColNumber = 3, uEndLineNumber = 895, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 953, uStartColNumber = 1, uEndLineNumber = 953, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 955, uStartColNumber = 17, uEndLineNumber = 955, uEndColNumber = 22, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ','"", uStartLineNumber = 955, uStartColNumber = 26, uEndLineNumber = 955, uEndColNumber = 27, uErrorNumber = 2148140108, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 955, uStartColNumber = 28, uEndLineNumber = 955, uEndColNumber = 34, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 955, uStartColNumber = 40, uEndLineNumber = 955, uEndColNumber = 41, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 959, uStartColNumber = 17, uEndLineNumber = 959, uEndColNumber = 22, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ','"", uStartLineNumber = 959, uStartColNumber = 26, uEndLineNumber = 959, uEndColNumber = 27, uErrorNumber = 2148140108, uSeverity = 0 }
new { lpszMessage = ""'t06' is already defined"", uStartLineNumber = 959, uStartColNumber = 5, uEndLineNumber = 959, uEndColNumber = 8, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 959, uStartColNumber = 28, uEndLineNumber = 959, uEndColNumber = 34, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 959, uStartColNumber = 40, uEndLineNumber = 959, uEndColNumber = 41, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 961, uStartColNumber = 17, uEndLineNumber = 961, uEndColNumber = 22, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ','"", uStartLineNumber = 961, uStartColNumber = 26, uEndLineNumber = 961, uEndColNumber = 27, uErrorNumber = 2148140108, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 979, uStartColNumber = 1, uEndLineNumber = 979, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 982, uStartColNumber = 21, uEndLineNumber = 982, uEndColNumber = 23, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1009, uStartColNumber = 5, uEndLineNumber = 1009, uEndColNumber = 6, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1009, uStartColNumber = 26, uEndLineNumber = 1009, uEndColNumber = 28, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1055, uStartColNumber = 33, uEndLineNumber = 1055, uEndColNumber = 35, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""'t07' is already defined"", uStartLineNumber = 1055, uStartColNumber = 17, uEndLineNumber = 1055, uEndColNumber = 20, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 1057, uStartColNumber = 2, uEndLineNumber = 1057, uEndColNumber = 8, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 1058, uStartColNumber = 1, uEndLineNumber = 1058, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1059, uStartColNumber = 21, uEndLineNumber = 1059, uEndColNumber = 23, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""'t07' is already defined"", uStartLineNumber = 1059, uStartColNumber = 5, uEndLineNumber = 1059, uEndColNumber = 8, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected ')'"", uStartLineNumber = 1186, uStartColNumber = 35, uEndLineNumber = 1186, uEndColNumber = 36, uErrorNumber = 2148140014, uSeverity = 0 }
new { lpszMessage = ""Expected ')'"", uStartLineNumber = 1233, uStartColNumber = 31, uEndLineNumber = 1233, uEndColNumber = 32, uErrorNumber = 2148140014, uSeverity = 0 }
new { lpszMessage = ""'name' is already defined"", uStartLineNumber = 1354, uStartColNumber = 13, uEndLineNumber = 1354, uEndColNumber = 17, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Illegal assignment"", uStartLineNumber = 1415, uStartColNumber = 31, uEndLineNumber = 1415, uEndColNumber = 32, uErrorNumber = 2148144016, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1636, uStartColNumber = 18, uEndLineNumber = 1636, uEndColNumber = 20, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1688, uStartColNumber = 18, uEndLineNumber = 1688, uEndColNumber = 20, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 1716, uStartColNumber = 5, uEndLineNumber = 1716, uEndColNumber = 8, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1716, uStartColNumber = 10, uEndLineNumber = 1716, uEndColNumber = 12, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1738, uStartColNumber = 3, uEndLineNumber = 1738, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1754, uStartColNumber = 3, uEndLineNumber = 1754, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Did you intend to write an assignment here?"", uStartLineNumber = 1777, uStartColNumber = 12, uEndLineNumber = 1777, uEndColNumber = 32, uErrorNumber = 2148140214, uSeverity = 1 }
new { lpszMessage = ""Did you intend to write an assignment here?"", uStartLineNumber = 1782, uStartColNumber = 12, uEndLineNumber = 1782, uEndColNumber = 32, uErrorNumber = 2148140214, uSeverity = 1 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1786, uStartColNumber = 3, uEndLineNumber = 1786, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 1811, uStartColNumber = 13, uEndLineNumber = 1811, uEndColNumber = 14, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1811, uStartColNumber = 19, uEndLineNumber = 1811, uEndColNumber = 24, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1813, uStartColNumber = 3, uEndLineNumber = 1813, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 1843, uStartColNumber = 5, uEndLineNumber = 1843, uEndColNumber = 6, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1843, uStartColNumber = 11, uEndLineNumber = 1843, uEndColNumber = 16, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1845, uStartColNumber = 3, uEndLineNumber = 1845, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 1879, uStartColNumber = 1, uEndLineNumber = 1879, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected '}'"", uStartLineNumber = 1890, uStartColNumber = 5, uEndLineNumber = 1890, uEndColNumber = 8, uErrorNumber = 2148140017, uSeverity = 0 }
new { lpszMessage = ""Expected '}'"", uStartLineNumber = 1911, uStartColNumber = 9, uEndLineNumber = 1911, uEndColNumber = 12, uErrorNumber = 2148140017, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1913, uStartColNumber = 2, uEndLineNumber = 1913, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1917, uStartColNumber = 2, uEndLineNumber = 1917, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1937, uStartColNumber = 2, uEndLineNumber = 1937, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1941, uStartColNumber = 2, uEndLineNumber = 1941, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 1961, uStartColNumber = 2, uEndLineNumber = 1961, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 1962, uStartColNumber = 16, uEndLineNumber = 1962, uEndColNumber = 17, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 2012, uStartColNumber = 20, uEndLineNumber = 2012, uEndColNumber = 21, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 2085, uStartColNumber = 18, uEndLineNumber = 2085, uEndColNumber = 19, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 2126, uStartColNumber = 18, uEndLineNumber = 2126, uEndColNumber = 19, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 2152, uStartColNumber = 15, uEndLineNumber = 2152, uEndColNumber = 17, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 2201, uStartColNumber = 23, uEndLineNumber = 2201, uEndColNumber = 25, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 2241, uStartColNumber = 26, uEndLineNumber = 2241, uEndColNumber = 27, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 2241, uStartColNumber = 39, uEndLineNumber = 2241, uEndColNumber = 40, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 2243, uStartColNumber = 31, uEndLineNumber = 2243, uEndColNumber = 32, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""'default' can only appear once in a 'switch' statement"", uStartLineNumber = 2256, uStartColNumber = 30, uEndLineNumber = 2256, uEndColNumber = 37, uErrorNumber = 2148140035, uSeverity = 0 }
new { lpszMessage = ""'default' can only appear once in a 'switch' statement"", uStartLineNumber = 2321, uStartColNumber = 26, uEndLineNumber = 2321, uEndColNumber = 33, uErrorNumber = 2148140035, uSeverity = 0 }
new { lpszMessage = ""'m' is already defined"", uStartLineNumber = 2681, uStartColNumber = 10, uEndLineNumber = 2681, uEndColNumber = 11, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 2714, uStartColNumber = 17, uEndLineNumber = 2714, uEndColNumber = 18, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 2734, uStartColNumber = 18, uEndLineNumber = 2734, uEndColNumber = 19, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 2836, uStartColNumber = 15, uEndLineNumber = 2836, uEndColNumber = 16, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'tmp' is already defined"", uStartLineNumber = 2855, uStartColNumber = 21, uEndLineNumber = 2855, uEndColNumber = 24, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'last' is already defined"", uStartLineNumber = 2861, uStartColNumber = 46, uEndLineNumber = 2861, uEndColNumber = 50, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 2864, uStartColNumber = 15, uEndLineNumber = 2864, uEndColNumber = 16, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'rl' is already defined"", uStartLineNumber = 2864, uStartColNumber = 22, uEndLineNumber = 2864, uEndColNumber = 24, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 2906, uStartColNumber = 19, uEndLineNumber = 2906, uEndColNumber = 34, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 2911, uStartColNumber = 19, uEndLineNumber = 2911, uEndColNumber = 34, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 2938, uStartColNumber = 41, uEndLineNumber = 2938, uEndColNumber = 47, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 2954, uStartColNumber = 41, uEndLineNumber = 2954, uEndColNumber = 47, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 2993, uStartColNumber = 45, uEndLineNumber = 2993, uEndColNumber = 51, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 3127, uStartColNumber = 11, uEndLineNumber = 3127, uEndColNumber = 19, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 3185, uStartColNumber = 19, uEndLineNumber = 3185, uEndColNumber = 27, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'break' outside of loop"", uStartLineNumber = 3280, uStartColNumber = 11, uEndLineNumber = 3280, uEndColNumber = 16, uErrorNumber = 2148140027, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 3366, uStartColNumber = 9, uEndLineNumber = 3366, uEndColNumber = 10, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 3366, uStartColNumber = 13, uEndLineNumber = 3366, uEndColNumber = 14, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected '}'"", uStartLineNumber = 3367, uStartColNumber = 4, uEndLineNumber = 3367, uEndColNumber = 9, uErrorNumber = 2148140017, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3376, uStartColNumber = 5, uEndLineNumber = 3376, uEndColNumber = 6, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3387, uStartColNumber = 5, uEndLineNumber = 3387, uEndColNumber = 6, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3401, uStartColNumber = 4, uEndLineNumber = 3401, uEndColNumber = 5, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3413, uStartColNumber = 5, uEndLineNumber = 3413, uEndColNumber = 6, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3424, uStartColNumber = 5, uEndLineNumber = 3424, uEndColNumber = 6, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 3508, uStartColNumber = 5, uEndLineNumber = 3508, uEndColNumber = 6, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 3508, uStartColNumber = 9, uEndLineNumber = 3508, uEndColNumber = 10, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected '}'"", uStartLineNumber = 3509, uStartColNumber = 2, uEndLineNumber = 3509, uEndColNumber = 8, uErrorNumber = 2148140017, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3532, uStartColNumber = 3, uEndLineNumber = 3532, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3542, uStartColNumber = 3, uEndLineNumber = 3542, uEndColNumber = 4, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 3565, uStartColNumber = 1, uEndLineNumber = 3565, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 3569, uStartColNumber = 11, uEndLineNumber = 3569, uEndColNumber = 12, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 3570, uStartColNumber = 9, uEndLineNumber = 3570, uEndColNumber = 10, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 3598, uStartColNumber = 2, uEndLineNumber = 3598, uEndColNumber = 3, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 3629, uStartColNumber = 13, uEndLineNumber = 3629, uEndColNumber = 14, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 5414, uStartColNumber = 4, uEndLineNumber = 5414, uEndColNumber = 5, uErrorNumber = 2148140010, uSeverity = 0 }";
        #endregion

    }

}