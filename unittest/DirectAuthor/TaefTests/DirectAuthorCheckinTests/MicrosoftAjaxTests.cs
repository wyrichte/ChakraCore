using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.JavaScript.LanguageService.Engine;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DirectAuthorCheckinTests
{
    [TestClass]
    public class MicrosoftAjaxTests : CompletionsBase
    {
        protected static readonly string MicrosoftAjaxFileText = TestFiles.MicrosoftAjax;
        protected static readonly string MicrosoftAjaxWebFormsFileText = TestFiles.MicrosoftAjaxWebForms_debug;

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
        [WorkItem(181411)]
        [WorkItem(182277)]
        [WorkItem(206677)]
        public void MSAjaxCompletion()
        {
            var domfile = _session.ReadFile(Paths.DomWebPath);
            var msajaxFile = _session.FileFromText(MicrosoftAjaxFileText);
            var msajaxWebFormsFile = _session.FileFromText(MicrosoftAjaxWebFormsFileText);

            PerformCompletionRequests(@"
                // Source: Microsoft Ajax Library Client Reference (http://msdn.microsoft.com/en-us/library/bb397536.aspx)

                // Sys namespace

                Sys.|ApplicationLoadEventArgs,Browser,CancelEventArgs,CollectionChange,CommandEventArgs,Component,CultureInfo,Debug,EventArgs,EventHandlerList,HistoryEventArgs,IContainer,IDisposable,INotifyDisposing,INotifyPropertyChange,NotifyCollectionChangedAction,NotifyCollectionChangedEventArgs,Observer,PropertyChangedEventArgs,StringBuilder|;
                Sys.Application.|add_init,add_load,add_navigate,add_unload,addComponent,addHistoryPoint,beginCreateComponents,beginUpdate,dispose,disposeElement,endCreateComponents,endUpdate,findComponent,getComponents,initialize,notifyScriptLoaded,raiseLoad,raisePropertyChanged,registerDisposableObject,removeComponent,unregisterDisposableObject,updated|;
                Sys.Browser.|agent,documentMode,hasDebuggerStatement,name,version|;
                Sys.Net.|NetworkRequestEventArgs,WebRequest,WebRequestExecutor,WebRequestManager,WebServiceError,WebServiceProxy,XMLHttpExecutor|;
                Sys.Serialization.|JavaScriptSerializer|;
                Sys.UI.|Behavior,Bounds,Control,DomElement,DomEvent,Key,MouseButton,Point,VisibilityMode|;
                Sys.Res.|actualValue,argument,argumentNull,argumentOutOfRange,argumentType,argumentTypeWithTypes,argumentUndefined,assertFailed,assertFailedCaller,badBaseUrl1,badBaseUrl2,badBaseUrl3,breakIntoDebugger,cannotAbortBeforeStart,cannotCallBeforeResponse,cannotCallOnceStarted,cannotCallOutsideHandler,cannotDeserializeEmptyString,cannotSerializeNonFiniteNumbers,enumInvalidValue,eventHandlerInvalid,format,formatBadFormatSpecifier,formatInvalidString,invalidExecutorType,invalidHttpVerb,invalidOperation,invalidTimeout,invokeCalledTwice,notImplemented,nullWebRequest|;

                // Sys.WebForms namespace
                Sys.WebForms.|BeginRequestEventArgs,EndRequestEventArgs,InitializeRequestEventArgs,PageLoadedEventArgs,PageLoadingEventArgs,PageRequestManager|;

                // JavaScript Base Type Extensions

                Array.|add,addRange,clear,clone,contains,dequeue,enqueue,forEach,indexOf,insert,parse,remove,removeAt|;
                Boolean.|parse|;
                Date.prototype.|format,localeFormat|;
                Function.|createCallback,createDelegate,emptyMethod,validateParameters|;
                Error.|argument,argumentNull,argumentOutOfRange,argumentType,argumentUndefined,create,format,invalidOperation,notImplemented,parameterCount|;
                Error.prototype.|popStackFrame|;
                Number.prototype.|format,localeFormat|;
                Object.|getType,getTypeName|;
                String.prototype.|endsWith,startsWith,trim,trimEnd,trimStart|;|$find|;

                // Types

                Type.|callBaseMethod,createCallback,createDelegate,getRootNamespaces,isClass,isEnum,isFlags,isInterface,isNamespace,parse,registerNamespace|;
                Type.prototype.|callBaseMethod,getBaseMethod,getBaseType,getInterfaces,getName,implementsInterface,inheritsFrom,initializeBase,isImplementedBy,isInstanceOfType,registerClass,registerEnum,registerInterface,resolveInheritance|;

                // Global Shortcuts to Commonly Used APIs

                ;|$find,$get,$create,$addHandler,$addHandlers,$clearHandlers,$removeHandler|;",
                domfile.Text, msajaxFile.Text, msajaxWebFormsFile.Text);
        }

        [TestMethod]
        [WorkItem(206738)]
        public void MSAjaxShortcutFunctions()
        {
            Action<IEnumerable<AuthorCompletion>, string, int> verify = (completions, data, index) => completions.ExpectContains(new[] { "UI", "Browser", "EventArgs" });
            var domfile = _session.ReadFile(Paths.DomWebPath);
            var msajaxfile = _session.FileFromText(MicrosoftAjaxFileText);


            PerformCompletionRequests(@"
                var x = $get('sadg');
                
                // x should be HTMLElement
                x.|onload,title,children,contains|;
                ", domfile.Text, msajaxfile.Text);
        }

        [TestMethod]
        [WorkItem(205878)]
        public void SourceFileHandle()
        {
            PerformRequests(@"Function.|", (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                
                    Assert.IsNotNull(completions);
                    var hint = completions.GetHintFor("createCallback");
                    Assert.IsNotNull(hint);
                    var functionHelp = hint.GetFunctionHelp();
                    Assert.IsNotNull(functionHelp);
                    Assert.IsNotNull(functionHelp.SourceFileHandle);
                }, 
                true, 
                "!!" + Paths.DomWebPath, 
                TestFiles.MicrosoftAjax, 
                TestFiles.MicrosoftAjax_intellisense
            );

            PerformRequests(@"Function.createCallback(|", (context, offset, data, index) =>
                {
                    var help = context.GetParameterHelpAt(offset);
                    Assert.IsNotNull(help);
                    Assert.IsNotNull(help.FunctionHelp.SourceFileHandle);
                },
                true,
                "!!" + Paths.DomWebPath,
                TestFiles.MicrosoftAjax,
                TestFiles.MicrosoftAjax_intellisense
            );
        }

        [TestMethod]
        public void MSAjaxExtension()
        {
            PerformRequests(@"
                ;|", (context, offset, data, index) =>
                {
                    var completions = context.GetCompletionsAt(offset);
                    Assert.IsNotNull(completions);
                    var internalEntries = completions.ToEnumerable().Where(item => item.Name.IndexOf('$') >= 1).ToArray();
                    internalEntries.Length.Expect(0);
                },
                true,
                "!!" + Paths.DomWebPath,
                TestFiles.MicrosoftAjax,
                TestFiles.MicrosoftAjax_intellisense
            );
        }

        #region Helpers
        protected override ParsedRequests ParseRequests(string text)
        {
            var builder = new StringBuilder(text.Length);
            var requests = new List<Request>();
            for (int i = 0; i < text.Length; i++)
            {
                var ch = text[i];

                if (ch == '|'
                    // Don't consider " |" or "||" a cursor location because it is most likely the | or || operators.
                    && (i == 0 || (text[i - 1] != ' ' && text[i - 1] != '|')))
                {
                    string data = null;
                    var j = i + 1;
                    if (j < text.Length)
                    {
                        while (true)
                        {
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