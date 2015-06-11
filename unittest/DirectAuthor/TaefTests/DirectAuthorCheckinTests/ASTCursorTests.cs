using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class ASTCursorTests : DirectAuthorTest
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

        internal void AssertNodeType(AuthorParseNodeDetails currentNode, string typeName)
        {
            switch (typeName)
            {
                case "for":
                    Assert.AreEqual(AuthorParseNodeKind.apnkFor, currentNode.Kind);
                    break;
                case "int":
                    Assert.AreEqual(AuthorParseNodeKind.apnkInt, currentNode.Kind);
                    break;
                case "func":
                    Assert.AreEqual(AuthorParseNodeKind.apnkFncDecl, currentNode.Kind);
                    break;
                case "var":
                    Assert.AreEqual(AuthorParseNodeKind.apnkVarDecl, currentNode.Kind);
                    break;
                case "let":
                    Assert.AreEqual(AuthorParseNodeKind.apnkLetDecl, currentNode.Kind);
                    break;
                case "const":
                    Assert.AreEqual(AuthorParseNodeKind.apnkConstDecl, currentNode.Kind);
                    break;
                case "while":
                    Assert.AreEqual(AuthorParseNodeKind.apnkWhile, currentNode.Kind);
                    break;
                case "block":
                    Assert.AreEqual(AuthorParseNodeKind.apnkBlock, currentNode.Kind);
                    break;
                case "if":
                    Assert.AreEqual(AuthorParseNodeKind.apnkIf, currentNode.Kind);
                    break;
                case "prog":
                    Assert.AreEqual(AuthorParseNodeKind.apnkProg, currentNode.Kind);
                    break;
                case "empty":
                    Assert.AreEqual(AuthorParseNodeKind.apnkEmptyNode, currentNode.Kind);
                    break;
                case "list":
                    Assert.AreEqual(AuthorParseNodeKind.apnkList, currentNode.Kind);
                    break;
                case "inc":
                    Assert.AreEqual(AuthorParseNodeKind.apnkIncPost, currentNode.Kind);
                    break;
                case "with":
                    Assert.AreEqual(AuthorParseNodeKind.apnkWith, currentNode.Kind);
                    break;
                case "forIn":
                    Assert.AreEqual(AuthorParseNodeKind.apnkForIn, currentNode.Kind);
                    break;
                case "doWhile":
                    Assert.AreEqual(AuthorParseNodeKind.apnkDoWhile, currentNode.Kind);
                    break;
                case "case":
                    Assert.AreEqual(AuthorParseNodeKind.apnkCase, currentNode.Kind);
                    break;
                case "default":
                    Assert.AreEqual(AuthorParseNodeKind.apnkDefaultCase, currentNode.Kind);
                    break;
                default:
                    Assert.Fail("Unexpected value.");
                    break;
            };
        }

        internal static string GetNodeSerializedString(AuthorParseNode node, IAuthorParseNodeCursor cursor)
        {
            StringBuilder output = new StringBuilder();

            StringBuilder tabString = new StringBuilder();
            for (int i = 0; i < node.Level; i++)
                tabString.Append("\t");

            output.Append(tabString.ToString() + "{");
            output.AppendFormat("kind  : {0},\t ", node.Details.Kind);
            output.AppendFormat("min   : {0},\t ", node.Details.StartOffset);
            output.AppendFormat("lim   : {0},\t ", node.Details.EndOffset);
            if (node.Label > 0)
                output.AppendFormat("lable : \"{0}\",\t ", cursor.GetPropertyById(node.Label));
            if (node.Name > 0)
                output.AppendFormat("name  : \"{0}\",\t ", cursor.GetPropertyById(node.Name));
            output.AppendFormat("level : {0},\t ", node.Level);
            output.AppendFormat("ChildLabel : {0}", node.EdgeLabel);
            output.Append("}");
            output.AppendLine();

            return output.ToString();
        }

        internal static void DumpTree(IAuthorParseNodeCursor cursor)
        {
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            Console.WriteLine(serializedNodeString);
        }

        [TestMethod]
        public void CursorMoveUp()
        {
            PerformRequests(CursorMoveUpInputString, (context, offset, data, index) =>
            {
                var cursor = context.GetASTCursor();
                var currentNode = cursor.SeekToOffset(offset);
                var parentNode = cursor.MoveUp();

                while (parentNode.Kind == AuthorParseNodeKind.apnkList || (parentNode.Flags & AuthorParseNodeFlags.apnfSyntheticNode) != 0)
                {
                    parentNode = cursor.MoveUp();
                }

                AssertNodeType(parentNode, data);
            });
        }
        #region Test Data
        public const string CursorMoveUpInputString = @"
function f1(|prog|)
{
    function f2()
    {
        while (|while|true)
        {
            if (false)
            {|if|
                for(;|block|;)
                    var x|for| =|var|0;
            }
            else
            {
                x = x + 1;
            }
        }
    }|func|
}
";
        #endregion

        [TestMethod]
        public void CursorMoveUpAboveRootNode()
        {
            var file = _session.FileFromText("var x;");
            var context = _session.OpenContext(file);
            context.Update();
            var cursor = context.GetASTCursor();
            var currentNode = cursor.Current();
            Assert.AreEqual(AuthorParseNodeKind.apnkProg, currentNode.Kind);

            // move up to the parent of the root node
            currentNode = cursor.MoveUp();
            Assert.AreEqual(AuthorParseNodeKind.apnkEmptyNode, currentNode.Kind);

            // move up again
            currentNode = cursor.MoveUp();
            Assert.AreEqual(AuthorParseNodeKind.apnkEmptyNode, currentNode.Kind);
        }

        [TestMethod]
        [WorkItem(id: 684333)]
        public void When_a_let_declaration_is_a_for_loop_initializer_the_for_node_should_not_have_the_same_start_offset()
        {
            var file = _session.FileFromText("for(let i = 0;;) { }");
            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();

            var forNode = cursor.SeekToOffset(offset: 0);

            Assert.AreEqual(expected: AuthorParseNodeKind.apnkFor, actual: forNode.Kind);
            Assert.AreEqual(expected: 0, actual: forNode.StartOffset);

            var letNode = cursor.Child(AuthorParseNodeEdge.apneInitialization, index: 0);
            Assert.AreEqual(expected: AuthorParseNodeKind.apnkLetDecl, actual: letNode.Kind);
            Assert.AreEqual(expected: 4, actual: letNode.StartOffset);
        }

        public void ExecuteEnclosingNodeTest(string text, bool execludeEndOffset)
        {
            int startOffset = 0;
            PerformRequests(text, (context, offset, data, index) =>
            {
                if (data.StartsWith("start"))
                {
                    startOffset = offset;
                }
                else
                {
                    var cursor = context.GetASTCursor();
                    var currentNode = cursor.MoveToEnclosingNode(startOffset, offset, execludeEndOffset);

                    while (currentNode.Kind == AuthorParseNodeKind.apnkList || (currentNode.Flags & AuthorParseNodeFlags.apnfSyntheticNode) != 0)
                    {
                        currentNode = cursor.MoveUp();
                    }

                    AssertNodeType(currentNode, data);
                }
            });
        }

        [TestMethod]
        public void CursorMoveToEnclosingNode()
        {
            ExecuteEnclosingNodeTest(@"
                function f1()
                {
                    function f2()
                    {
                        while (|start|true)
                        {
                            if (false)
                            {|while|
                                for(;|while|;)
                                {
                                    var x|while| =|while|0;
                                }
                            }
                            else
                            {
                                x = x + 1;
                            }
                        }
                    }|func|
                }
                ", false);
        }

        [TestMethod]
        [WorkItem(212664)]
        public void CursorMoveToEnclosingNodeExludeEndOffset()
        {
            ExecuteEnclosingNodeTest(@"
                    function foo()
                    {
                        while (|start|true)
                        {
                        }|func|
                    }|prog|
                ", true);
        }

        [TestMethod]
        public void CursorSeekToOffset()
        {
            PerformRequests(CursorSeekToOffsetInputString, (context, offset, data, index) =>
            {
                var cursor = context.GetASTCursor();
                var currentNode = cursor.SeekToOffset(offset);
                AssertNodeType(currentNode, data);
            });
        }
        #region Test Data
        public const string CursorSeekToOffsetInputString = @"
function f1()
{
    function f2(|func|)
    {
        while (true)|while|
        {
            if (false)
            {|block|
                for(;|for|;)
                {
                    var x|var| =|int|0;
                }
            }
            else
            {
                x = x + 1;
            }
        }
    }
}
";
        #endregion

        [TestMethod]
        [WorkItem(212664)]
        public void CursorSeekToOffsetExcludeEndOffset()
        {
            PerformRequests(CursorSeekToOffsetExcludeEndOffsetInputString, (context, offset, data, index) =>
            {
                var cursor = context.GetASTCursor();
                var currentNode = cursor.SeekToOffset(offset, true);
                AssertNodeType(currentNode, data);
            });
        }
        #region Test Data
        public const string CursorSeekToOffsetExcludeEndOffsetInputString = @"
var a|list|;|var|var c

for ( var i|for|;i <|int|9; i|inc|++;)
    a|empty|";
        #endregion

        [TestMethod]
        public void CursorTreeWalking()
        {
            var file = _session.FileFromText(CursorTreeWalkingInputString);
            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(CursorTreeWalkingExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string CursorTreeWalkingInputString = @"
        // var decl
        var x;
        var a = 0, b, c = ""string"";

        // for statement
        for (i = 0; i < 100; i++)
            for (j = 0;; j++)
                for (;;)
                {
                    i = i + j;
                }

        // if statement
        if (x == 0)
        {
            if (x == 1)
                x = 0;
        }
        else
        {
            x = 3;
        }
        
        // switch statement
        switch (x)
        {};

        switch (x)
        {
            case 0: break;
        };

        switch (x)
        {
            default: break;
        };

        switch (x)
        {
            case 0: break;
            default: break;
            case 1: break;
        };
        
        // while statement
        label: while (x)
        {
            label2: while (0)
            {
                break label;
            }
        }

        // function decl / call
        (function (x)
        {
            return x;
        })(a);
        
        function f(p1,p2,p3,p4,p5)
        {

        }
        x = new f(1, 3.4, ""string"", true, this);


        // object
        x = { a:1, b:3, c: {d:1}};

        // with statement
        with(x)
        {
            a = 0;
            c.d = 2;
        }

        // try-catch
        try
        {
            throw new Error(""message"");
        }
        catch (e)
        {

        }
        finally
        {

        }

        // block
        {
            {   
                // empty block
            }
            {
                // empty block
            }
        }
        ";

        public const string CursorTreeWalkingExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 1725,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 31,	 lim : 1715,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 31,	 lim : 36,	 name : ""x"",	 IdentifierMin : 35,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkVarDeclList,	 min : 47,	 lim : 73,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkVarDecl,	 min : 47,	 lim : 56,	 name : ""a"",	 IdentifierMin : 51,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkInt,	 min : 55,	 lim : 56,	 intValue : 0,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 58,	 lim : 59,	 name : ""b"",	 IdentifierMin : 58,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 61,	 lim : 73,	 name : ""c"",	 IdentifierMin : 61,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkStr,	 min : 65,	 lim : 73,	 stringValue : ""string"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkFor,	 min : 112,	 lim : 264,	 LParen : 116,	 RParen : 136,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkAsg,	 min : 117,	 lim : 122,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 117,	 lim : 118,	 name : ""i"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 121,	 lim : 122,	 intValue : 0,	 semicolon: ""none""}
apneCondition :	 {kind : apnkLt,	 min : 124,	 lim : 131,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 124,	 lim : 125,	 name : ""i"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 128,	 lim : 131,	 intValue : 100,	 semicolon: ""none""}
apneIncrement :	 {kind : apnkIncPost,	 min : 133,	 lim : 136,	 semicolon: ""none""}
apneOperand :	 {kind : apnkName,	 min : 133,	 lim : 134,	 name : ""i"",	 semicolon: ""none""}
apneBody :	 {kind : apnkFor,	 min : 151,	 lim : 264,	 LParen : 155,	 RParen : 167,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkAsg,	 min : 156,	 lim : 161,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 156,	 lim : 157,	 name : ""j"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 160,	 lim : 161,	 intValue : 0,	 semicolon: ""none""}
apneIncrement :	 {kind : apnkIncPost,	 min : 164,	 lim : 167,	 semicolon: ""none""}
apneOperand :	 {kind : apnkName,	 min : 164,	 lim : 165,	 name : ""j"",	 semicolon: ""none""}
apneBody :	 {kind : apnkFor,	 min : 186,	 lim : 264,	 LParen : 190,	 RParen : 193,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 212,	 lim : 264,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkAsg,	 min : 235,	 lim : 244,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 235,	 lim : 236,	 name : ""i"",	 semicolon: ""none""}
apneRight :	 {kind : apnkAdd,	 min : 239,	 lim : 244,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 239,	 lim : 240,	 name : ""i"",	 semicolon: ""none""}
apneRight :	 {kind : apnkName,	 min : 243,	 lim : 244,	 name : ""j"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkIf,	 min : 301,	 lim : 439,	 LParen : 304,	 RParen : 311,	 semicolon: ""none""}
apneCondition :	 {kind : apnkEq,	 min : 305,	 lim : 311,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 305,	 lim : 306,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 310,	 lim : 311,	 intValue : 0,	 semicolon: ""none""}
apneThen :	 {kind : apnkBlock,	 min : 322,	 lim : 383,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkIf,	 min : 337,	 lim : 371,	 LParen : 340,	 RParen : 347,	 semicolon: ""none""}
apneCondition :	 {kind : apnkEq,	 min : 341,	 lim : 347,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 341,	 lim : 342,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 346,	 lim : 347,	 intValue : 1,	 semicolon: ""none""}
apneThen :	 {kind : apnkAsg,	 min : 366,	 lim : 371,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 366,	 lim : 367,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 370,	 lim : 371,	 intValue : 0,	 semicolon: ""none""}
apneElse :	 {kind : apnkBlock,	 min : 407,	 lim : 439,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkAsg,	 min : 422,	 lim : 427,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 422,	 lim : 423,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 426,	 lim : 427,	 intValue : 3,	 semicolon: ""none""}
apneListItem :	 {kind : apnkSwitch,	 min : 488,	 lim : 510,	 LParen : 495,	RParen : 497,	LCurly : 508,	RCurly : 509,	semicolon: ""none""}
apneValue :	 {kind : apnkName,	 min : 496,	 lim : 497,	 name : ""x"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkSwitch,	 min : 523,	 lim : 583,	 LParen : 530,	RParen : 532,	LCurly : 543,	RCurly : 582,	semicolon: ""none""}
apneValue :	 {kind : apnkName,	 min : 531,	 lim : 532,	 name : ""x"",	 semicolon: ""none""}
apneCase :	 {kind : apnkCase,	 min : 558,	 lim : 582,	 LParen : 0,	RParen : 0,	LCurly : 0,	RCurly : 0,	semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 563,	 lim : 564,	 intValue : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 558,	 lim : 582,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 566,	 lim : 571,	 targetLabel : """",	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkSwitch,	 min : 596,	 lim : 657,	 LParen : 603,	RParen : 605,	LCurly : 616,	RCurly : 656,	semicolon: ""none""}
apneValue :	 {kind : apnkName,	 min : 604,	 lim : 605,	 name : ""x"",	 semicolon: ""none""}
apneCase :	 {kind : apnkDefaultCase,	 min : 631,	 lim : 656,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 631,	 lim : 656,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 640,	 lim : 645,	 targetLabel : """",	 semicolon: ""explicit""}
apneDefaultCase :	 {kind : apnkDefaultCase,	 min : 631,	 lim : 656,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 631,	 lim : 656,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 640,	 lim : 645,	 targetLabel : """",	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkSwitch,	 min : 670,	 lim : 787,	 LParen : 677,	RParen : 679,	LCurly : 690,	RCurly : 786,	semicolon: ""none""}
apneValue :	 {kind : apnkName,	 min : 678,	 lim : 679,	 name : ""x"",	 semicolon: ""none""}
apneCase :	 {kind : apnkCase,	 min : 705,	 lim : 732,	 LParen : 0,	RParen : 0,	LCurly : 0,	RCurly : 0,	semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 710,	 lim : 711,	 intValue : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 705,	 lim : 718,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 713,	 lim : 718,	 targetLabel : """",	 semicolon: ""explicit""}
apneCase :	 {kind : apnkDefaultCase,	 min : 733,	 lim : 761,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 733,	 lim : 747,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 742,	 lim : 747,	 targetLabel : """",	 semicolon: ""explicit""}
apneCase :	 {kind : apnkCase,	 min : 762,	 lim : 786,	 LParen : 0,	RParen : 0,	LCurly : 0,	RCurly : 0,	semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 767,	 lim : 768,	 intValue : 1,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 762,	 lim : 786,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 770,	 lim : 775,	 targetLabel : """",	 semicolon: ""explicit""}
apneDefaultCase :	 {kind : apnkDefaultCase,	 min : 733,	 lim : 761,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 733,	 lim : 747,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 742,	 lim : 747,	 targetLabel : """",	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkWhile,	 min : 843,	 lim : 965,	 label : ""label"",	 LParen : 849,	 RParen : 851,	 semicolon: ""none""}
apneCondition :	 {kind : apnkName,	 min : 850,	 lim : 851,	 name : ""x"",	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 862,	 lim : 965,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkWhile,	 min : 885,	 lim : 954,	 label : ""label2"",	 LParen : 891,	 RParen : 893,	 semicolon: ""none""}
apneCondition :	 {kind : apnkInt,	 min : 892,	 lim : 893,	 intValue : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 908,	 lim : 954,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 927,	 lim : 938,	 targetLabel : ""label"",	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkCall,	 min : 1010,	 lim : 1072,	 semicolon: ""explicit""}
apneTarget :	 {kind : apnkFncDecl,	 min : 1010,	 lim : 1069,	 name : """",	 IdentifierMin : 0,	 FunctionKeywordMin : 1011,	 LParen : 1020,	RParen : 1022,	LCurly : 1033,	RCurly : 1067,	semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1021,	 lim : 1022,	 name : ""x"",	 IdentifierMin : 1021,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 1048,	 lim : 1068,	 semicolon: ""none""}
apneListItem :	 {kind : apnkReturn,	 min : 1048,	 lim : 1056,	 semicolon: ""explicit""}
apneValue :	 {kind : apnkName,	 min : 1055,	 lim : 1056,	 name : ""x"",	 semicolon: ""none""}
apneArguments :	 {kind : apnkName,	 min : 1070,	 lim : 1071,	 name : ""a"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 1093,	 lim : 1143,	 name : ""f"",	 IdentifierMin : 1102,	 FunctionKeywordMin : 1093,	 LParen : 1103,	RParen : 1118,	LCurly : 1129,	RCurly : 1142,	semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1104,	 lim : 1106,	 name : ""p1"",	 IdentifierMin : 1104,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1107,	 lim : 1109,	 name : ""p2"",	 IdentifierMin : 1107,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1110,	 lim : 1112,	 name : ""p3"",	 IdentifierMin : 1110,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1113,	 lim : 1115,	 name : ""p4"",	 IdentifierMin : 1113,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 1116,	 lim : 1118,	 name : ""p5"",	 IdentifierMin : 1116,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 1153,	 lim : 1192,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 1153,	 lim : 1154,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkNew,	 min : 1157,	 lim : 1192,	 semicolon: ""none""}
apneTarget :	 {kind : apnkName,	 min : 1161,	 lim : 1162,	 name : ""f"",	 semicolon: ""none""}
apneArguments :	 {kind : apnkList,	 min : 1163,	 lim : 1191,	 semicolon: ""none""}
apneListItem :	 {kind : apnkInt,	 min : 1163,	 lim : 1164,	 intValue : 1,	 semicolon: ""none""}
apneListItem :	 {kind : apnkFlt,	 min : 1166,	 lim : 1169,	 floatValue : 3.4,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 1171,	 lim : 1179,	 stringValue : ""string"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkTrue,	 min : 1181,	 lim : 1185,	 semicolon: ""none""}
apneListItem :	 {kind : apnkThis,	 min : 1187,	 lim : 1191,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 1226,	 lim : 1251,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 1226,	 lim : 1227,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkObject,	 min : 1230,	 lim : 1251,	 LCurly : 1230,	 RCurly : 1250,	 semicolon: ""none""}
apneMembers :	 {kind : apnkList,	 min : 1232,	 lim : 1250,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMember,	 min : 1232,	 lim : 1235,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 1232,	 lim : 1233,	 stringValue : ""a"",	 semicolon: ""none""}
apneMember :	 {kind : apnkInt,	 min : 1234,	 lim : 1235,	 intValue : 1,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMember,	 min : 1237,	 lim : 1240,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 1237,	 lim : 1238,	 stringValue : ""b"",	 semicolon: ""none""}
apneMember :	 {kind : apnkInt,	 min : 1239,	 lim : 1240,	 intValue : 3,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMember,	 min : 1242,	 lim : 1250,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 1242,	 lim : 1243,	 stringValue : ""c"",	 semicolon: ""none""}
apneMember :	 {kind : apnkObject,	 min : 1245,	 lim : 1250,	 LCurly : 1245,	 RCurly : 1249,	 semicolon: ""none""}
apneMembers :	 {kind : apnkMember,	 min : 1246,	 lim : 1249,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 1246,	 lim : 1247,	 stringValue : ""d"",	 semicolon: ""none""}
apneMember :	 {kind : apnkInt,	 min : 1248,	 lim : 1249,	 intValue : 1,	 semicolon: ""none""}
apneListItem :	 {kind : apnkWith,	 min : 1291,	 lim : 1362,	 LParen : 1295,	 RParen : 1297,	 semicolon: ""none""}
apneObject :	 {kind : apnkName,	 min : 1296,	 lim : 1297,	 name : ""x"",	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 1308,	 lim : 1362,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkList,	 min : 1323,	 lim : 1350,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 1323,	 lim : 1328,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 1323,	 lim : 1324,	 name : ""a"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 1327,	 lim : 1328,	 intValue : 0,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 1343,	 lim : 1350,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkDot,	 min : 1343,	 lim : 1346,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 1343,	 lim : 1344,	 name : ""c"",	 semicolon: ""none""}
apneRight :	 {kind : apnkName,	 min : 1345,	 lim : 1346,	 name : ""d"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 1349,	 lim : 1350,	 intValue : 2,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 1396,	 lim : 1546,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkTryFinally,	 min : 1396,	 lim : 1546,	 semicolon: ""none""}
apneTry :	 {kind : apnkTry,	 min : 1396,	 lim : 1505,	 LCurly : 0,	 RCurly : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkTryCatch,	 min : 1396,	 lim : 1505,	 semicolon: ""none""}
apneTry :	 {kind : apnkTry,	 min : 1396,	 lim : 1462,	 LCurly : 1409,	 RCurly : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 1409,	 lim : 1462,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkThrow,	 min : 1424,	 lim : 1450,	 semicolon: ""explicit""}
apneValue :	 {kind : apnkNew,	 min : 1430,	 lim : 1450,	 semicolon: ""none""}
apneTarget :	 {kind : apnkName,	 min : 1434,	 lim : 1439,	 name : ""Error"",	 semicolon: ""none""}
apneArguments :	 {kind : apnkStr,	 min : 1440,	 lim : 1449,	 stringValue : ""message"",	 semicolon: ""none""}
apneCatch :	 {kind : apnkCatch,	 min : 1472,	 lim : 1505,	 LParen : 1478,	RParen : 1480,	LCurly : 1491,	RCurly : 0,	semicolon: ""none""}
apneVariable :	 {kind : apnkName,	 min : 1479,	 lim : 1480,	 name : ""e"",	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 1491,	 lim : 1505,	 semicolon: ""none""}
apneFinally :	 {kind : apnkFinally,	 min : 1515,	 lim : 1546,	 LCurly : 1532,	 RCurly : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 1532,	 lim : 1546,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 1576,	 lim : 1715,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkList,	 min : 1591,	 lim : 1704,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 1591,	 lim : 1642,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 1656,	 lim : 1704,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        public void GetValueMethods()
        {
            var file = _session.FileFromText(GetValueMethodsInputString);
            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(GetValueMethodsExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string GetValueMethodsInputString = @"
        var x;
        x = ""string"";    // GetStringValue
        x = 17;            // GetIntValue
        x = 2.456767;      // GetFltValue
        x =/^\d$/gmi;      // GetRegExpValue
        
        
        label: while (x)   // GetStatementLabel
        {
            label2: while (0)
            {
                break label;  //GetTargetLabel
            }
        }";

        public const string GetValueMethodsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 392,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 10,	 lim : 392,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 10,	 lim : 15,	 name : ""x"",	 IdentifierMin : 14,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkAsg,	 min : 26,	 lim : 38,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 26,	 lim : 27,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkStr,	 min : 30,	 lim : 38,	 stringValue : ""string"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 70,	 lim : 76,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 70,	 lim : 71,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 74,	 lim : 76,	 intValue : 17,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 113,	 lim : 125,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 113,	 lim : 114,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkFlt,	 min : 117,	 lim : 125,	 floatValue : 2.456767,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 156,	 lim : 168,	 semicolon: ""explicit""}
apneLeft :	 {kind : apnkName,	 min : 156,	 lim : 157,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkRegExp,	 min : 159,	 lim : 168,	 regexpValue : ""^\d$"",	  regexpOptions: ""areoGlobal, areoIgnoreCase, areoMultiline"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkWhile,	 min : 229,	 lim : 392,	 label : ""label"",	 LParen : 235,	 RParen : 237,	 semicolon: ""none""}
apneCondition :	 {kind : apnkName,	 min : 236,	 lim : 237,	 name : ""x"",	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 271,	 lim : 392,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkWhile,	 min : 294,	 lim : 381,	 label : ""label2"",	 LParen : 300,	 RParen : 302,	 semicolon: ""none""}
apneCondition :	 {kind : apnkInt,	 min : 301,	 lim : 302,	 intValue : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 317,	 lim : 381,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkBreak,	 min : 336,	 lim : 347,	 targetLabel : ""label"",	 semicolon: ""explicit""}
";
        #endregion

        [TestMethod]
        public void GetNodeProprties()
        {
            var file = _session.FileFromText(GetNodeProprtiesInputString);
            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(GetNodeProprtiesExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string GetNodeProprtiesInputString = @"
        var x = 2;
        switch (x)                  // apnpLParenMin, apnpRParenMin
        { }                         //apnpLCurlyMin, apnpRCurlyMin

        function f ()
        {
            var x = function f2_1()   
            {
                var y = function f3()
                {
                    // most nested
                }
            }    
            var x = function f2_2()   
            {
            }
        }";

        public const string GetNodeProprtiesExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 461,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 10,	 lim : 461,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 10,	 lim : 19,	 name : ""x"",	 IdentifierMin : 14,	 semicolon: ""explicit""}
apneInitialization :	 {kind : apnkInt,	 min : 18,	 lim : 19,	 intValue : 2,	 semicolon: ""none""}
apneListItem :	 {kind : apnkSwitch,	 min : 30,	 lim : 102,	 LParen : 37,	RParen : 39,	LCurly : 99,	RCurly : 101,	semicolon: ""none""}
apneValue :	 {kind : apnkName,	 min : 38,	 lim : 39,	 name : ""x"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 169,	 lim : 461,	 name : ""f"",	 IdentifierMin : 178,	 FunctionKeywordMin : 169,	 LParen : 180,	RParen : 181,	LCurly : 192,	RCurly : 460,	semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 207,	 lim : 461,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 207,	 lim : 376,	 name : ""x"",	 IdentifierMin : 211,	 semicolon: ""automatic""}
apneInitialization :	 {kind : apnkFncDecl,	 min : 215,	 lim : 376,	 name : ""f2_1"",	 IdentifierMin : 224,	 FunctionKeywordMin : 215,	 LParen : 228,	RParen : 229,	LCurly : 247,	RCurly : 375,	semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 266,	 lim : 376,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 266,	 lim : 361,	 name : ""y"",	 IdentifierMin : 270,	 semicolon: ""automatic""}
apneInitialization :	 {kind : apnkFncDecl,	 min : 274,	 lim : 361,	 name : ""f3"",	 IdentifierMin : 283,	 FunctionKeywordMin : 274,	 LParen : 285,	RParen : 286,	LCurly : 305,	RCurly : 360,	semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 394,	 lim : 450,	 name : ""x"",	 IdentifierMin : 398,	 semicolon: ""automatic""}
apneInitialization :	 {kind : apnkFncDecl,	 min : 402,	 lim : 450,	 name : ""f2_2"",	 IdentifierMin : 411,	 FunctionKeywordMin : 402,	 LParen : 415,	RParen : 416,	LCurly : 434,	RCurly : 449,	semicolon: ""none""}
";
        #endregion

        [TestMethod]
        public void GetSubTree()
        {
            var file = _session.FileFromText(CursorTreeWalkingInputString);

            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            Assert.AreEqual(GetSubTreeExpectedString, serializedNodeString.ToString());
        }
        #region Test Data
        public const string GetSubTreeExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 1725,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 31,	 lim   : 1715,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkVarDecl,	 min   : 31,	 lim   : 36,	 name  : ""x"",	 level : 2,	 ChildLabel : apneListItem}
		{kind  : apnkVarDeclList,	 min   : 47,	 lim   : 73,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkVarDecl,	 min   : 47,	 lim   : 56,	 name  : ""a"",	 level : 3,	 ChildLabel : apneListItem}
				{kind  : apnkInt,	 min   : 55,	 lim   : 56,	 level : 4,	 ChildLabel : apneInitialization}
			{kind  : apnkVarDecl,	 min   : 58,	 lim   : 59,	 name  : ""b"",	 level : 3,	 ChildLabel : apneListItem}
			{kind  : apnkVarDecl,	 min   : 61,	 lim   : 73,	 name  : ""c"",	 level : 3,	 ChildLabel : apneListItem}
				{kind  : apnkStr,	 min   : 65,	 lim   : 73,	 level : 4,	 ChildLabel : apneInitialization}
		{kind  : apnkFor,	 min   : 112,	 lim   : 264,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkAsg,	 min   : 117,	 lim   : 122,	 level : 3,	 ChildLabel : apneInitialization}
				{kind  : apnkName,	 min   : 117,	 lim   : 118,	 name  : ""i"",	 level : 4,	 ChildLabel : apneLeft}
				{kind  : apnkInt,	 min   : 121,	 lim   : 122,	 level : 4,	 ChildLabel : apneRight}
			{kind  : apnkLt,	 min   : 124,	 lim   : 131,	 level : 3,	 ChildLabel : apneCondition}
				{kind  : apnkName,	 min   : 124,	 lim   : 125,	 name  : ""i"",	 level : 4,	 ChildLabel : apneLeft}
				{kind  : apnkInt,	 min   : 128,	 lim   : 131,	 level : 4,	 ChildLabel : apneRight}
			{kind  : apnkIncPost,	 min   : 133,	 lim   : 136,	 level : 3,	 ChildLabel : apneIncrement}
				{kind  : apnkName,	 min   : 133,	 lim   : 134,	 name  : ""i"",	 level : 4,	 ChildLabel : apneOperand}
			{kind  : apnkFor,	 min   : 151,	 lim   : 264,	 level : 3,	 ChildLabel : apneBody}
				{kind  : apnkAsg,	 min   : 156,	 lim   : 161,	 level : 4,	 ChildLabel : apneInitialization}
					{kind  : apnkName,	 min   : 156,	 lim   : 157,	 name  : ""j"",	 level : 5,	 ChildLabel : apneLeft}
					{kind  : apnkInt,	 min   : 160,	 lim   : 161,	 level : 5,	 ChildLabel : apneRight}
				{kind  : apnkIncPost,	 min   : 164,	 lim   : 167,	 level : 4,	 ChildLabel : apneIncrement}
					{kind  : apnkName,	 min   : 164,	 lim   : 165,	 name  : ""j"",	 level : 5,	 ChildLabel : apneOperand}
				{kind  : apnkFor,	 min   : 186,	 lim   : 264,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBlock,	 min   : 212,	 lim   : 264,	 level : 5,	 ChildLabel : apneBody}
						{kind  : apnkAsg,	 min   : 235,	 lim   : 244,	 level : 6,	 ChildLabel : apneBlockBody}
							{kind  : apnkName,	 min   : 235,	 lim   : 236,	 name  : ""i"",	 level : 7,	 ChildLabel : apneLeft}
							{kind  : apnkAdd,	 min   : 239,	 lim   : 244,	 level : 7,	 ChildLabel : apneRight}
								{kind  : apnkName,	 min   : 239,	 lim   : 240,	 name  : ""i"",	 level : 8,	 ChildLabel : apneLeft}
								{kind  : apnkName,	 min   : 243,	 lim   : 244,	 name  : ""j"",	 level : 8,	 ChildLabel : apneRight}
		{kind  : apnkIf,	 min   : 301,	 lim   : 439,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkEq,	 min   : 305,	 lim   : 311,	 level : 3,	 ChildLabel : apneCondition}
				{kind  : apnkName,	 min   : 305,	 lim   : 306,	 name  : ""x"",	 level : 4,	 ChildLabel : apneLeft}
				{kind  : apnkInt,	 min   : 310,	 lim   : 311,	 level : 4,	 ChildLabel : apneRight}
			{kind  : apnkBlock,	 min   : 322,	 lim   : 383,	 level : 3,	 ChildLabel : apneThen}
				{kind  : apnkIf,	 min   : 337,	 lim   : 371,	 level : 4,	 ChildLabel : apneBlockBody}
					{kind  : apnkEq,	 min   : 341,	 lim   : 347,	 level : 5,	 ChildLabel : apneCondition}
						{kind  : apnkName,	 min   : 341,	 lim   : 342,	 name  : ""x"",	 level : 6,	 ChildLabel : apneLeft}
						{kind  : apnkInt,	 min   : 346,	 lim   : 347,	 level : 6,	 ChildLabel : apneRight}
					{kind  : apnkAsg,	 min   : 366,	 lim   : 371,	 level : 5,	 ChildLabel : apneThen}
						{kind  : apnkName,	 min   : 366,	 lim   : 367,	 name  : ""x"",	 level : 6,	 ChildLabel : apneLeft}
						{kind  : apnkInt,	 min   : 370,	 lim   : 371,	 level : 6,	 ChildLabel : apneRight}
			{kind  : apnkBlock,	 min   : 407,	 lim   : 439,	 level : 3,	 ChildLabel : apneElse}
				{kind  : apnkAsg,	 min   : 422,	 lim   : 427,	 level : 4,	 ChildLabel : apneBlockBody}
					{kind  : apnkName,	 min   : 422,	 lim   : 423,	 name  : ""x"",	 level : 5,	 ChildLabel : apneLeft}
					{kind  : apnkInt,	 min   : 426,	 lim   : 427,	 level : 5,	 ChildLabel : apneRight}
		{kind  : apnkSwitch,	 min   : 488,	 lim   : 510,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 496,	 lim   : 497,	 name  : ""x"",	 level : 3,	 ChildLabel : apneValue}
		{kind  : apnkSwitch,	 min   : 523,	 lim   : 583,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 531,	 lim   : 532,	 name  : ""x"",	 level : 3,	 ChildLabel : apneValue}
			{kind  : apnkCase,	 min   : 558,	 lim   : 582,	 level : 3,	 ChildLabel : apneCase}
				{kind  : apnkInt,	 min   : 563,	 lim   : 564,	 level : 4,	 ChildLabel : apneValue}
				{kind  : apnkBlock,	 min   : 558,	 lim   : 582,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 566,	 lim   : 571,	 level : 5,	 ChildLabel : apneBlockBody}
		{kind  : apnkSwitch,	 min   : 596,	 lim   : 657,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 604,	 lim   : 605,	 name  : ""x"",	 level : 3,	 ChildLabel : apneValue}
			{kind  : apnkDefaultCase,	 min   : 631,	 lim   : 656,	 level : 3,	 ChildLabel : apneDefaultCase}
				{kind  : apnkBlock,	 min   : 631,	 lim   : 656,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 640,	 lim   : 645,	 level : 5,	 ChildLabel : apneBlockBody}
		{kind  : apnkSwitch,	 min   : 670,	 lim   : 787,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 678,	 lim   : 679,	 name  : ""x"",	 level : 3,	 ChildLabel : apneValue}
			{kind  : apnkCase,	 min   : 705,	 lim   : 732,	 level : 3,	 ChildLabel : apneCase}
				{kind  : apnkInt,	 min   : 710,	 lim   : 711,	 level : 4,	 ChildLabel : apneValue}
				{kind  : apnkBlock,	 min   : 705,	 lim   : 718,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 713,	 lim   : 718,	 level : 5,	 ChildLabel : apneBlockBody}
			{kind  : apnkDefaultCase,	 min   : 733,	 lim   : 761,	 level : 3,	 ChildLabel : apneDefaultCase}
				{kind  : apnkBlock,	 min   : 733,	 lim   : 747,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 742,	 lim   : 747,	 level : 5,	 ChildLabel : apneBlockBody}
			{kind  : apnkCase,	 min   : 762,	 lim   : 786,	 level : 3,	 ChildLabel : apneCase}
				{kind  : apnkInt,	 min   : 767,	 lim   : 768,	 level : 4,	 ChildLabel : apneValue}
				{kind  : apnkBlock,	 min   : 762,	 lim   : 786,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 770,	 lim   : 775,	 level : 5,	 ChildLabel : apneBlockBody}
		{kind  : apnkWhile,	 min   : 843,	 lim   : 965,	 lable : ""label"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 850,	 lim   : 851,	 name  : ""x"",	 level : 3,	 ChildLabel : apneCondition}
			{kind  : apnkBlock,	 min   : 862,	 lim   : 965,	 level : 3,	 ChildLabel : apneBody}
				{kind  : apnkWhile,	 min   : 885,	 lim   : 954,	 lable : ""label2"",	 level : 4,	 ChildLabel : apneBlockBody}
					{kind  : apnkInt,	 min   : 892,	 lim   : 893,	 level : 5,	 ChildLabel : apneCondition}
					{kind  : apnkBlock,	 min   : 908,	 lim   : 954,	 level : 5,	 ChildLabel : apneBody}
						{kind  : apnkBreak,	 min   : 927,	 lim   : 938,	 name  : ""label"",	 level : 6,	 ChildLabel : apneBlockBody}
		{kind  : apnkCall,	 min   : 1010,	 lim   : 1072,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkFncDecl,	 min   : 1010,	 lim   : 1069,	 level : 3,	 ChildLabel : apneTarget}
				{kind  : apnkVarDecl,	 min   : 1021,	 lim   : 1022,	 name  : ""x"",	 level : 4,	 ChildLabel : apneArgument}
				{kind  : apnkList,	 min   : 1048,	 lim   : 1068,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkReturn,	 min   : 1048,	 lim   : 1056,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkName,	 min   : 1055,	 lim   : 1056,	 name  : ""x"",	 level : 6,	 ChildLabel : apneValue}
			{kind  : apnkName,	 min   : 1070,	 lim   : 1071,	 name  : ""a"",	 level : 3,	 ChildLabel : apneArguments}
		{kind  : apnkFncDecl,	 min   : 1093,	 lim   : 1143,	 name  : ""f"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkVarDecl,	 min   : 1104,	 lim   : 1106,	 name  : ""p1"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 1107,	 lim   : 1109,	 name  : ""p2"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 1110,	 lim   : 1112,	 name  : ""p3"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 1113,	 lim   : 1115,	 name  : ""p4"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 1116,	 lim   : 1118,	 name  : ""p5"",	 level : 3,	 ChildLabel : apneArgument}
		{kind  : apnkAsg,	 min   : 1153,	 lim   : 1192,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 1153,	 lim   : 1154,	 name  : ""x"",	 level : 3,	 ChildLabel : apneLeft}
			{kind  : apnkNew,	 min   : 1157,	 lim   : 1192,	 level : 3,	 ChildLabel : apneRight}
				{kind  : apnkName,	 min   : 1161,	 lim   : 1162,	 name  : ""f"",	 level : 4,	 ChildLabel : apneTarget}
				{kind  : apnkList,	 min   : 1163,	 lim   : 1191,	 level : 4,	 ChildLabel : apneArguments}
					{kind  : apnkInt,	 min   : 1163,	 lim   : 1164,	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkFlt,	 min   : 1166,	 lim   : 1169,	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkStr,	 min   : 1171,	 lim   : 1179,	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkTrue,	 min   : 1181,	 lim   : 1185,	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkThis,	 min   : 1187,	 lim   : 1191,	 level : 5,	 ChildLabel : apneListItem}
		{kind  : apnkAsg,	 min   : 1226,	 lim   : 1251,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 1226,	 lim   : 1227,	 name  : ""x"",	 level : 3,	 ChildLabel : apneLeft}
			{kind  : apnkObject,	 min   : 1230,	 lim   : 1251,	 level : 3,	 ChildLabel : apneRight}
				{kind  : apnkList,	 min   : 1232,	 lim   : 1250,	 level : 4,	 ChildLabel : apneMembers}
					{kind  : apnkMember,	 min   : 1232,	 lim   : 1235,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 1232,	 lim   : 1233,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkInt,	 min   : 1234,	 lim   : 1235,	 level : 6,	 ChildLabel : apneMember}
					{kind  : apnkMember,	 min   : 1237,	 lim   : 1240,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 1237,	 lim   : 1238,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkInt,	 min   : 1239,	 lim   : 1240,	 level : 6,	 ChildLabel : apneMember}
					{kind  : apnkMember,	 min   : 1242,	 lim   : 1250,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 1242,	 lim   : 1243,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkObject,	 min   : 1245,	 lim   : 1250,	 level : 6,	 ChildLabel : apneMember}
							{kind  : apnkMember,	 min   : 1246,	 lim   : 1249,	 level : 7,	 ChildLabel : apneMembers}
								{kind  : apnkStr,	 min   : 1246,	 lim   : 1247,	 level : 8,	 ChildLabel : apneTarget}
								{kind  : apnkInt,	 min   : 1248,	 lim   : 1249,	 level : 8,	 ChildLabel : apneMember}
		{kind  : apnkWith,	 min   : 1291,	 lim   : 1362,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 1296,	 lim   : 1297,	 name  : ""x"",	 level : 3,	 ChildLabel : apneObject}
			{kind  : apnkBlock,	 min   : 1308,	 lim   : 1362,	 level : 3,	 ChildLabel : apneBody}
				{kind  : apnkList,	 min   : 1323,	 lim   : 1350,	 level : 4,	 ChildLabel : apneBlockBody}
					{kind  : apnkAsg,	 min   : 1323,	 lim   : 1328,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkName,	 min   : 1323,	 lim   : 1324,	 name  : ""a"",	 level : 6,	 ChildLabel : apneLeft}
						{kind  : apnkInt,	 min   : 1327,	 lim   : 1328,	 level : 6,	 ChildLabel : apneRight}
					{kind  : apnkAsg,	 min   : 1343,	 lim   : 1350,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkDot,	 min   : 1343,	 lim   : 1346,	 level : 6,	 ChildLabel : apneLeft}
							{kind  : apnkName,	 min   : 1343,	 lim   : 1344,	 name  : ""c"",	 level : 7,	 ChildLabel : apneLeft}
							{kind  : apnkName,	 min   : 1345,	 lim   : 1346,	 name  : ""d"",	 level : 7,	 ChildLabel : apneRight}
						{kind  : apnkInt,	 min   : 1349,	 lim   : 1350,	 level : 6,	 ChildLabel : apneRight}
		{kind  : apnkBlock,	 min   : 1396,	 lim   : 1546,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkTryFinally,	 min   : 1396,	 lim   : 1546,	 level : 3,	 ChildLabel : apneBlockBody}
				{kind  : apnkTry,	 min   : 1396,	 lim   : 1505,	 level : 4,	 ChildLabel : apneTry}
					{kind  : apnkTryCatch,	 min   : 1396,	 lim   : 1505,	 level : 5,	 ChildLabel : apneBody}
						{kind  : apnkTry,	 min   : 1396,	 lim   : 1462,	 level : 6,	 ChildLabel : apneTry}
							{kind  : apnkBlock,	 min   : 1409,	 lim   : 1462,	 level : 7,	 ChildLabel : apneBody}
								{kind  : apnkThrow,	 min   : 1424,	 lim   : 1450,	 level : 8,	 ChildLabel : apneBlockBody}
									{kind  : apnkNew,	 min   : 1430,	 lim   : 1450,	 level : 9,	 ChildLabel : apneValue}
										{kind  : apnkName,	 min   : 1434,	 lim   : 1439,	 name  : ""Error"",	 level : 10,	 ChildLabel : apneTarget}
										{kind  : apnkStr,	 min   : 1440,	 lim   : 1449,	 level : 10,	 ChildLabel : apneArguments}
						{kind  : apnkCatch,	 min   : 1472,	 lim   : 1505,	 level : 6,	 ChildLabel : apneCatch}
							{kind  : apnkName,	 min   : 1479,	 lim   : 1480,	 name  : ""e"",	 level : 7,	 ChildLabel : apneVariable}
							{kind  : apnkBlock,	 min   : 1491,	 lim   : 1505,	 level : 7,	 ChildLabel : apneBody}
				{kind  : apnkFinally,	 min   : 1515,	 lim   : 1546,	 level : 4,	 ChildLabel : apneFinally}
					{kind  : apnkBlock,	 min   : 1532,	 lim   : 1546,	 level : 5,	 ChildLabel : apneBody}
		{kind  : apnkBlock,	 min   : 1576,	 lim   : 1715,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkList,	 min   : 1591,	 lim   : 1704,	 level : 3,	 ChildLabel : apneBlockBody}
				{kind  : apnkBlock,	 min   : 1591,	 lim   : 1642,	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkBlock,	 min   : 1656,	 lim   : 1704,	 level : 4,	 ChildLabel : apneListItem}
";
        #endregion

        [TestMethod]
        public void GetSubTreeMaxDepth()
        {
            var file = _session.FileFromText(CursorTreeWalkingInputString);

            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            // only get the root (depth = 0)
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(0);
            Assert.IsTrue(serializedNodes.Count == 1);

            // only get the root's direct children (depth = 1)
            serializedNodes = cursor.GetSubTree(1);
            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            Assert.AreEqual(GetSubTreeMaxDepthExpectedString, serializedNodeString.ToString());
        }
        #region Test Data
        public const string GetSubTreeMaxDepthExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 1725,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 31,	 lim   : 1715,	 level : 1,	 ChildLabel : apneBody}
";
        #endregion

        [TestMethod]
        public void CursorOffsetInComment()
        {
            PerformRequests(CursorOffsetInCommentInputString, (context, offset, data, index) =>
            {
                var cursor = context.GetASTCursor();
                bool inComment = cursor.OffsetInComment(offset);

                switch (data)
                {
                    case "t":
                        Assert.IsTrue(inComment, "Offset expected to be in a comment.");
                        break;
                    case "f":
                        Assert.IsFalse(inComment, "Offset is NOT expected to be in a comment.");
                        break;
                    default:
                        Assert.Fail("Unexpected value.");
                        break;
                }
            });
        }
        #region Test Data
        public const string CursorOffsetInCommentInputString = @"
|f| /*|t|   
|t|
*/
|f|
function f1()
{
///|t|<summary> </summary>
|f| /// <param>|t| </param>
/// <returns>|t|</returns>|t|
}|f|

|f| //|t| singleline comment|t|
|f|
";
        #endregion

        [TestMethod]
        public void GetEdgeLabelAtRootNode()
        {
            var file = _session.FileFromText("var x;");
            var context = _session.OpenContext(file);
            context.Update();
            var cursor = context.GetASTCursor();
            var currentNode = cursor.Current();
            Assert.AreEqual(AuthorParseNodeKind.apnkProg, currentNode.Kind);

            // move up to the parent of the root node
            var edgeLabel = cursor.GetEdgeLabel();
            Assert.AreEqual(AuthorParseNodeEdge.apneNone, edgeLabel);
        }

        [TestMethod]
        public void ReleaseCursor()
        {
            var file = _session.FileFromText(@"var a = 0; function foo(a,b) {}; ");
            var context = _session.OpenContext(file);

            var cursor = context.GetASTCursor();
            cursor.SeekToOffset(2);
            cursor.MoveUp();
            Marshal.ReleaseComObject(cursor);

            context.Update(); // make sure we do not fail

            var completions = context.GetCompletionsAt(file.Text.Length);
            completions.ToEnumerable().ExpectContains(new[] { "a", "foo" });

            var cursor2 = context.GetASTCursor();
            cursor2.SeekToOffset(5);
            Marshal.ReleaseComObject(cursor2);

            // verify we do not fail, and that the cursor was released on delete
            completions = context.GetCompletionsAt(file.Text.Length);
            completions.ToEnumerable().ExpectContains(new[] { "a", "foo" });
        }

        [TestMethod]
        public void ShowLists()
        {
            var file = _session.FileFromText(@"
{
    x = x - 1;
    break;
}
var x  = 0;
");
            var moves = new[] {
                new {edge = AuthorParseNodeEdge.apneBody, expectedNode = AuthorParseNodeKind.apnkList},
                new {edge = AuthorParseNodeEdge.apneListItem, expectedNode = AuthorParseNodeKind.apnkBlock},
                new {edge = AuthorParseNodeEdge.apneBlockBody, expectedNode = AuthorParseNodeKind.apnkList},
                new {edge = AuthorParseNodeEdge.apneListItem, expectedNode = AuthorParseNodeKind.apnkAsg},
                new {edge = AuthorParseNodeEdge.apneLeft, expectedNode = AuthorParseNodeKind.apnkName},
            };

            var context = _session.OpenContext(file);
            var forwardMovingCursor = context.GetASTCursor();

            var currentNode = forwardMovingCursor.Current();
            var expectedParentNode = currentNode;
            Assert.IsTrue(currentNode.Kind == AuthorParseNodeKind.apnkProg);
            for (int i = 0; i < moves.Length; i++)
            {
                currentNode = forwardMovingCursor.MoveToChild(moves[i].edge, 0);

                // verify the node is what is expected
                Assert.IsTrue(currentNode.Kind == moves[i].expectedNode);
                // verify the parent is consistent
                Assert.IsTrue(forwardMovingCursor.Parent().Kind == expectedParentNode.Kind);
                // check the current edge was the same one supplied to move to this node
                Assert.IsTrue(forwardMovingCursor.GetEdgeLabel() == moves[i].edge);

                expectedParentNode = currentNode;
            }

            var seekOffset = currentNode.StartOffset;

            // seek to the ofset of the inner most statement
            var backwardMovingCursor = context.GetASTCursor();
            backwardMovingCursor.SeekToOffset(seekOffset);

            currentNode = backwardMovingCursor.Current();
            //expectedParentNode = currentNode;
            for (int i = moves.Length - 1; i >= 0; i--)
            {
                // verify the node is what is expected
                Assert.IsTrue(currentNode.Kind == moves[i].expectedNode);
                // check the current edge was the same one supplied to move to this node
                Assert.IsTrue(backwardMovingCursor.GetEdgeLabel() == moves[i].edge);

                currentNode = backwardMovingCursor.MoveUp();
            }
            Assert.IsTrue(currentNode.Kind == AuthorParseNodeKind.apnkProg);
        }

        [TestMethod]
        public void GetStatementSpan()
        {
            var file = _session.FileFromText(@"
do {
var x = 0;
if (x < 0) break;
}   while (true);

switch (a)
{
    case 0:
        break;
    default:
        break;
}");

            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            StatementSpanPrinter walker = new StatementSpanPrinter(cursor);
            walker.Walk();

            Assert.AreEqual(GetStatementSpanExpectedString, walker.TreeString);
        }
        #region Test data
        const string GetStatementSpanExpectedString = @"{kind : apnkProg,	 min : 0,	 lim : 135,	 spanMin : 2,	 spanLim : 135,	 }
{kind : apnkList,	 min : 2,	 lim : 135,	 spanMin : 2,	 spanLim : 135,	 }
{kind : apnkDoWhile,	 min : 2,	 lim : 55,	 spanMin : 43,	 spanLim : 55,	 }
{kind : apnkTrue,	 min : 50,	 lim : 54,	 spanMin : 50,	 spanLim : 54,	 }
{kind : apnkBlock,	 min : 5,	 lim : 40,	 spanMin : 5,	 spanLim : 40,	 }
{kind : apnkList,	 min : 8,	 lim : 36,	 spanMin : 8,	 spanLim : 30,	 }
{kind : apnkVarDecl,	 min : 8,	 lim : 17,	 spanMin : 8,	 spanLim : 17,	 }
{kind : apnkInt,	 min : 16,	 lim : 17,	 spanMin : 16,	 spanLim : 17,	 }
{kind : apnkIf,	 min : 20,	 lim : 36,	 spanMin : 20,	 spanLim : 30,	 }
{kind : apnkLt,	 min : 24,	 lim : 29,	 spanMin : 24,	 spanLim : 29,	 }
{kind : apnkName,	 min : 24,	 lim : 25,	 spanMin : 24,	 spanLim : 25,	 }
{kind : apnkInt,	 min : 28,	 lim : 29,	 spanMin : 28,	 spanLim : 29,	 }
{kind : apnkBreak,	 min : 31,	 lim : 36,	 spanMin : 31,	 spanLim : 36,	 }
{kind : apnkSwitch,	 min : 60,	 lim : 135,	 spanMin : 60,	 spanLim : 70,	 }
{kind : apnkName,	 min : 68,	 lim : 69,	 spanMin : 68,	 spanLim : 69,	 }
{kind : apnkCase,	 min : 79,	 lim : 107,	 spanMin : 79,	 spanLim : 107,	 }
{kind : apnkInt,	 min : 84,	 lim : 85,	 spanMin : 84,	 spanLim : 85,	 }
{kind : apnkBlock,	 min : 79,	 lim : 101,	 spanMin : 79,	 spanLim : 86,	 }
{kind : apnkBreak,	 min : 96,	 lim : 101,	 spanMin : 96,	 spanLim : 101,	 }
{kind : apnkDefaultCase,	 min : 108,	 lim : 134,	 spanMin : 108,	 spanLim : 134,	 }
{kind : apnkBlock,	 min : 108,	 lim : 134,	 spanMin : 108,	 spanLim : 134,	 }
{kind : apnkBreak,	 min : 126,	 lim : 131,	 spanMin : 126,	 spanLim : 131,	 }
{kind : apnkDefaultCase,	 min : 108,	 lim : 134,	 spanMin : 108,	 spanLim : 134,	 }
{kind : apnkBlock,	 min : 108,	 lim : 134,	 spanMin : 108,	 spanLim : 134,	 }
{kind : apnkBreak,	 min : 126,	 lim : 131,	 spanMin : 126,	 spanLim : 131,	 }
";
        #endregion

        [TestMethod]
        [WorkItem(175650)]
        public void FunctionRCurlyMin()
        {
            var inputFile = "function f1() { } function f2() {";
            var context = _session.OpenContext(_session.FileFromText(inputFile));
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(FunctionRCurlyMinExpectedString, walker.TreeString);
        }
        #region Test data
        const string FunctionRCurlyMinExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 33,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 33,	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 0,	 lim : 17,	 name : ""f1"",	 IdentifierMin : 9,	 FunctionKeywordMin : 0,	 LParen : 11,	RParen : 12,	LCurly : 14,	RCurly : 16,	semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 18,	 lim : 33,	 name : ""f2"",	 IdentifierMin : 27,	 FunctionKeywordMin : 18,	 LParen : 29,	RParen : 30,	LCurly : 32,	RCurly : 0,	semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(175650)]
        public void StringTemplateTest()
        {
            var inputFile = @"`abd${2* 3}bb${3+4}d`;
            var strExpr1 = `Fifteen is ${a + 
`he${'l'+'l'}o ${'w' +  `o${'rl'}d`} !`} and not ${2     * a +     `b`}.`;
";
            var context = _session.OpenContext(_session.FileFromText(inputFile));
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(StringTemplateTestString, walker.TreeString);
        }
        #region Test data
        const string StringTemplateTestString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 147,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 144,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStrTemplate,	 min : 0,	 lim : 21,	 semicolon: ""explicit""}
apneStringLiterals :	 {kind : apnkList,	 min : 0,	 lim : 21,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 0,	 lim : 6,	 stringValue : ""abd"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 10,	 lim : 15,	 stringValue : ""bb"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 18,	 lim : 21,	 stringValue : ""d"",	 semicolon: ""none""}
apneSubstitutionExpression :	 {kind : apnkList,	 min : 6,	 lim : 18,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMul,	 min : 6,	 lim : 10,	 semicolon: ""none""}
apneLeft :	 {kind : apnkInt,	 min : 6,	 lim : 7,	 intValue : 2,	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 9,	 lim : 10,	 intValue : 3,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAdd,	 min : 15,	 lim : 18,	 semicolon: ""none""}
apneLeft :	 {kind : apnkInt,	 min : 15,	 lim : 16,	 intValue : 3,	 semicolon: ""none""}
apneRight :	 {kind : apnkInt,	 min : 17,	 lim : 18,	 intValue : 4,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 36,	 lim : 144,	 name : ""strExpr1"",	 IdentifierMin : 40,	 semicolon: ""explicit""}
apneInitialization :	 {kind : apnkStrTemplate,	 min : 51,	 lim : 144,	 semicolon: ""none""}
apneStringLiterals :	 {kind : apnkList,	 min : 51,	 lim : 144,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 51,	 lim : 65,	 stringValue : ""Fifteen is "",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 110,	 lim : 122,	 stringValue : "" and not "",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 141,	 lim : 144,	 stringValue : ""."",	 semicolon: ""none""}
apneSubstitutionExpression :	 {kind : apnkList,	 min : 65,	 lim : 141,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAdd,	 min : 65,	 lim : 110,	 semicolon: ""none""}
apneLeft :	 {kind : apnkName,	 min : 65,	 lim : 66,	 name : ""a"",	 semicolon: ""none""}
apneRight :	 {kind : apnkStrTemplate,	 min : 71,	 lim : 110,	 semicolon: ""none""}
apneStringLiterals :	 {kind : apnkList,	 min : 71,	 lim : 110,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 71,	 lim : 76,	 stringValue : ""he"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 83,	 lim : 88,	 stringValue : ""o "",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 106,	 lim : 110,	 stringValue : "" !"",	 semicolon: ""none""}
apneSubstitutionExpression :	 {kind : apnkList,	 min : 76,	 lim : 106,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAdd,	 min : 76,	 lim : 83,	 semicolon: ""none""}
apneLeft :	 {kind : apnkStr,	 min : 76,	 lim : 79,	 stringValue : ""l"",	 semicolon: ""none""}
apneRight :	 {kind : apnkStr,	 min : 80,	 lim : 83,	 stringValue : ""l"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkAdd,	 min : 88,	 lim : 106,	 semicolon: ""none""}
apneLeft :	 {kind : apnkStr,	 min : 88,	 lim : 91,	 stringValue : ""w"",	 semicolon: ""none""}
apneRight :	 {kind : apnkStrTemplate,	 min : 95,	 lim : 106,	 semicolon: ""none""}
apneStringLiterals :	 {kind : apnkList,	 min : 95,	 lim : 106,	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 95,	 lim : 99,	 stringValue : ""o"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkStr,	 min : 103,	 lim : 106,	 stringValue : ""d"",	 semicolon: ""none""}
apneSubstitutionExpression :	 {kind : apnkStr,	 min : 99,	 lim : 103,	 stringValue : ""rl"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkAdd,	 min : 122,	 lim : 141,	 semicolon: ""none""}
apneLeft :	 {kind : apnkMul,	 min : 122,	 lim : 131,	 semicolon: ""none""}
apneLeft :	 {kind : apnkInt,	 min : 122,	 lim : 123,	 intValue : 2,	 semicolon: ""none""}
apneRight :	 {kind : apnkName,	 min : 130,	 lim : 131,	 name : ""a"",	 semicolon: ""none""}
apneRight :	 {kind : apnkStrTemplate,	 min : 138,	 lim : 141,	 semicolon: ""none""}
apneStringLiterals :	 {kind : apnkStr,	 min : 138,	 lim : 141,	 stringValue : ""b"",	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(175208)]
        public void IndexBrackOffsets()
        {
            var inputFile = "a[0]; b[0; ++c[0]";
            var context = _session.OpenContext(_session.FileFromText(inputFile));

            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(IndexBrackOffsetsExpectedString, walker.TreeString);
        }
        #region Test data
        const string IndexBrackOffsetsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 17,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 17,	 semicolon: ""none""}
apneListItem :	 {kind : apnkIndex,	 min : 0,	 lim : 4,	 LBrack : 1,	 RBrack : 3,	 semicolon: ""explicit""}
apneTarget :	 {kind : apnkName,	 min : 0,	 lim : 1,	 name : ""a"",	 semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 2,	 lim : 3,	 intValue : 0,	 semicolon: ""none""}
apneListItem :	 {kind : apnkIndex,	 min : 6,	 lim : 10,	 LBrack : 7,	 RBrack : 0,	 semicolon: ""explicit""}
apneTarget :	 {kind : apnkName,	 min : 6,	 lim : 7,	 name : ""b"",	 semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 8,	 lim : 9,	 intValue : 0,	 semicolon: ""none""}
apneListItem :	 {kind : apnkIncPre,	 min : 11,	 lim : 17,	 semicolon: ""automatic""}
apneOperand :	 {kind : apnkIndex,	 min : 13,	 lim : 17,	 LBrack : 14,	 RBrack : 16,	 semicolon: ""none""}
apneTarget :	 {kind : apnkName,	 min : 13,	 lim : 14,	 name : ""c"",	 semicolon: ""none""}
apneValue :	 {kind : apnkInt,	 min : 15,	 lim : 16,	 intValue : 0,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        public void BraceMatchingExpectations()
        {
            var text = "function (a) { }{ }";
            var context = _session.OpenContext(_session.FileFromText(text));
            var cursor = context.GetASTCursor();
            var details = cursor.SeekToOffset(text.IndexOf("}{") + 1);
            Assert.AreEqual(AuthorParseNodeKind.apnkEndCode, details.Kind);
            details = cursor.SeekToOffset(text.IndexOf("}{") + 2);
            Assert.AreEqual(AuthorParseNodeKind.apnkBlock, details.Kind);
            Assert.AreEqual('{', text[details.StartOffset]);
            Assert.AreEqual('}', text[details.EndOffset - 1]);
        }

        [TestMethod]
        public void BreakpointValidationExpectations()
        {
            var context = _session.OpenContext(_session.FileFromText(BreakpointValidationExpectationsText));
            var cursor = context.GetASTCursor();
            var offset = BreakpointValidationExpectationsText.IndexOf('}');
            var details = cursor.SeekToOffset(offset);
            Assert.AreEqual(AuthorParseNodeKind.apnkEndCode, details.Kind);
            Assert.AreEqual(offset, details.StartOffset);
            Assert.AreEqual(offset + 1, details.EndOffset);
        }
        #region Test data
        const string BreakpointValidationExpectationsText = @"function(x)
{
    
    return 0;
}";
        #endregion

        [TestMethod]
        [WorkItem(172584)]
        public void FunctionParenOffsets()
        {
            var inputFile = "function f1() { }";
            var context = _session.OpenContext(_session.FileFromText(inputFile));
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(FunctionParenOffsetsExpectedString, walker.TreeString);
        }
        #region Test data
        const string FunctionParenOffsetsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 17,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 17,	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 0,	 lim : 17,	 name : ""f1"",	 IdentifierMin : 9,	 FunctionKeywordMin : 0,	 LParen : 11,	RParen : 12,	LCurly : 14,	RCurly : 16,	semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(176679)]
        public void CasesList()
        {
            var file = _session.FileFromText("switch(x) { case 0: break; case 1: break; default: break; }");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(ASTCursorTests.GetNodeSerializedString(node, cursor));
            }
            Assert.AreEqual(CasesListExpectedString, serializedNodeString.ToString());
        }
        #region Test data
        public const string CasesListExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 59,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 0,	 lim   : 59,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkSwitch,	 min   : 0,	 lim   : 59,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 7,	 lim   : 8,	 name  : ""x"",	 level : 3,	 ChildLabel : apneValue}
			{kind  : apnkCase,	 min   : 12,	 lim   : 26,	 level : 3,	 ChildLabel : apneCase}
				{kind  : apnkInt,	 min   : 17,	 lim   : 18,	 level : 4,	 ChildLabel : apneValue}
				{kind  : apnkBlock,	 min   : 12,	 lim   : 25,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 20,	 lim   : 25,	 level : 5,	 ChildLabel : apneBlockBody}
			{kind  : apnkCase,	 min   : 27,	 lim   : 41,	 level : 3,	 ChildLabel : apneCase}
				{kind  : apnkInt,	 min   : 32,	 lim   : 33,	 level : 4,	 ChildLabel : apneValue}
				{kind  : apnkBlock,	 min   : 27,	 lim   : 40,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 35,	 lim   : 40,	 level : 5,	 ChildLabel : apneBlockBody}
			{kind  : apnkDefaultCase,	 min   : 42,	 lim   : 58,	 level : 3,	 ChildLabel : apneDefaultCase}
				{kind  : apnkBlock,	 min   : 42,	 lim   : 58,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkBreak,	 min   : 51,	 lim   : 56,	 level : 5,	 ChildLabel : apneBlockBody}
";
        #endregion

        [TestMethod]
        [WorkItem(176679)]
        public void ArrgumentsList()
        {
            var file = _session.FileFromText("function f (a, b, c, d, e) {} f(1, 2, 3, 4, 5);");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(ASTCursorTests.GetNodeSerializedString(node, cursor));
            }
            Assert.AreEqual(ArrgumentsListExpectedString, serializedNodeString.ToString());
        }
        #region Test data
        public const string ArrgumentsListExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 47,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 0,	 lim   : 46,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkFncDecl,	 min   : 0,	 lim   : 29,	 name  : ""f"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkVarDecl,	 min   : 12,	 lim   : 13,	 name  : ""a"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 15,	 lim   : 16,	 name  : ""b"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 18,	 lim   : 19,	 name  : ""c"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 21,	 lim   : 22,	 name  : ""d"",	 level : 3,	 ChildLabel : apneArgument}
			{kind  : apnkVarDecl,	 min   : 24,	 lim   : 25,	 name  : ""e"",	 level : 3,	 ChildLabel : apneArgument}
		{kind  : apnkCall,	 min   : 30,	 lim   : 46,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 30,	 lim   : 31,	 name  : ""f"",	 level : 3,	 ChildLabel : apneTarget}
			{kind  : apnkList,	 min   : 32,	 lim   : 45,	 level : 3,	 ChildLabel : apneArguments}
				{kind  : apnkInt,	 min   : 32,	 lim   : 33,	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkInt,	 min   : 35,	 lim   : 36,	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkInt,	 min   : 38,	 lim   : 39,	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkInt,	 min   : 41,	 lim   : 42,	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkInt,	 min   : 44,	 lim   : 45,	 level : 4,	 ChildLabel : apneListItem}
";
        #endregion

        [TestMethod]
        [WorkItem(176729)]
        public void DoWhileSerializationOrder()
        {
            var file = _session.FileFromText("do { } while (true); while (true) { }");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(ASTCursorTests.GetNodeSerializedString(node, cursor));
            }
            string s = serializedNodeString.ToString();
            Assert.AreEqual(DoWhileSerializationOrderExpectedString, serializedNodeString.ToString());
        }
        #region Test data
        public const string DoWhileSerializationOrderExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 37,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 0,	 lim   : 37,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkDoWhile,	 min   : 0,	 lim   : 19,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkBlock,	 min   : 3,	 lim   : 6,	 level : 3,	 ChildLabel : apneBody}
			{kind  : apnkTrue,	 min   : 14,	 lim   : 18,	 level : 3,	 ChildLabel : apneCondition}
		{kind  : apnkWhile,	 min   : 21,	 lim   : 37,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkTrue,	 min   : 28,	 lim   : 32,	 level : 3,	 ChildLabel : apneCondition}
			{kind  : apnkBlock,	 min   : 34,	 lim   : 37,	 level : 3,	 ChildLabel : apneBody}
";
        #endregion

        [TestMethod]
        [WorkItem(183786)]
        public void ObjectCurlyOffsets()
        {
            var file = _session.FileFromText("var a = { a:0, b:0 };");

            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(ObjectCurlyOffsetsExpectedString, walker.TreeString);
        }
        #region Test data
        const string ObjectCurlyOffsetsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 21,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 21,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 0,	 lim : 20,	 name : ""a"",	 IdentifierMin : 4,	 semicolon: ""explicit""}
apneInitialization :	 {kind : apnkObject,	 min : 8,	 lim : 20,	 LCurly : 8,	 RCurly : 19,	 semicolon: ""none""}
apneMembers :	 {kind : apnkList,	 min : 10,	 lim : 18,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMember,	 min : 10,	 lim : 13,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 10,	 lim : 11,	 stringValue : ""a"",	 semicolon: ""none""}
apneMember :	 {kind : apnkInt,	 min : 12,	 lim : 13,	 intValue : 0,	 semicolon: ""none""}
apneListItem :	 {kind : apnkMember,	 min : 15,	 lim : 18,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 15,	 lim : 16,	 stringValue : ""b"",	 semicolon: ""none""}
apneMember :	 {kind : apnkInt,	 min : 17,	 lim : 18,	 intValue : 0,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(168891)]
        public void PropertyAccessors()
        {
            var file = _session.FileFromText(@"var myObject = { 
get ItemType() {
return; 
},
set ItemType(value) { }
};");

            var context = _session.OpenContext(file);
            context.Update();

            // get sub tree
            var cursor = context.GetASTCursor();
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            Assert.AreEqual(PropertyAccessors_Subtree_ExpectedString, serializedNodeString.ToString());

            // walk the cursor one node at a time
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(PropertyAccessors_Cursor_ExpectedString, walker.TreeString);
        }
        #region Test data
        const string PropertyAccessors_Cursor_ExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 78,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 78,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 0,	 lim : 77,	 name : ""myObject"",	 IdentifierMin : 4,	 semicolon: ""explicit""}
apneInitialization :	 {kind : apnkObject,	 min : 15,	 lim : 77,	 LCurly : 15,	 RCurly : 76,	 semicolon: ""none""}
apneMembers :	 {kind : apnkList,	 min : 19,	 lim : 74,	 semicolon: ""none""}
apneListItem :	 {kind : apnkGetMember,	 min : 19,	 lim : 48,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 23,	 lim : 31,	 stringValue : ""ItemType"",	 semicolon: ""none""}
apneValue :	 {kind : apnkFncDecl,	 min : 23,	 lim : 48,	 name : """",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 31,	RParen : 32,	LCurly : 34,	RCurly : 47,	semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 37,	 lim : 48,	 semicolon: ""none""}
apneListItem :	 {kind : apnkReturn,	 min : 37,	 lim : 43,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkSetMember,	 min : 51,	 lim : 74,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 55,	 lim : 63,	 stringValue : ""ItemType"",	 semicolon: ""none""}
apneValue :	 {kind : apnkFncDecl,	 min : 55,	 lim : 74,	 name : """",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 63,	RParen : 69,	LCurly : 71,	RCurly : 73,	semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 64,	 lim : 69,	 name : ""value"",	 IdentifierMin : 64,	 semicolon: ""none""}
";
        const string PropertyAccessors_Subtree_ExpectedString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 78,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 0,	 lim   : 78,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkVarDecl,	 min   : 0,	 lim   : 77,	 name  : ""myObject"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkObject,	 min   : 15,	 lim   : 77,	 level : 3,	 ChildLabel : apneInitialization}
				{kind  : apnkList,	 min   : 19,	 lim   : 74,	 level : 4,	 ChildLabel : apneMembers}
					{kind  : apnkGetMember,	 min   : 19,	 lim   : 48,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 23,	 lim   : 31,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkFncDecl,	 min   : 23,	 lim   : 48,	 level : 6,	 ChildLabel : apneValue}
							{kind  : apnkList,	 min   : 37,	 lim   : 48,	 level : 7,	 ChildLabel : apneBody}
								{kind  : apnkReturn,	 min   : 37,	 lim   : 43,	 level : 8,	 ChildLabel : apneListItem}
					{kind  : apnkSetMember,	 min   : 51,	 lim   : 74,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 55,	 lim   : 63,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkFncDecl,	 min   : 55,	 lim   : 74,	 level : 6,	 ChildLabel : apneValue}
							{kind  : apnkVarDecl,	 min   : 64,	 lim   : 69,	 name  : ""value"",	 level : 7,	 ChildLabel : apneArgument}
";

        #endregion

        [TestMethod]
        [WorkItem(203411)]
        public void IdentifierEndOffset()
        {
            var file = _session.FileFromText(IdentifierEndOffsetInputString);
            var context = _session.OpenContext(file);
            context.Update();

            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(IdentifierEndOffsetExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string IdentifierEndOffsetInputString = @"var /* */ bar;
           function /*comment*/ foo(a,b,c){}";

        public const string IdentifierEndOffsetExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 60,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 60,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 0,	 lim : 13,	 name : ""bar"",	 IdentifierMin : 10,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkFncDecl,	 min : 27,	 lim : 60,	 name : ""foo"",	 IdentifierMin : 48,	 FunctionKeywordMin : 27,	 LParen : 51,	RParen : 57,	LCurly : 58,	RCurly : 59,	semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 52,	 lim : 53,	 name : ""a"",	 IdentifierMin : 52,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 54,	 lim : 55,	 name : ""b"",	 IdentifierMin : 54,	 semicolon: ""none""}
apneArgument :	 {kind : apnkVarDecl,	 min : 56,	 lim : 57,	 name : ""c"",	 IdentifierMin : 56,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(166221)]
        public void ExpressionOffsets()
        {
            var file = _session.FileFromText(@"(((((function foo(){})))))");
            var context = _session.OpenContext(file);
            context.Update();
            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(ExpressionOffsetsExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string ExpressionOffsetsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 26,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 26,	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 0,	 lim : 26,	 name : ""foo"",	 IdentifierMin : 14,	 FunctionKeywordMin : 5,	 LParen : 17,	RParen : 18,	LCurly : 19,	RCurly : 20,	semicolon: ""automatic""}
";
        #endregion

        [TestMethod]
        [WorkItem(255551)]
        public void CallArgumentsSpan()
        {
            var file = _session.FileFromText(@"call(1, 


            {}

            , 2 )");

            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(CallArgumentsSpanExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string CallArgumentsSpanExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 49,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 49,	 semicolon: ""none""}
apneListItem :	 {kind : apnkCall,	 min : 0,	 lim : 49,	 semicolon: ""automatic""}
apneTarget :	 {kind : apnkName,	 min : 0,	 lim : 4,	 name : ""call"",	 semicolon: ""none""}
apneArguments :	 {kind : apnkList,	 min : 5,	 lim : 47,	 semicolon: ""none""}
apneListItem :	 {kind : apnkInt,	 min : 5,	 lim : 6,	 intValue : 1,	 semicolon: ""none""}
apneListItem :	 {kind : apnkObject,	 min : 26,	 lim : 28,	 LCurly : 26,	 RCurly : 27,	 semicolon: ""none""}
apneListItem :	 {kind : apnkInt,	 min : 46,	 lim : 47,	 intValue : 2,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(262750)]
        public void UpdateOnTextChange()
        {
            var file = _session.FileFromText(@"var x;");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();
            var current = cursor.Current();
            Assert.IsTrue(current.Kind == AuthorParseNodeKind.apnkProg);
            Assert.IsTrue(current.StartOffset == 0 && current.EndOffset == file.Text.Length);
            Marshal.ReleaseComObject(cursor);
            cursor = null;

            file.Replace(0, file.Text.Length, "var     x;");

            context.Update();
            cursor = context.GetASTCursor();
            current = cursor.Current();
            Assert.IsTrue(current.Kind == AuthorParseNodeKind.apnkProg);
            Assert.IsTrue(current.StartOffset == 0 && current.EndOffset == file.Text.Length);
            Marshal.ReleaseComObject(cursor);
            cursor = null;
        }

        [TestMethod]
        [WorkItem(264627)]
        public void MultilineCommentRanges()
        {
            var file = _session.FileFromText("/**/");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();
            Assert.IsFalse(cursor.OffsetInComment(0)); // before '/'
            Assert.IsTrue(cursor.OffsetInComment(1));  // before '*'
            Assert.IsTrue(cursor.OffsetInComment(2));  // before '*'
            Assert.IsTrue(cursor.OffsetInComment(3));  // before '/'
            Assert.IsFalse(cursor.OffsetInComment(4)); // after '/'
        }

        [TestMethod]
        [WorkItem(264627)]
        public void SinglelineCommentRanges()
        {
            var file = _session.FileFromText("//");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();
            Assert.IsFalse(cursor.OffsetInComment(0)); // before '/'
            Assert.IsTrue(cursor.OffsetInComment(1));  // before '/'
            Assert.IsTrue(cursor.OffsetInComment(2));  // after '/'
        }

        [TestMethod]
        [WorkItem(275193)]
        public void StatementSpanIncludeEOF()
        {
            var file = _session.FileFromText(@"
if (1)
    if (2)
        if (3)
  
  
  
  
  
  
  
/* EOF */");

            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(StatementSpanIncludeEOFExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string StatementSpanIncludeEOFExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 75,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 2,	 lim : 75,	 semicolon: ""none""}
apneListItem :	 {kind : apnkIf,	 min : 2,	 lim : 75,	 LParen : 5,	 RParen : 7,	 semicolon: ""none""}
apneCondition :	 {kind : apnkInt,	 min : 6,	 lim : 7,	 intValue : 1,	 semicolon: ""none""}
apneThen :	 {kind : apnkIf,	 min : 14,	 lim : 75,	 LParen : 17,	 RParen : 19,	 semicolon: ""none""}
apneCondition :	 {kind : apnkInt,	 min : 18,	 lim : 19,	 intValue : 2,	 semicolon: ""none""}
apneThen :	 {kind : apnkIf,	 min : 30,	 lim : 75,	 LParen : 33,	 RParen : 35,	 semicolon: ""none""}
apneCondition :	 {kind : apnkInt,	 min : 34,	 lim : 35,	 intValue : 3,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(275480)]
        public void AccessorsSpan()
        {
            var file = _session.FileFromText(@"x = {get x(){}, set x(){}}");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(AccessorsSpanExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string AccessorsSpanExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 26,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 26,	 semicolon: ""none""}
apneListItem :	 {kind : apnkAsg,	 min : 0,	 lim : 26,	 semicolon: ""automatic""}
apneLeft :	 {kind : apnkName,	 min : 0,	 lim : 1,	 name : ""x"",	 semicolon: ""none""}
apneRight :	 {kind : apnkObject,	 min : 4,	 lim : 26,	 LCurly : 4,	 RCurly : 25,	 semicolon: ""none""}
apneMembers :	 {kind : apnkList,	 min : 5,	 lim : 25,	 semicolon: ""none""}
apneListItem :	 {kind : apnkGetMember,	 min : 5,	 lim : 14,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 9,	 lim : 10,	 stringValue : ""x"",	 semicolon: ""none""}
apneValue :	 {kind : apnkFncDecl,	 min : 9,	 lim : 14,	 name : """",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 10,	RParen : 11,	LCurly : 12,	RCurly : 13,	semicolon: ""none""}
apneListItem :	 {kind : apnkSetMember,	 min : 16,	 lim : 25,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 20,	 lim : 21,	 stringValue : ""x"",	 semicolon: ""none""}
apneValue :	 {kind : apnkFncDecl,	 min : 20,	 lim : 25,	 name : """",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 21,	RParen : 22,	LCurly : 23,	RCurly : 24,	semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(286891)]
        public void FunctionKeywordLocation()
        {
            var file = _session.FileFromText(@"(/*0123456789*/function foo() {})");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(FunctionKeywordLocationExpectedString, walker.TreeString);
        }
        #region Test Data
        public const string FunctionKeywordLocationExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 33,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 33,	 semicolon: ""none""}
apneListItem :	 {kind : apnkFncDecl,	 min : 0,	 lim : 33,	 name : ""foo"",	 IdentifierMin : 24,	 FunctionKeywordMin : 15,	 LParen : 27,	RParen : 28,	LCurly : 30,	RCurly : 31,	semicolon: ""automatic""}
";
        #endregion

        [TestMethod]
        [WorkItem(281240)]
        public void IdentifierList()
        {
            var file = _session.FileFromText(@"
{
var a;
var b,c;
}
");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(IdentifierListExpectedString, walker.TreeString);

            // check the serialized tree
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            String s = serializedNodeString.ToString();
            Assert.AreEqual(IdentifierListExpectedTreeString, serializedNodeString.ToString());
        }
        #region Test Data
        public const string IdentifierListExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 26,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 2,	 lim : 26,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 2,	 lim : 24,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkList,	 min : 5,	 lim : 20,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 5,	 lim : 10,	 name : ""a"",	 IdentifierMin : 9,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkVarDeclList,	 min : 13,	 lim : 20,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkVarDecl,	 min : 13,	 lim : 18,	 name : ""b"",	 IdentifierMin : 17,	 semicolon: ""none""}
apneListItem :	 {kind : apnkVarDecl,	 min : 19,	 lim : 20,	 name : ""c"",	 IdentifierMin : 19,	 semicolon: ""none""}
";
        public const string IdentifierListExpectedTreeString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 26,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 2,	 lim   : 26,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkBlock,	 min   : 2,	 lim   : 24,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkList,	 min   : 5,	 lim   : 20,	 level : 3,	 ChildLabel : apneBlockBody}
				{kind  : apnkVarDecl,	 min   : 5,	 lim   : 10,	 name  : ""a"",	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkVarDeclList,	 min   : 13,	 lim   : 20,	 level : 4,	 ChildLabel : apneListItem}
					{kind  : apnkVarDecl,	 min   : 13,	 lim   : 18,	 name  : ""b"",	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkVarDecl,	 min   : 19,	 lim   : 20,	 name  : ""c"",	 level : 5,	 ChildLabel : apneListItem}
";
        #endregion

        [TestMethod]
        [WorkItem(280336)]
        public void SwitchSpanIncludeEOF()
        {
            var samples = new[] { 
                @"switch(                                      /* EOF */",
                @"switch(x)                                    /* EOF */",
                @"switch(x){                                   /* EOF */",
                @"switch(x){ case                              /* EOF */",
                @"switch(x){ case 1                            /* EOF */",
                @"switch(x){ case 1:                           /* EOF */",
                @"switch(x){ default                           /* EOF */",
                @"switch(x){ default:                          /* EOF */",
            };

            foreach (var sample in samples)
            {
                var file = _session.FileFromText(sample);
                var context = _session.OpenContext(file);
                var cursor = context.GetASTCursor();
                var nodes = cursor.GetSubTree(-1);

                foreach (var node in nodes.ToEnumerable())
                {
                    if (node.Details.Kind == AuthorParseNodeKind.apnkSwitch || 
                        node.Details.Kind == AuthorParseNodeKind.apnkCase || 
                        node.Details.Kind == AuthorParseNodeKind.apnkDefaultCase)
                    {
                        Assert.IsTrue(node.Details.EndOffset == sample.Length);
                    }
                }
            }
        }

        [TestMethod]
        [WorkItem(283203)]
        public void SubsumedFunctions()
        {
            var file = _session.FileFromText(@"foo(null, function bar() {}");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            // GetSubTree
            var nodes = cursor.GetSubTree(-1);
            foreach (var node in nodes.ToEnumerable())
            {
                if (node.Details.Kind == AuthorParseNodeKind.apnkFncDecl)
                    Assert.IsTrue((node.Details.Flags & AuthorParseNodeFlags.apnfSubsumedFunction) == AuthorParseNodeFlags.apnfSubsumedFunction);
                if (node.Details.Kind == AuthorParseNodeKind.apnkName && cursor.GetPropertyById(node.Name) == "undefined")
                {
                    Assert.IsTrue((node.Details.Flags & AuthorParseNodeFlags.apnfSyntheticNode) == AuthorParseNodeFlags.apnfSyntheticNode);
                    Assert.IsTrue(node.Details.StartOffset == node.Details.EndOffset);
                }
            }

            // Cursor
            NodeWalker.ForEach(cursor, (nodeDetails) => {
                if (nodeDetails.Kind == AuthorParseNodeKind.apnkFncDecl)
                    Assert.IsTrue((nodeDetails.Flags & AuthorParseNodeFlags.apnfSubsumedFunction) == AuthorParseNodeFlags.apnfSubsumedFunction);
                if (nodeDetails.Kind == AuthorParseNodeKind.apnkName && cursor.GetStringValue() == "undefined")
                {
                    Assert.IsTrue((nodeDetails.Flags & AuthorParseNodeFlags.apnfSyntheticNode) == AuthorParseNodeFlags.apnfSyntheticNode);
                    Assert.IsTrue(nodeDetails.StartOffset == nodeDetails.EndOffset);
                }
            });
        }

        [TestMethod]
        [WorkItem(293799)]
        public void CatchParenOffsets()
        {
            var file = _session.FileFromText("try{}catch(e){}");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();
            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();

            Assert.AreEqual(CatchParenOffsetsExpectedString, walker.TreeString);
        }
        #region Test data
        private const string CatchParenOffsetsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 15,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 0,	 lim : 15,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 0,	 lim : 15,	 automaticBlock : true,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkTryCatch,	 min : 0,	 lim : 15,	 semicolon: ""none""}
apneTry :	 {kind : apnkTry,	 min : 0,	 lim : 5,	 LCurly : 3,	 RCurly : 0,	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 3,	 lim : 5,	 semicolon: ""none""}
apneCatch :	 {kind : apnkCatch,	 min : 5,	 lim : 15,	 LParen : 10,	RParen : 12,	LCurly : 13,	RCurly : 0,	semicolon: ""none""}
apneVariable :	 {kind : apnkName,	 min : 11,	 lim : 12,	 name : ""e"",	 semicolon: ""none""}
apneBody :	 {kind : apnkBlock,	 min : 13,	 lim : 15,	 semicolon: ""none""}
";
        #endregion

        [TestMethod]
        [WorkItem(293102)]
        public void ExtendIncompleteStatments()
        {
            PerformRequests(@"
                if (false)
                                    /*SemiColon*/|if|;
                if (false) a; else
                                    /*SemiColon*/|if|;
                while (false)
                                    /*SemiColon*/|while|;
                with (a)
                                    /*SemiColon*/|with|;
                for (;;)
                                    /*SemiColon*/|for|;
                for (a in b)
                                    /*SemiColon*/|forIn|;

                /* other statements */
                var x = 0;
                ", (context, offset, data, index) => {
                var cursor = context.GetASTCursor();

                IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

                StringBuilder serializedNodeString = new StringBuilder();
                foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
                {
                    serializedNodeString.Append(GetNodeSerializedString(node, cursor));
                }

                var currentNode = cursor.SeekToOffset(offset);

                AssertNodeType(currentNode, data);
                Assert.IsTrue(currentNode.EndOffset == offset);
            });
        }

        [TestMethod]
        [WorkItem(293142)]
        public void TerminateDoWhileWithNoWhileAtCurly()
        {
            int inWhile = 0;
            
            PerformRequests(@"
                function f() { 
                    do{|inwhile|x++ }|end|
               }", 
                 (context, offset, data, index) =>
                 {
                     if (data == "inwhile")
                         inWhile = offset;
                     else
                     {
                         var cursor = context.GetASTCursor();
                         cursor.SeekToOffset(inWhile);
                         cursor.MoveUp();
                         cursor.MoveUp();
                         var node = cursor.MoveUp();
                         Assert.AreEqual(AuthorParseNodeKind.apnkDoWhile, node.Kind);
                         Assert.AreEqual(offset, node.EndOffset);
                     }
                 });
        }

        [TestMethod]
        [WorkItem(304613)]
        public void ExtendCaseOnRCurly()
        {
            PerformRequests(@"
                    switch (a) { 
                        default:               /*RCurly*/|default|}

                    switch (a) { 
                        case 1:               /*RCurly*/|case|}"
            , (context, offset, data, index) =>
                 {
                     var cursor = context.GetASTCursor();
                     var currentNode = cursor.SeekToOffset(offset);

                     AssertNodeType(currentNode, data);
                     Assert.IsTrue(currentNode.EndOffset == offset);
                 });
        }

        [TestMethod]
        [WorkItem(304613)]
        public void ExtendCase()
        {
            PerformRequests(@"
                    switch (a) {
                                           case 1:                
                        /* case 1  */|case| default:               
                        /* default */|default| case 2:                
                        /* case 2  */|case|}"
            , (context, offset, data, index) =>
            {
                var cursor = context.GetASTCursor();
                var currentNode = cursor.SeekToOffset(offset);

                AssertNodeType(currentNode, data);

            });
        }

        [TestMethod]
        [WorkItem(306584)]
        public void ExtendSwitch()
        {
            PerformRequests(@"switch (a) {

                /* inside Switch */|
               function foo () {}",
                 (context, offset, data, index) =>
                 {
                     var cursor = context.GetASTCursor();
                     var currentNode = cursor.SeekToOffset(offset); ;
                     Assert.IsTrue(currentNode.Kind == AuthorParseNodeKind.apnkSwitch);
                 });
        }

        [TestMethod]
        [WorkItem(281240)]
        public void GetEdgeLableOnVarDecl()
        {
            PerformRequests(@"
{
    var a;
    var f, b,
|test;
}
",
                 (context, offset, data, index) =>
                 {
                     var cursor = context.GetASTCursor();
                     var currentNode = cursor.SeekToOffset(offset);
                     var label = cursor.GetEdgeLabel();
                     Assert.IsTrue(currentNode.Kind == AuthorParseNodeKind.apnkVarDecl);
                 });
        }

        [TestMethod]
        [WorkItem(372164)]
        public void DoWhileSemicolon()
        {
            PerformRequests(@"do{}whi|explicit|le(false);  do{}whi|automatic|le(false)  do{}whi|automatic|le(false)
                ", (context, offset, data, index) =>
                 {
                     var cursor = context.GetASTCursor();
                     var current = cursor.SeekToOffset(offset);
                     Assert.IsTrue(current.Kind == AuthorParseNodeKind.apnkDoWhile);
                     switch (data)
                     {
                         case "explicit":
                             Assert.IsTrue((current.Flags & AuthorParseNodeFlags.apnfExplicitSimicolon) != 0);
                             break;
                         case "automatic":
                             Assert.IsTrue((current.Flags & AuthorParseNodeFlags.apnfAutomaticSimicolon) != 0);
                             break;
                         default:
                             Assert.Fail("Unexpected value");
                             break;
                     }
                 });
        }

        [TestMethod]
        [WorkItem(637206)]
        public void GetEdgeLabelWithBadCursor()
        {
            var file = _session.FileFromText("function foo() {}");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();
            cursor.MoveUp();
            var edgeLabel = cursor.GetEdgeLabel();
            edgeLabel.Expect(AuthorParseNodeEdge.apneNone);
        }

        [TestMethod]
        [WorkItem(672279)]
        public void LetAndConstDeclarations()
        {
            var file = _session.FileFromText(@"
{
const c_1 = 42;
const c_2 = 'b', c_3 = 'c';
let l_1;
let l_2, l_3 = true;
}
");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(LetAndConstDeclarationsExpectedString, walker.TreeString);

            // check the serialized tree
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            String s = serializedNodeString.ToString();
            Assert.AreEqual(LetAndConstDeclarationsExpectedTreeString, serializedNodeString.ToString());
        }
        #region Test Data
        public const string LetAndConstDeclarationsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 86,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 2,	 lim : 86,	 semicolon: ""none""}
apneListItem :	 {kind : apnkBlock,	 min : 2,	 lim : 84,	 semicolon: ""none""}
apneBlockBody :	 {kind : apnkList,	 min : 5,	 lim : 80,	 semicolon: ""none""}
apneListItem :	 {kind : apnkConstDecl,	 min : 5,	 lim : 19,	 name : ""c_1"",	 IdentifierMin : 11,	 semicolon: ""explicit""}
apneInitialization :	 {kind : apnkInt,	 min : 17,	 lim : 19,	 intValue : 42,	 semicolon: ""none""}
apneListItem :	 {kind : apnkConstDeclList,	 min : 22,	 lim : 48,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkConstDecl,	 min : 22,	 lim : 37,	 name : ""c_2"",	 IdentifierMin : 28,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkStr,	 min : 34,	 lim : 37,	 stringValue : ""b"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkConstDecl,	 min : 39,	 lim : 48,	 name : ""c_3"",	 IdentifierMin : 39,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkStr,	 min : 45,	 lim : 48,	 stringValue : ""c"",	 semicolon: ""none""}
apneListItem :	 {kind : apnkLetDecl,	 min : 51,	 lim : 58,	 name : ""l_1"",	 IdentifierMin : 55,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkLetDeclList,	 min : 61,	 lim : 80,	 semicolon: ""explicit""}
apneListItem :	 {kind : apnkLetDecl,	 min : 61,	 lim : 68,	 name : ""l_2"",	 IdentifierMin : 65,	 semicolon: ""none""}
apneListItem :	 {kind : apnkLetDecl,	 min : 70,	 lim : 80,	 name : ""l_3"",	 IdentifierMin : 70,	 semicolon: ""none""}
apneInitialization :	 {kind : apnkTrue,	 min : 76,	 lim : 80,	 semicolon: ""none""}
";
        public const string LetAndConstDeclarationsExpectedTreeString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 86,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 2,	 lim   : 86,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkBlock,	 min   : 2,	 lim   : 84,	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkList,	 min   : 5,	 lim   : 80,	 level : 3,	 ChildLabel : apneBlockBody}
				{kind  : apnkConstDecl,	 min   : 5,	 lim   : 19,	 name  : ""c_1"",	 level : 4,	 ChildLabel : apneListItem}
					{kind  : apnkInt,	 min   : 17,	 lim   : 19,	 level : 5,	 ChildLabel : apneInitialization}
				{kind  : apnkConstDeclList,	 min   : 22,	 lim   : 48,	 level : 4,	 ChildLabel : apneListItem}
					{kind  : apnkConstDecl,	 min   : 22,	 lim   : 37,	 name  : ""c_2"",	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 34,	 lim   : 37,	 level : 6,	 ChildLabel : apneInitialization}
					{kind  : apnkConstDecl,	 min   : 39,	 lim   : 48,	 name  : ""c_3"",	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkStr,	 min   : 45,	 lim   : 48,	 level : 6,	 ChildLabel : apneInitialization}
				{kind  : apnkLetDecl,	 min   : 51,	 lim   : 58,	 name  : ""l_1"",	 level : 4,	 ChildLabel : apneListItem}
				{kind  : apnkLetDeclList,	 min   : 61,	 lim   : 80,	 level : 4,	 ChildLabel : apneListItem}
					{kind  : apnkLetDecl,	 min   : 61,	 lim   : 68,	 name  : ""l_2"",	 level : 5,	 ChildLabel : apneListItem}
					{kind  : apnkLetDecl,	 min   : 70,	 lim   : 80,	 name  : ""l_3"",	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkTrue,	 min   : 76,	 lim   : 80,	 level : 6,	 ChildLabel : apneInitialization}
";
        #endregion

        [TestMethod]
        [WorkItem(797782)]
        [TestCategory("Classes"), TestCategory("ASTCursor")]
        public void ClassDeclarations()
        {
            var file = _session.FileFromText(@"
class Base { }
class Test extends Base {
    static foo() { }
    bar() { }
}
");
            var context = _session.OpenContext(file);
            var cursor = context.GetASTCursor();

            PrintWalker walker = new PrintWalker(cursor);
            walker.Walk();
            Assert.AreEqual(ClassDeclarationsExpectedString, walker.TreeString);

            // check the serialized tree
            IAuthorParseNodeSet serializedNodes = cursor.GetSubTree(-1);

            StringBuilder serializedNodeString = new StringBuilder();
            foreach (AuthorParseNode node in serializedNodes.ToEnumerable())
            {
                serializedNodeString.Append(GetNodeSerializedString(node, cursor));
            }
            String s = serializedNodeString.ToString();
            Assert.AreEqual(ClassDeclarationsExpectedTreeString, serializedNodeString.ToString());
        }
        #region Test Data
        public const string ClassDeclarationsExpectedString = @"apneNone :	 {kind : apnkProg,	 min : 0,	 lim : 85,	 semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 2,	 lim : 83,	 semicolon: ""none""}
apneListItem :	 {kind : apnkClassDecl,	 min : 2,	 lim : 16,	 name : ""Base"",	 IdentifierMin : 0,	 LCurly : 13,	 RCurly : 15,	 semicolon: ""none""}
apneCtor :	 {kind : apnkFncDecl,	 min : 13,	 lim : 14,	 name : ""Base"",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 0,	RParen : 0,	LCurly : 0,	RCurly : 0,	semicolon: ""none""}
apneListItem :	 {kind : apnkClassDecl,	 min : 18,	 lim : 83,	 name : ""Test"",	 IdentifierMin : 0,	 LCurly : 42,	 RCurly : 82,	 semicolon: ""none""}
apneExtends :	 {kind : apnkName,	 min : 37,	 lim : 41,	 name : ""Base"",	 semicolon: ""none""}
apneCtor :	 {kind : apnkFncDecl,	 min : 42,	 lim : 43,	 name : ""Test"",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 0,	RParen : 0,	LCurly : 0,	RCurly : 0,	semicolon: ""none""}
apneBody :	 {kind : apnkList,	 min : 42,	 lim : 43,	 semicolon: ""none""}
apneListItem :	 {kind : apnkCall,	 min : 42,	 lim : 43,	 semicolon: ""none""}
apneTarget :	 {kind : apnkSuper,	 min : 42,	 lim : 43,	 semicolon: ""none""}
apneArguments :	 {kind : apnkSpread,	 min : 42,	 lim : 43,	 semicolon: ""none""}
apneStaticMembers :	 {kind : apnkMember,	 min : 56,	 lim : 65,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 56,	 lim : 59,	 stringValue : ""foo"",	 semicolon: ""none""}
apneMember :	 {kind : apnkFncDecl,	 min : 56,	 lim : 65,	 name : ""foo"",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 59,	RParen : 60,	LCurly : 62,	RCurly : 64,	semicolon: ""none""}
apneMembers :	 {kind : apnkMember,	 min : 71,	 lim : 80,	 semicolon: ""none""}
apneTarget :	 {kind : apnkStr,	 min : 71,	 lim : 74,	 stringValue : ""bar"",	 semicolon: ""none""}
apneMember :	 {kind : apnkFncDecl,	 min : 71,	 lim : 80,	 name : ""bar"",	 IdentifierMin : 0,	 FunctionKeywordMin : 0,	 LParen : 74,	RParen : 75,	LCurly : 77,	RCurly : 79,	semicolon: ""none""}
";
        public const string ClassDeclarationsExpectedTreeString = @"{kind  : apnkProg,	 min   : 0,	 lim   : 85,	 level : 0,	 ChildLabel : apneNone}
	{kind  : apnkList,	 min   : 2,	 lim   : 83,	 level : 1,	 ChildLabel : apneBody}
		{kind  : apnkClassDecl,	 min   : 2,	 lim   : 16,	 name  : ""Base"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkFncDecl,	 min   : 13,	 lim   : 14,	 name  : ""Base"",	 level : 3,	 ChildLabel : apneCtor}
		{kind  : apnkClassDecl,	 min   : 18,	 lim   : 83,	 name  : ""Test"",	 level : 2,	 ChildLabel : apneListItem}
			{kind  : apnkName,	 min   : 37,	 lim   : 41,	 name  : ""Base"",	 level : 3,	 ChildLabel : apneExtends}
			{kind  : apnkFncDecl,	 min   : 42,	 lim   : 43,	 name  : ""Test"",	 level : 3,	 ChildLabel : apneCtor}
				{kind  : apnkList,	 min   : 42,	 lim   : 43,	 level : 4,	 ChildLabel : apneBody}
					{kind  : apnkCall,	 min   : 42,	 lim   : 43,	 level : 5,	 ChildLabel : apneListItem}
						{kind  : apnkSuper,	 min   : 42,	 lim   : 43,	 level : 6,	 ChildLabel : apneTarget}
						{kind  : apnkSpread,	 min   : 42,	 lim   : 43,	 level : 6,	 ChildLabel : apneArguments}
			{kind  : apnkMember,	 min   : 56,	 lim   : 65,	 level : 3,	 ChildLabel : apneStaticMembers}
				{kind  : apnkStr,	 min   : 56,	 lim   : 59,	 level : 4,	 ChildLabel : apneTarget}
				{kind  : apnkFncDecl,	 min   : 56,	 lim   : 65,	 name  : ""foo"",	 level : 4,	 ChildLabel : apneMember}
			{kind  : apnkMember,	 min   : 71,	 lim   : 80,	 level : 3,	 ChildLabel : apneMembers}
				{kind  : apnkStr,	 min   : 71,	 lim   : 74,	 level : 4,	 ChildLabel : apneTarget}
				{kind  : apnkFncDecl,	 min   : 71,	 lim   : 80,	 name  : ""bar"",	 level : 4,	 ChildLabel : apneMember}
";
        #endregion

    }

    #region Tree Walkers
    internal abstract class ParseTreeWalker
    {
        protected IAuthorParseNodeCursor cursor;

        public ParseTreeWalker(IAuthorParseNodeCursor cursor)
        {
            this.cursor = cursor;
        }

        public abstract void VisitNode(ref AuthorParseNodeDetails node, AuthorParseNodeEdge childLabel);

        public void VisitChildNode(AuthorParseNodeEdge childLabel)
        {
            AuthorParseNodeDetails childNode = cursor.MoveToChild(childLabel, 0);
            if (childNode.Kind != AuthorParseNodeKind.apnkEmptyNode)
            {
                // check the GetEdgeLabel method is consistent
                AuthorParseNodeEdge currentNodeEdgeLabel = cursor.GetEdgeLabel();
                Assert.AreEqual(currentNodeEdgeLabel, childLabel);

                Walk(ref childNode, childLabel);
            }
            cursor.MoveUp();
        }

        public void VisitChildList(AuthorParseNodeEdge childLabel)
        {
            int i = 0;
            AuthorParseNodeDetails node = cursor.MoveToChild(childLabel, i);
            while (node.Kind != AuthorParseNodeKind.apnkEmptyNode)
            {
                Walk(ref node, childLabel);
                cursor.MoveUp();
                i++;
                node = cursor.MoveToChild(childLabel, i);
            }
            cursor.MoveUp();
        }

        public void Walk()
        {
            AuthorParseNodeDetails node = cursor.Current();
            Walk(ref node, AuthorParseNodeEdge.apneNone);
        }

        public void Walk(ref AuthorParseNodeDetails node, AuthorParseNodeEdge childLabel)
        {
            VisitNode(ref node, childLabel);

            switch (node.Kind)
            {
                case AuthorParseNodeKind.apnkName:
                case AuthorParseNodeKind.apnkStr:
                case AuthorParseNodeKind.apnkInt:
                case AuthorParseNodeKind.apnkFlt:
                case AuthorParseNodeKind.apnkRegExp:
                case AuthorParseNodeKind.apnkThis:
                case AuthorParseNodeKind.apnkNull:
                case AuthorParseNodeKind.apnkFalse:
                case AuthorParseNodeKind.apnkTrue:
                case AuthorParseNodeKind.apnkEmpty:
                case AuthorParseNodeKind.apnkEndCode:
                case AuthorParseNodeKind.apnkDebugger:
                case AuthorParseNodeKind.apnkLabel:
                case AuthorParseNodeKind.apnkSuper:
                    break;
                case AuthorParseNodeKind.apnkBreak:
                case AuthorParseNodeKind.apnkContinue:
                    break;
                case AuthorParseNodeKind.apnkNot:
                case AuthorParseNodeKind.apnkNeg:
                case AuthorParseNodeKind.apnkPos:
                case AuthorParseNodeKind.apnkLogNot:
                case AuthorParseNodeKind.apnkIncPre:
                case AuthorParseNodeKind.apnkDecPre:
                case AuthorParseNodeKind.apnkTypeof:
                case AuthorParseNodeKind.apnkVoid:
                case AuthorParseNodeKind.apnkDelete:
                case AuthorParseNodeKind.apnkIncPost:
                case AuthorParseNodeKind.apnkDecPost:
                    VisitChildNode(AuthorParseNodeEdge.apneOperand);
                    break;
                case AuthorParseNodeKind.apnkArray:
                    VisitChildNode(AuthorParseNodeEdge.apneElements);
                    break;
                case AuthorParseNodeKind.apnkObject:
                    VisitChildNode(AuthorParseNodeEdge.apneMembers);
                    break;
                case AuthorParseNodeKind.apnkGetMember:
                case AuthorParseNodeKind.apnkSetMember:
                    VisitChildNode(AuthorParseNodeEdge.apneTarget);
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    break;
                case AuthorParseNodeKind.apnkAdd:
                case AuthorParseNodeKind.apnkSub:
                case AuthorParseNodeKind.apnkMul:
                case AuthorParseNodeKind.apnkDiv:
                case AuthorParseNodeKind.apnkMod:
                case AuthorParseNodeKind.apnkOr:
                case AuthorParseNodeKind.apnkXor:
                case AuthorParseNodeKind.apnkAnd:
                case AuthorParseNodeKind.apnkEq:
                case AuthorParseNodeKind.apnkNe:
                case AuthorParseNodeKind.apnkLt:
                case AuthorParseNodeKind.apnkLe:
                case AuthorParseNodeKind.apnkGe:
                case AuthorParseNodeKind.apnkGt:
                case AuthorParseNodeKind.apnkDot:
                case AuthorParseNodeKind.apnkAsg:
                case AuthorParseNodeKind.apnkInstOf:
                case AuthorParseNodeKind.apnkIn:
                case AuthorParseNodeKind.apnkEqv:
                case AuthorParseNodeKind.apnkNEqv:
                case AuthorParseNodeKind.apnkComma:
                case AuthorParseNodeKind.apnkLogOr:
                case AuthorParseNodeKind.apnkLogAnd:
                case AuthorParseNodeKind.apnkLsh:
                case AuthorParseNodeKind.apnkRsh:
                case AuthorParseNodeKind.apnkRs2:
                case AuthorParseNodeKind.apnkAsgAdd:
                case AuthorParseNodeKind.apnkAsgSub:
                case AuthorParseNodeKind.apnkAsgMul:
                case AuthorParseNodeKind.apnkAsgDiv:
                case AuthorParseNodeKind.apnkAsgMod:
                case AuthorParseNodeKind.apnkAsgAnd:
                case AuthorParseNodeKind.apnkAsgXor:
                case AuthorParseNodeKind.apnkAsgOr:
                case AuthorParseNodeKind.apnkAsgLsh:
                case AuthorParseNodeKind.apnkAsgRsh:
                case AuthorParseNodeKind.apnkAsgRs2:
                case AuthorParseNodeKind.apnkScope:
                    VisitChildNode(AuthorParseNodeEdge.apneLeft);
                    VisitChildNode(AuthorParseNodeEdge.apneRight);
                    break;
                case AuthorParseNodeKind.apnkMember:
                    VisitChildNode(AuthorParseNodeEdge.apneTarget);
                    VisitChildNode(AuthorParseNodeEdge.apneMember);
                    break;
                case AuthorParseNodeKind.apnkCall:
                case AuthorParseNodeKind.apnkNew:
                    VisitChildNode(AuthorParseNodeEdge.apneTarget);
                    VisitChildNode(AuthorParseNodeEdge.apneArguments);
                    break;
                case AuthorParseNodeKind.apnkIndex:
                    VisitChildNode(AuthorParseNodeEdge.apneTarget);
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    break;
                case AuthorParseNodeKind.apnkQmark:
                    VisitChildNode(AuthorParseNodeEdge.apneCondition);
                    VisitChildNode(AuthorParseNodeEdge.apneThen);
                    VisitChildNode(AuthorParseNodeEdge.apneElse);
                    break;
                case AuthorParseNodeKind.apnkProg:
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkFor:
                    VisitChildNode(AuthorParseNodeEdge.apneInitialization);
                    VisitChildNode(AuthorParseNodeEdge.apneCondition);
                    VisitChildNode(AuthorParseNodeEdge.apneIncrement);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkIf:
                    VisitChildNode(AuthorParseNodeEdge.apneCondition);
                    VisitChildNode(AuthorParseNodeEdge.apneThen);
                    VisitChildNode(AuthorParseNodeEdge.apneElse);
                    break;
                case AuthorParseNodeKind.apnkWhile:
                case AuthorParseNodeKind.apnkDoWhile:
                    VisitChildNode(AuthorParseNodeEdge.apneCondition);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkForIn:
                    VisitChildNode(AuthorParseNodeEdge.apneVariable);
                    VisitChildNode(AuthorParseNodeEdge.apneObject);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkReturn:
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    break;
                case AuthorParseNodeKind.apnkVarDeclList:
                case AuthorParseNodeKind.apnkLetDeclList:
                case AuthorParseNodeKind.apnkConstDeclList:
                case AuthorParseNodeKind.apnkList:
                    VisitChildList(AuthorParseNodeEdge.apneListItem);
                    break;
                case AuthorParseNodeKind.apnkVarDecl:
                case AuthorParseNodeKind.apnkLetDecl:
                case AuthorParseNodeKind.apnkConstDecl:
                    VisitChildNode(AuthorParseNodeEdge.apneInitialization);
                    break;
                case AuthorParseNodeKind.apnkFncDecl:
                    VisitChildList(AuthorParseNodeEdge.apneArgument);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkBlock:
                    VisitChildNode(AuthorParseNodeEdge.apneBlockBody);
                    break;
                case AuthorParseNodeKind.apnkWith:
                    VisitChildNode(AuthorParseNodeEdge.apneObject);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkSwitch:
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    VisitChildList(AuthorParseNodeEdge.apneCase);
                    VisitChildNode(AuthorParseNodeEdge.apneDefaultCase);
                    break;
                case AuthorParseNodeKind.apnkCase:
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkDefaultCase:
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkTryFinally:
                    VisitChildNode(AuthorParseNodeEdge.apneTry);
                    VisitChildNode(AuthorParseNodeEdge.apneFinally);
                    break;
                case AuthorParseNodeKind.apnkFinally:
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkCatch:
                    VisitChildNode(AuthorParseNodeEdge.apneVariable);
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkTryCatch:
                    VisitChildNode(AuthorParseNodeEdge.apneTry);
                    VisitChildNode(AuthorParseNodeEdge.apneCatch);
                    break;
                case AuthorParseNodeKind.apnkTry:
                    VisitChildNode(AuthorParseNodeEdge.apneBody);
                    break;
                case AuthorParseNodeKind.apnkThrow:
                    VisitChildNode(AuthorParseNodeEdge.apneValue);
                    break;
                case AuthorParseNodeKind.apnkClassDecl:
                    VisitChildNode(AuthorParseNodeEdge.apneExtends);
                    VisitChildNode(AuthorParseNodeEdge.apneCtor);
                    VisitChildNode(AuthorParseNodeEdge.apneStaticMembers);
                    VisitChildNode(AuthorParseNodeEdge.apneMembers);
                    break;
                case AuthorParseNodeKind.apnkStrTemplate:
                    VisitChildNode(AuthorParseNodeEdge.apneStringLiterals);
                    VisitChildNode(AuthorParseNodeEdge.apneSubstitutionExpression);
                    VisitChildNode(AuthorParseNodeEdge.apneStringRawLiterals);
                    break;
            };
        }
    }

    internal class PrintWalker : ParseTreeWalker
    {
        StringBuilder treeString;

        public PrintWalker(IAuthorParseNodeCursor cursor)
            : base(cursor)
        {
            treeString = new StringBuilder();
        }

        public override void VisitNode(ref AuthorParseNodeDetails node, AuthorParseNodeEdge childLabel)
        {
            PrintNode(ref node, childLabel);
        }

        public void PrintNode(ref AuthorParseNodeDetails nodeDetails, AuthorParseNodeEdge childLabel)
        {
            treeString.AppendFormat("{0} :\t ", childLabel.ToString());
            treeString.Append("{");
            treeString.AppendFormat("kind : {0},\t ", nodeDetails.Kind);
            treeString.AppendFormat("min : {0},\t ", nodeDetails.StartOffset);
            treeString.AppendFormat("lim : {0},\t ", nodeDetails.EndOffset);
            
            string label = cursor.GetStatementLabel();
            if (!String.IsNullOrEmpty(label))
            {
                treeString.AppendFormat("label : \"{0}\",\t ", label);
            }

            switch (nodeDetails.Kind)
            {
                case AuthorParseNodeKind.apnkInt:
                    treeString.AppendFormat("intValue : {0},\t ", cursor.GetIntValue());
                    break;
                case AuthorParseNodeKind.apnkFlt:
                    treeString.AppendFormat("floatValue : {0},\t ", cursor.GetFloatValue());
                    break;
                case AuthorParseNodeKind.apnkStr:
                    treeString.AppendFormat("stringValue : \"{0}\",\t ", cursor.GetStringValue());
                    break;
                case AuthorParseNodeKind.apnkName:
                    treeString.AppendFormat("name : \"{0}\",\t ", cursor.GetStringValue());
                    break;
                case AuthorParseNodeKind.apnkRegExp:
                    string regexpString;
                    AuthorRegExpOptions regexpOptions;
                    cursor.GetRegExpValue(out regexpString, out regexpOptions);
                    treeString.AppendFormat("regexpValue : \"{0}\",\t  regexpOptions: \"{1}\",\t ", regexpString, regexpOptions);
                    break;
                case AuthorParseNodeKind.apnkBreak:
                case AuthorParseNodeKind.apnkContinue:
                    treeString.AppendFormat("targetLabel : \"{0}\",\t ", cursor.GetTargetLabel());
                    break;
                case AuthorParseNodeKind.apnkBlock:
                    if ((nodeDetails.Flags & AuthorParseNodeFlags.apnfSyntheticNode) != 0)
                    treeString.Append("automaticBlock : true,\t ");
                    break;
                case AuthorParseNodeKind.apnkVarDecl:
                case AuthorParseNodeKind.apnkLetDecl:
                case AuthorParseNodeKind.apnkConstDecl:
                    treeString.AppendFormat("name : \"{0}\",\t ", cursor.GetStringValue());
                    treeString.AppendFormat("IdentifierMin : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpIdentifierMin));
                    break;
                case AuthorParseNodeKind.apnkFncDecl:
                case AuthorParseNodeKind.apnkCase:
                case AuthorParseNodeKind.apnkSwitch:
                case AuthorParseNodeKind.apnkDoWhile:
                case AuthorParseNodeKind.apnkCatch:
                    if (nodeDetails.Kind == AuthorParseNodeKind.apnkFncDecl)
                    {
                        treeString.AppendFormat("name : \"{0}\",\t ", cursor.GetStringValue());
                        treeString.AppendFormat("IdentifierMin : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpIdentifierMin));
                        treeString.AppendFormat("FunctionKeywordMin : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpFunctionKeywordMin));
                    }
                    treeString.AppendFormat("LParen : {0},\t", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLParenMin));
                    treeString.AppendFormat("RParen : {0},\t", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRParenMin));
                    treeString.AppendFormat("LCurly : {0},\t", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLCurlyMin));
                    treeString.AppendFormat("RCurly : {0},\t", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRCurlyMin));
                    break;
                case AuthorParseNodeKind.apnkTry:
                case AuthorParseNodeKind.apnkFinally:
                    treeString.AppendFormat("LCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLCurlyMin));
                    treeString.AppendFormat("RCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRCurlyMin));
                    break;
                case AuthorParseNodeKind.apnkIf:
                case AuthorParseNodeKind.apnkFor:
                case AuthorParseNodeKind.apnkForIn:
                case AuthorParseNodeKind.apnkWhile:
                case AuthorParseNodeKind.apnkWith:
                    treeString.AppendFormat("LParen : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLParenMin));
                    treeString.AppendFormat("RParen : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRParenMin));
                    break;
                case AuthorParseNodeKind.apnkIndex:
                case AuthorParseNodeKind.apnkArray:
                    treeString.AppendFormat("LBrack : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLBrackMin));
                    treeString.AppendFormat("RBrack : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRBrackMin));
                    break;
                case AuthorParseNodeKind.apnkObject:
                    treeString.AppendFormat("LCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLCurlyMin));
                    treeString.AppendFormat("RCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRCurlyMin));
                    break;
                case AuthorParseNodeKind.apnkClassDecl:
                    treeString.AppendFormat("name : \"{0}\",\t ", cursor.GetStringValue());
                    treeString.AppendFormat("IdentifierMin : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpIdentifierMin));
                    treeString.AppendFormat("LCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpLCurlyMin));
                    treeString.AppendFormat("RCurly : {0},\t ", cursor.GetNodeProperty(AuthorParseNodeProperty.apnpRCurlyMin));
                    break;
            };

            if ((nodeDetails.Flags & AuthorParseNodeFlags.apnfAutomaticSimicolon) != 0)
                treeString.AppendFormat("semicolon: \"automatic\"");
            else if ((nodeDetails.Flags & AuthorParseNodeFlags.apnfExplicitSimicolon) != 0)
                treeString.AppendFormat("semicolon: \"explicit\"");
            else if ((nodeDetails.Flags & AuthorParseNodeFlags.apnfMissingSimicolon) != 0)
                treeString.AppendFormat("semicolon: \"missing\"");
            else
                treeString.AppendFormat("semicolon: \"none\"");

            treeString.Append("}");
            treeString.AppendLine();
        }

        public string TreeString
        {
            get { return this.treeString.ToString(); }
        }
    }

    internal class StatementSpanPrinter : ParseTreeWalker
    {
        StringBuilder treeString;

            public StatementSpanPrinter(IAuthorParseNodeCursor cursor)
            : base(cursor)
        {
            treeString = new StringBuilder();
        }

        public override void VisitNode(ref AuthorParseNodeDetails node, AuthorParseNodeEdge childLabel)
        {
            PrintNodeSpan(ref node);
        }

        public void PrintNodeSpan(ref AuthorParseNodeDetails nodeDetails)
        {
            int spanMin;
            int spanLim;
            cursor.GetStatementSpan(out spanMin, out spanLim);

            treeString.Append("{");
            treeString.AppendFormat("kind : {0},\t ", nodeDetails.Kind);
            treeString.AppendFormat("min : {0},\t ", nodeDetails.StartOffset);
            treeString.AppendFormat("lim : {0},\t ", nodeDetails.EndOffset);
            treeString.AppendFormat("spanMin : {0},\t ", spanMin);
            treeString.AppendFormat("spanLim : {0},\t ", spanLim);
            treeString.Append("}");
            treeString.AppendLine();
        }

        public string TreeString
        {
            get { return this.treeString.ToString(); }
        }
    }

    internal class NodeWalker : ParseTreeWalker
    {
        private Action<AuthorParseNodeDetails> action;

        public NodeWalker(IAuthorParseNodeCursor cursor, Action<AuthorParseNodeDetails> action)
            : base(cursor)
        {
            this.action = action;
        }

        public override void VisitNode(ref AuthorParseNodeDetails node, AuthorParseNodeEdge childLabel)
        {
            action(node);
        }

        public static void ForEach(IAuthorParseNodeCursor cursor, Action<AuthorParseNodeDetails> action)
        {
            NodeWalker walker = new NodeWalker(cursor, action);
            walker.Walk();
        }
    }
    #endregion

    public static class ASTCursorHelpers
    {
        public static void ExpectContains(this IAuthorParseNodeSet nodeSet, IEnumerable<AuthorParseNodeDetails> expect)
        {
            var nodes = nodeSet.ToEnumerable();
            foreach (var expectedNode in expect)
            {
                bool found = false;
                foreach (var node in nodes)
                {
                    if (node.Details.Kind == expectedNode.Kind &&
                        node.Details.StartOffset == expectedNode.StartOffset &&
                        node.Details.EndOffset == expectedNode.EndOffset)
                    {
                        found = true;
                        break;
                    }
                }
                Assert.IsTrue(found, "Cound not find node : " + expectedNode.ToString());
            }
        }
    }
}