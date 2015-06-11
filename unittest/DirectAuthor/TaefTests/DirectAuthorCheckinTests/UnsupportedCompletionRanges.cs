//----------------------------------------------------------------------------------------------------------------------
// <copyright file="UnsupportedCompletionRanges.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the UnsupportedCompletionRanges type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System.Collections.Generic;
    using System.Text;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class UnsupportedCompletionRanges : CompletionsBase
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
        [WorkItem(83841)]
        public void VarDeclaration()
        {
            ValidateHasCompletions("var variable; var|", "variable");

            ValidateNoCompletion("var |");
            ValidateNoCompletion("var x|");
            ValidateNoCompletion("var x|x");
            ValidateNoCompletion("var x,|");
            ValidateNoCompletion("var x, y|");
            ValidateNoCompletion("var x, y|, z;");
            ValidateNoCompletion("var x, y = 0,| z;");
            ValidateNoCompletion("var x, y = 0, z|;");
            ValidateNoCompletion("var x, y = 0, zz|z;");

            ValidateHasCompletions("var x =|");
            ValidateHasCompletions("var x = 1 +|");
            ValidateHasCompletions("var x, y =| 0, z;");
            ValidateHasCompletions("var x, y = x|, z;");
            ValidateHasCompletions("var x, y = 0, z;|");

            ValidateHasCompletions("var x |");
            ValidateHasCompletions("var x; |");
        }

        [TestMethod]
        [WorkItem(83841)]
        public void FunctionDeclarion()
        {
            ValidateHasCompletions("var function_var; function|", "function_var");

            ValidateNoCompletion("function |");
            ValidateNoCompletion("function foo|");
            ValidateNoCompletion("function fo|o");
            ValidateNoCompletion("function foo |");
            ValidateNoCompletion("function foo (|");
            ValidateNoCompletion("function foo (a|");
            ValidateNoCompletion("function foo (a|b");
            ValidateNoCompletion("function foo (a, |b");
            ValidateNoCompletion("function foo (a, b|)");
            ValidateNoCompletion("function foo a, b|)");
            ValidateNoCompletion("function foo (a, b| {");

            ValidateHasCompletions("function foo (a, b) {|}");
            ValidateHasCompletions("function foo (a, b) { } |");
        }

        [TestMethod]
        public void ObjectLiterals()
        {
            // members
            ValidateNoCompletion(" x = {|}");
            ValidateNoCompletion(" x = { a| }");
            ValidateNoCompletion(" x = { a|:1 }");
            ValidateNoCompletion(" x = { a : 1,| }");
            ValidateNoCompletion(" x = { a : 1, b| }");
            ValidateNoCompletion(" x = { a : 1, b|b }");
            ValidateNoCompletion(" x = { a : 1, b  /*comment*/| }");

            ValidateHasCompletions(" x =| { a : 1 }");
            ValidateHasCompletions(" x = { a :| 1 }");
            ValidateHasCompletions(" x = { a : 1, b : | ");
            ValidateHasCompletions(" x = { a : 1, b : a| ");
            ValidateHasCompletions(" x = { a : 1, b : a|a ");
            ValidateHasCompletions(" x = { a : 1 } |");

            // Error skipping
            ValidateNoCompletion(" x = { a , | ");
            ValidateHasCompletions(" x = { a , b : | ");

            // getters and setters
            ValidateNoCompletion(" x = { ge|t  }");
            ValidateNoCompletion(" x = { se|t  }");
            ValidateNoCompletion(" x = { get| a  }");
            ValidateNoCompletion(" x = { set| a  }");
            ValidateNoCompletion(" x = { get |a  }");
            ValidateNoCompletion(" x = { set |a  }");
            ValidateNoCompletion(" x = { get a|  }");
            ValidateNoCompletion(" x = { set a|  }");
            ValidateNoCompletion(" x = { get a|b  }");
            ValidateNoCompletion(" x = { set a|b  }");
            ValidateNoCompletion(" x = { get a (|)  }");
            ValidateNoCompletion(" x = { set a (|)  }");
            ValidateNoCompletion(" x = { get a (b|b)  }");
            ValidateNoCompletion(" x = { set a (b|b)  }");
            ValidateNoCompletion(" x = { get a () { } |  }");
            ValidateNoCompletion(" x = { set a () { } |  }");


            ValidateHasCompletions(" x = { get a () {| ");
            ValidateHasCompletions(" x = { set a (value) {| ");
        }

        [TestMethod]
        [WorkItem(83841)]
        public void CatchVariable()
        {
            ValidateHasCompletions("var catch_var; try { } catch|", "catch_var");

            ValidateNoCompletion("try { } catch (| ");
            ValidateNoCompletion("try { } catch (e| ");
            ValidateNoCompletion("try { } catch (e|rr ");
            ValidateNoCompletion("try { } catch (err|) ");

            ValidateHasCompletions("try { } catch (err) {|} ", "err");
            ValidateHasCompletions("try { } catch (err) {} |");
        }

        [TestMethod]
        public void StringLiterals()
        {
            ValidateNoCompletion("'str|ing';");
            ValidateNoCompletion("'|string';");
            ValidateNoCompletion("var x = 'str|");

            // unicode sequances
            ValidateNoCompletion(@"'\u|");
            ValidateNoCompletion(@"'\up|");
            ValidateNoCompletion(@"'\u12|");
            ValidateNoCompletion(@"'\u1245|");

            // hexadecimal sequances
            ValidateNoCompletion(@"'\x|");
            ValidateNoCompletion(@"'\xp|");
            ValidateNoCompletion(@"'\x12|");
            ValidateNoCompletion(@"'\x1245|");

            // invalid escape sequances
            ValidateNoCompletion(@"'\|");
        }

        [TestMethod]
        public void IntegerLiterals()
        {
            ValidateNoCompletion("x = 2|2;");

            ValidateHasCompletions("(1).|", CompletionsBase.NumberMethods);
        }

        [TestMethod]
        [WorkItem(234715)]
        public void FloatLiterals()
        {
            ValidateNoCompletion("x = 2|2.09;");
            ValidateNoCompletion("x = 22.0|9;");
            ValidateNoCompletion("x = 22.|09;");

            ValidateNoCompletion("x = 22.|");

            ValidateNoCompletion("x = 2|");
            ValidateNoCompletion("x = 2.|");

            ValidateHasCompletions("(1.5).|", CompletionsBase.NumberMethods);
        }

        [TestMethod]
        [WorkItem(402824)]
        public void RegularExpressionLiterals()
        {
            ValidateNoCompletion("x = /te|st/;");
            ValidateNoCompletion(@"var x =/^\*\/|\s*$/;");
            ValidateNoCompletion("/te|");
        }

        [TestMethod]
        public void Comments()
        {
            // Multiline
            ValidateNoCompletion("/* var x =| */");

            // Single line
            ValidateNoCompletion("// x.| ");
        }

        [TestMethod]
        public void InvalidDot()
        {
            // debugger
            ValidateNoCompletion("debugger.|");

            // delete
            ValidateNoCompletion("delete.|");

            // break / continue
            ValidateNoCompletion("while(false) { break.| }");
            ValidateNoCompletion("while(false) { continue.| }");

            // if
            ValidateNoCompletion("var x; if.|");
            ValidateNoCompletion("var x; if (1) else.|");
            ValidateNoCompletion("var x; if.| (1) {}");
            ValidateNoCompletion("var x; if.|(1) {}");
            ValidateNoCompletion("var x; if (1).|");
            ValidateNoCompletion("var x; if (1) {}.|");
            ValidateNoCompletion("var x; if (1) {} else {}.|");
            ValidateNoCompletion("function f() { if.| }");
            ValidateNoCompletion("function f() { if (1) {} else.| }");
            ValidateNoCompletion("function f() { if(true).| }");
            ValidateNoCompletion("function f() { if().| }");
            ValidateNoCompletion("function f() { if(0) {}.| }");
            ValidateNoCompletion("function f() { if(0) {} else {}.| }");

            // instanceof
            ValidateNoCompletion("instanceof.|");

            // function
            ValidateNoCompletion("function.|");
            ValidateNoCompletion("function foo.|");
            ValidateNoCompletion("function().|");
            ValidateNoCompletion("function foo(){}.|");

            // for
            ValidateNoCompletion("var x; for.|");
            ValidateNoCompletion("var x; for.| (var i = 0; i < 10; i++ ) {}");
            ValidateNoCompletion("var x; for.|(var i = 0; i < 10; i++ ) {}");
            ValidateNoCompletion("var x; for (var i = 0; i < 10; i++ ).|");
            ValidateNoCompletion("var x; for (var i = 0; i < 10; i++ ) {}.|");
            ValidateNoCompletion("function f() { for.| }");
            ValidateNoCompletion("function f() { for.| (var i = 0; i < 10; i++); }");
            ValidateNoCompletion("function f() { for().| }");
            ValidateNoCompletion("function f() { x=1; for(var i = 0; i < 10; i++) { }.| }");
            ValidateNoCompletion("var x; for.| (var i in.|");

            //new 
            ValidateNoCompletion("new.|");
            ValidateNoCompletion("new().|");

            // switch
            ValidateNoCompletion("var x; switch.|");
            ValidateNoCompletion("var x; switch.| (x) {}");
            ValidateNoCompletion("var x; switch.|(x) {}");
            ValidateNoCompletion("var x; switch (x).|");
            ValidateNoCompletion("var x; switch (x) {}.|");
            ValidateNoCompletion("switch(a) { case.| } }");
            ValidateNoCompletion("switch(a) { default.| } }");
            ValidateNoCompletion("function f() { switch.| }");
            ValidateNoCompletion("function f() { switch().| }");
            ValidateNoCompletion("function f() { var x; switch(x).| }");
            ValidateNoCompletion("function f() { var x; switch(x) {}.| }");
            ValidateNoCompletion("function f() { switch(a) { case.| } }");
            ValidateNoCompletion("function f() { switch(a) { case 1:.| } }");

            // try/catch/finally
            ValidateNoCompletion("var x; try.|");
            ValidateNoCompletion("var x; try {}.|");
            ValidateNoCompletion("var x; try {} catch.|");
            ValidateNoCompletion("var x; try {} catch().|");
            ValidateNoCompletion("var x; try {} catch(e).|");
            ValidateNoCompletion("var x; try {} catch(e) {}.|");
            ValidateNoCompletion("var x; try {} catch(e) {} finally.|");
            ValidateNoCompletion("var x; try {} catch(e) {} finally {}.|");
            ValidateNoCompletion("var x; try {} catch(e) {} finally().|");

            //typeof
            ValidateNoCompletion("typeof.|");

            // while
            ValidateNoCompletion("var x; while.|");
            ValidateNoCompletion("var x; while.| (false) {}");
            ValidateNoCompletion("var x; while.|(false) {}");
            ValidateNoCompletion("var x; while (false).|");
            ValidateNoCompletion("var x; while (false) {}.|");
            ValidateNoCompletion("function f() { while.| }");
            ValidateNoCompletion("function f() { while().| }");
            ValidateNoCompletion("function f() { while(false) {}.| }");

            // do while
            ValidateNoCompletion("var x; do.|");
            ValidateNoCompletion("var x; do.| while (false)");
            ValidateNoCompletion("var x; do.|{} while (false)");
            ValidateNoCompletion("var x; do {}.|");
            ValidateNoCompletion("var x; do {}.| while (false)");
            ValidateNoCompletion("var x; do {} while.|");
            ValidateNoCompletion("var x; do {} while.| (false);");
            ValidateNoCompletion("var x; do {} while.|(false);");
            ValidateNoCompletion("var x; do {} while(false).|");
            ValidateNoCompletion("function f() { do.| }");
            ValidateNoCompletion("function f() { do {} while().| }");
            ValidateNoCompletion("function f() { do {}.| while(false) }");

            // with
            ValidateNoCompletion("var a={}; with.|");
            ValidateNoCompletion("var a={}; with.| (a) {}");
            ValidateNoCompletion("var a={}; with.|(a) {}");
            ValidateNoCompletion("var a={}; with(a).|");
            ValidateNoCompletion("var a={}; with(a){}.|");
            ValidateNoCompletion("function f() { var a={}; with.| }");
            ValidateNoCompletion("function f() { var a={}; with().| }");
            ValidateNoCompletion("function f() { var a={}; with(a){}.| }");

            // return
            ValidateNoCompletion("function f() { return.| }");

            // var
            ValidateNoCompletion("var.|");

            // Operator dot
            ValidateNoCompletion("=.|");
            ValidateNoCompletion("+.|");
            ValidateNoCompletion("--.|");
            ValidateNoCompletion("===.|");

            // Punctuation dot
            ValidateNoCompletion(",.|");
            ValidateNoCompletion(";.|");
            ValidateNoCompletion("[.|");
            ValidateNoCompletion(").|");
            ValidateNoCompletion("{.|");

            // just dot
            ValidateNoCompletion(".|");

            // keywords are legal as field names
            ValidateHasCompletions("var x = { var: 1, if: 2, while: 'while' }; x.|", "var", "if", "while");

            // Multiple dots are leagal
            ValidateHasCompletions("x..|");
            ValidateHasCompletions("x...|");

        }

        #region Helpers
        protected override ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();
            for (int i = 0; i < text.Length; i++)
            {
                var ch = text[i];

                if (ch == '|' && text[i - 1] != '|')
                {
                    string data = null;
                    var j = i + 1;
                    if (j < text.Length)
                    {
                        while (true)
                        {
                            if (j == text.Length)
                                break;
                            ch = text[j++];
                            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ',' || ch == '$' || ch == '_')
                                continue;
                            if (ch == '|')
                            {
                                data = text.Substring(i + 1, j - i - 2);
                                i = j - 1;
                            }
                            break;
                        }
                    }
                    requests.Add(new Request() { Offset = builder.Length, Data = data });
                }
                else
                    builder.Append(ch);
            }
            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }
        #endregion
    }
}