using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class ReferenceTests : DirectAuthorTest
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
        public void References_Simple()
        {
            TestReferences(@"
                var |global:b|b;
                var |global:c|c;
                var |global:f|f = { |member:a|a: 1, |member:b|b: 2 };
                function |function:a|a(|param:b|b, |param:c|c) {
                    var |local:e|e = |global:f|f.|member:a|a + |global:f|f.|member:b|b;
                    return |param:b|b + |param:c|c;
                }
                var |global:d|d = |global:b|b + |global:c|c;
                |function:a|a(|global:b|b, |global:c|c);");
        }


        [TestMethod]
        public void References_Catch()
        {
            TestReferences(@"
                var |global:a|a;
                var |global:l|l = { |member:a|a: 1};
                function |global:foo|foo() {
                    var |local:a|a = 1;
                    try {
                      |local:a|a;
                    }
                    catch(|catch:a|a) {
                       |catch:a|a;
                    }
                    finally {
                        |local:a|a;
                    }
                }
                |global:a|a;");
        }

        [TestMethod]
        public void Reference_Captures()
        {
            TestReferences(@"
               var |global:a|a;
               |global:a|a;
               var |global:bg|bg;
               |global:bg|bg;
               (function () {
                   var |local:a1|a;
                   var |local:bl1|bl1;
                   |global:bg|bg;
                   |local:a1|a;
                   |local:bl1|bl1;
                   (function () {
                       var |local:a2|a;
                       var |local:bl2|bl2;
                       |global:bg|bg;
                       |local:bl1|bl1;
                       |local:a2|a;
                       |local:bl2|bl2;
                       (function () {
                           var |local:a3|a;
                           var |local:bl3|bl3;
                           |global:bg|bg;
                           |local:bl1|bl1;
                           |local:bl2|bl2;
                           |local:a3|a;
                           |local:bl3|bl3;
                       })(); 
                       |local:a2|a;
                   })();
                   |local:a1|a;
               })();
               |global:a|a;");
        }

        [TestMethod]
        public void References_AuthoritativeVsSuggestive()
        {
            TestReferenceAuthorities(@"
                var |authoritative|a;
                |suggestive|a;
                function |authoritative|foo(|authoritative|a) {
                  |authoritative|a;
                }
                function |authoritative|goo() {
                  var |authoritative|a;
                  |authoritative|a;
                  with (foo) {
                    |suggestive|a;
                  }
                }
                function |authoritative|zoo() {
                  var |suggestive|a;
                  |suggestive|a;
                  eval('var a');
                }");
        }

        [TestMethod]
        public void References_LVsR()
        {
            TestReferenceValues(@"
                var |lvalue|a;
                function |lvalue|foo(|lvalue|p) {
                    var |lvalue|c = 1;
                    |lvalue|a = |rvalue|a + 1;
                    |lvalue|p = |rvalue|p + 1;
                    |lvalue|a += 1;
                    |lvalue|a -= 1;
                    |lvalue|a *= 1;
                    |lvalue|a /= 1;
                    |lvalue|a |= 1;
                    |lvalue|a &= 1;
                    |lvalue|a <<= 1;
                    |lvalue|a >>= 1;

                    return |rvalue|a + |rvalue|p + |rvalue|c;
                }");
        }

        [TestMethod]
        public void Reference_StringLiteralReferences()
        {
            TestReferences(@"
                var t = {
                  |a|'a': 1,
                  |b|'b': 2,
                };
                t[|a|'a'] = t[|a|'a'];
                t[|b|'b'] = t[|b|'b'];
                ");
        }
        // TODO: Add let and const and block scope function declaration tests once ES6 is enabled.

        [TestMethod]
        public void Reference_InvalidFunctionNotAReference()
        {
            var text = @"function    ";
            var primaryFile = _session.FileFromText(text);
            var context = _session.OpenContext(primaryFile);
            var references = context.GetReferences(3);
            Assert.AreEqual(0, references.Count);
        }

        [TestMethod]
        public void Reference_VariableDeclarationEnd()
        {
            var text = @"var foo = 1";
            var primaryFile = _session.FileFromText(text);
            var context = _session.OpenContext(primaryFile);
            var result = context.GetReferences(primaryFile.OffsetAfter("foo"));
            Assert.AreEqual(result.Count, 1);
        }

        [TestMethod]
        [WorkItem(608629)]
        public void Reference_UnclosedString()
        {
            var text = @"RegExp[""\x2";
            var primaryFile = _session.FileFromText(text);
            var context = _session.OpenContext(primaryFile);
            var offset = text.Length;
            var result = context.GetReferences(offset);
            var r = result.ToEnumerable().FirstOrDefault(f => f.Position + f.Length > offset);
            Assert.IsTrue(r.Length == 0);
        }

        [TestMethod]
        public void Reference_UndeclaredIdentifier()
        {
            TestReferences(@"
                (function () {
                    'use strict';
 
                    var app = |WinJS|WinJS.Application;
                    var activation = Windows.ApplicationModel.Activation;
 
                    app.onactivated = function (args) {
                        if (true) {
                            if (true) {
                            }                                                                                                                                                                                                                                               
                            args.setPromise(|WinJS|WinJS.UI.processAll());
                        }
                    };
                })();
                ");
        }

        [TestMethod]
        public void Reference_ClassField()
        {
            TestReferences(@"
                WinJS.Namespace.define('Robotics.Space', {
                    Robot: WinJS.Class.define(function (name) {
                        this.name = name;
                    },
                        { |modelName|modelName: "" },
                        { harmsHumans: false, obeysOrders: true })
                });
                var myRobot = new Robotics.Space.Robot('Mickey');
 
                myRobot.|modelName|modelName = '4500';
                ");
        }

        [TestMethod]
        public void Reference_ObjectIndex()
        {
            TestReferences(@"
                var counter = {
                    |val|val: 0,
                    increment: function () {
                        this.|val|val += 1;
                    }
                };
 
                counter.|val|val = 0;
                counter[|val|'val']++;
                counter[|val|""val""]++;
                ");
        }

        [TestMethod]
        public void Reference_SpacesLinesSeparations()
        {
            TestReferences(@"
var |counter|counter = {
    |val|val
        : 0,
    increment: function () {
        this
            .
            |val|val += 1;
    },
    field: {another:2}
};
 
|counter|counter
    .
    |val|val
    = 0;
|counter|counter[
    |val|'val\
'
]
    [
    'another'
]++;

                ");
        }

        #region Helpers
        private void TestReferences(string primaryFile)
        {
            // Parse the requests
            var requests = ParseRequests(primaryFile);

            // If any files are in the texts read them off disk
            var context = _session.OpenContext(_session.FileFromText(requests.Text));

            // Group the reference locations by there data. 
            var locations = requests.Requests.GroupBy(r => r.Data).ToDictionary(rs => rs.Key, rs => rs.ToArray());

            // Verify reference are what are expected.
            foreach (var request in requests.Requests)
            {
                var result = context.GetReferences(request.Offset);
                AssertAreStructurallyEqual(locations[request.Data].Select(r => r.Offset).ToArray(), result.ToEnumerable().Select(r => r.Position).ToArray());
            }
        }

        private void TestReferenceFlags(string primaryFile, string dataValue, AuthorSymbolReferenceFlags trueFlag, AuthorSymbolReferenceFlags falseFlag)
        {
            // Parse requests
            var requests = ParseRequests(primaryFile);

            // If any files are in the texts read them off disk
            var context = _session.OpenContext(_session.FileFromText(requests.Text));

            foreach (var request in requests.Requests)
            {
                var trueFlagExpected = request.Data == dataValue;
                var result = context.GetReferences(request.Offset);
                Assert.AreEqual(0, context.GetMessages().Count);
                var flags = result.ToEnumerable().Where(r => r.Position == request.Offset).First().Flags;
                Assert.AreEqual(trueFlagExpected, (flags & trueFlag) != 0);
                Assert.AreEqual(!trueFlagExpected, (flags & falseFlag) != 0);
            }
        }

        private void TestReferenceAuthorities(string primaryFile)
        {
            TestReferenceFlags(primaryFile, "suggestive", AuthorSymbolReferenceFlags.asrfSuggestive, AuthorSymbolReferenceFlags.asrfAuthoritative);
        }

        private void TestReferenceValues(string primaryFile)
        {
            TestReferenceFlags(primaryFile, "lvalue", AuthorSymbolReferenceFlags.asrfLValue, AuthorSymbolReferenceFlags.asrfRValue);
        }

        protected override ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();
            for (int i = 0; i < text.Length; i++)
            {
                var ch = text[i];

                if (ch == '|' && (i < text.Length - 1) && text[i + 1] != '=' && text[i + 1] != '|')
                {
                    string data = null;
                    var j = i + 1;
                    while (j < text.Length)
                    {
                        ch = text[j++];
                        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == ',' || ch == ':')
                            continue;
                        if (ch == '|')
                        {
                            data = text.Substring(i + 1, j - i - 2);
                            j = j - 1;
                        }
                        break;
                    }
                    if (data != null)
                    {
                        requests.Add(new Request() { Offset = builder.Length, Data = data });
                        i = j;
                    }
                    else
                        builder.Append(ch);
                }
                else
                    builder.Append(ch);
            }
            return new ParsedRequests() { Requests = requests.ToArray(), Text = builder.ToString() };
        }
        #endregion
    }
}
