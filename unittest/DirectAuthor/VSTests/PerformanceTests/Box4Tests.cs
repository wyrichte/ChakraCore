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
    public class Box4Tests : PerformanceTests
    {
        protected static readonly string box4FileLocation = Path.Combine(Paths.FilesPath, @"Box4\box4.js");
        protected static readonly string box4ErrorsFileLocation = Path.Combine(Paths.FilesPath, @"Box4\box4_error.js");

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var boxFile = _session.ReadFile(box4FileLocation);

            var requests = PerformGetParameterHelp(boxFile.Text + " Common._logRecord(_|r|);");
            requests.ExpectFunction("_logRecord");
        }

        [TestMethod]
        public void GetFunctionHelpSecondRequest()
        {
            var boxFile = _session.ReadFile(box4FileLocation);

            var requests = PerformGetParameterHelp(boxFile.Text + " Common._frameMode(_|s|); \n Common._logRecord(_|r|);");
            requests.ExpectFunction("_logRecord");
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var boxFile = _session.ReadFile(box4FileLocation);

            var result = PerformGetCompletions(boxFile.Text + " Common._|r|");
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }
        #region Test data
        static readonly string[] DomCompletionsExpected = new[] { "_keyModifiers", "_perfMarker", "_aOperation", "_clientUtils" };
        #endregion

        [TestMethod]
        public void GetCompletionsVarRequest()
        {
            var boxFile = _session.ReadFile(box4FileLocation);
            var result = PerformGetCompletions("var x = 5; " + boxFile.Text + "; x._|r|");
        }

        [TestMethod]
        public void GetIdentifierListUnhurried()
        {
            var boxFile = _session.ReadFile(box4FileLocation);
            var context = _session.OpenContext(boxFile);
            long time;
            var completions = context.GetCompletionsAt(0, AuthorCompletionFlags.acfFileIdentifiersFilter, out time);
            RecordTestTime(time);
        }

        [TestMethod]
        public void GetIdentifierListHurried()
        {
            WithMTASession(delegate
            {
                var boxFile = _session.ReadFile(box4FileLocation);
                var context = _session.OpenContext(boxFile);
                using (var hurry = ExecutionLimiter(context))
                {
                    long time;
                    var completions = context.GetCompletionsAt(0, AuthorCompletionFlags.acfFileIdentifiersFilter, out time);
                    RecordTestTime(time);
                }
            });
        }

        [TestMethod]
        public void GetCompletionsSecondRequest()
        {
            var boxFile = _session.ReadFile(box4FileLocation);

            var result = PerformGetCompletions(boxFile.Text + "Common._|s|; Common._|r|");
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }

        [TestMethod]
        public void GetRegionsFirstRequest()
        {
            var result = PerformGetRegions(box4FileLocation, 1, 1);
            result.Contains(DomGetRegionsExpected);
        }
        #region Test data
        const string DomGetRegionsExpected = @"";
        #endregion

        [TestMethod]
        public void GetRegionsSecondRequest()
        {
            var result = PerformGetRegions(box4FileLocation, 2, 2);
            result.Contains(DomGetRegionsExpected);
        }

        [TestMethod]
        public void GetASTFirstRequest()
        {
            var result = PerformGetAST(box4FileLocation, 1, 1);
            result.ToString().CompactJSON().Contains(GetASTExpected.CompactJSON());
        }
        #region Test data
        const string GetASTExpected = @"{
    ""__type"": ""AssignmentOper"",
    ""offset"": 54136,
    ""length"": 191,
    ""left"": {
        ""__type"": ""DotOper"",
        ""offset"": 54136,
        ""length"": 40,
        ""left"": {
            ""__type"": ""NameExpr"",
            ""offset"": 54136,
            ""length"": 16,
            ""value"": ""SVGMarkerElement""
        },
        ""right"": {
            ""__type"": ""NameExpr"",
            ""offset"": 54153,
            ""length"": 23,
            ""value"": ""compareDocumentPosition""
        }
    },
    ""right"": {
        ""__type"": ""FuncDecl"",
        ""offset"": 54179,
        ""length"": 148,
        ""arguments"": [
            ""other""
        ],
        ""body"": [
            {
                ""__type"": ""ReturnStmt"",
                ""offset"": 54313,
                ""length"": 8,
                ""value"": {
                    ""__type"": ""NumberLit"",
                    ""offset"": 54320,
                    ""length"": 1,
                    ""value"": 0
                }
            }
        ]
    }
}";
        #endregion

        [TestMethod]
        public void GetASTSecondRequest()
        {
            var result = PerformGetAST(box4FileLocation, 2, 2);
            result.ToString().CompactJSON().Contains(GetASTExpected.CompactJSON());
        }

        [TestMethod]
        public void GetASTCursorFirstRequest()
        {
            PerformGetASTCursor(box4FileLocation, 1, 1);
        }

        [TestMethod]
        public void GetASTCursorSecondRequest()
        {
            PerformGetASTCursor(box4FileLocation, 2, 2);
        }

        [TestMethod]
        public void GetASTSubTree()
        {
            var subtree = PerformGetASTSubTree(box4FileLocation, 1, 1);
            Assert.IsNotNull(subtree[0]);
        }

        [TestMethod]
        public void GetMessagesFirstRequest()
        {
            var result = PerformGetMessages(box4ErrorsFileLocation, 1, 1);
            result.ToString().Expect(DomMessagesExpected);
        }
        #region Test data
        const string DomMessagesExpected = @"new [] { 
    new { Kind = AuthorMessageKind.amkError, Position = 11, Length = 4, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 16, Length = 3, Message = ""Expected '/'"", MessageID = 1012 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 7179, Length = 5, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 13953, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 231378, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 251958, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 330117, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 424738, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 482272, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 557637, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 679111, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2166121, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2166127, Length = 5, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2166637, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2167182, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2167184, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2167283, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2168288, Length = 2, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2216185, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2350524, Length = 7, Message = ""'default' can only appear once in a 'switch' statement"", MessageID = 1027 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2350713, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2350785, Length = 6, Message = ""Label not found"", MessageID = 1026 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 5992855, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 5992917, Length = 1, Message = ""Can't have 'break' outside of loop"", MessageID = 1019 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 5992974, Length = 6, Message = ""'return' statement outside of function"", MessageID = 1018 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 5993039, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }
}";
        #endregion

        [TestMethod]
        public void GetMessagesSecondRequest()
        {
            var result = PerformGetMessages(box4ErrorsFileLocation, 2, 2);
            result.ToString().Expect(DomMessagesExpected);
        }

    }
}