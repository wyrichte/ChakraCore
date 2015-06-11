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
    public class DomTests : PerformanceTests
    {
        protected static readonly string dom_jsFileLocation = Paths.DomWebPath;
        protected static readonly string dom_jsErrorsFileLocation = Path.Combine(Paths.FilesPath, @"Dom_js\dom_error.js");

        [TestMethod]
        public void GetFunctionHelpFirstRequest()
        {
            var domFile = _session.ReadFile(dom_jsFileLocation);

            var requests = PerformGetParameterHelp(domFile.Text + " window.alert(_|r|);");
            requests.ExpectFunction("alert");
        }

        [TestMethod]
        public void GetFunctionHelpSecondRequest()
        {
            var domFile = _session.ReadFile(dom_jsFileLocation);

            var requests = PerformGetParameterHelp(domFile.Text + " window.alert(_|s|); \nwindow.confirm(_|r|);");
            requests.ExpectFunction("confirm");
        }

        [TestMethod]
        public void GetCompletionsFirstRequest()
        {
            var domFile = _session.ReadFile(dom_jsFileLocation);

            var result = PerformGetCompletions(domFile.Text + " window._|r|");
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }
        #region Test data
        static readonly string[] DomCompletionsExpected = new[] { "document", "alert", "history", "ondblclick" };
        #endregion

        [TestMethod]
        public void GetCompletionsSecondRequest()
        {
            var domFile = _session.ReadFile(dom_jsFileLocation);

            var result = PerformGetCompletions(domFile.Text + "window._|s|; window._|r|");
            result[0].ToEnumerable().ExpectContains(DomCompletionsExpected);
        }

        [TestMethod]
        public void GetRegionsFirstRequest()
        {
            var result = PerformGetRegions(dom_jsFileLocation, 1 ,1);
            result.Contains(DomGetRegionsExpected);
        }
        #region Test data
        const string DomGetRegionsExpected = @"new { Offset = 19146, Length = 4 }, 
    new { Offset = 19193, Length = 4 }, 
    new { Offset = 19241, Length = 4 }, 
    new { Offset = 19286, Length = 4 }, 
    new { Offset = 19331, Length = 4 }, 
    new { Offset = 19378, Length = 4 }, 
    new { Offset = 19427, Length = 4 }, 
    new { Offset = 19505, Length = 4 }, 
    new { Offset = 19589, Length = 4 }, 
    new { Offset = 19638, Length = 4 }, 
    new { Offset = 19713, Length = 4 }, 
    new { Offset = 19794, Length = 4 }, 
    new { Offset = 19843, Length = 4 }, 
    new { Offset = 19919, Length = 4 }, 
    new { Offset = 19966, Length = 4 }, 
    new { Offset = 20090, Length = 4 }, 
    new { Offset = 20200, Length = 4 }, 
    new { Offset = 20247, Length = 4 }, 
    new { Offset = 20294, Length = 4 }";
        #endregion

        [TestMethod]
        public void GetRegionsSecondRequest()
        {
            var result = PerformGetRegions(dom_jsFileLocation, 2, 2);
            result.Contains(DomGetRegionsExpected);
        }

        [TestMethod]
        public void GetASTFirstRequest()
        {
            var result = PerformGetAST(dom_jsFileLocation, 1, 1);
            result.ToString().CompactJSON().Contains(DomASTExpected.CompactJSON());
        }
        #region Test data
        const string DomASTExpected = @"{
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
            var result = PerformGetAST(dom_jsFileLocation, 2, 2);
            result.ToString().CompactJSON().Contains(DomASTExpected.CompactJSON());
        }

        [TestMethod]
        public void GetASTCursorFirstRequest()
        {
            PerformGetASTCursor(dom_jsFileLocation, 1, 1);
        }

        [TestMethod]
        public void GetASTCursorSecondRequest()
        {
            PerformGetASTCursor(dom_jsFileLocation, 2, 2);
        }

        [TestMethod]
        public void GetASTSubTree()
        {
            var subtree = PerformGetASTSubTree(dom_jsFileLocation, 1, 1);
            Assert.IsNotNull(subtree[0]);
        }

        [TestMethod]
        public void GetMessagesFirstRequest()
        {
            var result = PerformGetMessages(dom_jsErrorsFileLocation, 1, 1);

            result.ToString().Expect(DomMessagesExpected);
        }
        #region Test data
        const string DomMessagesExpected = @"new [] { 
    new { Kind = AuthorMessageKind.amkError, Position = 940646, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1032173, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1051128, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1051465, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1287320, Length = 1, Message = ""Can't have 'break' outside of loop"", MessageID = 1019 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1302937, Length = 1, Message = ""Can't have 'break' outside of loop"", MessageID = 1019 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1351408, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1381919, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1408332, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1438714, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1462392, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1475999, Length = 6, Message = ""Label not found"", MessageID = 1026 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1476393, Length = 6, Message = ""Label not found"", MessageID = 1026 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1488662, Length = 6, Message = ""Label not found"", MessageID = 1026 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1489627, Length = 6, Message = ""Label not found"", MessageID = 1026 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1515851, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1516902, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1517503, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1544126, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1561341, Length = 1, Message = ""Can't have 'continue' outside of loop"", MessageID = 1020 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1634659, Length = 7, Message = ""'default' can only appear once in a 'switch' statement"", MessageID = 1027 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1637868, Length = 7, Message = ""'default' can only appear once in a 'switch' statement"", MessageID = 1027 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1638664, Length = 7, Message = ""'default' can only appear once in a 'switch' statement"", MessageID = 1027 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1651934, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1688720, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1688987, Length = 1, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1780925, Length = 2, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1803235, Length = 2, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1803552, Length = 2, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1813528, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1829165, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1829456, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1844314, Length = 1, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1844316, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1863257, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1882433, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1882439, Length = 5, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1908326, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1908332, Length = 5, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1923571, Length = 1, Message = ""Expected '{'"", MessageID = 1008 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1923577, Length = 5, Message = ""Expected ';'"", MessageID = 1004 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1957095, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1984633, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1984920, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 1996939, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2016848, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2017320, Length = 1, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2032565, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2046462, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2046743, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2061585, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2075201, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2085930, Length = 2, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2103207, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2123448, Length = 1, Message = ""Expected ')'"", MessageID = 1006 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2146876, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2151109, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2172283, Length = 2, Message = ""Expected ':'"", MessageID = 1003 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2188403, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2208254, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2225712, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2225946, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2246499, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2296669, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2300715, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2302883, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2314290, Length = 1, Message = ""Expected identifier, string or number"", MessageID = 1028 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2335825, Length = 5, Message = ""Expected identifier"", MessageID = 1010 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2337375, Length = 4, Message = ""Syntax error"", MessageID = 1002 }, 
    new { Kind = AuthorMessageKind.amkError, Position = 2337380, Length = 3, Message = ""Expected '/'"", MessageID = 1012 }
}";
        #endregion

        [TestMethod]
        public void GetMessagesSecondRequest()
        {
            var result = PerformGetMessages(dom_jsErrorsFileLocation, 2, 2);

            result.ToString().Expect(DomMessagesExpected);
        }
    }

    [TestClass]
    public class Dev10DomTests : Dev10PerformanceTests
    {
        protected static readonly string dom_jsFileLocation = Paths.DomWebPath;
        protected static readonly string dom_jsErrorsFileLocation = Path.Combine(Paths.FilesPath, @"Dom_js\dom_error.js");

        [TestMethod]
        [Ignore]
        public void GetCompletions()
        {
            var file = _session.ReadFile(dom_jsFileLocation);
            file.InsertText(file.Text.Length, "\nwindow.");
            var result = PerformDev10GetCompletions(file.Text);

            result.ToString().Expect(Dev10DomCompletionsExpected);
        }
        #region Test data
        const string Dev10DomCompletionsExpected = @" { Name = ""msCapture"" , MemberType = ""Field"" }
 { Name = ""sessionStorage"" , MemberType = ""Field"" }
 { Name = ""localStorage"" , MemberType = ""Field"" }
 { Name = ""media"" , MemberType = ""Field"" }
 { Name = ""screenX"" , MemberType = ""Field"" }
 { Name = ""innerHeight"" , MemberType = ""Field"" }
 { Name = ""screen"" , MemberType = ""Field"" }
 { Name = ""outerHeight"" , MemberType = ""Field"" }
 { Name = ""screenY"" , MemberType = ""Field"" }
 { Name = ""pageYOffset"" , MemberType = ""Field"" }
 { Name = ""innerWidth"" , MemberType = ""Field"" }
 { Name = ""pageXOffset"" , MemberType = ""Field"" }
 { Name = ""outerWidth"" , MemberType = ""Field"" }
 { Name = ""msPerformance"" , MemberType = ""Field"" }
 { Name = ""screenTop"" , MemberType = ""Field"" }
 { Name = ""event"" , MemberType = ""Field"" }
 { Name = ""external"" , MemberType = ""Field"" }
 { Name = ""closed"" , MemberType = ""Field"" }
 { Name = ""clientInformation"" , MemberType = ""Field"" }
 { Name = ""defaultStatus"" , MemberType = ""Field"" }
 { Name = ""clipboardData"" , MemberType = ""Field"" }
 { Name = ""maxConnectionsPerServer"" , MemberType = ""Field"" }
 { Name = ""offscreenBuffering"" , MemberType = ""Field"" }
 { Name = ""screenLeft"" , MemberType = ""Field"" }
 { Name = ""status"" , MemberType = ""Field"" }
 { Name = ""navigator"" , MemberType = ""Field"" }
 { Name = ""window"" , MemberType = ""Field"" }
 { Name = ""frameElement"" , MemberType = ""Field"" }
 { Name = ""location"" , MemberType = ""Field"" }
 { Name = ""parent"" , MemberType = ""Field"" }
 { Name = ""onerror"" , MemberType = ""Field"" }
 { Name = ""self"" , MemberType = ""Field"" }
 { Name = ""length"" , MemberType = ""Field"" }
 { Name = ""frames"" , MemberType = ""Field"" }
 { Name = ""opener"" , MemberType = ""Field"" }
 { Name = ""top"" , MemberType = ""Field"" }
 { Name = ""name"" , MemberType = ""Field"" }
 { Name = ""history"" , MemberType = ""Field"" }
 { Name = ""msBlob"" , MemberType = ""Field"" }
 { Name = ""document"" , MemberType = ""Field"" }
 { Name = ""setInterval"" , MemberType = ""Method"" }
 { Name = ""clearInterval"" , MemberType = ""Method"" }
 { Name = ""setTimeout"" , MemberType = ""Method"" }
 { Name = ""clearTimeout"" , MemberType = ""Method"" }
 { Name = ""dispatchEvent"" , MemberType = ""Method"" }
 { Name = ""addEventListener"" , MemberType = ""Method"" }
 { Name = ""removeEventListener"" , MemberType = ""Method"" }
 { Name = ""scrollTo"" , MemberType = ""Method"" }
 { Name = ""scrollBy"" , MemberType = ""Method"" }
 { Name = ""scroll"" , MemberType = ""Method"" }
 { Name = ""showHelp"" , MemberType = ""Method"" }
 { Name = ""moveBy"" , MemberType = ""Method"" }
 { Name = ""moveTo"" , MemberType = ""Method"" }
 { Name = ""msWriteProfilerMark"" , MemberType = ""Method"" }
 { Name = ""execScript"" , MemberType = ""Method"" }
 { Name = ""toStaticHTML"" , MemberType = ""Method"" }
 { Name = ""createPopup"" , MemberType = ""Method"" }
 { Name = ""resizeTo"" , MemberType = ""Method"" }
 { Name = ""item"" , MemberType = ""Method"" }
 { Name = ""resizeBy"" , MemberType = ""Method"" }
 { Name = ""navigate"" , MemberType = ""Method"" }
 { Name = ""showModelessDialog"" , MemberType = ""Method"" }
 { Name = ""onfocusin"" , MemberType = ""Method"" }
 { Name = ""onfocusout"" , MemberType = ""Method"" }
 { Name = ""onhelp"" , MemberType = ""Method"" }
 { Name = ""onmouseenter"" , MemberType = ""Method"" }
 { Name = ""onmouseleave"" , MemberType = ""Method"" }
 { Name = ""detachEvent"" , MemberType = ""Method"" }
 { Name = ""attachEvent"" , MemberType = ""Method"" }
 { Name = ""getComputedStyle"" , MemberType = ""Method"" }
 { Name = ""blur"" , MemberType = ""Method"" }
 { Name = ""getSelection"" , MemberType = ""Method"" }
 { Name = ""showModalDialog"" , MemberType = ""Method"" }
 { Name = ""postMessage"" , MemberType = ""Method"" }
 { Name = ""close"" , MemberType = ""Method"" }
 { Name = ""confirm"" , MemberType = ""Method"" }
 { Name = ""open"" , MemberType = ""Method"" }
 { Name = ""prompt"" , MemberType = ""Method"" }
 { Name = ""print"" , MemberType = ""Method"" }
 { Name = ""focus"" , MemberType = ""Method"" }
 { Name = ""alert"" , MemberType = ""Method"" }
 { Name = ""oninput"" , MemberType = ""Method"" }
 { Name = ""onvolumechange"" , MemberType = ""Method"" }
 { Name = ""onload"" , MemberType = ""Method"" }
 { Name = ""onmousewheel"" , MemberType = ""Method"" }
 { Name = ""onscroll"" , MemberType = ""Method"" }
 { Name = ""onunload"" , MemberType = ""Method"" }
 { Name = ""onhashchange"" , MemberType = ""Method"" }
 { Name = ""onended"" , MemberType = ""Method"" }
 { Name = ""onmouseout"" , MemberType = ""Method"" }
 { Name = ""ondrop"" , MemberType = ""Method"" }
 { Name = ""onselect"" , MemberType = ""Method"" }
 { Name = ""onresize"" , MemberType = ""Method"" }
 { Name = ""ontimeupdate"" , MemberType = ""Method"" }
 { Name = ""onmessage"" , MemberType = ""Method"" }
 { Name = ""onfocus"" , MemberType = ""Method"" }
 { Name = ""onsuspend"" , MemberType = ""Method"" }
 { Name = ""onloadeddata"" , MemberType = ""Method"" }
 { Name = ""onkeypress"" , MemberType = ""Method"" }
 { Name = ""onreadystatechange"" , MemberType = ""Method"" }
 { Name = ""onabort"" , MemberType = ""Method"" }
 { Name = ""oncanplaythrough"" , MemberType = ""Method"" }
 { Name = ""onplaying"" , MemberType = ""Method"" }
 { Name = ""onplay"" , MemberType = ""Method"" }
 { Name = ""onloadedmetadata"" , MemberType = ""Method"" }
 { Name = ""onchange"" , MemberType = ""Method"" }
 { Name = ""oncontextmenu"" , MemberType = ""Method"" }
 { Name = ""ondblclick"" , MemberType = ""Method"" }
 { Name = ""onprogress"" , MemberType = ""Method"" }
 { Name = ""onsubmit"" , MemberType = ""Method"" }
 { Name = ""ondragenter"" , MemberType = ""Method"" }
 { Name = ""onloadstart"" , MemberType = ""Method"" }
 { Name = ""onstorage"" , MemberType = ""Method"" }
 { Name = ""onratechange"" , MemberType = ""Method"" }
 { Name = ""onbeforeunload"" , MemberType = ""Method"" }
 { Name = ""onoffline"" , MemberType = ""Method"" }
 { Name = ""onmousemove"" , MemberType = ""Method"" }
 { Name = ""onstalled"" , MemberType = ""Method"" }
 { Name = ""oncanplay"" , MemberType = ""Method"" }
 { Name = ""onseeking"" , MemberType = ""Method"" }
 { Name = ""onemptied"" , MemberType = ""Method"" }
 { Name = ""onblur"" , MemberType = ""Method"" }
 { Name = ""ondurationchange"" , MemberType = ""Method"" }
 { Name = ""ononline"" , MemberType = ""Method"" }
 { Name = ""onwaiting"" , MemberType = ""Method"" }
 { Name = ""onclick"" , MemberType = ""Method"" }
 { Name = ""onseeked"" , MemberType = ""Method"" }
 { Name = ""onmousedown"" , MemberType = ""Method"" }
 { Name = ""onbeforeprint"" , MemberType = ""Method"" }
 { Name = ""onpause"" , MemberType = ""Method"" }
 { Name = ""onafterprint"" , MemberType = ""Method"" }
 { Name = ""ondragleave"" , MemberType = ""Method"" }
 { Name = ""onmouseover"" , MemberType = ""Method"" }
 { Name = ""ondrag"" , MemberType = ""Method"" }
 { Name = ""ondragstart"" , MemberType = ""Method"" }
 { Name = ""onmouseup"" , MemberType = ""Method"" }
 { Name = ""onreset"" , MemberType = ""Method"" }
 { Name = ""onkeyup"" , MemberType = ""Method"" }
 { Name = ""ondragover"" , MemberType = ""Method"" }
 { Name = ""onkeydown"" , MemberType = ""Method"" }
 { Name = ""ondragend"" , MemberType = ""Method"" }
 { Name = ""undefined"" , MemberType = ""Field"" }
 { Name = ""Infinity"" , MemberType = ""Field"" }
 { Name = ""NaN"" , MemberType = ""Field"" }
 { Name = ""encodeURIComponent"" , MemberType = ""Method"" }
 { Name = ""encodeURI"" , MemberType = ""Method"" }
 { Name = ""decodeURIComponent"" , MemberType = ""Method"" }
 { Name = ""decodeURI"" , MemberType = ""Method"" }
 { Name = ""CollectGarbage"" , MemberType = ""Method"" }
 { Name = ""ScriptEngineBuildVersion"" , MemberType = ""Method"" }
 { Name = ""ScriptEngineMinorVersion"" , MemberType = ""Method"" }
 { Name = ""ScriptEngineMajorVersion"" , MemberType = ""Method"" }
 { Name = ""ScriptEngine"" , MemberType = ""Method"" }
 { Name = ""GetObject"" , MemberType = ""Method"" }
 { Name = ""unescape"" , MemberType = ""Method"" }
 { Name = ""parseFloat"" , MemberType = ""Method"" }
 { Name = ""parseInt"" , MemberType = ""Method"" }
 { Name = ""isFinite"" , MemberType = ""Method"" }
 { Name = ""isNaN"" , MemberType = ""Method"" }
 { Name = ""eval"" , MemberType = ""Method"" }
 { Name = ""escape"" , MemberType = ""Method"" }
 { Name = ""Error"" , MemberType = ""Method"" }
 { Name = ""VBArray"" , MemberType = ""Method"" }
 { Name = ""ActiveXObject"" , MemberType = ""Method"" }
 { Name = ""Math"" , MemberType = ""Method"" }
 { Name = ""Object"" , MemberType = ""Method"" }
 { Name = ""Number"" , MemberType = ""Method"" }
 { Name = ""Function"" , MemberType = ""Method"" }
 { Name = ""Boolean"" , MemberType = ""Method"" }
 { Name = ""Date"" , MemberType = ""Method"" }
 { Name = ""String"" , MemberType = ""Method"" }
 { Name = ""RegExp"" , MemberType = ""Method"" }
 { Name = ""Array"" , MemberType = ""Method"" }
";
        #endregion

        [TestMethod]
        public void GetMessages()
        {
            var file = _session.ReadFile(dom_jsErrorsFileLocation);
            var result = PerformDev10GetMessages(file.Text);

            result.ToString().Expect(DomMessagesExpected);
        }
        #region Test data
        const string DomMessagesExpected = @"
new { lpszMessage = ""'char' is a new reserved word and should not be used as an identifier"", uStartLineNumber = 21113, uStartColNumber = 16, uEndLineNumber = 21113, uEndColNumber = 20, uErrorNumber = 2148140145, uSeverity = 2 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 27586, uStartColNumber = 9, uEndLineNumber = 27586, uEndColNumber = 10, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 30382, uStartColNumber = 9, uEndLineNumber = 30382, uEndColNumber = 10, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 30892, uStartColNumber = 9, uEndLineNumber = 30892, uEndColNumber = 10, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 30899, uStartColNumber = 13, uEndLineNumber = 30899, uEndColNumber = 14, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Can't have 'break' outside of loop"", uStartLineNumber = 37645, uStartColNumber = 7, uEndLineNumber = 37645, uEndColNumber = 12, uErrorNumber = 2148140027, uSeverity = 0 }
new { lpszMessage = ""Can't have 'break' outside of loop"", uStartLineNumber = 38137, uStartColNumber = 7, uEndLineNumber = 38137, uEndColNumber = 12, uErrorNumber = 2148140027, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 39574, uStartColNumber = 7, uEndLineNumber = 39574, uEndColNumber = 15, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 40536, uStartColNumber = 7, uEndLineNumber = 40536, uEndColNumber = 15, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 41316, uStartColNumber = 7, uEndLineNumber = 41316, uEndColNumber = 15, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 42196, uStartColNumber = 7, uEndLineNumber = 42196, uEndColNumber = 15, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 42880, uStartColNumber = 7, uEndLineNumber = 42880, uEndColNumber = 15, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 43262, uStartColNumber = 37, uEndLineNumber = 43262, uEndColNumber = 43, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 43274, uStartColNumber = 41, uEndLineNumber = 43274, uEndColNumber = 47, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 43669, uStartColNumber = 41, uEndLineNumber = 43669, uEndColNumber = 47, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Label not found"", uStartLineNumber = 43696, uStartColNumber = 37, uEndLineNumber = 43696, uEndColNumber = 43, uErrorNumber = 2148140034, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 44485, uStartColNumber = 15, uEndLineNumber = 44485, uEndColNumber = 30, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 44514, uStartColNumber = 19, uEndLineNumber = 44514, uEndColNumber = 34, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 44535, uStartColNumber = 15, uEndLineNumber = 44535, uEndColNumber = 30, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 45332, uStartColNumber = 19, uEndLineNumber = 45332, uEndColNumber = 34, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""Can't have 'continue' outside of loop"", uStartLineNumber = 45816, uStartColNumber = 15, uEndLineNumber = 45816, uEndColNumber = 30, uErrorNumber = 2148140028, uSeverity = 0 }
new { lpszMessage = ""'default' can only appear once in a 'switch' statement"", uStartLineNumber = 47941, uStartColNumber = 30, uEndLineNumber = 47941, uEndColNumber = 37, uErrorNumber = 2148140035, uSeverity = 0 }
new { lpszMessage = ""'default' can only appear once in a 'switch' statement"", uStartLineNumber = 48011, uStartColNumber = 30, uEndLineNumber = 48011, uEndColNumber = 37, uErrorNumber = 2148140035, uSeverity = 0 }
new { lpszMessage = ""'default' can only appear once in a 'switch' statement"", uStartLineNumber = 48028, uStartColNumber = 34, uEndLineNumber = 48028, uEndColNumber = 41, uErrorNumber = 2148140035, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 48406, uStartColNumber = 27, uEndLineNumber = 48406, uEndColNumber = 28, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 49481, uStartColNumber = 31, uEndLineNumber = 49481, uEndColNumber = 32, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""'i' is already defined"", uStartLineNumber = 49487, uStartColNumber = 14, uEndLineNumber = 49487, uEndColNumber = 15, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 49487, uStartColNumber = 27, uEndLineNumber = 49487, uEndColNumber = 28, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 52178, uStartColNumber = 19, uEndLineNumber = 52178, uEndColNumber = 21, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 52858, uStartColNumber = 23, uEndLineNumber = 52858, uEndColNumber = 25, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""'a' is already defined"", uStartLineNumber = 52868, uStartColNumber = 14, uEndLineNumber = 52868, uEndColNumber = 15, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'b' is already defined"", uStartLineNumber = 52868, uStartColNumber = 17, uEndLineNumber = 52868, uEndColNumber = 18, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 52868, uStartColNumber = 19, uEndLineNumber = 52868, uEndColNumber = 21, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 53157, uStartColNumber = 22, uEndLineNumber = 53157, uEndColNumber = 23, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 53639, uStartColNumber = 22, uEndLineNumber = 53639, uEndColNumber = 23, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 53646, uStartColNumber = 26, uEndLineNumber = 53646, uEndColNumber = 27, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 54128, uStartColNumber = 20, uEndLineNumber = 54128, uEndColNumber = 21, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 55282, uStartColNumber = 13, uEndLineNumber = 55282, uEndColNumber = 14, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 55282, uStartColNumber = 19, uEndLineNumber = 55282, uEndColNumber = 24, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 56060, uStartColNumber = 9, uEndLineNumber = 56060, uEndColNumber = 10, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 56060, uStartColNumber = 15, uEndLineNumber = 56060, uEndColNumber = 20, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected '{'"", uStartLineNumber = 56459, uStartColNumber = 9, uEndLineNumber = 56459, uEndColNumber = 10, uErrorNumber = 2148140016, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 56459, uStartColNumber = 15, uEndLineNumber = 56459, uEndColNumber = 20, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 57434, uStartColNumber = 10, uEndLineNumber = 57434, uEndColNumber = 12, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 58212, uStartColNumber = 10, uEndLineNumber = 58212, uEndColNumber = 12, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 58219, uStartColNumber = 14, uEndLineNumber = 58219, uEndColNumber = 16, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Illegal assignment"", uStartLineNumber = 58612, uStartColNumber = 19, uEndLineNumber = 58612, uEndColNumber = 20, uErrorNumber = 2148144016, uSeverity = 0 }
new { lpszMessage = ""Illegal assignment"", uStartLineNumber = 59172, uStartColNumber = 23, uEndLineNumber = 59172, uEndColNumber = 24, uErrorNumber = 2148144016, uSeverity = 0 }
new { lpszMessage = ""Illegal assignment"", uStartLineNumber = 59189, uStartColNumber = 19, uEndLineNumber = 59189, uEndColNumber = 20, uErrorNumber = 2148144016, uSeverity = 0 }
new { lpszMessage = ""'t09' is already defined"", uStartLineNumber = 59189, uStartColNumber = 9, uEndLineNumber = 59189, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected ')'"", uStartLineNumber = 59667, uStartColNumber = 27, uEndLineNumber = 59667, uEndColNumber = 28, uErrorNumber = 2148140014, uSeverity = 0 }
new { lpszMessage = ""Expected ')'"", uStartLineNumber = 60063, uStartColNumber = 31, uEndLineNumber = 60063, uEndColNumber = 32, uErrorNumber = 2148140014, uSeverity = 0 }
new { lpszMessage = ""Expected ')'"", uStartLineNumber = 60073, uStartColNumber = 27, uEndLineNumber = 60073, uEndColNumber = 28, uErrorNumber = 2148140014, uSeverity = 0 }
new { lpszMessage = ""'t08' is already defined"", uStartLineNumber = 60073, uStartColNumber = 9, uEndLineNumber = 60073, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 60550, uStartColNumber = 29, uEndLineNumber = 60550, uEndColNumber = 31, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 60941, uStartColNumber = 25, uEndLineNumber = 60941, uEndColNumber = 27, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 61239, uStartColNumber = 29, uEndLineNumber = 61239, uEndColNumber = 31, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 61716, uStartColNumber = 25, uEndLineNumber = 61716, uEndColNumber = 30, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ','"", uStartLineNumber = 61716, uStartColNumber = 34, uEndLineNumber = 61716, uEndColNumber = 35, uErrorNumber = 2148140108, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 62321, uStartColNumber = 21, uEndLineNumber = 62321, uEndColNumber = 26, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ','"", uStartLineNumber = 62321, uStartColNumber = 30, uEndLineNumber = 62321, uEndColNumber = 31, uErrorNumber = 2148140108, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 63074, uStartColNumber = 19, uEndLineNumber = 63074, uEndColNumber = 21, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 63182, uStartColNumber = 23, uEndLineNumber = 63182, uEndColNumber = 25, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 63798, uStartColNumber = 19, uEndLineNumber = 63798, uEndColNumber = 21, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'t05' is already defined"", uStartLineNumber = 63798, uStartColNumber = 9, uEndLineNumber = 63798, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 64293, uStartColNumber = 24, uEndLineNumber = 64293, uEndColNumber = 25, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 64859, uStartColNumber = 24, uEndLineNumber = 64859, uEndColNumber = 25, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""'t04' is already defined"", uStartLineNumber = 64859, uStartColNumber = 9, uEndLineNumber = 64859, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 65342, uStartColNumber = 24, uEndLineNumber = 65342, uEndColNumber = 25, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""'t04' is already defined"", uStartLineNumber = 65342, uStartColNumber = 9, uEndLineNumber = 65342, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""Expected identifier or string"", uStartLineNumber = 65347, uStartColNumber = 28, uEndLineNumber = 65347, uEndColNumber = 29, uErrorNumber = 2148140036, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 65933, uStartColNumber = 28, uEndLineNumber = 65933, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""Expected ';'"", uStartLineNumber = 65933, uStartColNumber = 33, uEndLineNumber = 65933, uEndColNumber = 34, uErrorNumber = 2148140012, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 65933, uStartColNumber = 35, uEndLineNumber = 65933, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 65933, uStartColNumber = 47, uEndLineNumber = 65933, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 67520, uStartColNumber = 28, uEndLineNumber = 67520, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 67520, uStartColNumber = 35, uEndLineNumber = 67520, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 67520, uStartColNumber = 47, uEndLineNumber = 67520, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 67655, uStartColNumber = 28, uEndLineNumber = 67655, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'t03' is already defined"", uStartLineNumber = 67655, uStartColNumber = 9, uEndLineNumber = 67655, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 67655, uStartColNumber = 35, uEndLineNumber = 67655, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 67655, uStartColNumber = 47, uEndLineNumber = 67655, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 67724, uStartColNumber = 28, uEndLineNumber = 67724, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'t03' is already defined"", uStartLineNumber = 67724, uStartColNumber = 9, uEndLineNumber = 67724, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 67724, uStartColNumber = 35, uEndLineNumber = 67724, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 67724, uStartColNumber = 47, uEndLineNumber = 67724, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected ':'"", uStartLineNumber = 68016, uStartColNumber = 28, uEndLineNumber = 68016, uEndColNumber = 29, uErrorNumber = 2148140011, uSeverity = 0 }
new { lpszMessage = ""'t03' is already defined"", uStartLineNumber = 68016, uStartColNumber = 9, uEndLineNumber = 68016, uEndColNumber = 12, uErrorNumber = 2148140119, uSeverity = 1 }
new { lpszMessage = ""'return' statement outside of function"", uStartLineNumber = 68016, uStartColNumber = 35, uEndLineNumber = 68016, uEndColNumber = 41, uErrorNumber = 2148140026, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 68016, uStartColNumber = 47, uEndLineNumber = 68016, uEndColNumber = 48, uErrorNumber = 2148140010, uSeverity = 0 }
new { lpszMessage = ""Expected identifier"", uStartLineNumber = 68708, uStartColNumber = 18, uEndLineNumber = 68708, uEndColNumber = 23, uErrorNumber = 2148140018, uSeverity = 0 }
new { lpszMessage = ""Expected expression"", uStartLineNumber = 68755, uStartColNumber = 16, uEndLineNumber = 68755, uEndColNumber = 20, uErrorNumber = 2148140203, uSeverity = 0 }
new { lpszMessage = ""Syntax error"", uStartLineNumber = 68764, uStartColNumber = 1, uEndLineNumber = 68764, uEndColNumber = 2, uErrorNumber = 2148140010, uSeverity = 0 }";
        #endregion
    }
}