using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using Microsoft.VisualStudio.TestTools.UnitTesting;

using Microsoft.BPT.Tests.DirectAuthor;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;

namespace DirectAuthorTests
{
    [TestClass]
    public class Errors : DirectAuthorTest
    {
        [TestMethod]
        public void AllParseErrors()
        {
            var file = _session.FileFromText(AllErrorsText);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages();

            var expected = new[] 
            {
                new { Position = 11,   Length = 4, Message = "Syntax error", MessageID = 1002 },
                new { Position = 16,   Length = 3, Message = "Expected '/'", MessageID = 1012 },
                new { Position = 73,   Length = 5, Message = "Expected identifier", MessageID = 1010 },
                new { Position = 144,  Length = 1, Message = "Expected identifier, string or number", MessageID = 1028 },
                new { Position = 207,  Length = 1, Message = "Expected identifier, string or number", MessageID = 1028 },
                new { Position = 269,  Length = 2, Message = "Expected ':'", MessageID = 1003 },
                new { Position = 338,  Length = 1, Message = "Expected ')'", MessageID = 1006 },
                new { Position = 398,  Length = 2, Message = "Expected identifier", MessageID = 1010 },
                new { Position = 461,  Length = 1, Message = "Expected ')'", MessageID = 1006 },
                new { Position = 515,  Length = 1, Message = "Syntax error", MessageID = 1002 },
                new { Position = 566,  Length = 2, Message = "Expected identifier", MessageID = 1010 },
                new { Position = 628,  Length = 1, Message = "Expected '{'", MessageID = 1008 },
                new { Position = 702,  Length = 1, Message = "Expected '{'", MessageID = 1008 },
                new { Position = 763,  Length = 1, Message = "Expected identifier", MessageID = 1010 },
                new { Position = 765,  Length = 1, Message = "Expected ')'", MessageID = 1006 },
                new { Position = 826,  Length = 1, Message = "Expected '{'", MessageID = 1008 },
                new { Position = 885,  Length = 2, Message = "Syntax error", MessageID = 1002 },
                new { Position = 953,  Length = 1, Message = "Expected ';'", MessageID = 1004 },
                new { Position = 1017, Length = 7, Message = "'default' can only appear once in a 'switch' statement", MessageID = 1027 },
                new { Position = 1081, Length = 1, Message = "Can't have 'continue' outside of loop", MessageID = 1020 },
                new { Position = 1153, Length = 6, Message = "Label not found", MessageID = 1026 },
                new { Position = 1192, Length = 1, Message = "Can't have 'continue' outside of loop", MessageID = 1020 },
                new { Position = 1254, Length = 1, Message = "Can't have 'break' outside of loop", MessageID = 1019 },
                new { Position = 1311, Length = 6, Message = "'return' statement outside of function", MessageID = 1018 },
                new { Position = 1376, Length = 1, Message = "Expected ';'", MessageID = 1004 }
            };
            AssertAreStructurallyEqual(expected, messages.ToEnumerable());
        }
        #region Test data
        public const string AllErrorsText = @"var t01 = (case / 3);                         // ERRsyntax
var t02 = a. 31122;                           // ERRnoIdent
var t03 = { a: 12, get > () { return 3; } };  // ERRnoMemberIdent
var t04 = { a: 12, > : 32 };                  // ERRnoMemberIdent
var t05 = { a 12 };                           // ERRnoColon
var t06 = { get count(a) { return 3; } };     // ERRnoRparen
var t07 = function (23) { };                  // ERRnoIdent
var t08 = function (a { };                    // ERRnoRparen
var t09 = +23 = 4;                            // ERRsyntax
var  10 = 10;                                 // ERRnoIdent

try * { } catch(a) { };                       // ERRnoLcurly
try { } finally * { };                        // ERRnoLcurly
try { } catch( ) { };                         // ERRnoIdent
try { } catch(a) * { };                       // ERRnoLcurly
for (var a, b in c) { };                      // ERRsyntax
for (var i = 0; i < 10) { }                   // ERRnoSemic
switch (a) { default: 2; default: 3; }        // ERRdupDefault
label1: { continue label1; }                  // ERRbadContinue
label2: while(a > 0) { continue lable1; }     // ERRnoLabel
{ continue; }                                 // ERRbadContinue
{ break; }                                    // ERRbadBreak
{ return; }                                   // ERRbadReturn
{ a b }                                       // ERRnoSemic
";
        #endregion

        [TestMethod]
        public void StrictMode_Arguments()
        {
            var file = _session.FileFromText(StrictMode_Arguments_Text);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();
            Assert.AreEqual(1, messages.Length);
            Assert.AreEqual(30, messages[0].Position);
            Assert.AreEqual(9, messages[0].Length);
        }
        #region Test Data
        string StrictMode_Arguments_Text = @"
""use strict"";
function foo(arguments) { 
var x;
x = 0;
x++;
}
function moo() { }

";
        #endregion

        [TestMethod]
        [WorkItem(207132)]
        public void ParameterList_ErrorCorrection()
        {
            string text = @"function Pet(ID, Breed, Age) {
                            }
                            function foo() {
                                var a = new Pet(
                                var b = a;
                                // put ( after b.constructor to see if it has parameter info
                                // put ; after 'Pet(', type ';', then type ( after b.constructor
                                b.constructor(
                                ;
                            }";

            var file = _session.FileFromText(text);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();
            Assert.AreEqual(2, messages.Length);
            Assert.AreEqual(1002, messages[0].MessageID);
            Assert.AreEqual(PositionOf(text, "new Pet("), messages[0].Position);
            Assert.AreEqual(1002, messages[1].MessageID);
            Assert.AreEqual(PositionOf(text, "b.constructor("), messages[1].Position);
        }

        [TestMethod]
        [WorkItem(263713)]
        public void UpdateOnTextChange()
        {
            var file = _session.FileFromText(@"");
            var context = _session.OpenContext(file);

            context.Update();
            var messages = context.GetMessages();
            Assert.IsTrue(messages.Count == 0);

            file.Replace(0, file.Text.Length, "break;");

            context.Update();
            messages = context.GetMessages();
            Assert.IsTrue(messages.Count == 1);
        }

        [TestMethod]
        public void MultipleExternalSourcesWellformed()
        {
            var file = _session.FileFromText(MultipleExternalSourcesWellformedText);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();

            // The errors should occur at the END EXTERNAL SOURCE positions
            var positions = PositionsOf(file.Text, "/* END EXTERNAL SOURCE */").ToArray();
            var expected = new[] 
            {
                new { Position = positions[0],   Length = 25, Message = "Expected '}'", MessageID = 1009 },
                new { Position = positions[1],   Length = 25, Message = "Expected '}'", MessageID = 1009 },
                new { Position = positions[2],   Length = 25, Message = "Expected '}'", MessageID = 1009 },
            };
            AssertAreStructurallyEqual(expected, messages);
        }
        #region Test data
        const string MultipleExternalSourcesWellformedText = @"

/* BEGIN EXTERNAL SOURCE */

var a, b = 2, c = 1;
a = b + c;
function foo() {        
    
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */

var a, b = 2, c = 1;
a = b + c;

function bar() {
    
/* END EXTERNAL SOURCE */
        
/* BEGIN EXTERNAL SOURCE */

switch (a) {
  case b:
  
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */

function zoo() { }        
function goo() { }
var e = 20;
/* END EXTERNAL SOURCE */";

        #endregion

        [TestMethod]
        public void MultipleExternalSourcesMalformed1()
        {
            var file = _session.FileFromText(MultipleExternalSourcesMalformed1Text);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();

            // The errors should occur at the END EXTERNAL SOURCE positions
            var positions = PositionsOf(file.Text, "/* END EXTERNAL SOURCE */").ToArray();
            var expected = new[] 
            {
                new { Position = positions[0],   Length = 25, Message = "Expected '}'", MessageID = 1009 },
            };
            AssertAreStructurallyEqual(expected, messages);
        }
        #region Test data
        const string MultipleExternalSourcesMalformed1Text = @"
/* BEGIN EXTERNAL SOURCE */

var a, b = 2, c = 1;
a = b + c;
function foo() {        
    
/* END EXTERNAL SOURCE */

/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */

function zoo() { }        
function goo() { }
var e = 20;
/* END EXTERNAL SOURCE */";
        #endregion

        [TestMethod]
        public void MultipleExternalSourcesMalformed2()
        {
            var file = _session.FileFromText(MultipleExternalSourcesMalformed2Text);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();

            // The error should occur at the end of the file. The text is malformed so the external sources are ignored.
            var expected = new[] 
            {
                new { Position = file.Text.Length,   Length = 1, Message = "Expected '}'", MessageID = 1009 },
            };
            AssertAreStructurallyEqual(expected, messages);
        }
        #region Test data
        const string MultipleExternalSourcesMalformed2Text = @"
var a, b = 2, c = 1;
a = b + c;
function foo() {        
    
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
function zoo() {
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
function zoo() { }        
function goo() { }
var e = 20;
/* END EXTERNAL SOURCE */";
        #endregion

        [TestMethod]
        public void MultipleExternalSourcesMalformed3()
        {
            var file = _session.FileFromText(MultipleExternalSourcesMalformed3Text);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();

            // One error should occur at the first END EXTERNAL SOURCE position.
            // The other should occur at the end fo the file since the file is malformed.
            var positions = PositionsOf(file.Text, "/* END EXTERNAL SOURCE */").ToArray();
            var expected = new[] 
            {
                new { Position = positions[0],      Length = 25, Message = "Expected '}'", MessageID = 1009 },
                new { Position = file.Text.Length,  Length = 1,  Message = "Expected '}'", MessageID = 1009 },
            };
            AssertAreStructurallyEqual(expected, messages);
        }
        #region Test data
        const string MultipleExternalSourcesMalformed3Text = @"
/* BEGIN EXTERNAL SOURCE */
var a, b = 2, c = 1;
a = b + c;
function foo() {        
    
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
function zoo() {";
        #endregion

        [TestMethod]
        [WorkItem(386168)]
        public void MultipleExternalSourceStrictMode()
        {
            var file = _session.FileFromText(MultipleExternalSourceStrictModeText);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages().ToEnumerable().ToArray();

            // The "delete x"'s are not in strict mode so should not be reported.
            // The "delete y" is in strict mode and should be reported.
            // The error is reported on the ';' following the delete.
            var positions = PositionsOf(file.Text, "delete y").Select(p => file.Text.IndexOf(';', p)).ToArray();
            
            var expected = new[]
            {
                new { Position = positions[0], Length = 1, Message = "Calling delete on expression not allowed in strict mode", MessageID = 1045 },
                new { Position = positions[1], Length = 1, Message = "Calling delete on expression not allowed in strict mode", MessageID = 1045 },
            };
            AssertAreStructurallyEqual(expected, messages);
        }
        #region Test data
        const string MultipleExternalSourceStrictModeText = @"
/* BEGIN EXTERNAL SOURCE */
function f1(){
  alert(""1"");
  delete x;
}
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
""use strict"";
function f2(){
  alert(""2"");
  delete y;
}  
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
function f3(){
  alert(""3"");
  delete x;
}
/* END EXTERNAL SOURCE */

/* BEGIN EXTERNAL SOURCE */
""use strict"";
function f4(){
  alert(""4"");
  delete y;
}  
/* END EXTERNAL SOURCE */
";
        #endregion

        [TestMethod]
        [WorkItem(325800)]
        public void ErrorRecoveryAtFileEnd()
        {
            var file = _session.FileFromText("case");
            var context = _session.OpenContext(file);
            context.Update();
            var message = context.GetMessages();
            Assert.IsNotNull(message);
        }

        [TestMethod]
        [WorkItem(143040)]
        public void Bug143040()
        {
            var file = _session.FileFromText(@"/sometext/othertext");
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages();

            var expected = new[] 
            {
                new { Position = 0, Length = 10, Message = "Syntax error in regular expression" }, 
                new { Position = 10, Length = 9, Message = "Expected ';'"}
            };

            AssertAreStructurallyEqual(expected, messages.ToEnumerable());
        }

        [TestMethod]
        [WorkItem(418288)]
        public void ForStatementErrorCorrection()
        {
            var file = _session.FileFromText(ForStatementErrorCorrectionText);
            var context = _session.OpenContext(file);
            context.Update();
            var messages = context.GetMessages();
            var expected = new[]
            {
                new { Position = ForStatementErrorCorrectionText.IndexOf('}'), Length = 1, Message = "Syntax error" }
            };

            AssertAreStructurallyEqual(expected, messages.ToEnumerable());
        }
        #region Test data
        const string ForStatementErrorCorrectionText = @"
function init() {
    function foo() {
        function bar() {
            for (var index = 1; 
        }
    }
}
function ready() {
    init();
}
setTimeout(ready);";
        #endregion

        static int PositionOf(string text, string str)
        {
            var i = text.IndexOf(str);
            if (i >= 0) i += str.Length;
            while (i < text.Length && char.IsWhiteSpace(text[i]))
                i++;
            return i;
        }

        static IEnumerable<int> PositionsOf(string text, string str)
        {
            int index = 0;
            while (true)
            {
                var i = text.IndexOf(str, index);
                if (i >= 0)
                {
                    yield return i;
                    index = i + 1;
                }
                else
                    break;
            }
        }
    }
}
