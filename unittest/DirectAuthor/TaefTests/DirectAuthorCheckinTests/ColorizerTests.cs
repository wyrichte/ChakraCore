using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class ColorizerTests : DirectAuthorTest
    {
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
        [WorkItem(151872)]
        public void Bug151872()
        {
            var colorizer = _session.GetColorizer();
            var colorization = colorizer.Colorize(Bug151872Data.Replace("gopher", "\0gopher"), Bug151872Data.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
            AuthorTokenColorInfo info;
            do
            {
                info = colorization.Next();
            }
            while (info.Kind != AuthorTokenKind.atkEnd);
            Marshal.ReleaseComObject(colorization);
            Marshal.ReleaseComObject(colorizer);
        }
        #region Test data
        const string Bug151872Data = @"function u8xmrn4judd8U7Q8mhXsb5(){
/// <summary>â¤ŒìŸµî”½ê¬ ì’°ë²æ™²ç‘±è¿è¹Œåµ…ï³ºã‘“ãŠæ”ªæŽºé©å•›ëµ„ç³€ç§£ã¥¥âŠ½è‰“íŸ¾ã¡á‹¤ç¬ë¹·ç¦šæ±Œë³«çºï™µáˆ”ï‚¸</summary>
<param name=""$"" type=""J3RjdaR0oCWDk.E.1.m.p4.1.Ti5t.N2"" optional=""false"" parameterArray=""true"" elementType=""cMO"" ele%00mentDomElement=""false"" elementMayBeNull=""true"" />
<param name=""$"" type=""dF"" parameterArray=""false"" integer=""true"" domElement=""true"" elementType=""0Ii.4"" elementMayBeNull=""false"" >æª˜ë†åŽ¦é®©äƒ³ç©¡èµ“â“ˆë–·â¬\000¿à§Œá´›å¥¼è¤†ï¸•æ›è§Œç²¡ïš±ç‰™gopher://%22%2f><script>alert('hello')%3b<%2fscript>";
        #endregion

        [TestMethod]
        [WorkItem(151872)]
        public void ColorizerPunctuationMarks()
        {
            var colorizer = _session.GetColorizer();
            var colorization = colorizer.Colorize(ColorizerPunctuationMarksInputFile, ColorizerPunctuationMarksInputFile.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
            AuthorTokenColorInfo info;
            int i = 0;
            do
            {
                info = colorization.Next();
                Assert.AreEqual(ColorizerPunctuationMarksExpectedTokens[i++], info.Kind);
            }
            while (info.Kind != AuthorTokenKind.atkEnd);

            Marshal.ReleaseComObject(colorization);
            Marshal.ReleaseComObject(colorizer);
        }
        #region Test data
        const string ColorizerPunctuationMarksInputFile = @"function f(a,b){ var a = []; return b;}";
        static readonly AuthorTokenKind[] ColorizerPunctuationMarksExpectedTokens = new AuthorTokenKind[] {
            AuthorTokenKind.atkFunction,
            AuthorTokenKind.atkIdentifier,
            AuthorTokenKind.atkLParen,
            AuthorTokenKind.atkIdentifier,
            AuthorTokenKind.atkComma,
            AuthorTokenKind.atkIdentifier,
            AuthorTokenKind.atkRParen,
            AuthorTokenKind.atkLCurly,
            AuthorTokenKind.atkVar,
            AuthorTokenKind.atkIdentifier,
            AuthorTokenKind.atkAsg,
            AuthorTokenKind.atkLBrack,
            AuthorTokenKind.atkRBrack,
            AuthorTokenKind.atkSColon,
            AuthorTokenKind.atkReturn,
            AuthorTokenKind.atkIdentifier,
            AuthorTokenKind.atkSColon,
            AuthorTokenKind.atkRCurly,
            AuthorTokenKind.atkEnd};
        #endregion

        [TestMethod]
        [WorkItem(125666)]
        public void Bug125666()
        {
            var colorizer = _session.GetColorizer();
            var inputString = "let yield";
            var colorization = colorizer.Colorize(inputString, inputString.Length, AuthorSourceState.SOURCE_STATE_INITIAL);

            var info = colorization.Next();
            Assert.AreEqual(info.Kind, AuthorTokenKind.atkLet);

            info = colorization.Next();
            Assert.AreEqual(info.Kind, AuthorTokenKind.atkYield);

            Marshal.ReleaseComObject(colorization);
            Marshal.ReleaseComObject(colorizer);
        }

        [TestMethod]
        public void RegularExpression()
        {
            var inputText = @"var expression = /test/; ";
            var colorizer = _session.GetColorizer();
            var colorization = colorizer.Colorize(inputText, inputText.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
            AuthorTokenColorInfo info;
            do
            {
                info = colorization.Next();
            }
            while (info.Kind != AuthorTokenKind.atkEnd);

            Marshal.ReleaseComObject(colorization);
            Marshal.ReleaseComObject(colorizer);
        }

        [TestMethod]
        [WorkItem(289743)]
        public void ColorizerState()
        {
            var inputText = @"

/* multi|c|   line|c|
   comment */|n|

var expression = '\|s|
    multi         \|s|
    line          \|s|
    string';|n|

""\|s|
    multi         \|s|
    line          \|s|
    string"";|n|

function|n| foo|n|() {|n|}|n|
";
            var colorizer = _session.GetColorizer();
            PerformRequests(inputText, (context, offset, data, index) =>
            {

                var state = colorizer.GetStateForText(context.PrimaryFile.Text, offset + 1, AuthorSourceState.SOURCE_STATE_INITIAL);
                var multilineTokenKind = colorizer.GetMultilineTokenKind(state);
                switch (data)
                {
                    case "c":
                        Assert.IsTrue(multilineTokenKind == AuthorMultilineTokenKind.amtkMultilineComment);
                        break;
                    case "s":
                        Assert.IsTrue(multilineTokenKind == AuthorMultilineTokenKind.amtkMultilineString);
                        break;
                    case "n":
                        Assert.IsTrue(multilineTokenKind == AuthorMultilineTokenKind.amtkNone);
                        break;
                    default:
                        Assert.Fail("unknown value");
                        break;
                }

            });
            Marshal.ReleaseComObject(colorizer);
        }

        [TestMethod]
        [WorkItem(387499)]
        public void IdentifierDotKeyword()
        {
            var keywords = new [] {
                "break",
                "case",
                "catch",
                "class",
                "const",
                "continue",
                "debugger",
                "default",
                "delete",
                "do",
                "else",
                "enum",
                "export",
                "extends",
                "false",
                "finally",
                "for",
                "function",
                "if",
                "import",
                "in",
                "instanceof",
                "new",
                "null",
                "return",
                "super",
                "switch",
                "this",
                "throw",
                "true",
                "try",
                "typeof",
                "var",
                "void",
                "while",
                "with",
                "implements",
                "interface",
                "let",
                "package",
                "private",
                "protected",
                "public",
                "static",
                "yield"
            };

            var colorizer = _session.GetColorizer();
            foreach (var keyword in keywords)
            {
                var text = String.Format(@"id.{0}", keyword);

                var colorization = colorizer.Colorize(text, text.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
                // id
                Assert.AreEqual(AuthorTokenKind.atkIdentifier, colorization.Next().Kind);
                // .
                Assert.AreEqual(AuthorTokenKind.atkDot, colorization.Next().Kind);
                // keywrod
                Assert.AreEqual(AuthorTokenKind.atkIdentifier, colorization.Next().Kind, String.Format("keyword '{0}' is not colorized as an identifier.", keyword));
                Marshal.ReleaseComObject(colorization);
            }
            Marshal.ReleaseComObject(colorizer);
        }

        [TestMethod]
        [WorkItem(484536)]
        public void EmbeddedUnicodeLineBreak()
        {
            var colorizer = _session.GetColorizer();
            var colorization = colorizer.Colorize(EmbeddedUnicodeLineBreakText, EmbeddedUnicodeLineBreakText.Length, AuthorSourceState.SOURCE_STATE_INITIAL);
            while (true)
            {
                var token = colorization.Next();
                if (token.Kind == AuthorTokenKind.atkEnd) break;
            }

        }
        const string EmbeddedUnicodeLineBreakText = "                minChars: 0, //\u53cc\u51fb\u7a7a\u767d\u6587\u672c\u6846\u65f6\u663e\u793a\u5168\u90e8\u63d0\u793a\u6570\u636e\u2028";
    }
}