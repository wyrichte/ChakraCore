//----------------------------------------------------------------------------------------------------------------------
// <copyright file="EventArgCompletion.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the EventArgCompletion type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System.Linq;
    using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class EventArgCompletion : CompletionsBase
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
        [WorkItem(182683)]
        public void InvalidCodeInValueAttribute()
        {
            PerformRequests(@"
                function getJson(complete) {
                /// <returns value='complete("")' />   
                }   
                getJson(function (json) {
                     json.|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
             });

            PerformRequests(@"
                function foo(completed, error, progress) {
                    /// <returns value=""error();progress(0);completed(new XMLHttpRequest());""/> 
                    foo.|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
             });
        }

        [TestMethod]
        [WorkItem(196055)]
        public void EventHandlerDeclaredBelowAddEventListener()
        {
            var domjs = _session.ReadFile(Paths.DomWebPath).Text;
            PerformRequests(@"
                document.addEventListener('load', eventHandler);
                function eventHandler(event) {
                    event.|
            ", (context, offset, data, index) =>
             {
                 var completions = context.GetCompletionsAt(offset);
                 completions.ToEnumerable().ExpectContains("bubbles", "currentTarget", "stopPropagation");
             }, domjs);
        }

        [TestMethod]
        [WorkItem(194861)]
        public void NoPrivatesInGlobalScope()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(@";|", (completions, data, i) =>
            {
                completions.Count(c => c.Name.StartsWith("_")).Expect(0);
            }, AuthorCompletionFlags.acfAny, new[] { dom });
        }

        [TestMethod]
        public void createEvent_Web()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(
                @"
                    document.createEvent('Event').|target,type,bubbles,initEvent|;
                    document.createEvent('Events').|target,type,bubbles,initEvent|;
                    document.createEvent('CompositionEvent').|locale,data,initCompositionEvent|;
                    document.createEvent('CustomEvent').|detail,initCustomEvent|;
                    document.createEvent('DragEvent').|dataTransfer,initDragEvent|;
                    document.createEvent('FocusEvent').|relatedTarget,initFocusEvent|;
                    document.createEvent('KeyboardEvent').|location,shiftKey,key,getModifierState,initKeyboardEvent|;
                    document.createEvent('MessageEvent').|source,origin,data,initMessageEvent|;
                    document.createEvent('MouseEvent').|x,y,buttons,initMouseEvent|;
                    document.createEvent('MouseEvents').|x,y,buttons,initMouseEvent|;
                    document.createEvent('MouseWheelEvent').|wheelDelta,initMouseWheelEvent|;
                    document.createEvent('MutationEvent').|initMutationEvent|;
                    document.createEvent('MutationEvents').|initMutationEvent|;
                    document.createEvent('StorageEvent').|initStorageEvent|;
                    document.createEvent('TextEvent').|initTextEvent|;
                    document.createEvent('UIEvent').|initUIEvent|;
                    document.createEvent('UIEvents').|initUIEvent|;
                    document.createEvent('foo').|initEvent|;
                ",
                dom);
        }

        [TestMethod]
        public void createEvent_Windows()
        {
            var dom = _session.ReadFile(Paths.DomWindowsPath).Text;
            PerformCompletionRequests(
                @"
                    document.createEvent('Event').|target,type,bubbles,initEvent|;
                    document.createEvent('Events').|target,type,bubbles,initEvent|;
                    document.createEvent('CompositionEvent').|locale,data,initCompositionEvent|;
                    document.createEvent('CustomEvent').|detail,initCustomEvent|;
                    document.createEvent('DragEvent').|dataTransfer,initDragEvent|;
                    document.createEvent('FocusEvent').|relatedTarget,initFocusEvent|;
                    document.createEvent('KeyboardEvent').|location,shiftKey,key,getModifierState,initKeyboardEvent|;
                    document.createEvent('MessageEvent').|source,origin,data,initMessageEvent|;
                    document.createEvent('MouseEvent').|x,y,buttons,initMouseEvent|;
                    document.createEvent('MouseEvents').|x,y,buttons,initMouseEvent|;
                    document.createEvent('MouseWheelEvent').|wheelDelta,initMouseWheelEvent|;
                    document.createEvent('MutationEvent').|initMutationEvent|;
                    document.createEvent('MutationEvents').|initMutationEvent|;
                    document.createEvent('StorageEvent').|initStorageEvent|;
                    document.createEvent('TextEvent').|initTextEvent|;
                    document.createEvent('UIEvent').|initUIEvent|;
                    document.createEvent('UIEvents').|initUIEvent|;
                    document.createEvent('foo').|initEvent|;
                    document.createEvent('UIEvents').|initUIEvent|;

                    // Win8 events
                    document.createEvent('errorEvent').|initErrorEvent|;
                    document.createEvent('AnimationEvent').|initAnimationEvent|;
                    document.createEvent('msGestureEvent').|initGestureEvent|;
                    document.createEvent('msPointerEvent').|initPointerEvent|;
                    document.createEvent('TransitionEvent').|initTransitionEvent|;
                    document.createEvent('progressEvent').|initProgressEvent|;
                ",
                dom);
        }

        [TestMethod]
        public void GetElementById()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });
                    document.getElementById('form').|target,action,name,method,reset|; 
                    document.getElementById('a').|href|; 
                    document.getElementById('img').|src,alt|; 
                    document.getElementById('select').|form,selectedIndex|; 
                    document.getElementById('tr').|rowIndex,sectionRowIndex|; 
                    document.getElementById('li').|value|; 
                    document.getElementById('input').|width,maxLength,checked,type,defaultValue|; 
                    document.getElementById('iframe').|contentWindow,src,contentDocument|; 
                    document.getElementById('td').|rowSpan,cellIndex,colSpan|; 
                    document.getElementById('th').|rowSpan,cellIndex,colSpan|; 
                    document.getElementById('head').|profile,innerHTML|; 
                    document.getElementById('span').|innerHTML|; 
                    document.getElementById('ol').|start|; 
                    document.getElementById('area').|protocol,alt|; 
                    document.getElementById('area1').|innerHTML|; 
                    document.getElementById('video').|videoWidth,videoHeight|; 
                    document.getElementById('table').|tHead,rows|; 
                    document.getElementById('link').|rel,target|; 
                    document.getElementById('base').|target,href|; 
                    document.getElementById('table2').|tHead,rows|; 
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset).ToEnumerable().ToArray();
                     Assert.IsTrue(completions.Length > 0);
                     completions.ExpectContains(data.Split(','));
                 }, dom,
                @"
                    intellisense.logMessage('--- before: ' + JSON.stringify(document._$documentElements));
                    document._$documentElements = { 
                            form: document.createElement('form'),
                            a: document.createElement('a'),
                            select: document.createElement('select'),
                            img: document.createElement('img'),
                            tr: document.createElement('tr'),
                            html: document.createElement('html'),
                            li: document.createElement('li'),
                            input: document.createElement('input'),
                            iframe: document.createElement('iframe'),
                            td: document.createElement('td'),
                            th: document.createElement('th'),
                            head: document.createElement('head'),
                            span: document.createElement('span'),
                            ol: document.createElement('ol'),
                            area: document.createElement('area'),
                            area1: document.createElement('area'),
                            video: document.createElement('video'),
                            table: document.createElement('table'),
                            link: document.createElement('link'),
                            base: document.createElement('base'),
                            table2: document.createElement('TAble')
                        };
                    intellisense.logMessage('--- after: ' + JSON.stringify(document._$documentElements));
                ");
        }

        [TestMethod]
        [WorkItem(235897)]
        public void DomEventArgs()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });

                    document.onselect = function(e) {
                        e.|initUIEvent|
                    };
                    
                    document.onclick = function(e) {
                        e.|type,target,preventDefault,stopPropagation,timeStamp,x,y,button|
                    };

                    document.body.onclick = function(e) {
                        e.|type,target,preventDefault,stopPropagation,timeStamp,x,y,button|
                    };
                ", dom);

            PerformCompletionRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });

                    document.createElement('object').getSVGDocument().rootElement.addEventListener('SVGZoom', function(e) {
                        e.|zoomRectScreen,previousScale,newScale|
                    };
                ", dom);

            PerformRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });

                    document.addEventListener('select', function(e) {
                        e.|addEventListener|
                    });
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset).ToEnumerable().ToArray();
                     Assert.IsTrue(completions.Length > 0);
                     if (data == "addEventListener")
                     {
                         // Expect UIEvent object
                         completions.ExpectContains(new[] { 
                            "initUIEvent"
                         });
                     }
                 }, dom);

            PerformRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });
                    
                    document.addEventListener('load', function(e) {
                        e.|load|
                    });
                    
                    document.addEventListener('DOMContentLoaded', function(e) {
                        e.|DOMContentLoaded|
                    }, false);
                    
                    document.addEventListener('click', function(e) {
                        e.|click|
                    }, false);

                    document.addEventListener('abort', function(e) {
                        e.|abort|
                    }, false);

                    document.body.addEventListener('focus', function(e) {
                        e.|focus|
                    }, false);

                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset).ToEnumerable().ToArray();
                     Assert.IsTrue(completions.Length > 0);

                     switch (data)
                     {
                         case "load":
                         case "DOMContentLoaded":
                             completions.ExpectContains(new[] { "type", "target", "preventDefault", "stopPropagation", "timeStamp" });
                             break;
                         case "click":
                             completions.ExpectContains(new[] { "type", "target", "preventDefault", "stopPropagation", "timeStamp", "x", "y", "button" });
                             break;
                         case "abort":
                             completions.ExpectContains(new[] { "detail", "view" });
                             break;
                         case "focus":
                             completions.ExpectContains(new[] { "detail", "view", "isTrusted" });
                             break;
                     }

                 }, dom);

            PerformRequests(
                @"
                    intellisense.addEventListener('statementcompletion',  function() { });

                    document.addEventListener('load', function(e) {
                        document.body.addEventListener('click', function(e) {
                            intellisense.logMessage('arrived e:' + e);
                            e.|
                        });
                    });
                ", (context, offset, data, index) =>
                 {
                     var completions = context.GetCompletionsAt(offset).ToEnumerable().ToArray();
                     Assert.IsTrue(completions.Length > 0);

                     switch (data)
                     {
                         case "DOMContentLoaded":
                             completions.ExpectContains(new[] { "type", "target", "preventDefault", "stopPropagation", "timeStamp" });
                             break;
                         case "click":
                             completions.ExpectContains(new[] { "type", "target", "preventDefault", "stopPropagation", "timeStamp", "x", "y", "button" });
                             break;
                         case "abort":
                             completions.ExpectContains(new[] { "detail", "view" });
                             break;
                         case "focus":
                             completions.ExpectContains(new[] { "detail", "view", "isTrusted" });
                             break;
                     }

                 }, dom);
        }

        [TestMethod]
        [WorkItem(396400)]
        public void DomEventNameCase()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(
                @"
                    // Verify event with mixed case name (e.g. MSPointerDown) is triggered with the correct argument
                    document.onmspointerdown = function(e) {
                        e.|width,rotation,pressure,pointerType,pointerId|;
                    };

                    // addEventListener should get the same argument
                    document.addEventListener('MSPointerDown', function(e) {
                        e.|width,rotation,pressure,pointerType,pointerId|;
                    };

                    // addEventListener should not work if the event name case is not correct
                    document.addEventListener('mspointerdown', function(e) {
                        e.|!width,!rotation,!pressure,!pointerType,!pointerId|;
                    };

                    // nor should it work if the event name has 'on' prefix
                    document.addEventListener('onMSPointerDown', function(e) {
                        e.|!width,!rotation,!pressure,!pointerType,!pointerId|;
                    };
                ", dom);
        }

        [TestMethod]
        [WorkItem(268767)]
        public void ThisObject()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(@"
            var x = 0;
            document.addEventListener('load', 
                function eventHandler(event) {
                    // Ensure that 'this' is the document object
                    if (this == document)
                        x = 'string';
                    x.|;
                });", (completion, data, index) =>
                    {
                        completion.ExpectContains(CompletionsBase.StringMethods);
                    }, AuthorCompletionFlags.acfMembersFilter, dom);
        }

        [TestMethod]
        [WorkItem(279473)]
        public void NestedEventHandler()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;

            PerformCompletionRequests(@"
                (function(){
                    document.addEventListener('click', function(e) { 
                        // Ensure event object is correct
                        e.|target,x,y,button|;

                        // Ensure that 'this' is the document object
                        var x = (this === document) ? 'string' : 0;
                        x.|anchor|;
                    });
                })();
            ", dom);

            // In a prototype function defenition
            PerformCompletionRequests(@"
                function myObj() {
                    myObj.prototype.myFunc = function () {
                            document.addEventListener('click', function(e) { 
                            e.|target,x,y,button|;
                            });
                    };
                }
            ", dom);
        }

        [TestMethod]
        [WorkItem(329250)]
        public void HasChildren()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                    var form = document.createElement('form');
                    var a;
                    if(form.hasChildNodes() === false) {
                        a = 1;                        
                    }
                    a.|toFixed|;
                });", dom);
        }

        [TestMethod]
        [WorkItem(329250)]
        public void GetElementsByTagName()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                    var foo = document.getElementsByTagName(undefined);
                    foo.|length|;
                });", dom);
        }

        [TestMethod]
        [WorkItem(329250)]
        public void HTMLElementCollections()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            PerformCompletionRequests(@"
                    document.images[0].|width|;
                    document.scripts[0].|src|;
                    document.applets[0].|code|;
                    document.forms[0].|target|;
                    document.anchors[0].|href|;
                    document.links[0].|href|;
                    document.embeds[0].|src|;
                
                    var table = document.createElement('table');
                    table.rows[0].|rowIndex|;

                    var tr = document.createElement('tr');
                    tr.cells[0].|tagName|;

                    var thead = document.createElement('thead');
                    thead.rows[0].|rowIndex|;

                    var tfoot = document.createElement('tfoot');
                    tfoot.rows[0].|rowIndex|;
                });", dom);
        }

        [TestMethod]
        [Ignore] /// The new IE spec doesn't define method "applyElement" any more
        [WorkItem(326542)]
        public void ApplyElement()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var dump = @"
                    intellisense.addEventListener('statementcompletion',  function() { });
                    
                    function dumpNodes(e, level) {
                        var ident = '';
                        for(var i=0; i<level; i++) ident = ident + ' ';
                        intellisense.logMessage(ident + '<' + e.tagName + '>');
                        for(var i=0; i<e.childNodes.length; i++) {
                            dumpNodes(e.childNodes[i], level == undefined ? 1 : level + 1);
                        }
                        intellisense.logMessage(ident + '</' + e.tagName + '>');
                    }
            ";

            PerformCompletionRequests(@"
                    function test(where) {
                        intellisense.logMessage('>>> testing applyElement, where=' + where);

                        var div = document.createElement('div');
                        var ul=document.createElement('ul');
                        div.appendChild(ul);
                        ul.appendChild(document.createElement('li'));
                        ul.appendChild(document.createElement('li'));
                        var p=document.createElement('p');

                        intellisense.logMessage('>>> DOM before applyElement');
                        dumpNodes(div);

                        ul.applyElement(p, where);
                    
                        intellisense.logMessage('>>> DOM after applyElement');
                        dumpNodes(div);
                        
                        var result = false;
                        if(where == undefined || where=='outside') {
                            // Ensure P contains the UL
                            if(div.childNodes[0].tagName=='P' && 
                                div.childNodes[0].childNodes[0].tagName=='UL' && 
                                div.childNodes[0].childNodes[0].childNodes[0].tagName=='LI' &&
                                div.childNodes[0].childNodes[0].childNodes[1].tagName=='LI') {
                                result=true;
                            }
                        } else if(where=='inside') {
                            // Ensure UL contains P and UL's children
                            if(div.childNodes[0].tagName=='UL' && 
                                div.childNodes[0].childNodes[0].tagName=='P' && 
                                div.childNodes[0].childNodes[0].childNodes[0].tagName=='LI' &&
                                div.childNodes[0].childNodes[0].childNodes[1].tagName=='LI') {
                                result=true;
                            }
                        }

                        div.removeChild(div.childNodes[0]);                        
                        return result;
                    }
                    
                    var result = {};
                    if(test(undefined) && 
                       test('outside') && 
                        test('inside')) {
                        result.success=true;
                    }
                    result.|success|;
                ", dump, dom);
        }

        [TestMethod]
        [WorkItem(351733)]
        public void EventHandlerInContextFiles()
        {
            var dom = _session.ReadFile(Paths.DomWebPath).Text;
            var contextFile = @"
                var foo = (function foofunction() {
                    // This should not be called when the context file is applied
                    window.addEventListener('unload', function loader() { foo = null; });
                    return { bar: null };
                })();";

            PerformCompletionRequests(@"foo.|bar|;", dom, contextFile);
        }

        [TestMethod]
        [WorkItem(418288)]
        public void ForVarDeclarationInScope()
        {
            PerformCompletionRequests(ForVarDeclarationInScopeText);
        }
        #region Test data
        const string ForVarDeclarationInScopeText = @"
function init() {
    function foo() {
        function bar() {
            for (var index = 1;|index|
        }
    }
}
function ready() {
    init();
}
setTimeout(ready);";
        #endregion
    }
}