using System;
using System.Linq;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class DomTraversalTests : CompletionsBase
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
        [WorkItem(294095)]
        [WorkItem(346312)]
        public void InfiniteLoops()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            Action<string, string> TestForInfiniteLoops = (string input, string name) =>
            {
                System.Diagnostics.Debug.WriteLine("");
                System.Diagnostics.Debug.WriteLine(string.Format("Beginning test: '{0}' ", name));
                PerformCompletionRequests(input + " ;'s'.|String|;", dom);
                System.Diagnostics.Debug.WriteLine(string.Format("Done with test: '{0}' ", name));
            };

            // removeChild
            TestForInfiniteLoops(@"
                while (document.firstChild) {
                    document.removeChild(document.firstChild);
                }
            ", "loop on first child");

            TestForInfiniteLoops(@"
                while (document.lastChild) {
                    document.removeChild(document.lastChild);
                }
            ", "loop on last child");

            TestForInfiniteLoops(@"
                while (document.firstElementChild) {
                    document.removeChild(document.firstElementChild);
                }
            ", "loop on firstElementChild");

            TestForInfiniteLoops(@"
                while (document.lastElementChild) {
                    document.removeChild(document.lastElementChild);
                }
            ", "loop on lastElementChild");

            // document
            TestForInfiniteLoops(@"
                var node = document;

                var cur = node;
                while (cur) cur = cur.nextSibling;

                cur = node;
                while (cur) cur = cur.previousSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.nextElementSibling;

                cur = node;
                while (cur) cur = cur.previousElementSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.parentElement;

            ", "loops for document");

            // document.body 
            TestForInfiniteLoops(@"
                var node = document.body;

                var cur = node;
                while (cur) cur = cur.nextSibling;

                cur = node;
                while (cur) cur = cur.previousSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.nextElementSibling;

                cur = node;
                while (cur) cur = cur.previousElementSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.parentElement;

            ", "loops for document.body");

            // create element 
            TestForInfiniteLoops(@"
                var node = document.createElement('div');

                var cur = node;
                while (cur) cur = cur.nextSibling;

                cur = node;
                while (cur) cur = cur.previousSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.nextElementSibling;

                cur = node;
                while (cur) cur = cur.previousElementSibling;

                cur = node;
                while (cur) cur = cur.parentNode;

                cur = node;
                while (cur) cur = cur.parentElement;

            ", "loops for createElement");

            TestForInfiniteLoops(@"
                var div = document.createElement('div');
                div.appendChild(document.createElement('p'));

                var children = div.childNodes;

                while(children.length > 0)
                    div.removeChild(children[0]);
            ", "loops on childNodes");

            TestForInfiniteLoops(@"
                var div = document.createElement('div');
                div.appendChild(document.createElement('p'));

                var children = div.getElementsByTagName('p');

                while(children.length > 0)
                    div.removeChild(children[0]);
            ", "loops on query results");
        }

        [TestMethod]
        [WorkItem(294095)]
        public void DocumentStructure()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            // document
            VerifyType("document.body", "HTMLBodyElement");
            VerifyType("document.head", "HTMLHeadElement");
            VerifyIsTrue("document.parentNode == null");
            VerifyIsTrue("document.ownerDocument == null");
            VerifyIsTrue("document.nodeType == Node.DOCUMENT_NODE");

            // document.documentElement
            VerifyType("document.firstChild", "HTMLHtmlElement");
            VerifyType("document.lastChild", "HTMLHtmlElement");
            VerifyType("document.documentElement", "HTMLHtmlElement");
            VerifyIsTrue("document.lastChild === document.firstChild");
            VerifyIsTrue("document.documentElement === document.lastChild");
            VerifyIsTrue("document.documentElement.parentNode === document");
            VerifyIsTrue("document.documentElement.parentElement == null");
            VerifyIsTrue("document.documentElement.ownerDocument === document");
            VerifyIsTrue("document.documentElement.childNodes.length === 2");
            VerifyIsTrue("document.documentElement.childNodes[0] === document.head");
            VerifyIsTrue("document.documentElement.childNodes[1] === document.body");

            // head
            VerifyType("document.documentElement.firstChild", "HTMLHeadElement");
            VerifyIsTrue("document.head === document.documentElement.firstChild");
            VerifyIsTrue("document.head.nextSibling === document.body");
            VerifyIsTrue("document.head.nextElementSibling === document.body");
            VerifyIsTrue("document.head.parentNode === document.documentElement");
            VerifyIsTrue("document.head.parentElement === document.documentElement");
            VerifyIsTrue("document.head.ownerDocument === document");

            // title
            VerifyType("document.head.firstChild", "HTMLTitleElement");
            VerifyType("document.head.lastChild", "HTMLScriptElement");
            VerifyIsTrue("document.head.childNodes.length === 2");

            // body
            VerifyType("document.documentElement.lastChild", "HTMLBodyElement");
            VerifyIsTrue("document.body === document.documentElement.lastChild");
            VerifyIsTrue("document.body.previousSibling === document.head");
            VerifyIsTrue("document.body.previousElementSibling === document.head");
            VerifyIsTrue("document.body.parentNode === document.documentElement");
            VerifyIsTrue("document.body.parentElement === document.documentElement");
            VerifyIsTrue("document.body.ownerDocument === document");

            // CreateElement
            VerifyType("document.createElement('p')", "HTMLParagraphElement");
            VerifyIsTrue("document.createElement('p').parentNode === document.body");
            VerifyIsTrue("document.createElement('p').parentElement === document.body");
            VerifyIsTrue("document.createElement('p').previousSibling == null");
            VerifyIsTrue("document.createElement('p').nextSibling == null");
            VerifyIsTrue("document.createElement('p').previousElementSibling == null");
            VerifyIsTrue("document.createElement('p').nextElementSibling == null");
            VerifyIsTrue("document.createElement('p').ownerDocument === document");
        }

        [TestMethod]
        public void InfiniteLoopOnGetElementsByTagName()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var jquery = Bug271685TestFiles._07_jQueryv1_3_2;

            PerformCompletionRequests(@"$('<div/>').appendTo(document.body); 's'.|String|;", dom, jquery);
        }

        [TestMethod]
        public void InsersionValidation()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(@"
                var element = document.createElement('p');
                element.appendChild(element);

                var x = (element.parentNode != element) ? 'string' : 0; x.|String|;", dom);

            PerformCompletionRequests(@"
                var origianlCount = document.body.children.length;
                document.body.appendChild(document.documentElement);
                var x = (document.body.children.length == origianlCount) ? 'string' : 0; x.|String|;", dom);
        }

        [TestMethod]
        public void GetElementsByTagName()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            var primary = @"
                // searchable nodes
                var div = document.createElement('div');
                div.appendChild(document.createElement('p'));
                div.appendChild(document.createElement('p'));
                div.appendChild(document.createElement('a'));
                div.appendChild(document.createElement('p'));

                var inner = document.createElement('div');
                inner.appendChild(document.createElement('p'));
                var outter = document.createElement('div');
                outter.appendChild(document.createElement('p'));
                outter.appendChild(inner);

                // unsearchable nodes
                var unsearchable = document.createElement('div');
                unsearchable.innerHTML = '<a></a>';";

            VerifyIsTrue("div.getElementsByTagName('p').length === 3 ", dom, primary);
            VerifyIsTrue("div.getElementsByTagName('a').length === 1 ", dom, primary);
            VerifyIsTrue("div.getElementsByTagName('img').length === 0 ", dom, primary);
            VerifyIsTrue("div.getElementsByTagName('*').length === 4 ", dom, primary);

            VerifyIsTrue("inner.getElementsByTagName('p').length === 1 ", dom, primary);
            VerifyIsTrue("outter.getElementsByTagName('p').length === 2 ", dom, primary);

            VerifyIsTrue("unsearchable.getElementsByTagName('br').length === 1 ", dom, primary);

            // check for tracking null for empty searches
            VerifyIsTrue("div.getElementsByTagName('span')[23].tagName === 'SPAN' ", dom, primary);

            // maintaining state
            primary = @"
                var div = document.createElement('div');
                div.appendChild(document.createElement('p'));
                div.getElementsByTagName('p');";
            VerifyIsTrue("div.getElementsByTagName('p').length === 1", dom, primary);
        }

        [TestMethod]
        [WorkItem(363838)]
        public void QuerySelectors()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var generatedElements = @"
                (function(window){
                    // ----- GENERATED ELEMENTS FROM REFERENCED HTML DOCUMENTS ------
                    window.document._$documentElements = {
                        ""myCanvas"" : window.document.createElement(""canvas""),
                        ""myDiv"" : window.document.createElement(""div""),
                        ""myParagraph"" : window.document.createElement(""p"")
                    };
                    window.myCanvas = window.document._$documentElements[""myCanvas""];
                    window.myDiv = window.document._$documentElements[""myDiv""];
                    window.myParagraph = window.document._$documentElements[""myParagraph""];

                })(window);";

            // querySelectorAll
            VerifyIsTrue(@"document.querySelectorAll('#myCanvas')[0] === window.myCanvas ", dom, generatedElements);
            VerifyIsTrue(@"document.querySelectorAll('#myCanvas , #myDiv, #myParagraph')[0] === window.myCanvas ", dom, generatedElements);
            VerifyIsTrue(@"document.querySelectorAll('#myCanvas,   #myDiv , #myParagraph')[1] === window.myDiv ", dom, generatedElements);
            VerifyIsTrue(@"document.querySelectorAll('#myCanvas, #myDiv , #myParagraph')[2] === window.myParagraph ", dom, generatedElements);
            VerifyIsTrue(@"document.querySelectorAll('#notefound, #myDiv , #myCanvas input[type=""radio""]:checked')[0] === window.myDiv ", dom, generatedElements);

            // querySelector
            VerifyIsTrue(@"document.querySelector('#myCanvas') === window.myCanvas ", dom, generatedElements);
            VerifyIsTrue(@"document.querySelector('    class, class2, #myDiv') === window.myDiv ", dom, generatedElements);
        }

        [TestMethod]
        [WorkItem(346312)]
        public void GetElementsByTagNameResult()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);

            var primary = _session.FileFromText(@"
                intellisense.addEventListener('statementcompletion',  function (e) {});

                // searchable nodes
                var div = document.createElement('div');
                var p1 = document.createElement('p');
                var p2 = document.createElement('p');
                var p3 = document.createElement('p');
                var p11 = document.createElement('p');
                var p12 = document.createElement('p');

                div.appendChild(p1);
                div.appendChild(p2);

                p1.appendChild(p11);
                p1.appendChild(p12);

                var result = div.getElementsByTagName('p');

                // Check 1: expect result to contain p1, p2, p11, p12
                intellisense.logMessage(' Check 1: result.length = ' + result.length);

                div.appendChild(p3);

                // Check 2: expect result to contain p1, p2, p11, p12, and p3
                intellisense.logMessage(' Check 2: result.length = ' + result.length);

                div.removeChild(p2);

                // Check 3: expect result to contain p1, p11, p12, and p3
                intellisense.logMessage(' Check 3: result.length = ' + result.length);

                div.removeChild(p1);

                // Check 4: expect result to contain only p3
                intellisense.logMessage(' Check 4: result.length = ' + result.length);

                div.removeChild(p3);

                // Check 5: expect result to be empty
                intellisense.logMessage(' Check 5: result.length = ' + result.length);


                div.appendChild(p1);

                // Check 6: expect result to contain p1, p11, p12
                intellisense.logMessage(' Check 6: result.length = ' + result.length);");

            var context = _session.OpenContext(primary, dom);
            var completions = context.GetCompletionsAt(primary.Text.Length);
            Assert.IsNotNull(completions);

            EnsureMessageLogged(context, " Check 1: result.length = 4");
            EnsureMessageLogged(context, " Check 2: result.length = 5");
            EnsureMessageLogged(context, " Check 3: result.length = 4");
            EnsureMessageLogged(context, " Check 4: result.length = 1");
            EnsureMessageLogged(context, " Check 5: result.length = 0");
            EnsureMessageLogged(context, " Check 6: result.length = 3");
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                {
                                    $tag: 'script',
                                    src: 'http:\\www.src.com\somefile.js',
                                    'data-main': 'data for script tag'
                                }
                            ]
                        },
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'form',
                                    method:'get',
                                    name:'searchform',
                                    action:'http://www.msn.com/search',
                                    target:'_blank',
                                    id: 'form1',
                                    $children: [
                                            {
                                                $tag: 'input',
                                                $formElement: true,
                                                id: 'input1',
                                                name:'sitesearch',
                                                value:'www.w3schools.com',
                                                type:'hidden'
                                            },
                                            {
                                                $tag: 'input',
                                                $formElement: true,
                                                id: 'input2',
                                                style:'margin: 0px;',
                                                title:'Search',
                                                value:'Search',
                                                type:'submit'
                                            }
                                    ]
                                },
                                {
                                    $tag: 'div',
                                    class: 'divClass',
                                    style: 'display: none;'
                                }
                            ]
                        }
                    ]
                );";


            VerifyIsTrue("document.head.getElementsByTagName('script')[0].tagName === 'SCRIPT'", dom.Text, domStructure);
            VerifyIsTrue("document.head.getElementsByTagName('script')[0]['data-main'] === 'data for script tag'", dom.Text, domStructure);
            VerifyIsTrue("document.body.getElementsByTagName('form')[0].target === '_blank'", dom.Text, domStructure);
            VerifyIsTrue("document.body.getElementsByTagName('form')[0].firstChild.value === 'www.w3schools.com'", dom.Text, domStructure);

            // ensure the form was only added to the body and the script was only added to the head
            VerifyIsTrue("document.head.getElementsByTagName('form').length === 0", dom.Text, domStructure);
            VerifyIsTrue("document.body.getElementsByTagName('script').length === 0", dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure_ParentId()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                {
                                    $tag: 'script',
                                    id: 'script1'
                                }
                            ]
                        },
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'form',
                                    id: 'form1',
                                    $children: [
                                        {
                                            $tag: 'input',
                                            id: 'input11',
                                            $formElement: true
                                        },
                                        {
                                            $tag: 'input',
                                            id: 'input12',
                                            $formElement: true
                                        },
                                        {
                                            $tag: 'div',
                                            id: 'div1',
                                            $children: [
                                                {
                                                    $tag: 'input',          // form element nested under a non form element parent
                                                    id: 'input13',
                                                    $formElement: true
                                                }
                                            ]
                                        }
                                    ]
                                },
                                {
                                    $tag: 'div',
                                    id: 'div2'
                                },
                                {
                                    $tag: 'form',
                                    $children: [
                                        {
                                            $tag: 'div',
                                            id: 'div21',
                                            $children: [
                                                {
                                                    $tag: 'input',          // form parent does not have an id
                                                    id: 'input21',
                                                    $formElement: true
                                                }
                                            ]
                                        }
                                    ]
                                },
                                {
                                    $tag: 'input',          // form element with no form parent
                                    id: 'input3',
                                    $formElement: true
                                }
                            ]
                        }
                    ]
                );";

            // Verify that elements with id's are exposed on the expected parent element
            VerifyType("window.script1", "HTMLScriptElement", dom.Text, domStructure);
            VerifyType("window.form1", "HTMLFormElement", dom.Text, domStructure);
            VerifyType("window.form1.input11", "HTMLInputElement", dom.Text, domStructure);
            VerifyType("window.form1.input12", "HTMLInputElement", dom.Text, domStructure);
            VerifyType("window.form1.input13", "HTMLInputElement", dom.Text, domStructure);
            VerifyType("window.div1", "HTMLDivElement", dom.Text, domStructure);
            VerifyType("window.div2", "HTMLDivElement", dom.Text, domStructure);
            VerifyType("window.input3", "HTMLInputElement", dom.Text, domStructure);

            // Verify elements were not defined on the wrong parent
            VerifyIsTrue("typeof window.form1.div1 == 'undefined'", dom.Text, domStructure);
            VerifyIsTrue("typeof window.div1.input13 == 'undefined'", dom.Text, domStructure);
            VerifyIsTrue("typeof window.input11 == 'undefined'", dom.Text, domStructure);

            // Parent form does not have an id
            VerifyType("document.body.getElementsByTagName('form')[1].input21", "HTMLInputElement", dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure_GetElementById()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                {
                                    $tag: 'script',
                                    id: 'script_id'
                                }
                            ]
                        },
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'form',
                                    id: 'form_id',
                                    $children: [
                                            {
                                                $tag: 'input',
                                                $formElement: true,
                                                id: 'input_id1'
                                            },
                                            {
                                                $tag: 'input',
                                                $formElement: true,
                                                id: 'input_id2'
                                            },
                                            {
                                                $tag: 'div',
                                            }
                                    ]
                                },
                                {
                                    $tag: 'div',
                                    id: 'div_id'
                                }
                            ]
                        }
                    ]
                );";


            VerifyType("document.getElementById('script_id')", "HTMLScriptElement", dom.Text, domStructure);
            VerifyType("document.getElementById('form_id')", "HTMLFormElement", dom.Text, domStructure);
            VerifyType("document.getElementById('input_id1')", "HTMLInputElement", dom.Text, domStructure);
            VerifyType("document.getElementById('input_id2')", "HTMLInputElement", dom.Text, domStructure);
            VerifyType("document.getElementById('div_id')", "HTMLDivElement", dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure_Scripts()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'head',
                            $children: [
                                {
                                    $tag: 'script',
                                    src: 'somefolder/somefile1.js'
                                },
                                {
                                    $tag: 'script',
                                    type: 'text/javascript',
                                    src: 'somefile2.js'
                                }
                            ]
                        }
                    ]
                );";

            VerifyIsTrue("document.head.getElementsByTagName('script').length === 2", dom.Text, domStructure);
            VerifyIsTrue("document.body.getElementsByTagName('script').length === 0", dom.Text, domStructure);
            VerifyIsTrue("document.getElementsByTagName('script').length === 2", dom.Text, domStructure);
            VerifyIsTrue("document.getElementsByTagName('script')[0].src === 'somefolder/somefile1.js'", dom.Text, domStructure);
            VerifyIsTrue("document.getElementsByTagName('script')[1].src === 'somefile2.js'", dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure_Object()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                document._$recordDomStructure(
                    [
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'form',
                                    id: form1
                                },
                                {
                                    $tag: '__object',
                                    id: 'DISystemMonitor1',
                                    $object: {
                                        ""QueryInterface"": function(riid, ppvObj) {},
                                        ""AddRef"": function() {},
                                        ""Release"": function() {},
                                        ""CollectSample"": function() {},
                                        ""UpdateGraph"": function() {},
                                        ""BrowseCounters"": function() {},
                                        ""DisplayProperties"": function() {},
                                        ""Counter"": function(iIndex, ppICounter) {
                                                        ///<summary>function summary</summary>
                                                        ///<param name='iIndex' type='number'>iIndex description</param>
                                                        ///<param name='ppICounter' type='number'>ppICounter description</param>
                                                    },
                                        ""AddCounter"": function(bsPath, ppICounter) {},
                                        ""DeleteCounter"": function(pCtr) {},
                                        ""Paste"": function() {},
                                        ""Copy"": function() {},
                                        ""Reset"": function() {},
                                        ""ScaleToFit"": function(bSelectedCountersOnly) {},
                                        ""SaveAs"": function(bstrFileName, eSysmonFileType) {},
                                        ""Relog"": function(bstrFileName, eSysmonFileType, iFilter) {},
                                        ""ClearData"": function() {},
                                        ""SetLogViewRange"": function(StartTime, StopTime) {},
                                        ""GetLogViewRange"": function(StartTime, StopTime) {},
                                        ""BatchingLock"": function(fLock, eBatchReason) {},
                                        ""LoadSettings"": function(bstrSettingFileName) {},
                                        get Appearance() { return new Object(); }, 
                                        set Appearance(value) {},
                                        get BackColor () { return new Object(); },
                                        set BackColor(value) {}
                                    }
                                }
                            ]
                        }
                    ]
                );";

            VerifyIsTrue("document.body.getElementsByTagName('__object').length === 1", dom.Text, domStructure);
            VerifyIsTrue("typeof document.body.getElementsByTagName('__object')[0].QueryInterface === 'function'", dom.Text, domStructure);

            // Verify function help on new properties
            PerformRequests(@"document.body.getElementsByTagName('__object')[0].Counter(|);",
                (context, offset, data, index) =>
                {
                    var help = context.GetParameterHelpAt(offset);
                    Assert.AreEqual("Counter", help.FunctionHelp.FunctionName);
                    Assert.AreEqual("function summary", help.FunctionHelp.GetSignatures().ToEnumerable().First().Description);
                }, dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(367102)]
        public void RecordDomStructure_HideMetaData()
        {
            var dom = _session.ReadFile(Paths.DomWebPath);
            var domStructure = @"
                 document._$recordDomStructure(
                    [
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'form',
                                    id: 'form1',
                                    $children: [
                                        {
                                            $tag: 'input',
                                            id: 'input1',
                                            $formElement: true
                                        },
                                        {
                                            $tag: 'input',
                                            id: 'input2',
                                            $formElement: true,
                                            $object: {
                                                $tag: 'object',
                                                $children:: 'object',
                                                $formElement: 'object',
                                                $object: 'object'
                                            }
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                );";


            PerformCompletionRequests(@"
                    // meta data properties should not show on elements
                    window.form1.|element|;
                    window.form1.input1.|element|;

                    // properties defined though $object should not be altered
                    window.form1.input2.|metaData|;",
                (completions, data, index) =>
                {
                    switch (data)
                    {
                        case "element":
                            completions.ExpectContains("tagName", "id");
                            completions.ExpectNotContains("$tag", "$children", "$object", "$formElement");
                            break;
                        case "metaData":
                            completions.ExpectContains("tagName", "id", "$tag", "$children", "$object", "$formElement");
                            break;
                        default:
                            Assert.Fail("Unknown value : " + data);
                            break;
                    }
                },
                AuthorCompletionFlags.acfMembersFilter, dom.Text, domStructure);
        }

        [TestMethod]
        [WorkItem(436209)]
        public void ErrorObjectHasStackProperty()
        {
            PerformCompletionRequests(@"
                    var err = new Error();
                    err.|stack|;
                    try {
                        throw err;
                    } catch (e) {
                        e.|stack|;
                    }
                });");
        }

        [TestMethod]
        [WorkItem(515673)]
        public void VerifyPROGNOZ()
        {
            PerformCompletionRequests("|NaN|", TestFiles._515673_PP);
        }

        [TestMethod]
        [WorkItem(723224)]
        public void RecordDomStructure_WinControl()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath);
            var domStructure = @"
                 document._$recordDomStructure(
                    [
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'div',
                                    id: 'ratingControlHost',
                                    'data-win-control': 'WinJS.UI.Rating'
                                },
                                {
                                    $tag: 'div',
                                    id: 'missingControlHost',
                                    'data-win-control': 'Missing.WinControl.Type'
                                }
                            ]
                        }
                    ]
                );";

            PerformCompletionRequests(@"
                    var hostElement = document.getElementById('ratingControlHost');
                    hostElement.|winControl|;

                    var ratingControl = hostElement.winControl;
                    ratingControl.|averageRating,disabled,element,enableClear,maxRating,tooltipStrings,userRating|;
                    
                    var missingControlHost = document.getElementById('missingControlHost');
                    missingControlHost.|winControl|;
                    missingControlHost.winControl.|!Object|;",
                dom.Text, WinJSTestFiles.latest_base, WinJSTestFiles.latest_ui, domStructure);
        }

        [TestMethod]
        [WorkItem(723224)]
        public void RecordDomStructrure_WinControl_AddedToElementByConstructor()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath);
            var domStructure = @"
                 document._$recordDomStructure(
                    [
                        {
                            $tag: 'body',
                            $children: [
                                {
                                    $tag: 'div',
                                    id: 'ratingControlHost'
                                }
                            ]
                        }
                    ]
                );";

            PerformCompletionRequests(@"
                    var hostElement = document.getElementById('ratingControlHost');
                    hostElement.|!winControl|;

                    var ratingControl = new WinJS.UI.Rating(hostElement);
                    ratingControl.|averageRating,disabled,element,enableClear,maxRating,tooltipStrings,userRating|;

                    hostElement.|winControl|;
                    var controlFromHostElement = hostElement.winControl;
                    controlFromHostElement.|averageRating,disabled,element,enableClear,maxRating,tooltipStrings,userRating|;",
                dom.Text, WinJSTestFiles.latest_base, WinJSTestFiles.latest_ui, domStructure);
        }

        #region Helper Methods
        private void VerifyIsTrue(string condition)
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            VerifyIsTrue(condition, dom);
        }

        private void VerifyIsTrue(string condition, params string[] contextFiles)
        {
            PerformRequests(String.Format(@"var x = {0} ? 'string' : 0; x.|", condition),
                 (context, offset, data, index) =>
                 {
                     var completion = context.GetCompletionsAt(offset);
                     Assert.IsNotNull(completion);
                     if (!completion.ToEnumerable().Contains(CompletionsBase.StringMethods))
                     {
                         Assert.Fail(String.Format("Condition '{0}' evaluated to false.", condition));
                     }
                 }, contextFiles);
        }

        private void VerifyType(string obj, string type)
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            VerifyType(obj, type, dom);
        }

        private void VerifyType(string obj, string type, params string[] contextFiles)
        {
            VerifyIsTrue(String.Format("window.{1}.prototype.isPrototypeOf({0})", obj, type), contextFiles);
        }
        #endregion
    }
}
