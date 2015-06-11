//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

#if DBG_DUMP
void PrintPnodeWIndent(ParseNode *pnode, int indentAmt);
#endif

namespace Authoring
{
    namespace Names
    {
        const wchar_t def[] = L"_$def";
        const wchar_t dummy[] = L"$$dummy$$";
        const wchar_t isParameter[] = L"_$isParameter";
        const wchar_t offsets[] = L"_$offsets";
        const wchar_t staticFieldsDocApplied[] = L"_$staticFieldsDocApplied";
        const wchar_t _this[] = L"this";
        const wchar_t with[] = L"_$with";
        const wchar_t callLss[] = L"_$callLss";
        const wchar_t executeGetter[] = L"_$executeGetter";
        const wchar_t executeSetter[] = L"_$executeSetter";
        const wchar_t createCapturingProxyHolder[] = L"_$createCapturingProxyHolder";
        const wchar_t capturingProxyHolderProxyPropertyName[] = L"proxy";
        const wchar_t capturingProxyHolderPropertiesPropertyName[] = L"properties";
    }
    
    class PropertyFilter
    {
    public:
        PropertyFilter(JsUtil::List<LPCWCHAR, ArenaAllocator>* usedProperties = nullptr) : m_usedProperties(usedProperties)
        {
        }

        template< size_t N >
        bool operator ()(const Js::PropertyRecord *propertyRecordToConsider, const wchar_t(&prefix)[N])
        {
            CompileAssert(N >= 1);
            if (this->m_usedProperties == nullptr)
            {
                return false;
            }
            else
            {
                size_t prefixLength = N - 1;
                const WCHAR* canonicalizedPropertyNameToConsider = propertyRecordToConsider->GetBuffer();
                if (propertyRecordToConsider->GetLength() > prefixLength && wcsncmp(prefix, canonicalizedPropertyNameToConsider, prefixLength) == 0)
                {
                    canonicalizedPropertyNameToConsider += prefixLength;
                }

                return this->m_usedProperties->Contains(canonicalizedPropertyNameToConsider);
            }
        }
    private:
        JsUtil::List<LPCWCHAR, ArenaAllocator>* m_usedProperties;
    };

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    Js::HiResTimer g_timer;
#endif

    class IsInCallArgs
    {
        charcount_t m_offset;
        LanguageServiceExtension* m_lsExtension;
        JsUtil::Stack<LPCWCHAR>* m_propertyPath;
        JsUtil::List<LPCWCHAR, ArenaAllocator>* m_usedProperties;
        bool m_usedPropertiesPopulated;
    public:

        IsInCallArgs(charcount_t offset, LanguageServiceExtension* lsExtension, JsUtil::Stack<LPCWCHAR>* propertyPath, JsUtil::List<LPCWCHAR, ArenaAllocator>* usedProperties) : m_offset(offset), m_lsExtension(lsExtension), m_propertyPath(propertyPath), m_usedProperties(usedProperties), m_usedPropertiesPopulated(false)
        {
            Assert((propertyPath == nullptr) == (usedProperties == nullptr));
        }

        bool operator ()(ParseNode* node)
        {
            if (node != nullptr)
            {
                if (node->nop == knopMember && this->m_propertyPath != nullptr && node->sxBin.pnode1->nop == knopStr)
                {
                    m_propertyPath->Push(node->sxBin.pnode1->sxPid.pid->Psz());
                }

                if (node->nop == knopObject && this->m_usedProperties != nullptr && !this->m_usedPropertiesPopulated)
                {
                    this->m_usedPropertiesPopulated = true;
                    ParseNodePtr memberNode = nullptr;
                    ParseNodePtr memberNameNode = nullptr;
                    ParseNodePtr memberListNode = node->sxUni.pnode1;
                    if (memberListNode != nullptr)
                    {
                        while (memberListNode->nop == knopList)
                        {
                            memberNode = memberListNode->sxBin.pnode1;
                            if (memberNode->nop != knopMember)
                            {
                                // The node could be a getter or setter, and in that case we will not consider it in call args anymore
                                return false;
                            }

                            memberNameNode = memberNode->sxBin.pnode1;
                            if (memberNameNode->nop == knopStr)
                            {
                                this->m_usedProperties->Add(memberNameNode->sxPid.pid->Psz());
                            }
                            memberListNode = memberListNode->sxBin.pnode2;
                        }

                        memberNode = memberListNode;
                        if (memberNode->nop != knopMember)
                        {
                            // The node could be a getter or setter, and in that case we will not consider it in call args anymore
                            return false;
                        }
                        memberNameNode = memberNode->sxBin.pnode1;
                        if (memberNameNode->nop == knopStr)
                        {
                            this->m_usedProperties->Add(memberNameNode->sxPid.pid->Psz());
                        }
                    }
                }

                if (node->nop == knopCall || node->nop == knopNew)
                {
                    return IsInCallParenthesis(m_offset, node, true, m_lsExtension);
                }
            }
            return false;
        }
    };

    class InFuncBodyOrObject
    {
        charcount_t m_offset;
        LanguageServiceExtension* m_lsExtension;
        bool m_stopOnObjectLiteral;
    public:
        InFuncBodyOrObject(charcount_t offset, LanguageServiceExtension* lsExtension, bool stopOnObjectLiteral) : m_offset(offset), m_lsExtension(lsExtension), m_stopOnObjectLiteral(stopOnObjectLiteral) { }
        bool operator ()(ParseNode* node)
        {
            if (node != nullptr && ((node->nop == knopFncDecl && node->sxFnc.pnodeBody) || (this->m_stopOnObjectLiteral && node->nop == knopObject)))
            {
                auto LCurly = m_lsExtension->LCurly(node);
                auto RCurly = m_lsExtension->RCurly(node);
                // We want the parameter help when the cursor is at the opening curly, for example: 
                //   f(|{}) 
                // or: 
                //   function() |{};
                auto min = LCurly ? LCurly + 1 : node->ichMin;
                auto lim = RCurly ? RCurly : node->ichLim;
                if (InRange(m_offset, min, lim))
                {
                    return true;
                }
            }

            return false;
        }
    };

    const int HURRY_SKIP_LIMIT = 5;

    TYPE_STATS(FileAuthoring, L"FileAuthoring")

    bool AsLeftOfDotObject(Js::ScriptContext* scriptContext, Js::Var value, Js::RecyclableObject*& obj)
    {
        if (Js::TaggedNumber::Is(value))
        {
            obj = scriptContext->GetLibrary()->GetNumberPrototype();
            return true;
        }
        else if (value)
        {
            Js::RecyclableObject* tmpObj = nullptr;
            if (Convert::FromVar(nullptr, value, tmpObj))
            {
                auto typeId = Js::JavascriptOperators::GetTypeId(tmpObj);
                if (typeId != Js::TypeIds_Null && typeId != Js::TypeIds_Undefined)
                {
                    obj = tmpObj;
                    return true;
                }
            }
        }
        return false;
    };

    HRESULT FileAuthoring::GetInterface(REFIID iid, __out void** ppvObject)
    {
#ifdef DEBUG
        Assert(ppvObject);
        if (iid == __uuidof(IAuthorDiagnostics) && m_scriptContext)
        {
            auto authorDiagnostics = AuthorDiagnostics::CreateInstance(m_scriptContext);
            if(!authorDiagnostics)
                return E_OUTOFMEMORY;
            *ppvObject = authorDiagnostics;
            return S_OK;
        }
#endif
        return E_NOINTERFACE;
    }

    HRESULT FileAuthoring::GetInternalHandle(IAuthorFileHandle *externalHandle, AuthoringFileHandle *&internalHandle)
    {
        METHOD_PREFIX;

        if (!externalHandle || !((AuthoringFileHandle *)externalHandle)->ValidateInstance(m_factory))
        {
            if (externalHandle)
                externalHandle->Release();
            hr = E_INVALIDARG;
            goto Error;
        }

        internalHandle = reinterpret_cast<AuthoringFileHandle *>(externalHandle);

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ValidateInternalHandles(__in_ecount(count) AuthoringFileHandle **handles, int count)
    {
        METHOD_PREFIX;

        for (int i = 0; i < count; i++)
        {
            if (!handles[i] || !handles[i]->ValidateInstance(m_factory))
            {
                if (handles[i])
                {
                    handles[i]->Release();
                    handles[i] = NULL;
                }
                hr = E_INVALIDARG;
            }
        }

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::GetHostType(Js::HostType *hostType)
    {
        METHOD_PREFIX;

        // Get the host type
        AuthorHostType authorHostType;
        IfFailGo(m_fileContext->GetHostType(&authorHostType));

        switch (authorHostType)
        {
        case AuthorHostType::ahtBrowser:
            *hostType = Js::HostType::HostTypeBrowser;
            break;
        case AuthorHostType::ahtApplication:
            *hostType = Js::HostType::HostTypeApplication;
            break;
        default:
            hr = E_INVALIDARG;
            goto Error;
        }

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::CollectSources(bool primaryOnly)
    {
        METHOD_PREFIX;

        if (m_contextChanged && !m_parserOnlyContext)
        {
            m_primaryTree.ForgetTree();
            m_primaryFile.ReleaseAndNull();
        }
        else if (m_primaryFile && (m_primaryFile->Text() == nullptr || !m_primaryTree.IsValidFor(m_primaryFile->Text())))
        {
            m_primaryTree.ForgetTree();
        }

        if (!m_primaryFile)
        {
            IAuthorFileHandle *primaryFile = nullptr;
            IfFailGo(m_fileContext->GetPrimaryFile(&primaryFile));
            AuthoringFileHandle* internalHandle = nullptr;
            IfFailGo(GetInternalHandle(primaryFile, internalHandle));
            m_primaryFile.Assign(internalHandle);
            ReleasePtr(primaryFile);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            CComBSTR name;
            m_primaryFile->GetDisplayName(&name);
            if (!name) name = L"<no name>";
            OUTPUT_TRACE(Js::JSLSPhase, L"PrimaryFile : %ls, length : %d @ FileAuthoring::CollectSources\n", name, m_primaryFile->GetLength());
#endif

        }

        if (!primaryOnly && !IsScriptContextPathValid())
        {
            int cContextFiles = 0;
            IfFailGo(m_fileContext->GetContextFileCount(&cContextFiles));
            Validate(cContextFiles >= 0);

            AuthoringFileHandle **contextFiles = nullptr;
            if(cContextFiles > 0)
            {
                contextFiles = AnewArray(m_contextAlloc, AuthoringFileHandle *, cContextFiles);
                IfFailGo(m_fileContext->GetContextFiles(0, cContextFiles, (IAuthorFileHandle **)contextFiles));
                IfFailGo(ValidateInternalHandles(contextFiles, cContextFiles));

                // TODO: take care of references such as base*.js, ui*.js, references_target_platform*.js --
                //       tag them with ScriptContextUsageFlags as References/UsedForCompletions so that we don't invalidate them
                //       in the end of GetStructure. For that, support from front-end team is needed. The need for this is that
                //       they don't come as part of GetCompletions prior to 1st GetStructure to establish initial context.
            }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            OUTPUT_TRACE(Js::JSLSPhase, L"Creating Context Path for %d context files: \n", cContextFiles);
            for (int i = 0; i < cContextFiles; i++)
            {
                CComBSTR name;
                contextFiles[i]->GetDisplayName(&name);
                if (!name) name = L"<no name>";
                OUTPUT_TRACE(Js::JSLSPhase, L"Context file [%d] : %ls\n", i, name);
            }
#endif

            // Get host type
            Js::HostType hostType;
            IfFailGo(GetHostType(&hostType));

            // Create script context path for the context files.
            ValidatePtr(m_authoringServicesInfo, E_FAIL);
            RefCountedPtr<ScriptContextPath> newScriptContextPath = m_authoringServicesInfo->GetScriptContextManager()->CreatePathFor(contextFiles, cContextFiles, hostType);

            // If we still holding the context path from before a ContextChanged event was raised, dereference it
            ADeletePtr(m_contextAlloc, m_leafScriptContext);
            m_leafScriptContext = null;
            m_scriptContextPath.ReleaseAndNull();
            m_scriptContext = null;

            // The scriptcontext is invalidated. Invalidate the parse tree if we have not done that already.
            m_primaryTree.ForgetTree();


            // Use the new context path
            m_scriptContextPath.Assign(newScriptContextPath); 

            // Reset the change flag after the new context path has been populated
            m_contextChanged = false;

            if (contextFiles)
            {
                for(int i = 0; i < cContextFiles; i++)
                {
                    ReleasePtr(contextFiles[i]);
                }

                AdeleteArray(m_contextAlloc, cContextFiles, contextFiles);
            }
        }

        METHOD_POSTFIX;
    }

    void FileAuthoring::ReleasePrimaryContext()
    {
        // Delete the parse tree before the scriptContext, to avoid the parser accessing a deleted scriptContext
        m_primaryTree.ForgetTree();

        ADeletePtr(m_contextAlloc, m_leafScriptContext);
        m_primaryFile.ReleaseAndNull();

        m_scriptContext = null;
        m_contextAlloc->Reset();

        CleanupParseOnlyContext();
    }

    void FileAuthoring::ForgetContext()
    {
        m_contextChanged = true;
        m_scriptContextPathComplete = false;
    }

    void FileAuthoring::CleanupParseOnlyContext()
    {
        if (m_parserOnlyContext)
        {
            // delete the parse tree generated while using the parse-only context
            m_primaryTree.ForgetTree();

            m_scriptContext = null;
            m_parseOnlyScriptContext.ReleaseAndNull();
            m_parserOnlyContext = false;
        }
    }

    void FileAuthoring::DecommitUnusedPages()
    {
        auto scriptContext = m_rootScriptContext;
        if (scriptContext) 
            AuthoringFactory::DecommitUnusedPages(scriptContext->GetThreadContext()->GetPageAllocator());
    }

    HRESULT FileAuthoring::UpdatePrimary(bool fullContext)
    {
        METHOD_PREFIX;

        // When source has changed, we want to throw the context away since it has a pointer
        // to a source buffer which was already released. Continuing using the context is dangerous in this case
        // since some operations like Function.toString may try to access the released buffer.
        bool sourceChanged = m_primaryFile && (m_primaryFile->Text() == nullptr || !m_primaryTree.IsValidFor(m_primaryFile->Text()));

        if (fullContext || (IsScriptContextPathValid() && m_scriptContextPath->IsUpToDate()))
        {
            // Ensure we never parse in a polluted context. 
            // Also refresh the context when source has changed.
            if (this->m_polluted || sourceChanged)
                ReleasePrimaryContext();
            IfFailGo(EnsureContext());
            IfFailGo(TestAbort());
        }
        else
        {
            // Refresh the context when source has changed.
            if(sourceChanged)
                ReleasePrimaryContext();

            if (!IsScriptContextValid() && !IsValidParseOnlyContext())
            {
                CleanupParseOnlyContext();

                // Get host type
                Js::HostType hostType;
                IfFailGo(GetHostType(&hostType));

                Js::ScriptContext *scriptContext = ScriptContextPath::CreateEmptyScriptContext(hostType);
                ValidateAlloc(scriptContext);
                m_parseOnlyScriptContext.TakeOwnership(RefCountedScriptContext::Create(scriptContext));
                m_scriptContext = m_parseOnlyScriptContext;
                m_parserOnlyContext = true;
            }
        }

        IfFailGo(CollectSources(true));
        IfFailGo(TestAbort());
        IfFailGo(m_primaryFile->UpdateParseTree(this, m_scriptContext, /* enableDeferedParsing = */false, m_primaryTree, /* isPrimaryFile = */true, &m_primarySourceIndex));

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ApplyContext()
    {
        METHOD_PREFIX;

        // Applying context files may result in detecting new asynchrony script requests, which means that the context is 
        // not complete. Make sure ApplyContext always result in complete context.
        hr = EnsureQuiescentContext([&]()->HRESULT 
        {
            INTERNALMETHOD_PREFIX;

            if (!IsScriptContextPathValid())
                IfFailGoInternal(CollectSources(false));
            Js::ScriptContext *scriptContext;
            IfFailGoInternal(m_scriptContextPath->EnsureScriptContext(this, &scriptContext));
            if (!m_scriptContextPathComplete)
            {
                IfFailGoInternal(m_scriptContextPath->EnsureCompletePath(this));
                m_scriptContextPathComplete = true;
            }
            INTERNALMETHOD_POSTFIX;
        });
        IfFailGo(hr);

        METHOD_POSTFIX;
    }

    bool FileAuthoring::IsScriptContextPathValid()
    {
        return m_scriptContextPath && m_scriptContextPathComplete && !m_contextChanged;
    }

    bool FileAuthoring::IsScriptContextValid()
    {
        return IsScriptContextPathValid() && m_scriptContext && m_leafScriptContext && m_leafScriptContext->IsValid() && !m_parserOnlyContext && m_leafScriptContext->ValidateScriptContext(m_scriptContext);
    }

    HRESULT FileAuthoring::AllocateContext()
    {
        METHOD_PREFIX;

        if (!IsScriptContextValid())
        {
            CleanupParseOnlyContext();

            IfFailGo(ApplyContext());

            if (!m_leafScriptContext)
                m_leafScriptContext = m_scriptContextPath->CreateLeafScriptContext(m_contextAlloc, &m_primaryTree);

            IfFailGo(m_leafScriptContext->GetScriptContext(this, &m_scriptContext));

            Assert(IsScriptContextValid());
            m_scriptContext->ForceNoNative();
            m_scriptContext->SetInDebugMode();            
            m_scriptContext->SetCanOptimizeGlobalLookupFlag(false);

            // The scriptcontext is invalidated. Invalidate the parse tree if we have not done that already.
            m_primaryTree.ForgetTree();

            m_polluted = false;
            m_parserOnlyContext = false;
        }

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::EnsureContext()
    {
        if (!IsScriptContextValid())
            return AllocateContext();
        return S_OK;
    }

    void FileAuthoring::ExecuteRootFunction(AuthoringProbe& authorProbe, Js::JavascriptFunction *jsFunc, Js::Arguments& arguments, Js::Var *result, bool doNotReportPhase)
    {
        Assert(jsFunc);
        Assert(result);
        m_polluted = true;
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Js::HiResTimer timer;
        auto start = timer.Now();
#endif
#endif
        Assert(IsScriptContextValid());

        AutoPhaseReporterExecuting autoPhaseReporterExecuting(this, &authorProbe, m_scriptContext, !doNotReportPhase);
        try
        {
            CallRootFunction(jsFunc, arguments, result);
        }
        catch (ExecutionStop)
        {
        }

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Output::Print(L"Executed for %g\n", timer.Now() - start);
#endif
#endif
    }

    HRESULT FileAuthoring::ExecuteFunction(Js::JavascriptFunction *jsFunc, Js::Arguments& arguments, Js::Var *result, bool doNotDisableLoopGuard/* = false */, bool doNotReportPhase/* = false */)
    {
        METHOD_PREFIX;

        Assert(jsFunc);
        Assert(result);
        Assert(doNotDisableLoopGuard == doNotReportPhase);

        IfFailGo(TestAbort());

        // If we are about to execute a function, a check to see if its deserialized won't be too expensive.
        if(jsFunc->GetFunctionProxy() != null){
            auto parseableFunctionInfo = jsFunc->GetFunctionProxy()->EnsureDeserialized();
            // Ensure the method is parsed as well.
            if (parseableFunctionInfo->IsDeferred())
            {
                if (jsFunc->IsScriptFunction())
                {
                    Js::ScriptFunction * scriptFunction = (Js::ScriptFunction*) jsFunc;
                    scriptFunction->EnsureCopyFunction();
                    // Update the function info again as the Ensure would have invalidated the old ones.
                    parseableFunctionInfo = scriptFunction->GetParseableFunctionInfo();
                }
                BEGIN_ENTER_SCRIPT(m_scriptContext, false, false, false)
                {
                    parseableFunctionInfo->Parse();
                }
                END_ENTER_SCRIPT;
            }
        }

        WithProbe([&] (AuthoringProbe& authorProbe) {
            authorProbe.SetKeepLoopGuardOnUninstall(doNotDisableLoopGuard);
            ExecuteRootFunction(authorProbe, jsFunc, arguments, result, doNotReportPhase);
        },
        0, 
        jsFunc->GetFunctionBody());

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ExecuteSetter(Js::RecyclableObject* obj, LPCWSTR propertyName, Js::Var value)
    {
        METHOD_PREFIX;

        Assert(obj);
        Assert(value);

        JsHelpers::WithArguments([&] (Js::Arguments& arguments)
        {
            auto executeSetter = JsHelpers::GetProperty<Js::JavascriptFunction*>(m_scriptContext->GetGlobalObject(), Names::executeSetter, nullptr, m_scriptContext);
            if (executeSetter)
            {
                Js::Var result;
                ExecuteFunction(executeSetter, arguments, &result);
            }
        }, 
        m_scriptContext, 
        obj,
        obj,
        Convert::ToVar(propertyName, m_scriptContext),
        value);

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ExecuteGetter(Js::RecyclableObject* obj, LPCWSTR propertyName, Js::Var* result)
    {
        METHOD_PREFIX;

        Assert(obj);
        Assert(result);

        JsHelpers::WithArguments([&] (Js::Arguments& arguments)
        {
            auto executeGetter = JsHelpers::GetProperty<Js::JavascriptFunction*>(m_scriptContext->GetGlobalObject(), Names::executeGetter, nullptr, m_scriptContext);
            *result = nullptr;
            if (executeGetter)
                ExecuteFunction(executeGetter, arguments, result);
        }, 
        m_scriptContext, 
        obj,
        obj,
        Convert::ToVar(propertyName, m_scriptContext));

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ExecuteToBreakpoint(int loadLocation, Js::RegSlot slot, Js::FunctionBody *exprContainer, Js::FunctionBody *executingFunction, bool isRoot, Js::DynamicObject* environment, AuthorCompletionDiagnosticFlags *diagFlags, bool isFinalPass, Js::Var *var)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        Assert(IsScriptContextValid());

        Js::JavascriptFunction *jsFunc = m_scriptContext->GetLibrary()->CreateScriptFunction(executingFunction);

        bool enableLoopGuards = false;
        if (this->IsHurryCalled())
        {
            ResetHurryAndAbort();
            enableLoopGuards = true;
        }

        // Set up to stop where we need to.
        exprContainer->InstallProbe(loadLocation);
        {
            WithProbe([&] (AuthoringProbe& authorProbe) {
                // Call the root function and run to the break-point if hit.
                Js::Var args[1];
                args[0] = m_scriptContext->GetGlobalObject();
                Js::Arguments jsArguments(isRoot ? 0 : 1, args);

                if (enableLoopGuards)
                {
                    LoopGuardsHelpers::EnableLoopGuards(m_scriptContext);
                }

                Js::Var varResult;
                ExecuteRootFunction(authorProbe, jsFunc, jsArguments, &varResult); 

                if (authorProbe.Triggered())
                {
                    *var = authorProbe.Value();
                }

                bool firstPassTriggered = authorProbe.Triggered();
                bool firstPassAsyncBreak = authorProbe.AsyncBreakTriggered();

                if (!authorProbe.Triggered() || !(*var) || ((*var) && IsExceptionObject(Alloc(), m_scriptContext, *var)))
                {
                    // if we have no value try executing any event listeners, setTimeout or other _$ls methods
                    auto callLss = JsHelpers::GetProperty<Js::JavascriptFunction*>(m_scriptContext->GetGlobalObject(), Names::callLss, nullptr, m_scriptContext);

                    if (callLss)
                    {
                        authorProbe.ClearAsyncBreak(m_scriptContext);
                        authorProbe.Reset();
                        exprContainer->InstallProbe(loadLocation);
                        Js::Var args[1];
                        args[0] = m_scriptContext->GetGlobalObject();
                        Js::Arguments jsArguments(1, args);

                        ExecuteRootFunction(authorProbe, callLss, jsArguments, &varResult); 

                        // if we have a better value set it.
                        if (authorProbe.Triggered())
                        {
                            *var = authorProbe.Value();
                        }
                    }
                }

                if (diagFlags)
                {
                    if (authorProbe.Triggered() || firstPassTriggered)
                    {
                        // If primary execution or callLss execution resulted in the breakpoint being hit, track this as a successful execution.
                        *diagFlags = AuthorCompletionDiagnosticFlags(*diagFlags | acdfSuccessfulExecution);
                    }
                    else if (authorProbe.AsyncBreakTriggered() || firstPassAsyncBreak)
                    {
                        // If the breakpoint was not hit by either call and one or both calls resulted in an async break, track this as a timeout.
                        *diagFlags = AuthorCompletionDiagnosticFlags(*diagFlags | acdfPrimaryFileHalted);
                    }
                }

                if (enableLoopGuards)
                {
                    LoopGuardsHelpers::DisableLoopGuards(m_scriptContext);
                }
            },
            slot, exprContainer, !enableLoopGuards, loadLocation, m_diagCallback, isFinalPass);
        }

        METHOD_POSTFIX;
    }

    // Sets result to a non-null value if a result was found. 
    // HRESULT is used to only indicate a abnormal termination.
    HRESULT FileAuthoring::Execute(ParseNode *expr, AuthorCompletionDiagnosticFlags *diagFlags, bool isFinalPass, Js::Var *var)
    {
        CompileScriptException se;

        METHOD_PREFIX;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::HiResTimer timer;
        double start = timer.Now();
        double bytecodetime = 0;
        OUTPUT_TRACE(Js::JSLSStatsPhase, L"FileAuthoring::Execute starting @ absolute time %10.3f\n", g_timer.Now());
#endif

        IfFailGo(TestAbort());

        ValidateArg(expr);
        ValidateArg(var);

        *var = NULL;

        Assert(IsScriptContextValid());

        SetExecutionFile(m_primaryFile);


        {
            auto originalAuthoringData = m_scriptContext->authoringData;
            Js::LoadData loadData(nullptr, expr);
            m_scriptContext->authoringData = &loadData;
            Js::FunctionBody *body = NULL;
            IfFailGo(m_primaryFile->GenerateBody(this, &m_primaryTree, m_scriptContext, m_primarySourceIndex, &body));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            bytecodetime = timer.Now() - start;
            start = timer.Now();
#endif

            m_scriptContext->authoringData = originalAuthoringData;

            Js::RegSlot reg = loadData.reg;

            // Check the load register. If it doesn't have a register then the node is not an expresion or was not generated.
            if (reg == NoRegister)
                goto Error;

            int loadLocation = loadData.loadLocation;
            if (loadLocation > 0)
            {
                Js::FunctionBody *exprContainer = loadData.loadBody;
                Assert(exprContainer);

                IfFailGo(ExecuteToBreakpoint(loadLocation, exprContainer->MapRegSlot(reg), exprContainer, body, true, NULL, diagFlags, isFinalPass, var));
            }
        }

        OUTPUT_TRACE(Js::JSLSStatsPhase, L"Executing primary file, isFinalPass : %ls, bytecodegen time %8.3f and execution time %8.3f\n", isFinalPass ? L"yes" : L"no", bytecodetime, timer.Now() - start);

        METHOD_POSTFIX_CLEAN_START;
        ClearExecutionFile();
        METHOD_POSTFIX_CLEAN_END;
    }

    template<typename TOperation>
    void FileAuthoring::WithProbe(TOperation operation, Js::RegSlot slot, Js::FunctionBody *exprContainer, bool enableLoopGuardsOnAsyncBreak, int breakpointLocation, IAuthorCompletionDiagnosticCallback *diagCallback, bool isFinalPass)
    {
        if (breakpointLocation != -1)
            exprContainer->InstallProbe(breakpointLocation);
        AuthoringProbe authorProbe(slot, exprContainer, enableLoopGuardsOnAsyncBreak, breakpointLocation, diagCallback, isFinalPass);
        InstallProbe(&authorProbe);
            
        operation(authorProbe);
            
        UnistallProbe(&authorProbe);
        if (breakpointLocation != -1)
            exprContainer->UninstallProbe(breakpointLocation);
    }

    void FileAuthoring::InstallProbe(AuthoringProbe* authorProbe)
    {
        Assert(authorProbe);
        {
            AutoCriticalSection autocs(&hurryCriticalSection);
            m_currentProbe = authorProbe;
        }
        Assert(IsScriptContextValid());
        m_scriptContext->diagProbesContainer.AddProbe(authorProbe);
        m_scriptContext->diagProbesContainer.InitializeInlineBreakEngine(authorProbe);
        if (m_scriptContextPath)
            m_scriptContextPath->InstallInlineBreakpointProbes(authorProbe);
    }

    void FileAuthoring::UnistallProbe(AuthoringProbe* authorProbe)
    {
        Assert(authorProbe);
        authorProbe->ClearAsyncBreak(m_scriptContext);

        Assert(IsScriptContextValid());
        m_scriptContext->diagProbesContainer.RemoveProbe(authorProbe);
        m_scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(authorProbe);
        if (m_scriptContextPath)
            m_scriptContextPath->UninstallInlineBreakpointProbes(authorProbe);

        {
            AutoCriticalSection autocs(&hurryCriticalSection);
            m_currentProbe = nullptr;
        }
    }

    bool FileAuthoring::IdentifierLikeNode(ParseNodePtr pnode)
    {
        if (pnode)
            switch (pnode->nop)
        {
            case knopName:
            case knopTrue:
            case knopFalse:
            case knopThis:
            case knopNull:
            case knopSuper:
                return true;
        }
        return false;
    }

    bool FileAuthoring::IsNameNode(ParseNodePtr pnode)
    {
        return pnode && (pnode->nop == knopName || pnode->nop == knopStr);
    }

#define COMPOSED_REWRITING
#ifdef COMPOSED_REWRITING
    HRESULT FileAuthoring::RewriteExecutionTree(ParseNodePtr current)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendRewriteTreeBegin);
        {
            // Rewrite the tree and see if we get a result.
            // The following visitor will simultaniously execute CleanTree, DirectExecution and AddLoopGuards
#ifdef DEBUG
            typedef ParseNodeVisitor<TreeValidator> PreValidatorVisitor;
            typedef ParseNodeVisitor<VisitorComposer<TL4(CleanTree, DirectExecution, AddNestedCalls, TreeValidator)> > RewritingVisitor;
#else
            typedef ParseNodeVisitor<VisitorComposer<TL3(CleanTree, DirectExecution, AddNestedCalls)> > RewritingVisitor;
#endif
            Assert(IsScriptContextValid());

            // Initialize the context for the composed visitor
            ArenaAllocator localArena(L"ls: DirectedExecution", m_scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            DirectExecutionContext directExecutionContext;
            directExecutionContext.parser = m_primaryTree.GetParser();
            directExecutionContext.target = current;
            directExecutionContext.pnodeFnc = nullptr;
            directExecutionContext.stack = Anew(&localArena, JsUtil::Stack<ParseNode *>, &localArena);

            AddNestedCallsContext nestedCallContext;
            nestedCallContext.offset = current->ichMin;
            nestedCallContext.parser = m_primaryTree.GetParser();

            CleanTreeContext cleanContext = { m_primaryTree.LanguageServiceExtension() };

            RewritingVisitor::VisitorContext context;
            context.h = &cleanContext;
            context.t.h = &directExecutionContext;
            context.t.t.h = &nestedCallContext;

#ifdef DEBUG
            PreValidatorVisitor preValidatorVisitor;
            PreValidatorVisitor::VisitorContext preValidatorContext;
            preValidatorVisitor.DisableCleanTreeValidation();
            preValidatorVisitor.Visit(m_primaryTree.TreeRoot(), preValidatorContext);
#endif
            RewritingVisitor visitor;
            visitor.Visit(m_primaryTree.TreeRoot(), context);
            m_primaryTree.SetMutated(ParseNodeTree::mutDirectExecution);
        }
        METHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendRewriteTreeEnd);
        METHOD_POSTFIX_CLEAN_END;
    }
#else
    // This is a decomposed version of the above method that is useful when debugging individual rewrittings.
    HRESULT FileAuthoring::RewriteExecutionTree(ParseNodePtr current)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendRewriteTreeBegin);
        {
            Assert(IsScriptContextValid());

            // Initialize the context for the composed visitor
            ArenaAllocator localArena(L"ls: DirectedExecution", m_scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            auto root = m_primaryTree.TreeRoot();

            // Prevalidate the tree
            typedef ParseNodeVisitor<TreeValidator> ValidatorVisitor;
            ValidatorVisitor validatorVisitor;
            ValidatorVisitor::VisitorContext validatorContext;
            validatorVisitor.DisableCleanTreeValidation();
            validatorVisitor.Visit(root, validatorContext);

            // Clean the tree
            typedef ParseNodeVisitor<CleanTree> CleanTreeVisitor;
            CleanTreeContext cleanContext = { m_primaryTree.LanguageServiceExtension() };
            CleanTreeVisitor cleanTreeVisitor;
            cleanTreeVisitor.Visit(root, &cleanContext);

            // Validate the tree
            validatorVisitor.EnableCleanTreeValidateion();
            validatorVisitor.Visit(root, validatorContext);

            // Directed execution
            typedef ParseNodeVisitor<DirectExecution> DirectExecutionVisitor;
            DirectExecutionContext directExecutionContext;
            directExecutionContext.parser = m_primaryTree.GetParser();
            directExecutionContext.target = current;
            directExecutionContext.pnodeFnc = nullptr;
            directExecutionContext.stack = Anew(&localArena, JsUtil::Stack<ParseNode *>, &localArena);
            DirectExecutionVisitor directExecutionVisitor;
            directExecutionVisitor.Visit(root, &directExecutionContext);

            // Validate the tree
            validatorVisitor.Visit(root, validatorContext);

            // Add nested calls
            typedef ParseNodeVisitor<AddNestedCalls> AddNestedCallsVisitor;
            AddNestedCallsContext nestedCallContext;
            nestedCallContext.offset = current->ichMin;
            nestedCallContext.parser = m_primaryTree.GetParser();
            AddNestedCallsVisitor addNestedCallsVisitor;
            addNestedCallsVisitor.Visit(root, &nestedCallContext);

            // Validate the tree
            validatorVisitor.Visit(root, validatorContext);

            m_primaryTree.SetMutated(ParseNodeTree::mutDirectExecution);
        }
        METHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendRewriteTreeEnd);
        METHOD_POSTFIX_CLEAN_END;
    }
#endif

    HRESULT FileAuthoring::GetValueOf(ArenaAllocator *localArena, ParseNodePtr pnode, AuthorCompletionDiagnosticFlags *diagFlags, Js::Var& value)
    {
        METHOD_PREFIX;

        Assert(localArena);
        Assert(pnode);

        IfFailGo(TestAbort());

        Assert(IsScriptContextValid());

        IfFailGo(m_primaryFile->AddGuardsAndApplyComments(m_scriptContext, &m_primaryTree, /* isPrimaryFile = */true, pnode->ichMin, /* suppressThrows */true));

        AuthorCompletionDiagnosticFlags tmpDiagFlags = AuthorCompletionDiagnosticFlags(acdfNone);

        // Try executing the entire script and see if we hit the expression.
        // Note: The logic for flagging the final execution pass is approximate. 
        // It may not be correct in the presence of async requests added as a result of directed execution.
        IfFailGo(Execute(pnode, &tmpDiagFlags, /*isFinalPass*/false, &value));

        // Prevent the value from begin collected prematurely.
        if (value) 
            JsHelpers::PreventRecycling(m_scriptContext, value);

        // Check if the execution resulted in new async script requests being added
        JsUtil::List<Js::RecyclableObject*, ArenaAllocator>* asyncRequests = JsUtil::List<Js::RecyclableObject*, ArenaAllocator>::New(localArena);
        IfFailGo(m_primaryFile->GetAsyncRequests(localArena, this, m_scriptContext, nullptr, asyncRequests));

        if (asyncRequests->Count() > 0 && (!value || (value && IsExceptionObject(localArena, m_scriptContext, value))))
        {
            IfFailGo(TestAbort());

            // excute any pending async Requests
            IfFailGo(m_primaryFile->ExecuteAsyncRequests(localArena, this, m_scriptContext, nullptr, asyncRequests));

            IfFailGo(TestAbort());

            // Reset flags in preparation for next execution
            tmpDiagFlags = AuthorCompletionDiagnosticFlags(acdfNone);
            // Try executing the entire script again after executing the async request and see if we hit the expression.
            IfFailGo(Execute(pnode, &tmpDiagFlags, /*isFinalPass*/false, &value));

            // Check if the execution resulted in new async script requests being added
            asyncRequests->Clear();
            IfFailGo(m_primaryFile->GetAsyncRequests(localArena, this, m_scriptContext, nullptr, asyncRequests));
        }

        if (pnode->nop != knopProg && (!value || (value && IsExceptionObject(localArena, m_scriptContext, value))))
        {
            OUTPUT_TRACE(Js::JSLSPhase, L"Directed execution - starting\n");

            IfFailGo(RewriteExecutionTree(pnode));
#if DBG_DUMP
            if (PHASE_TRACE1(Js::DirectExecutionParseTreePhase))
            {
                PrintPnodeWIndent(this->m_primaryTree.TreeRoot(), 0);
            }
#endif
            // Reset flags in preparation for next execution
            tmpDiagFlags = AuthorCompletionDiagnosticFlags(acdfNone);
            // Try executing the now rewritten tree.
            IfFailGo(Execute(pnode, &tmpDiagFlags, /*isFinalPass*/true, &value));

            // Check if the execution resulted in new async script requests being added
            asyncRequests->Clear();
            IfFailGo(m_primaryFile->GetAsyncRequests(localArena, this, m_scriptContext, nullptr, asyncRequests));
            OUTPUT_TRACE(Js::JSLSPhase, L"Directed execution - ending\n");
        }

        // Set diag flags from last execution
        if (diagFlags)
        {
            *diagFlags = AuthorCompletionDiagnosticFlags(*diagFlags | tmpDiagFlags);
        }

        // In case the resulting value is a tracking null or undefined, unwrap it
        if (value)
        {
            bool isTrackingValue = false;
            value = UnwrapUndefined(value, &isTrackingValue);
            if (diagFlags && isTrackingValue)
            {
                *diagFlags = AuthorCompletionDiagnosticFlags(*diagFlags | acdfObjectTypeInferred);
            }
        }

        METHOD_POSTFIX;
    }


    HRESULT FileAuthoring::GetValueAt(long offset, ArenaAllocator *localArena, AuthorCompletionDiagnosticFlags &diagFlags, Js::Var& value, bool leftOfDot)
    {
        METHOD_PREFIX;

        value = NULL;
        ParseNodePtr current = nullptr;
        ParseNode *nameNode = nullptr;

        IfFailGo(TestAbort());

        // Executing the primary file may result in detecting new asynchrony script requests, which means that the context is 
        // not complete. If detected, rebuild the context, and reparse the primary file and retry.
        hr = EnsureQuiescentContext([&]() -> HRESULT
        {
            INTERNALMETHOD_PREFIX;

            IfFailGoInternal(EnsureContext());

            IfFailGoInternal(UpdatePrimary(true));

            ParseNodeCursor* cursor = Anew(localArena, ParseNodeCursor, localArena, &m_primaryTree);
            cursor->SeekToOffset(offset);
            current = cursor->Current();

            if (current)
            {
                nameNode = nullptr;

                // If we are in a name we want the parent expression.
                if (cursor->RightOfDot() && current->nop == knopName)
                    current = cursor->Up();

                // If we are in an dot expression then we want the left hand side.
                if (leftOfDot && current->nop == knopDot && current->sxBin.pnode1) {
                    // We don't need to remember that we are in a dot expression since
                    // the right is probably not valid (the user is int he middle of 
                    // typing it.
                    current = current->sxBin.pnode1;
                }

                if (current && current->nop == knopDot && IsNameNode(current->sxBin.pnode2) && cursor->IsCallOrAssignmentTarget())
                {
                    // If the node is the target of a call or an assignemnt statement the
                    // value will not be loaded (calls of dotted methods, for example, are
                    // called directly with a call-field instruction, and assignment calls
                    // are generated using a instruction field set). In this case we want to 
                    // Get the value of the left-hand side of the dotted expression but 
                    // remember the name 
                    nameNode = current->sxBin.pnode2; // Remember the name.
                    current = current->sxBin.pnode1; // Get the value of the left-hand side of the dot.
                }

                IfFailGoInternal(GetValueOf(localArena, current, &diagFlags, value));
            }

            INTERNALMETHOD_POSTFIX;
        });
        IfFailGo(hr);

        if (current && value && (nameNode || (current->location == NoRegister && current->nop == knopDot)))
        {
            // The node is a dot node and value is the value of the left hand side. If the right is a
            // name or string, look it up like it is a property.
            ParseNode *right = nameNode ? nameNode : current->sxBin.pnode2;
            if (IsNameNode(right))
            {
                IdentPtr name = right->sxPid.pid;

                Assert(IsScriptContextValid());

                Js::PropertyId id = m_scriptContext->GetOrAddPropertyIdTracked(name->Psz(), name->Cch());
                // TODO: Use AsLeftOfDotObject
                Js::RecyclableObject* object = Js::TaggedNumber::Is(value)
                    ? m_scriptContext->GetLibrary()->GetNumberPrototype()
                    : (Js::RecyclableObject*)value;

                Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(object);
                if (typeId != Js::TypeIds_Null && typeId != Js::TypeIds_Undefined)
                {
                    object = Js::RecyclableObject::FromVar(object);
                    IfFailGo(GetPropertyOf(object, id, m_scriptContext, &value));
                }
            }
        }

        METHOD_POSTFIX;
    }

    bool  FileAuthoring::IsExceptionObject(ArenaAllocator *alloc, Js::ScriptContext* scriptContext, Js::Var& var)
    {
        return JsHelpers::IsExceptionObject(alloc, scriptContext, var);
    }

    bool FileAuthoring::IsUndefinedObject(Js::ScriptContext *scriptContext, Js::Var value)
    {
        return value && value == scriptContext->GetLibrary()->GetUndefined();
    }

    bool FileAuthoring::IsNullObject(Js::ScriptContext *scriptContext, Js::Var value)
    {
        return value && value == scriptContext->GetLibrary()->GetNull();
    }

    // Called to unwrap undefined and null values that are standing in the place of another object. This happens, for example, when a <field ...> comment
    // is given for a field but the object has not been initialized yet. By JavaScript semantics, accessing such a property must result in undefined. 
    // The runtime, (throught the language service hooks) creates a new object of undefined type to stand-in for the value of the property so the value 
    // is still undefined but the type information declared in the <field ...> data can be retrieved even when the field access has long since 
    // disappeared. This routine retrives the value that the undefined value is standing in for.
    Js::Var FileAuthoring::UnwrapUndefined(Js::Var value, bool *isTrackingValue)
    {
        Assert(IsScriptContextValid());

        if (isTrackingValue)
        {
            *isTrackingValue = false;
        }

        if (JsHelpers::IsTrackingKeyValue(m_scriptContext, value) && m_scriptContext->authoringData && m_scriptContext->authoringData->Callbacks())
        {
            if (isTrackingValue)
            {
                *isTrackingValue = true;
            }
            return m_scriptContext->authoringData->Callbacks()->GetTrackingValue(m_scriptContext, Js::RecyclableObject::FromVar(value));
        }
        return value;
    }

    void FileAuthoring::AddPropertyCompletion(Completions *completions, Js::Var value, Js::RecyclableObject *object, Js::RecyclableObject *root, Js::PropertyId id, AuthorScope defaultScope)
    {
        try
        {
            if (!completions->Ids()->Test(id))
            {
                Js::Var propVar = nullptr;
                Js::Var propGet = nullptr;
                Js::Var propSet = nullptr;

                // Filter RegExp $* properties
                switch (id)
                {
                case Js::PropertyIds::$_:
                case Js::PropertyIds::$Ampersand:
                case Js::PropertyIds::$Plus:
                case Js::PropertyIds::$BackTick:
                case Js::PropertyIds::$Tick:
                case Js::PropertyIds::$1:
                case Js::PropertyIds::$2:
                case Js::PropertyIds::$3:
                case Js::PropertyIds::$4:
                case Js::PropertyIds::$5:
                case Js::PropertyIds::$6:
                case Js::PropertyIds::$7:
                case Js::PropertyIds::$8:
                case Js::PropertyIds::$9:
                    return;
                };

                AuthorCompletionKind kind = ackField;

                Assert(IsScriptContextValid());

                if (object->GetAccessors(id, &propGet, &propSet, m_scriptContext))
                {
                    kind = ackProperty;
                }
                else
                {
                    try
                    {
                        bool isTrackingValue = false;
                        propVar = UnwrapUndefined(Js::JavascriptOperators::GetProperty(object, id, m_scriptContext), &isTrackingValue);
                        if (isTrackingValue)
                        {
                            completions->SetDiagnosticFlags(acdfMemberTypeInferred);
                        }
                        if (propVar)
                        {
                            Js::JavascriptFunction* func = nullptr;
                            Convert::FromVar(nullptr, propVar, func);
                            if (func)
                                kind = ackMethod;
                            JsHelpers::PreventRecycling(m_scriptContext, propVar);
                        }
                    }
                    catch (Js::JavascriptExceptionObject *)
                    {
                        // An exception was thrown while getting the value of a property, we cannot get the value.
                        propVar = nullptr;
                    }
                }

                int fileId = -1;
                charcount_t offset = 0;
                AuthorScope scope = defaultScope;

                auto offsetsId = GetOrAddPropertyIdFromLiteral(m_scriptContext, Names::offsets);
                Js::Var offsets;
                if (object->GetProperty(object, offsetsId, &offsets, NULL, m_scriptContext) && Js::DynamicObject::Is(offsets))
                {
                    Js::Var offsetVar;
                    auto offsetsObject = Js::DynamicObject::FromVar(offsets);
                    if (offsetsObject && offsetsObject->GetProperty(offsetsObject, id, &offsetVar, NULL, m_scriptContext))
                    {
                        if (Js::TaggedInt::Is(offsetVar))
                        {
                            offset = (charcount_t)Js::TaggedInt::ToInt32(offsetVar);
                            fileId = (int)m_primaryFile->FileId();
                        }
                    }

                    auto parameterId = GetOrAddPropertyIdFromLiteral(m_scriptContext, Names::isParameter);
                    Js::Var isParameter;
                    scope = ascopeLocal;
                    kind = ackVariable;
                    if (object->GetProperty(object, parameterId, &isParameter, NULL, m_scriptContext) && Js::DynamicObject::Is(isParameter))
                    {
                        Js::Var isParameterResult;
                        if (Js::DynamicObject::FromVar(isParameter)->GetProperty(isParameter, id, &isParameterResult, NULL, m_scriptContext))
                        {
                            scope = ascopeParameter;
                            kind = ackParameter;
                        }
                    }
                }

                auto hintInfo = Anew(completions->Alloc(), HintInfo, scope, atUnknown, fileId, offset);
                completions->AddUnique((kind == ackProperty || kind == ackField || kind == ackMethod) ? root : nullptr, propVar, kind, acfMembersFilter, id, nullptr, hintInfo);
            }
            else
            {
                completions->UpdateFlags(id, acfMembersFilter);
            }
        }
        catch (Js::JavascriptExceptionObject *)
        {
            // An exception was thrown while fetching the value of a property
            // Eat if this is javascript exception.
        }
    }

    bool IsWithId(Js::ScriptContext *scriptContext, Js::PropertyId id)
    {
        auto name = scriptContext->GetPropertyName(id);
        auto const length = LengthOfLiteral(Names::with);
        return name && name->GetLength() == (length + 2) && wcsncmp(Names::with, name->GetBuffer(), length) == 0;
    }

    HRESULT FileAuthoring::AddCompletions(Js::Var value, Completions *completions, bool ownOnly, bool excludeObjectPrototype, bool excludeConstructor, bool checkWith, AuthorScope defaultScope, PropertyFilter propertyFilter)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
        BEGIN_ENTER_SCRIPT(m_scriptContext, false, false, false)
        {
            IfFailGo(AddCompletionsImpl(value, completions, ownOnly, excludeObjectPrototype, excludeConstructor, checkWith, defaultScope, propertyFilter));
        }
        END_ENTER_SCRIPT
        END_TRANSLATE_SO_OOM_JSEXCEPTION(hr)

        METHOD_POSTFIX;
    }

    template< size_t N >
    void UpdateIdIfNamePrefixedBy(Js::ScriptContext *scriptContext, const Js::PropertyRecord *name, const wchar_t (&w)[N], Js::PropertyId &id)
    {
        auto const length = N - 1;
        if (name->GetLength() > length && wcsncmp(w, name->GetBuffer(), length) == 0)
        {
            id = scriptContext->GetOrAddPropertyIdTracked(name->GetBuffer() + length, name->GetLength() - length);
        }
    }

    HRESULT FileAuthoring::AddCompletionsImpl(Js::Var value, Completions *completions, bool ownOnly, bool excludeObjectPrototype, bool excludeConstructor, bool checkWith, AuthorScope defaultScope, PropertyFilter propertyFilter)
    {
        METHOD_PREFIX;
        
        bool isTrackingValue = false;
        value = UnwrapUndefined(value, &isTrackingValue);
        if (isTrackingValue)
        {
            completions->SetDiagnosticFlags(acdfMemberTypeInferred);
        }

        Assert(IsScriptContextValid());

        Js::RecyclableObject *rootObject;
        if (Js::TaggedNumber::Is(value))
            rootObject = m_scriptContext->GetLibrary()->GetNumberPrototype();
        else
            rootObject = Js::RecyclableObject::FromVar(value);

        if (rootObject->GetTypeId() == Js::TypeIds_String)
            rootObject = m_scriptContext->GetLibrary()->GetStringPrototype();

        if (rootObject->GetTypeId() == Js::TypeIds_Function)
            AddStaticFieldsDoc(completions->Alloc(), static_cast<Js::JavascriptFunction*>(rootObject));
        
        // Prevent the value from being lost during garbage collection
        JsHelpers::PreventRecycling(m_scriptContext, value);

        Js::Var enumeratorVar;
        auto undefined = m_scriptContext->GetLibrary()->GetUndefined();
        auto object = rootObject;
        while (object != nullptr && Js::JavascriptOperators::GetTypeId(object) != Js::TypeIds_Null)
        {
            if (object->GetEnumerator(true, &enumeratorVar, m_scriptContext, false, false))
            {
                uint index = 0;
                Js::Var propertyName;
                Js::JavascriptEnumerator *enumerator = Js::JavascriptEnumerator::FromVar(enumeratorVar);
                Assert(enumerator);
                while (enumerator->MoveNext())
                {
                    Js::PropertyId id;
                    uint32 indexVal;

                    if (!enumerator->GetCurrentPropertyId(&id))
                    {
                        Js::Var obj = enumerator->GetCurrentIndex();
                        if ( !Js::JavascriptString::Is(obj) ) //This can be undefined
                        {
                            continue;
                        }
                        Js::JavascriptString *pString = (Js::JavascriptString *)obj;
                        id = m_scriptContext->GetOrAddPropertyIdTracked(pString->GetSz(), pString->GetLength());
                    }

                    // Skip NoProperty and numeric properties.
                    if (id == Js::Constants::NoProperty || m_scriptContext->IsNumericPropertyId(id, &indexVal))
                        continue;

                    if (checkWith && IsWithId(m_scriptContext, id))
                    {
                        Js::Var propValue;
                        if (object->GetProperty(object, id, &propValue, NULL, m_scriptContext) && propValue)
                        {
                            IfFailGo(AddCompletionsImpl(propValue, completions, /* ownOnly = */false, /*excludeObjectPrototype = */false, /* excludeConstructor = */false, /* checkWith = */false, defaultScope, PropertyFilter()));
                        }
                    }
                    else
                    {
                        auto name = m_scriptContext->GetPropertyName(id);
                        if (InternalName(name->GetBuffer()))
                        {
                            // For _$fieldDoc$ properties add the property it is documenting if that property
                            // doesn't already exist. This allows <field ...> doc comments to add fields to
                            // the completions list that have not appeared in the resulting object yet.
                            UpdateIdIfNamePrefixedBy(m_scriptContext, name, Names::fieldDocPrefix, id);
                        }
                        if (!propertyFilter(name, Names::fieldDocPrefix))
                        {
                            if (!excludeConstructor || id != Js::PropertyIds::constructor)
                            {
                                AddPropertyCompletion(completions, value, object, rootObject, id, defaultScope);
                            }
                        }
                    }
                }

                // Check for special non-enumerable properties
                index = 0;
                while(object->GetSpecialPropertyName(index, &propertyName, m_scriptContext))
                {
                    if (!Js::JavascriptOperators::IsUndefinedObject(propertyName, undefined))
                    {
                        Js::JavascriptString *str = Js::JavascriptString::FromVar(propertyName);
                        if (str)
                        {
                            Js::PropertyId id = m_scriptContext->GetOrAddPropertyIdTracked(str->UnsafeGetBuffer(), str->GetLength());
                            AddPropertyCompletion(completions, value, object, rootObject, id, defaultScope);
                        }
                    }
                    index++;
                }

            }

            if (ownOnly)
            {
                break;
            }

            object = object->GetPrototype();

            if (excludeObjectPrototype && object == object->GetLibrary()->GetObjectPrototype())
            {
                break;
            }
        }

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::GetFuncDocComments(ArenaAllocator* alloc, Js::JavascriptFunction* func, FunctionDocComments** funcDoc)
    {
        METHOD_PREFIX;

        Assert(alloc != nullptr);
        Assert(func != nullptr);
        Assert(funcDoc != nullptr);

        AuthoringFileHandle* file = nullptr;

        Assert(IsScriptContextValid());

        Js::ScriptContext* scriptContext = m_scriptContext;
        IfFailGo(FindFunctionSource(alloc, func, file));
        if (file != nullptr)
        {
            // Get function doc comments string
            auto body = func->GetParseableFunctionInfo();
            if (body == nullptr)
            {
                goto Error;
            }

            CommentBuffer* functionComments = file->GetFunctionComments(alloc, scriptContext, func, commenttypeAnyDoc);
            LPCWSTR functionCommentsText = functionComments == nullptr ? nullptr : (functionComments->GetBuffer() == nullptr ? nullptr : functionComments->Sz());
            if (functionCommentsText == nullptr)
            {
                // no buffer available for parsing
                goto Error;
            }

            // Parse function doc comments
            IfFailGo(ParseFuncDocComments(alloc, functionCommentsText, functionComments->GetCommentType(), funcDoc));
        }

        METHOD_POSTFIX;
    }

    // Constructor functions may have <field static='true'> elements specified in doc comments.
    // We want this documentation to appear in completion hint for function object fields.
    // For example:
    // function Win() {
    //     /// <field name='MaxSize' static='true' type='Number'>Max size field description</field>
    // }
    // Win.MaxSize = 1024;
    // Win.MaxSize|   <== The completion hint should show the info specified in the <field> element.
    HRESULT FileAuthoring::AddStaticFieldsDoc(ArenaAllocator* alloc, Js::JavascriptFunction* func)
    {
        METHOD_PREFIX;
        Assert(alloc != nullptr);
        Assert(func != nullptr);

        Assert(IsScriptContextValid());

        Js::ScriptContext* scriptContext = m_scriptContext;
        if (!JsHelpers::GetProperty<bool>(func, Names::staticFieldsDocApplied, nullptr, scriptContext))
        {
            // Try to get function doc comments to see if we have any <field static='true'> entries.
            // For each entry, add field doc comments to the function object. 

            FunctionDocComments* funcDoc = nullptr;
            IfFailGo(this->GetFuncDocComments(alloc, func, &funcDoc));

            if (funcDoc == nullptr)
            {
                goto Error;
            }

            ListOperations::ForEach(&funcDoc->fields, [&](int index, FieldDocComments* fieldDoc)
            {
                Assert(fieldDoc != nullptr);
                if (fieldDoc->isStatic && !String::IsNullOrEmpty(fieldDoc->name))
                {
                    auto valueDoc = JsValueDoc::FromDocComments(alloc, fieldDoc);
                    valueDoc->SetFieldDoc(alloc, func, fieldDoc->name, scriptContext);
                }
            });

            // Indicate that static fields doc comments were applied to that function
            JsHelpers::SetField(func, Names::staticFieldsDocApplied, true, scriptContext, false);
        }

        METHOD_POSTFIX;
    }

    struct AssignmentCompletionVisitor : public VisitorPolicyBase<IdentifierContext *>
    {
    protected:
        bool Preorder(ParseNode *pnode, Context context)
        {
            ParseNodePtr leftHandSide;

            switch(pnode->nop)
            {
            case knopAsg:
            case knopAsgAdd:
            case knopAsgSub:
            case knopAsgMul:
            case knopAsgDiv:
            case knopAsgMod:
            case knopAsgAnd:
            case knopAsgXor:
            case knopAsgOr:
            case knopAsgLsh:
            case knopAsgRsh:
            case knopAsgRs2:
                leftHandSide = pnode->sxBin.pnode1;
                Assert(leftHandSide != nullptr);
                while (leftHandSide->nop == knopDot || leftHandSide->nop == knopIndex)
                {
                    if (leftHandSide->sxBin.pnode1->nop != knopThis)
                        leftHandSide = leftHandSide->sxBin.pnode1;
                    else
                        return false; // ignore assignments to "this" as we cannot determine which object it refers to without executing
                }

                if (leftHandSide->nop == knopName)
                {
                    // TODO: basipov - consider providing completion hint for this case
                    context->AddPid(leftHandSide->sxPid.pid, ackVariable, acfMembersFilter, nullptr);
                    return false;
                }

                Assert (true); // should not reach here
                break;
            case knopFncDecl:
                return false; // do not walk function declarations
            }
            return true;
        }
    };

    HRESULT FileAuthoring::AddSymbolsAt(long offset, ArenaAllocator *localArena, Completions *completions)
    {
        METHOD_PREFIX;

        // Executing the primary file may result in detecting new asynchrony script requests, which means that the context is 
        // not complete. If detected, rebuild the context, and reparse the primary file and retry.
        hr = EnsureQuiescentContext([&]()->HRESULT
        {
            INTERNALMETHOD_PREFIX;

            if (m_polluted)
                ReleasePrimaryContext();

            IfFailGoInternal(ApplyContext());
            IfFailGoInternal(UpdatePrimary(true));

            {
                ParseNodeCursor cursor(localArena, &m_primaryTree);
                cursor.SeekToOffset(offset);
                ParseNodePtr current = cursor.Current();

                if (current)
                {
                    IfFailGoInternal(AddSymbolsOf(localArena, &cursor, completions));
                }
            }

            INTERNALMETHOD_POSTFIX;
        });
        IfFailGo(hr);

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::AddSymbolsOf(ArenaAllocator *localArena, ParseNodeCursor *cursor, Completions *completions)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        ParseNodePtr scopeRecord = nullptr;
        IfFailGo(CreateScopeRecord(localArena, m_primaryTree.GetParser(), cursor, &scopeRecord));

        if (scopeRecord)
        {
            Js::Var value;
            AuthorCompletionDiagnosticFlags diagFlags = AuthorCompletionDiagnosticFlags(acdfNone);
            IfFailGo(GetValueOf(localArena, scopeRecord, &diagFlags, value));
            completions->SetDiagnosticFlags(diagFlags);
            if (value)
                IfFailGo(AddCompletions(value, completions, /* ownOnly = */true, /* excludeObjectPrototype = */false, /* excludeConstructor = */false, /* checkWith = */true, AuthorScope::ascopeLocal, PropertyFilter()));
        }

        Assert(IsScriptContextValid());

        // Add the symbols from the global scope.
        auto rootObject = m_scriptContext->GetGlobalObject();

        IfFailGo(AddCompletions(rootObject, completions, /* ownOnly = */true, /* excludeObjectPrototype = */false, /* excludeConstructor = */false, /* checkWith = */false, AuthorScope::ascopeGlobal, PropertyFilter()));

        // Add let/const globals
        rootObject->MapLetConstGlobals([&](const Js::PropertyRecord* propertyRecord, Js::Var value, bool isConst)
        {
            if (!rootObject->GetScriptContext()->IsUndeclBlockVar(value))
            {
                AddPropertyCompletion(completions, value, rootObject, rootObject, propertyRecord->GetPropertyId(), AuthorScope::ascopeGlobal);
            }
        });

        METHOD_POSTFIX;
    }

    template <class Handler>
    struct ForEachPolicy: WalkerPolicyBase<bool, Handler> 
    {
        inline bool ContinueWalk(bool result) { return result; }
        bool WalkFirstChild(ParseNode *&pnode, Context context) { return context(pnode); }
        bool WalkSecondChild(ParseNode *&pnode, Context context) { return context(pnode); }
        bool WalkNthChild(ParseNodePtr parentNode, ParseNode *&pnode, Context context) { return context(pnode); }
    };
    template <class Handler>
    void ForEachChildNode(ParseNodePtr pnode, Handler callback)
    {
        ParseNodeWalker<ForEachPolicy<Handler>> walker;
        walker.Walk(pnode, callback);
    }

    enum SymbolScopeKind
    {
        sskParameter,
        sskLocal,
        sskWith,
    };

    struct ScopeSymbol
    {
        IdentPtr name;
        SymbolScopeKind kind;
        charcount_t ichMin;
        charcount_t ichLim;

        ScopeSymbol(IdentPtr name, SymbolScopeKind kind, charcount_t ichMin, charcount_t ichLim): 
            name(name), kind(kind), ichMin(ichMin), ichLim(ichLim) { }
    };

    
    template <typename THandler, typename TRecurse>
    void ProcessScopes(ParseNodePtr pnodeScopes, THandler handler, TRecurse recurseBlock)
    {
        auto current = pnodeScopes;
        while(current)
        {
            Assert(ASTHelpers::Scope::IsScopeNode(current));
            handler(current);
            if (current->nop == knopBlock)
            {
                if (recurseBlock(current))
                {
                    ProcessScopes(current->sxBlock.pnodeScopes, handler, recurseBlock);
                }
            }
            current = *ASTHelpers::Scope::GetNextRef(current);
        }
    }

    template <typename THandler, typename TRecurse>
    void ProcessBlockAndScopes(ParseNodePtr block, THandler handler, TRecurse recurseBlock)
    {
        handler(block);
        if (block->sxBlock.pnodeScopes)
            ProcessScopes(block->sxBlock.pnodeScopes, handler, recurseBlock);
    }

    // Construct an object literal similar that contains all the values in scope as members capturing their current values.
    // So in a function such as,
    //
    //  function bar(p_a) { var p_b; function foo(a, b) { var c = 1; with (d) { | } } }
    //
    // would get transformed into 
    //
    //  function bar(p_a) { var p_b; function foo(a, b) { var _$with_1; var c = 1; with(_$with_1 = d) { { _$with_1: _$with_1, c: c, b: b, a: a, p_b: p_b, p_a: p_a, $isParameter: {a:true, b:true, p_a: true} } }
    //
    HRESULT FileAuthoring::CreateScopeRecord(ArenaAllocator *localArena, Parser *parser, ParseNodeCursor *cursor, __out ParseNodePtr *scopeRecord)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        {
            BVSparse<ArenaAllocator> ids(localArena);
            JsUtil::List<ScopeSymbol *, ArenaAllocator> symbols(localArena, 16);
            JsUtil::List<IdentPtr, ArenaAllocator> withVariables(localArena, 16);

            int withCounter = 1;
            wchar_t withTemp[] = L"_$with_ ";
            Assert(AllocSize(withTemp) - 2 == 7); 
            wchar_t* withTempSlot = &withTemp[7]; // Using 7 instead of AllocSize(withTemp) - 2 here because using AllocSize() confuses PreFix.
            ParseNodePtr parent = nullptr;

            Assert(IsScriptContextValid());

            // Nested procedure to add an element to the symbol list.
            auto add = [&](IdentPtr name, SymbolScopeKind kind, ParseNode *node, ParseNode* parent, charcount_t ichMin, charcount_t ichLim) {
                auto id = m_scriptContext->GetOrAddPropertyIdTracked(name->Psz(), name->Cch());
                if (!ids.TestAndSet(id))
                {
                    if (node)
                    {
                        m_primaryFile->RememberSingleNodeComments(localArena, /* searchLimOverride = */0, node, parent, /* previousNode = */nullptr, /* searchPreviousNode = */true, ichMin);
                    }

                    symbols.Add(Anew(localArena, ScopeSymbol, name, kind, ichMin, ichLim));
                }
            };

            // Nested function to determine if the given node contians the cursor.
            auto containsCursor = [&](ParseNodePtr pnode) -> bool { return pnode && InRange(cursor->Offset(), ActualMin(pnode), ActualLim(pnode)); };
            ParseNodePtr start = cursor->Current();
            ParseNodePtr startParent = cursor->Parent();

            auto addLocalDecl = [&](ParseNodePtr decl, ParseNodePtr current) -> void {
                Assert(decl->nop == knopVarDecl || decl->nop == knopConstDecl || decl->nop == knopLetDecl);
                auto lsExtension = parser->GetLanguageServiceExtension();
                auto nameMin = lsExtension->IdentMin(decl);
                auto nameLim = lsExtension->IdentLim(decl);
                if (!nameMin) nameMin = decl->ichMin;
                if (!nameLim) nameLim = decl->ichLim;
                if (!decl->sxVar.isBlockScopeFncDeclVar)
                {
                    add(decl->sxVar.pid, sskLocal, decl, current, nameMin, nameLim);
                }
            };
            
            // Find all the symbols in scope and store them in a symbol array marking what
            // kind of symbol it is (a local, parameter or a with expression).

            for (ParseNodePtr current = cursor->Current(); current; current = cursor->Up())
            {
                auto processScope = [&](ParseNodePtr scope) {
                    switch (scope->nop)
                    {
                    case knopFncDecl:
                        if (scope->sxFnc.IsDeclaration() && scope->sxFnc.pid)
                        {
                            add(scope->sxFnc.pid, sskLocal, scope, current, scope->ichMin, scope->ichLim);
                        }
                        break;
                    case knopBlock:
                        if (containsCursor(scope))
                        {
                            for (ParseNodePtr lexvar = scope->sxBlock.pnodeLexVars; lexvar != null; lexvar = lexvar->sxVar.pnodeNext)
                            {
                                addLocalDecl(lexvar, current);
                            }
                        }
                    }
                };
                switch (current->nop)
                {
                case knopFncDecl:
                    // Add all the function arguments, variables and function declarations to the scope.
                    for (ParseNodePtr arg = current->sxFnc.pnodeArgs; arg; arg = arg->sxVar.pnodeNext)
                    {
                        add(arg->sxVar.pid, sskParameter, arg, current, arg->ichMin, arg->ichLim);
                    }
                    if (current->sxFnc.pnodeRest != nullptr)
                    {
                        ParseNode *arg = current->sxFnc.pnodeRest;
                        add(arg->sxVar.pid, sskParameter, arg, current, arg->ichMin, arg->ichLim);
                    }

                    for (ParseNodePtr var = current->sxFnc.pnodeVars; var; var = var->sxVar.pnodeNext)
                    {
                        addLocalDecl(var, current);
                    }

                    if (current->sxFnc.pnodeScopes)
                    {
                        ProcessScopes(current->sxFnc.pnodeScopes, processScope, containsCursor);
                    }

                    // If the function is a function expression that has a name the name can be used to
                    // is in scope and can be used to recusively call the function.
                    if (!current->sxFnc.IsDeclaration() && current->sxFnc.pnodeNames && current->sxFnc.pnodeNames->nop == knopVarDecl)
                    {
                        add(current->sxFnc.pnodeNames->sxVar.pid, sskLocal, current->sxFnc.pnodeNames, current,
                            current->sxFnc.pnodeNames->ichMin, current->sxFnc.pnodeNames->ichLim);
                    }

                    // If any with variables have been added in scope then add them this function before we leave.
                    // This is intentionally skipped for the knopProg because the temporaries will be added to the
                    // global scope anyway, no need to declare them.
                    for (int i = 0; i < withVariables.Count(); i++)
                    {
                        IdentPtr name = withVariables.Item(i);
                        ParseNodePtr decl = parser->CreateNode(knopVarDecl);
                        decl->sxVar.InitDeclNode(name, nullptr);
                        decl->sxVar.pnodeNext = current->sxFnc.pnodeVars;
                        current->sxFnc.pnodeVars = decl;
                        ParseNodePtr *endCodeReference = FindEndCodeRef(current);
                        if (endCodeReference)
                        {
                            ApplyLocation(decl, (*endCodeReference)->ichMin);
                            *endCodeReference = parser->CreateBinNode(knopList, decl, *endCodeReference);
                        }
                    }
                    withVariables.Clear();
                    break;

                case knopClassDecl:
                    if (current->sxClass.isDeclaration) {
                        Assert(current->sxClass.pnodeDeclName && current->sxClass.pnodeDeclName->nop == knopLetDecl);
                        addLocalDecl(current->sxClass.pnodeDeclName, current);
                    }
                    break;

                case knopSwitch:
                    if (current->sxSwitch.pnodeBlock)
                        ProcessBlockAndScopes(current->sxSwitch.pnodeBlock, processScope, containsCursor);
                    break;

                case knopBlock:
                    ProcessBlockAndScopes(current, processScope, containsCursor);
                    break;

                case knopForIn:
                case knopForOf:
                    // The ForIn or ForOF might be in global scope so this knopVarDecl might not be processed so do it here.
                    if (current->sxForInOrForOf.pnodeLval && current->sxForInOrForOf.pnodeLval->nop == knopVarDecl)
                    {
                        add(current->sxForInOrForOf.pnodeLval->sxVar.pid, sskLocal, current->sxForInOrForOf.pnodeLval, current,
                            current->sxForInOrForOf.pnodeLval->ichMin, current->sxForInOrForOf.pnodeLval->ichLim);
                    }
                    break;

                case knopCatch:
                    // Add the catch parameter to the scope if there was one.
                    if (current->sxCatch.pnodeParam)
                    {
                        add(current->sxCatch.pnodeParam->sxPid.pid, sskLocal, current->sxCatch.pnodeParam, current,
                            current->sxCatch.pnodeParam->ichMin, current->sxCatch.pnodeParam->ichLim);
                    }
                    break;

                case knopWith:
                    // Add the with expression by copying it into a unique local variable and then 
                    // capturing the local variable into the scope object.
                    if (current->sxWith.pnodeObj)
                    {
                        // Create a fresh temporary
                        *withTempSlot = (wchar_t)(withCounter++);
                        IdentPtr withTempName = CreatePidFromLiteral(parser, withTemp);
                        withVariables.Add(withTempName);
                        add(withTempName, sskWith, nullptr, nullptr, 0, 0);

                        // Replace the with expression with an assignment to the variable to capture the with value.
                        current->sxWith.pnodeObj = parser->CreateBinNode(knopAsg, 
                            parser->CreateNameNode(withTempName, current->sxWith.pnodeObj->ichMin, current->sxWith.pnodeObj->ichMin), 
                            current->sxWith.pnodeObj);
                    }
                }

                auto betterParent = [&](ParseNodePtr node) {
                    return containsCursor(node) && (node->nop == knopList || node->nop == knopBlock) ? node : nullptr; 
                };
                if ( !parent && (current != start || current->nop == knopWith || current->nop == knopProg) ) 
                {
                    // Record the parent if we see a good potential parent.
                    switch (current->nop)
                    {
                        // Exclude the following if the cursor is not in the body of the statement (e.g. it is in an expression or initializer)
                    case knopWith:
                        if (containsCursor(current->sxWith.pnodeObj)) continue;
                        parent = betterParent(current->sxWith.pnodeBody);
                        if (!parent) continue; // If it is not in the body, ignore with as a parent.
                        break;

                    case knopBlock:
                        parent = betterParent(current->sxBlock.pnodeStmt);
                        break;

                    case knopFncDecl: 
                    case knopProg:
                        parent = betterParent(current->sxFnc.pnodeBody);
                        break;

                    case knopCase:
                        // A knopCase cannot be a parent unless it already has a body.
                        if (!current->sxCase.pnodeBody) 
                            continue;

                        parent = betterParent(current->sxCase.pnodeBody);
                        break;

                    case knopTry:
                        parent = betterParent(current->sxTry.pnodeBody);
                        break;

                    case knopCatch:
                        parent = betterParent(current->sxCatch.pnodeBody);
                        break;

                    case knopFinally:
                        parent = betterParent(current->sxFinally.pnodeBody);
                        break;

                    default: continue;
                    }

                    if (!parent)
                        parent = current;
                }
            }

            // If we don't find a parent then global scope is sufficient for the completion list because we are in the context of the global scope, not nested in a with or function.
            // If we don't find any symbols the global scope is sufficient as well.
            ParseNodePtr result = nullptr;
            if (parent)
            {
                // Create the scope record
                result = parser->CreateUniNode(knopObject, nullptr);
                ParseNodePtr *literalMembers = &result->sxUni.pnode1;
                auto append = [&](ParseNodePtr pnode, ParseNodePtr *&members) {
                    if (*members == nullptr)
                    {
                        *members = pnode;
                    }
                    else 
                    {
                        *members = parser->CreateBinNode(knopList, *members, pnode);
                        members = &(*members)->sxBin.pnode2;
                    }
                };

                ParseNodePtr *parametersMembers = nullptr;
                ParseNodePtr *offsetMembers = nullptr;
                for (int i = 0; i < symbols.Count(); i++)
                {
                    ScopeSymbol *symbol = symbols.Item(i);
                    ParseNodePtr member = parser->CreateBinNode(knopMember, parser->CreateNameNode(symbol->name), parser->CreateNameNode(symbol->name));
                    append(member, literalMembers);

                    // If the symbol is a parameter record that in the _$isParameters record.
                    if (symbol->kind == sskParameter)
                    {
                        if (!parametersMembers)
                        {
                            ParseNodePtr parameters = parser->CreateUniNode(knopObject, nullptr);
                            parametersMembers = &parameters->sxUni.pnode1;
                            append(parser->CreateBinNode(knopMember, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::isParameter)), parameters), literalMembers);
                        }

                        append(parser->CreateBinNode(knopMember, parser->CreateNameNode(symbol->name), parser->CreateNode(knopTrue)), parametersMembers);                
                    }

                    // Record the offset of the node that defines the symbol in _$offsets
                    if (symbol->ichMin || symbol->ichLim)
                    {
                        // If the symbol's span has a non-zero min offset or a non-zero lim offset 
                        // (in the case that the symbol begins at offset 0), record the definition.
                        if (!offsetMembers)
                        {
                            ParseNodePtr offsets = parser->CreateUniNode(knopObject, nullptr);
                            offsetMembers = &offsets->sxUni.pnode1;
                            append(parser->CreateBinNode(knopMember, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::offsets)), offsets), literalMembers);
                        }
                        append(parser->CreateBinNode(knopMember, parser->CreateNameNode(symbol->name), parser->CreateIntNode((long)symbol->ichMin)), offsetMembers);
                    }
                }

                // make sure the list is a statement list, and not a list of array values or object memebers
                auto isStatementList =  [&](ParseNodePtr pnode, ParseNodePtr parent) {
                    return pnode && pnode->nop == knopList && parent && parent->nop != knopArray && parent->nop != knopObject;
                };

                if (start->nop == knopList && containsCursor(start) && isStatementList(start, startParent))
                    // If we are asked for a completion in between statements we end up in the list that contains the statements we are between. This handles this case.
                    parent = start;

                // Find the node in the parent that contains the cursor or is just in front of the cursor.
                ParseNodePtr *last = nullptr;
                ParseNodePtr *child = nullptr;
                ForEachChildNode(parent, [&](ParseNodePtr &childNode) -> bool {
                    if (containsCursor(childNode) || (charcount_t)ActualMin(childNode) > cursor->Offset())
                    {
                        child = &childNode;
                        return false;
                    }
                    last = &childNode;
                    return true;
                });
                if (!child) child = last;

                // Assign the scope record to a global to ensure it is generated.
                ParseNodePtr asgScope = parser->CreateBinNode(knopAsg, parser->CreateNameNode(CreatePidFromLiteral(parser, Names::scope)), result);

                // If this is a catch statement's parameter then put the scope in the catch.
                if (parent->nop == knopCatch && parent->sxCatch.pnodeParam == start)
                {
                    auto block = parent->sxCatch.pnodeBody;
                    Assert(block && block->nop == knopBlock);
                    ApplyLocation(asgScope, block->ichMin);
                    if (!block->sxBlock.pnodeStmt)
                        block->sxBlock.pnodeStmt = asgScope;
                    else
                        block->sxBlock.pnodeStmt = parser->CreateBinNode(knopList, asgScope, block->sxBlock.pnodeStmt);
                }
                else if (parent->nop == knopCase && parent->sxCase.pnodeExpr == start)
                {
                    if (!parent->sxCase.pnodeBody)
                    {
                        ApplyLocation(asgScope, parent->ichLim);
                        parent->sxCase.pnodeBody = asgScope;
                    }
                    else
                    {
                        ApplyLocation(asgScope, parent->sxCase.pnodeBody->ichMin);
                        parent->sxCase.pnodeBody = parser->CreateBinNode(knopList, asgScope, parent->sxCase.pnodeBody);
                    }
                }
                // Place the scope record after the child by replacing with a node containing both.
                else if (child)
                {
                    if ((charcount_t)ActualMin(*child) < cursor->Offset() && (*child)->nop != knopEndCode)
                    {
                        ApplyLocation(asgScope, ActualLim(*child));
                        *child = parser->CreateBinNode(knopList, *child, asgScope);
                    }
                    else
                    {
                        bool wasEndCode = (*child)->nop == knopEndCode;
                        ApplyLocation(asgScope, cursor->Offset());
                        *child = parser->CreateBinNode(knopList, asgScope, *child);
                        if (wasEndCode)
                        {
                            (*child)->ichLim = cursor->Offset();
                        }
                    }
                }
                else if (parent->nop == knopWith && (!parent->sxWith.pnodeBody || parent->sxWith.pnodeBody->nop == knopBlock))
                {
                    ApplyLocation(asgScope, parent->ichLim);
                    parent->sxWith.pnodeBody = asgScope;
                }
                else if (parent->nop == knopBlock && !parent->sxBlock.pnodeStmt)
                {
                    ApplyLocation(asgScope, parent->ichMin);
                    parent->sxBlock.pnodeStmt = asgScope;
                }
                else
                {
                    // This shouldn't happen since we should at least find the node that cursor originally referred to.
                    // The special case above is for empty with statements, all other empty statements the scope record
                    // can be placed after the statement instead of in it. If the statement is empty then cursor will
                    // be pointing to it and we specifically exclude the node cursor is pointing to first as a parent
                    // unless it is a with.
                    Assert(false);
                    result = nullptr;
                }
            }

            *scopeRecord = result;
        }

        METHOD_POSTFIX;
    }

    void UpdateExtent(Completions *completions, ParseNode *pnode, LanguageServiceExtension *extensions)
    {
        if (pnode)
        {
            auto min = pnode->ichMin;
            auto lim = pnode->ichLim;
            if (pnode->nop == knopName)
            {
                auto actualMin = extensions->LParen(pnode);
                auto actualLim = extensions->RParen(pnode);
                if (actualMin && actualLim)
                {
                    min = actualMin;
                    lim = actualLim;
                }
            }
            completions->SetExtent(min, lim - min);
        }
    }

    // Get the symbols accessible from offset
    HRESULT FileAuthoring::GetSymbolCompletions(long offset, ArenaAllocator *localArena, Completions *completions, __out Js::Var* symbolValue)
    {
        METHOD_PREFIX;

        Assert(completions);
        Assert(symbolValue);

        *symbolValue = nullptr;

        if (m_polluted)
            ReleasePrimaryContext();

        IfFailGo(TestAbort());

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));

        {
            ParseNodeCursor cursor(localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);
            ParseNodePtr current = cursor.Current();

            if (current)
            {
                if (IdentifierLikeNode(current))
                {
                    UpdateExtent(completions, current, m_primaryTree.LanguageServiceExtension());
                }

                // If the node is a knopName that is right of a knopDot or a knopDot expression then we need to evaluate the expression
                // to determine the result. Otherwise we just need to collect the symbols in lexical scope.
                if (cursor.RightOfDot())
                {
                    Js::Var value;
                    AuthorCompletionDiagnosticFlags diagFlags = AuthorCompletionDiagnosticFlags(acdfNone);
                    IfFailGo(GetValueAt(offset, localArena, diagFlags, value, true));
                    completions->SetDiagnosticFlags(diagFlags);
                    if (value)
                    {
                        *symbolValue = value;
                        IfFailGo(AddCompletions(value, completions, /* ownOnly = */false, /* excludeObjectPrototype = */false, /* excludeConstructor = */false, /* checkWith = */false, AuthorScope::ascopeMember, PropertyFilter()));

                        Assert(IsScriptContextValid());

                        if (IsExceptionObject(localArena, m_scriptContext, value))
                            completions->SetObjectKind(acokError);
                        else if (IsUndefinedObject(m_scriptContext, value))
                            completions->SetObjectKind(acokUndefined);
                        else if (IsNullObject(m_scriptContext, value))
                            completions->SetObjectKind(acokNull);
                        else
                            completions->SetObjectKind(acokDynamic);
                    }
                }
                else
                {
                    IfFailGo(AddSymbolsAt(offset, localArena, completions));
                    completions->SetKind(acskStatement);
                }
            }
        }

        METHOD_POSTFIX;
    }

    // Get the available completion options for literals
    HRESULT FileAuthoring::GetObjectLiteralCompletions(long offset, ArenaAllocator *localArena, Completions *completions, bool* objectLiteralCompletionAvailable)
    {
        METHOD_PREFIX;

        Assert(completions);
        Assert(objectLiteralCompletionAvailable);

        *objectLiteralCompletionAvailable = false;

        if (m_polluted)
            ReleasePrimaryContext();

        IfFailGo(TestAbort());

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));

        {
            ParseNodeCursor cursor(localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);
            ParseNodePtr current = cursor.Current();

            if (current)
            {
                if (IdentifierLikeNode(current))
                {
                    UpdateExtent(completions, current, m_primaryTree.LanguageServiceExtension());
                }

                JsUtil::Stack<LPCWCHAR> members(localArena);
                JsUtil::List<LPCWCHAR, ArenaAllocator> usedProperties(localArena);
                ParseNode* callNode = cursor.Up(IsInCallArgs(offset, m_primaryTree.LanguageServiceExtension(), &members, &usedProperties), InFuncBodyOrObject(offset, m_primaryTree.LanguageServiceExtension(), /* stopOnObjectLiteral = */false));

                if (callNode != nullptr)
                {
                    int paramIndexOfCall = this->GetArgumentIndex(callNode, offset);

                    Js::Var callTargetValue = nullptr;
                    Js::RecyclableObject* parentObject = nullptr;

                    hr = EnsureQuiescentContext([&]()->HRESULT
                    {
                        INTERNALMETHOD_PREFIX;

                        IfFailGoInternal(GetFunctionValueAt(offset, localArena, callTargetValue, parentObject));

                        INTERNALMETHOD_POSTFIX;
                    });
                    IfFailGo(hr);

                    Js::JavascriptFunction* function = nullptr;
                    if (callTargetValue != nullptr && Convert::FromVar(nullptr, callTargetValue, function))
                    {
                        FunctionDocComments* funcDoc = nullptr;
                        IfFailGo(GetFuncDocComments(localArena, function, &funcDoc));

                        if (funcDoc)
                        {
                            FunctionDocComments::Signature* signature = funcDoc->FirstSignature();
                            if (signature)
                            {
                                if (signature->params.Count() > paramIndexOfCall)
                                {
                                    auto param = signature->params.Item(paramIndexOfCall);
                                    Js::ScriptContext* scriptContext = function->GetScriptContext();
                                    JsHelpers::WithArguments([&](Js::Arguments& arguments)
                                    {
                                        Js::Var getInstanceByTypeStringVar = JsHelpers::GetPropertyVar(scriptContext->GetGlobalObject(), L"_$getInstanceByTypeString", scriptContext);
                                        if (getInstanceByTypeStringVar)
                                        {
                                            Assert(Js::JavascriptFunction::Is(getInstanceByTypeStringVar));
                                            Js::JavascriptFunction* getInstanceByTypeString = Js::JavascriptFunction::FromVar(getInstanceByTypeStringVar);
                                            Js::Var value = nullptr;
                                            ExecuteFunction(getInstanceByTypeString, arguments, &value);
                                            
                                            while (value != nullptr && members.Count() > 0)
                                            {
                                                value = JsHelpers::GetPropertyVar(Js::RecyclableObject::FromVar(value), members.Pop(), scriptContext);
                                            }

                                            if (value == nullptr)
                                            {
                                                return;
                                            }

                                            AddCompletions(value, completions, /* ownOnly = */false, /* excludeObjectPrototype = */true, /* excludeConstructor = */true, /* checkWith = */false, AuthorScope::ascopeMember, PropertyFilter(&usedProperties));
                                            *objectLiteralCompletionAvailable = true;
                                        }
                                    },
                                    scriptContext,
                                    scriptContext->GetGlobalObject(),
                                    Convert::ToVar(param->type, m_scriptContext));
                                    IfFailGo(hr);
                                    
                                }
                            }
                        }
                        
                        if (!(*objectLiteralCompletionAvailable))
                        {
                            if (function->GetFunctionBody() != nullptr)
                            {
                                //
                                // Function execution heuristic
                                //
                                // Since no doc comment is available, we will execute the script with a capturing proxy to understand how
                                // the object literal is read and use that to provide intellisense.

                                // Step 1: Creating a capturingProxyHolder
                                Js::Var capturingProxyHolder = nullptr;
                                this->CreateCapturingProxyHolder(function->GetScriptContext(), function->GetFunctionBody(), &capturingProxyHolder);

                                if (capturingProxyHolder != nullptr)
                                {
                                    // Step 2: Extract the capturing proxy
                                    Js::Var capturingProxyVar = JsHelpers::GetPropertyVar(Js::RecyclableObject::FromVar(capturingProxyHolder), Names::capturingProxyHolderProxyPropertyName, function->GetScriptContext());;

                                    if (capturingProxyVar != nullptr)
                                    {
                                        Assert(Js::JavascriptProxy::Is(capturingProxyVar));
                                        Js::JavascriptProxy* capturingProxy = Js::JavascriptProxy::FromVar(capturingProxyVar);

                                        // Step 3: Call the target function with the capturing proxy
                                        this->ExecuteFunctionWithCapturingProxy(function, paramIndexOfCall, capturingProxy);

                                        // Step 4: Extract the read properties
                                        Js::Var capturedPropertiesVar = JsHelpers::GetPropertyVar(Js::RecyclableObject::FromVar(capturingProxyHolder), Names::capturingProxyHolderPropertiesPropertyName, function->GetScriptContext());
                                        if (capturedPropertiesVar != nullptr)
                                        {
                                            Assert(Js::JavascriptArray::Is(capturedPropertiesVar));
                                            Js::JavascriptArray* capturedProperties = Js::JavascriptArray::FromVar(capturedPropertiesVar);

                                            // Step 5: Add them to the completion list
                                            Js::Var enumeratorVar = nullptr;
                                            capturedProperties->GetEnumerator(false, &enumeratorVar, function->GetScriptContext(), true, false);
                                            Js::JavascriptEnumerator *enumerator = Js::JavascriptEnumerator::FromVar(enumeratorVar);
                                            Assert(enumerator);
                                            while (enumerator->MoveNext())
                                            {
                                                Js::Var capturedProperty = enumerator->GetCurrentValue();
                                                if (Js::JavascriptString::Is(capturedProperty))
                                                {
                                                    Js::JavascriptString* capturedPropertyString = Js::JavascriptString::FromVar(capturedProperty);
                                                    const WCHAR* optionString = capturedPropertyString->GetSzCopy(this->Alloc());
                                                    if (!usedProperties.Contains(optionString))
                                                    {
                                                        completions->AddUnique(AuthorCompletionKind::ackField, AuthorCompletionFlags::acfMembersFilter, optionString, capturedPropertyString->GetLength(), optionString, nullptr);
                                                    }

                                                    *objectLiteralCompletionAvailable = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        completions->SetKind(acskMember);
                    }
                }
            }
        }

        METHOD_POSTFIX;
    }

    void FileAuthoring::CreateCapturingProxyHolder(Js::ScriptContext* scriptContext, Js::FunctionBody* exprContainer, /* out */ Js::Var* capturingProxyHolder)
    {
        Js::Var createCaptureProxyArgs[1];
        createCaptureProxyArgs[0] = scriptContext->GetGlobalObject();
        Js::JavascriptFunction* createCapturingProxyHolder = JsHelpers::GetProperty<Js::JavascriptFunction*>(scriptContext->GetGlobalObject(), Names::createCapturingProxyHolder, this->Alloc(), scriptContext);

        *capturingProxyHolder = nullptr;

        // The createCapturingProxyHolder could be null if the helpers.js is not executed properly (let's say if Hurry is called while executing the helpers.js)
        if (createCapturingProxyHolder != nullptr)
        {
            Js::CallInfo createCaptureProxyArgsCallInfo(Js::CallFlags_Value, /* count = */2);
            Js::Arguments createCaptureProxyArguments(createCaptureProxyArgsCallInfo, createCaptureProxyArgs);

            this->WithProbe([&](AuthoringProbe& authorProbe) {
                ExecuteRootFunction(authorProbe, createCapturingProxyHolder, createCaptureProxyArguments, capturingProxyHolder);
            }, 0, exprContainer);
        }
    }

    void FileAuthoring::ExecuteFunctionWithCapturingProxy(Js::JavascriptFunction* function, int paramIndexOfCall, Js::JavascriptProxy* capturingProxy)
    {
        Js::Var* targetFunctionArgs = AnewArray(this->Alloc(), Js::Var, paramIndexOfCall + 2);
        unsigned short argsCount = (unsigned short)(paramIndexOfCall + 2);
        for (int i = 0; i < paramIndexOfCall + 1; i++)
        {
            // Since this is a heuristic anyway, the best guess argument value are 'true' values 
            // so that there is a check on the argument value, it is easier to get in those branches.
            targetFunctionArgs[i] = Convert::ToVar(true, function->GetScriptContext());
        }
        targetFunctionArgs[paramIndexOfCall + 1] = capturingProxy;

        Js::CallInfo targetFunctionCallInfo(Js::CallFlags_Value, argsCount);
        auto targetFunctionArguments = Js::Arguments(targetFunctionCallInfo, targetFunctionArgs);
        Js::Var targetFunctionResult;
        WithProbe([&](AuthoringProbe& authorProbe) {
            ExecuteRootFunction(authorProbe, function, targetFunctionArguments, &targetFunctionResult);
        }, 0, function->GetFunctionBody());
    }

    HRESULT FileAuthoring::GetLabelCompletions(long offset, ArenaAllocator *localArena, Completions *completions)
    {
        METHOD_PREFIX;

        Assert(completions);

        IfFailGo(TestAbort());

        IfFailGo(UpdatePrimary(false));

        Assert(m_primaryTree.LanguageServiceExtension());

        {
            ParseNodeCursor cursor(localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);
            ParseNodePtr current = cursor.Current();

            if (current && cursor.InAJumpStatement())
            {
                while (ParseNodePtr parent = cursor.Up())
                {
                    // do not cross function boundaries
                    if (parent->nop == knopFncDecl || parent->nop == knopProg)
                        break;

                    LPCWSTR label = m_primaryTree.LanguageServiceExtension()->GetLabel(parent);
                    if (!String::IsNullOrEmpty(label))
                        completions->AddUnique(AuthorCompletionKind::ackLabel, AuthorCompletionFlags::acfMembersFilter, label, ::wcslen(label), nullptr, nullptr);
                }
            }
        }

        METHOD_POSTFIX;
    }
    
    static tokens rwss_version10[] = { tkBREAK, tkCASE, tkCATCH, tkCONTINUE, tkDEBUGGER, tkDEFAULT, tkDELETE, tkDO, tkELSE, tkFALSE, 
        tkFINALLY, tkFOR, tkFUNCTION, tkIF, tkIN, tkINSTANCEOF, tkNEW, tkNULL, tkRETURN, tkSWITCH, tkTHIS, tkTHROW, tkTRUE, 
        tkTRY, tkTYPEOF, tkVAR, tkVOID, tkWHILE, tkWITH };
    static tokens rwss_version11[] = { tkCONST, tkLET };
    static tokens rwss_version12[] = { tkCLASS, tkSUPER, tkEXTENDS };

    void FileAuthoring::GetSymOf(tokens tk, _Outptr_result_buffer_(*cch) OLECHAR** ppSz, _Out_ ulong* cch)
    {
        switch (tk)
        {
#define KEYWORD(tk,f,prec2,nop2,prec1,nop1,name) \
        case tk: \
            *ppSz = (OLECHAR*)g_ssym_##name.sz; \
            *cch = g_ssym_##name.cch; \
            break;
#include "keywords.h"
        }
    }

    void FileAuthoring::AddReservedWordList(Completions *completions, __in_ecount(wordsLen) tokens* words, size_t wordsLen)
    {
        for (size_t i = 0; i < wordsLen; i++)
        {
            ulong cch = 0;
            OLECHAR* psz;
            GetSymOf(words[i], &psz, &cch);
            if (cch > 0)
                completions->AddUnique(ackReservedWord, acfSyntaxElementsFilter, psz, cch, nullptr, nullptr);
        }
    }

    HRESULT FileAuthoring::AddReservedWordsFor(ArenaAllocator *localArena, charcount_t offset, Completions *completions)
    {
        METHOD_PREFIX;

        IfFailGo(TestAbort());

        IfFailGo(UpdatePrimary(false));

        { // Introduce scope for cursor

            // Find the node that contains the cursor.
            ParseNodeCursor cursor(localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);            

            if (!cursor.RightOfDot())
            {
                AddReservedWordList(completions, rwss_version10, sizeof(rwss_version10)/sizeof(rwss_version10[0]));
                AddReservedWordList(completions, rwss_version11, sizeof(rwss_version11)/sizeof(rwss_version11[0]));
                AddReservedWordList(completions, rwss_version12, sizeof(rwss_version12) / sizeof(rwss_version12[0]));                
            }
        }

        METHOD_POSTFIX;
    }

    AuthoringFileHandle* FileAuthoring::GetAuthoringFile(Js::ParseableFunctionInfo* jsFuncBody)
    {
        Assert(jsFuncBody);
        return GetAuthoringFileBySourceInfo(jsFuncBody->GetUtf8SourceInfo());
    }

    AuthoringFileHandle* FileAuthoring::GetAuthoringFile(uint sourceIndex, Js::ScriptContext *scriptContext)
    {
        return GetAuthoringFileByIndex(sourceIndex, scriptContext);
    }

    int FileAuthoring::GetFileIdOf(uint sourceIndex, Js::ScriptContext *scriptContext)
    {
        auto file = GetAuthoringFileByIndex(sourceIndex, scriptContext);
        return file ? file->FileId() : -1;
    }

    uint FileAuthoring::GetSourceIndexOf(int fileId)
    {
        return GetSourceIndexOf(GetAuthoringFileById(fileId));
    }

    uint FileAuthoring::GetSourceIndexOf(AuthoringFileHandle *file)
    {
        if (file && file->Text())
        {
            Assert(IsScriptContextValid());

            auto buffer = file->Text()->Buffer();
            auto len = m_scriptContext->SourceCount();
            for (uint i = 0; i < len; i++)
            {
                if (m_scriptContext->GetSource(i)->GetSource(L"FileAuthoring::GetSourceIndexOf") == buffer) return i;
            }
        }
        return MAXUINT;
    }

    AuthoringFileHandle* FileAuthoring::GetAuthoringFileBySourceInfo(Js::Utf8SourceInfo * sourceInfo)
    {
        if (m_primaryFile && m_primaryFile->IsSourceMatched(sourceInfo))
        {
            return m_primaryFile;
        }

        if (m_scriptContextPath)
        {
            return m_scriptContextPath->GetAuthoringFileBySourceInfo(sourceInfo);
        }

        return nullptr;
    }

    AuthoringFileHandle* FileAuthoring::GetAuthoringFileByIndex(uint sourceIndex, Js::ScriptContext *scriptContext)
    {
        Assert(scriptContext || IsScriptContextValid());

        if (!scriptContext) 
            scriptContext = m_scriptContext;

        if (m_primaryFile && m_primaryFile->IsSourceAtIndex(scriptContext, sourceIndex))
            return m_primaryFile;

        AuthoringFileHandle* file = nullptr;

        if (m_scriptContextPath)
            file = m_scriptContextPath->GetAuthoringFileByIndex(scriptContext, sourceIndex);

        return file;
    }

    AuthoringFileHandle *FileAuthoring::GetAuthoringFileById(int fileId)
    {
        if (m_primaryFile->FileId() == fileId)
            return m_primaryFile;

        AuthoringFileHandle *file = nullptr;
        if (m_scriptContextPath)
            file = m_scriptContextPath->GetAuthoringFileById(fileId);

        return file;
    }

    void FileAuthoring::ForceRegisterLoad(ParseNode* callNode, Parser* parser)
    {
        Assert(callNode != nullptr);

        if(callNode->nop != knopCall) 
        {
            return;
        }

        // Adding a . operator to call target will force it to be loaded into a register.
        // Since the call is not actually happening during execution we don't care that the target 
        // is always resolved as undefined.
        ParseNodePtr pnodeName = parser->CreateNameNode(CreatePidFromLiteral(parser, Names::dummy));
        pnodeName->ichMin = callNode->ichLim;
        pnodeName->ichLim = callNode->ichLim;
        callNode->sxCall.pnodeTarget = parser->CreateBinNode(knopDot,
            callNode->sxCall.pnodeTarget, pnodeName);
    }

    HRESULT FileAuthoring::TestAbort()
    {
        return m_abortCalled ? E_ABORT : S_OK;
    }

    bool FileAuthoring::IsCallOrNew(ParseNode* node)
    {
        return node->nop == knopCall || node->nop == knopNew;
    }

    bool FileAuthoring::IsFuncDeclOrProg(ParseNode* node)
    {
        return node->nop == knopFncDecl || node->nop == knopProg;
    }

    bool FileAuthoring::GetArgumentLimit(ParseNode* node, charcount_t offset, int& argIndex)
    {
        charcount_t argLim = m_primaryTree.LanguageServiceExtension()->ArgLim(node);
        if (argLim == 0) argLim = node->ichLim;
        Assert(argLim > 0);

        argIndex++;
        return offset <= argLim;
    }

    bool FileAuthoring::FindArgument(ParseNode* node, charcount_t offset, int& argIndex)
    {
        if (node == nullptr)
        {
            return false;
        }

        ParseNode * iterNode = node;
        while (iterNode->nop == knopList)
        {
            Assert(iterNode->sxBin.pnode1->nop != knopList);

            if (GetArgumentLimit(iterNode->sxBin.pnode1, offset, argIndex))
            {
                // Found the node.
                return true;
            }
            iterNode = iterNode->sxBin.pnode2;
        }

        Assert(iterNode != nullptr);
        return GetArgumentLimit(iterNode, offset, argIndex);
    }

    int FileAuthoring::GetArgumentIndex(ParseNode* callNode, long offset)
    {
        Assert(ContainsOffset(callNode, offset));

        if (!IsInCallParenthesis(offset, callNode, false, m_primaryTree.LanguageServiceExtension()))
        {
            // The offset is not between the call parenthesis
            return -1;
        }

        int argIndex = -1;
        auto node = callNode->sxCall.pnodeArgs;
        FindArgument(node, offset, argIndex);
        return argIndex > 0 ? argIndex : 0;
    }

    HRESULT FileAuthoring::FindFunctionSource(ArenaAllocator *alloc, Js::JavascriptFunction *&func, AuthoringFileHandle* &file)
    {
        METHOD_PREFIX;

        Assert(alloc);
        Assert(func);

        file = nullptr;

        Assert(IsScriptContextValid());

        // A function can have a _$doc property which points to a function containing the comments
        auto docFunc = JsHelpers::GetProperty<Js::JavascriptFunction*>(func, Names::_doc, alloc, m_scriptContext);            
        if(docFunc)
            func = docFunc;

        auto jsFuncBody = func->GetParseableFunctionInfo();
        if(jsFuncBody == nullptr)
            goto Error;

        auto authoringFile = GetAuthoringFile(jsFuncBody);
        if(authoringFile == nullptr)
        {
            if(jsFuncBody->GetSource() && jsFuncBody->IsDynamicFunction())
            {
                // This is a function defined via Function ctor, like this: 
                //    new Function("param1", "param2", "return param1+param2")
                TextBuffer js(alloc);
                js.AddUtf8(jsFuncBody->GetSource(), jsFuncBody->LengthInChars());
                auto memfile = Anew(alloc, MemoryAuthoringFile, js.Sz(true), jsFuncBody->LengthInChars());
                authoringFile = Anew(alloc, AuthoringFileHandle, m_factory, memfile, AuthoringFileHandleKind_Transient);
            }
            else
                goto Error;
        }

        file = authoringFile;

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::GetFunctionHelp(LPCWSTR funcAlias, __in Js::JavascriptFunction* func, Js::RecyclableObject* parentObject, __inout_opt AuthorDiagStatus *diagStatus, __out IAuthorFunctionHelp **result)
    {
        METHOD_PREFIX;

        Assert(func != nullptr);
        Assert(result != nullptr);

        *result = nullptr;

        AuthorDiagStatus unused = AuthorDiagStatus(); 
        diagStatus = diagStatus ? diagStatus : &unused;

        Assert(IsScriptContextValid());

        ArenaAllocator localArena(L"ls:GetFunctionHelp", m_scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

        AuthoringFileHandle* file = nullptr;
        IfFailGo(FindFunctionSource(&localArena, func, file));

        if(!file)
        {
            // ParseNode is not available for the function. This is going to be the case for native functions.
            *diagStatus = adsFunctionDeclarationUnavailable;
            return S_OK;
        }
        Assert(file);

        auto funcName = funcAlias;
        if(String::IsNullOrEmpty(funcName))
        {
            auto funcBody = func->GetParseableFunctionInfo();
            if(funcBody)
            {
                funcName =  funcBody->GetDisplayName();
            }
        }

        IfFailGo(GetFunctionHelp(funcName, file, func, parentObject, diagStatus, result));

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::GetFunctionHelp(LPCWSTR name, AuthoringFileHandle* srcFile, Js::JavascriptFunction *function, Js::RecyclableObject* parentObject, __inout AuthorDiagStatus *diagStatus, __out IAuthorFunctionHelp **result)
    {
        METHOD_PREFIX;

        Assert(srcFile != nullptr);
        Assert(result != nullptr);

        *result = nullptr;

        Assert(IsScriptContextValid());

        auto pageAlloc = m_scriptContext->GetThreadContext()->GetPageAllocator();

        ArenaAllocator localArena(L"ls:GetFunctionHelp", pageAlloc, Js::Throw::OutOfMemory);

        JsUtil::List<LPCWSTR, ArenaAllocator> namedArgs(&localArena);

        Js::ParseableFunctionInfo *body = nullptr;
        if (function) body = function->GetParseableFunctionInfo();

        if (!body) 
        {
            // ParseNode is not available for the function. This is going to be the case for native functions.
            *diagStatus = adsFunctionDeclarationUnavailable;
            return S_OK;
        }       

        bool isValid = true;
        charcount_t functionStartPosition;
        charcount_t lcurlyPosition;
        charcount_t firstStatementPosition;
        charcount_t  functionEndPosition;
        srcFile->ForEachArgument(&localArena, m_scriptContext, function, [&](ParseNode* arg, bool isRest)
        {
            if (isValid) 
            {
                if (arg->nop != knopVarDecl || !arg->sxVar.pid)
                    isValid = false;
                else
                {
                    auto pid = arg->sxVar.pid;
                    if (isRest)
                    {
                        // Adding rest ellipses for better experience.
                        namedArgs.Add(String::Copy(&localArena, L"...", 3, pid->Psz(), pid->Cch()));
                    }
                    else
                    {
                        namedArgs.Add(String::Copy(&localArena, pid->Psz(), pid->Cch()));
                    }
                }
            }
        }, &functionStartPosition, &lcurlyPosition, &firstStatementPosition, &functionEndPosition);

        if (!isValid)
        {
            *diagStatus = adsInvalidAST;
            goto Error;
        }

        {
            CommentBuffer* functionComments = srcFile->GetFunctionComments(&localArena, functionStartPosition, lcurlyPosition, firstStatementPosition, functionEndPosition, commenttypeAnyDoc);

            JsValueDoc* fieldDoc = nullptr;
            if (parentObject)
            {
                // In case of a member function try to get the field doc comments as they might be used if function doc comments are not available. 
                auto docRef = JsValueDoc::GetFieldDoc(&localArena, parentObject, name, m_scriptContext);
                if (docRef && docRef->isDefinitionRef)
                {
                    AuthoringFileHandle* file = nullptr;
                    IfFailGo(ResolveDocCommentRef(&localArena, docRef, ascopeMember, /* fieldName = */name, /* out */ fieldDoc, /* out */ file));
                }
            }

            FunctionHelpInfoParseStatus parseStatus = FunctionHelpInfoParseStatus::fhipsOK;
            IfFailGo(Authoring::FunctionHelpInfo::CreateInstance(pageAlloc, name, namedArgs, functionComments, fieldDoc, result, &parseStatus, srcFile && !(srcFile->IsDisableRewrite()) ? srcFile : nullptr));
            if (parseStatus != FunctionHelpInfoParseStatus::fhipsOK)
            {
                if (diagStatus != nullptr)
                {
                    // Error occured while parsing the XML doc comment.
                    *diagStatus = adsInvalidDocComment;
                }
                return hr;
            }
        }

        METHOD_POSTFIX;
    }

    AuthorType FileAuthoring::GetAuthorType(ParseNode* node)
    {
        Assert(node != nullptr);
        switch(node->nop)
        {
        case knopInt:
        case knopFlt:
            return atNumber;
        case knopArray:
            return atArray;
        case knopObject:
            return atObject;
        case knopStr:
            return atString;
        }

        return atUnknown;
    }

    AuthorType FileAuthoring::GetAuthorType(Js::TypeId typeId)
    {
        switch(typeId)
        {
        case Js::TypeIds_Undefined:
        case Js::TypeIds_Null:
            return atUnknown;
        case Js::TypeIds_Boolean:
            return atBoolean;
        case Js::TypeIds_Integer:
        case Js::TypeIds_Number:
        case Js::TypeIds_Int64Number:
        case Js::TypeIds_UInt64Number:
            return atNumber;
        case Js::TypeIds_String:
            return atString;
        case Js::TypeIds_Enumerator:
        case Js::TypeIds_SafeArray:
        case Js::TypeIds_VariantDate:
        case Js::TypeIds_HostDispatch:
        case Js::TypeIds_Object:
            return atObject;
        case Js::TypeIds_Function:
            return atFunction;
        case Js::TypeIds_Array:
            return atArray;
        case Js::TypeIds_Date:
            return atDate;
        case Js::TypeIds_Symbol:
            return atSymbol;
        case Js::TypeIds_RegEx:
            return atRegEx;
        case Js::TypeIds_Error:
        case Js::TypeIds_BooleanObject:
        case Js::TypeIds_NumberObject:
        case Js::TypeIds_StringObject:
        case Js::TypeIds_GlobalObject:
        case Js::TypeIds_ExtensionEnumerator:
        case Js::TypeIds_ModuleRoot:
        case Js::TypeIds_SafeArrayObject:
        case Js::TypeIds_Arguments:
        case Js::TypeIds_ArrayIterator:
        case Js::TypeIds_MapIterator:
        case Js::TypeIds_SetIterator:
        case Js::TypeIds_StringIterator:
        case Js::TypeIds_SymbolObject:
        case Js::TypeIds_Generator:
        case Js::TypeIds_Promise:
        case Js::TypeIds_Proxy:
            return atObject;
        }

        return atUnknown;
    }

    /* static */ HRESULT FileAuthoring::ResolveDocCommentRef(ArenaAllocator* alloc, FileAuthoring* fileAuthoring, ScriptContextPath *contextPath, JsValueDoc *docCommentRef, AuthorScope scope, LPCWCHAR fieldName, __out JsValueDoc *&doc, __out AuthoringFileHandle *& file)
    {
        METHOD_PREFIX;

        Assert(alloc);
        ValidateArg(fileAuthoring || contextPath);
        ValidateArg(docCommentRef);
        Validate(docCommentRef->isDefinitionRef);
        
        doc = nullptr;
        file = fileAuthoring ? fileAuthoring->GetAuthoringFileById(docCommentRef->fileId) : contextPath->GetAuthoringFileById(docCommentRef->fileId);
        if(file)
        {
            // Accept only /// comments as xml doc comments
            auto comment = file->GetNodeComments(alloc, docCommentRef->pos, commenttypeAnyDoc);
            if(comment)
            {
                // Field or a variable 
                if(scope != ascopeMember)
                {
                    Authoring::VarDocComments* varDoc = nullptr;
                    IfFailGo(Authoring::ParseVarDocComments(alloc, comment->Sz(), comment->GetCommentType(), &varDoc));
                    if(varDoc)
                    {
                        doc = JsValueDoc::FromDocComments(alloc, varDoc);
                    }
                }

                // Global variables are a special case. 
                // They may be accessed as a variable or as a field of the global object.
                // They can be documented using <var> element above them or using intellisense.annotate in which case the doc comment will be a <field> element.
                // So for global variables, when parsing as variable doc comment fails, and the doc comment is an annotation, try parsing as a field doc comment. 
                if(scope == ascopeMember || (scope == ascopeGlobal && docCommentRef->annotation && !doc))
                {
                    Authoring::FieldDocComments* fieldDoc = nullptr;
                    IfFailGo(Authoring::ParseFieldDocComments(alloc, fieldName, comment->Sz(), comment->GetCommentType(), /* isGlobalVariableAsField = */true, &fieldDoc));
                    if(fieldDoc)
                    {
                        doc = JsValueDoc::FromDocComments(alloc, fieldDoc);
                    }
                }
            }
        }

        METHOD_POSTFIX;
    }

    HRESULT FileAuthoring::ResolveDocCommentRef(ArenaAllocator* alloc, JsValueDoc* docCommentRef, AuthorScope scope, LPCWCHAR fieldName, __out JsValueDoc*& doc, __out AuthoringFileHandle*& file)
    {
        return /*static */ ResolveDocCommentRef(alloc, this, nullptr, docCommentRef, scope, fieldName, doc, file);
    }

    Js::CharClassifier *FileAuthoring::lsCharClassifier = nullptr;

    Js::CharClassifier *FileAuthoring::GetLSCharClassifier()
    {
        if (lsCharClassifier == nullptr)
        {
            lsCharClassifier = HeapNew(Js::CharClassifier, Js::CharClassifierModes::ES6, true);
        }

        return lsCharClassifier;
    }

    HRESULT FileAuthoring::CreateSymbolHelp(Js::ScriptContext* scriptContext, Js::RecyclableObject* jsParentObj, Js::Var jsValue, LPCWSTR name, AuthorScope scope, AuthorType authorType,
        int fileId, charcount_t declarationPos, IAuthorSymbolHelp** result)
    {
        METHOD_PREFIX;

        Assert(scriptContext);
        Assert(!String::IsNullOrEmpty(name));
        Assert(result);

        *result = nullptr;

        auto symbolHelp = new SymbolHelp(scriptContext->GetThreadContext()->GetPageAllocator());
        RefCountedPtr<IAuthorFunctionHelp> funcHelp;
        RefCountedPtr<IAuthorDeprecatedInfo> deprecated;
        RefCountedPtr<IAuthorCompatibleWithSet> compatibleWith;

        auto alloc = symbolHelp->Alloc();
        JsValueDoc* doc = nullptr;
        Js::RecyclableObject* obj = nullptr;
        JsValueDoc* attachedDoc = nullptr;
        JsValueDoc* fieldDoc = nullptr;

        if(jsValue)
        {
            if(Js::RecyclableObject::Is(jsValue))
            {
                obj = Js::RecyclableObject::FromVar(jsValue);
                authorType = GetAuthorType(obj->GetTypeId());
                if (obj->GetTypeId() == Js::TypeIds_Function)
                {
                    IfFailGo(GetFunctionHelp(name, static_cast<Js::JavascriptFunction*>(jsValue), jsParentObj, nullptr, &funcHelp));
                }
                else
                {
                    attachedDoc = JsValueDoc::GetDoc(alloc, obj, scriptContext);
                }
            }
            else
            {
                if(Js::TaggedNumber::Is(jsValue))
                {
                    authorType = atNumber;
                }
            }
        }

        if(jsParentObj)
        {
            // Try to get field doc comments. 
            // Global variables also have field doc comments on the global object.
            fieldDoc = JsValueDoc::GetFieldDoc(alloc, jsParentObj, name, scriptContext);
        }

        if(fieldDoc)
        {
            // Start with field doc comments if available. 
            doc = fieldDoc;
        }
        else if (fileId > 0)
        {
            // If fileId was exteranlly provided, use it.
            doc = Anew(alloc, JsValueDoc, alloc);
            doc->isDefinitionRef = true;
            doc->fileId = fileId;
            doc->pos = declarationPos;
        }

        AuthoringFileHandle* file = nullptr;

        if(doc)
        {
            if(doc->isDefinitionRef)
            {
                IfFailGo(ResolveDocCommentRef(alloc, doc, scope, /* fieldName = */name, /* out */ doc, /* out */ file));
            }
            else if(doc->fileId >= 0)
            {
                file = GetAuthoringFileById(doc->fileId);
            }
        }

        // If doc comments were attached by <returns>, <param> or <var> try to use them.
        // If field doc comments were available, merge the information.
        // For example:
        //      function A() { 
        //          /// <returns type='A' />
        //      }
        //      ///<var>a variable</var>
        //      var a = A();
        //      a| <= COMPLETION HINT WILL SHOW: A a, a variable 
        //                                       ^    ^----- DESCRIPTION FROM <var>
        //                                       |---------- TYPE FROM <returns>
        //                                          
        if(attachedDoc && !attachedDoc->IsEmpty())
        {
            Assert(attachedDoc->isDefinitionRef == false);
            if(!doc)
            {
                doc = Anew(alloc, JsValueDoc, alloc);
            }
            doc->MergeFrom(attachedDoc);
        }

        if (doc && doc->deprecated)
        {
            auto depInfo = Anew(alloc, DeprecatedInfo<SymbolHelp>, symbolHelp);
            depInfo->ApplyDocComments(doc->deprecated);
            deprecated.Assign(depInfo);
        }

        if (doc && doc->compatibleWith.Count() > 0)
        {
            auto compatInfoSet = Anew(alloc, CompatibleWithSet, symbolHelp, symbolHelp->Alloc());
            for (int i = 0; i < doc->compatibleWith.Count(); i++)
            {
                auto compatibleWithDoc = doc->compatibleWith.Item(i);
                auto compatInfo = Anew(alloc, CompatibleWithInfo<SymbolHelp>, symbolHelp);
                compatInfo->ApplyDocComments(compatibleWithDoc);
                compatInfoSet->Add(compatInfo);
            }
            compatibleWith.Assign(compatInfoSet);
        }

        symbolHelp->Initialize(
            authorType, 
            scope, 
            doc ? doc->type : nullptr, 
            name, 
            doc ? doc->description : nullptr, 
            doc ? doc->locid : nullptr, 
            doc ? doc->elementType : nullptr, 
            doc ? doc->helpKeyword : nullptr, 
            doc ? doc->externalFile : nullptr, 
            doc ? doc->externalid : nullptr, 
            deprecated,
            compatibleWith,
            funcHelp, 
            file && !(file->IsDisableRewrite()) ? file : nullptr);

        *result = symbolHelp;

        METHOD_POSTFIX;
    }

    FileAuthoring::FileAuthoring(AuthoringFactory *factory, AuthoringServicesInfo *authoringServicesInfo, Js::ScriptContext *scriptContext, IAuthorFileContext *fileContext): 
    PhaseReporter(fileContext, authoringServicesInfo),
        SimpleComObjectWithAlloc<IAuthorFileAuthoring>(scriptContext->GetThreadContext()->GetPageAllocator(), L"ls: FileAuthoring"),
        m_contextAlloc(HeapNew(ArenaAllocator, L"ls: FileAuthoring", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory)),
        m_factory(factory),
        m_rootScriptContext(scriptContext),
        m_leafScriptContext(NULL),
        m_scriptContext(NULL), 
        m_parseOnlyScriptContext(NULL),
        m_primaryFile(NULL),
        m_primaryTree(Alloc()),
        m_primarySourceIndex(0),
        m_hurrySkipped(0),
        m_hurryCalled(false),
        m_contextInitialized(false),
        m_parserOnlyContext(false),
        m_polluted(false),
        m_contextChanged(true),
        m_scriptContextPathComplete(false),
        _completionLists(Alloc()),
        m_diagCallback(NULL)
    { 
        m_fileContext->AddRef(); 
        authoringServicesInfo->RegisterFileAuthoring(this);
    }

    void FileAuthoring::OnDelete()
    { 
        if (m_authoringServicesInfo)
        {
            m_authoringServicesInfo->UnregisterFileAuthoring(this);
            ReleasePtr(m_fileContext);
            ReleasePtr(m_diagCallback);
            ReleasePrimaryContext();
            m_scriptContextPath.ReleaseAndNull();
            HeapDeletePtr(m_contextAlloc);
            m_authoringServicesInfo.ReleaseAndNull();

            // Call the base class's OnDelete
            SimpleComObjectWithAlloc<IAuthorFileAuthoring>::OnDelete();
        }
    }

    void FileAuthoring::EngineClosing()
    {
        // Prevent deletion during this method
        RefCountedPtr<FileAuthoring> thisPtr(this);

        // Assume Completions object is calling RemoveCompletionList upon disposal.
        while(_completionLists.Count())
            _completionLists.Item(_completionLists.Count() - 1)->EngineClosing();
       
        this->OnDelete();
    }

    STDMETHODIMP FileAuthoring::ContextChanged()
    {
        return DebugApiWrapper([&]
        {
        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::ContextChanged Started\n");

        ForgetContext();

        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::ContextChanged Ended\n");
        STDMETHOD_POSTFIX;
        });
    }

    STDMETHODIMP FileAuthoring::Update()
    {
        return DebugApiWrapper([&]
        {
        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::Update Started\n");

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendUpdateBegin);

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif

        ResetHurryAndAbort();

        IfFailGo(UpdatePrimary(false));

        STDMETHOD_POSTFIX_CLEAN_START;
        Phase(afpDormant);
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendUpdateEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::Update Ended\n");
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    class RegionPreorderAddContext
    {
    public:
        RegionSet* regions;
        LanguageServiceExtension* lsExtension;
        RegionPreorderAddContext(RegionSet* regions, LanguageServiceExtension* lsExtension) : regions(regions), lsExtension(lsExtension) { }
    };

    class RegionPreorderAddPolicy : public VisitorPolicyBase<RegionPreorderAddContext*>
    {
    protected:
        bool Preorder(ParseNode *pnode, RegionPreorderAddContext *context)
        {
            if (pnode->nop == knopFncDecl)
            {
                auto ichRParenMin = context->lsExtension->RParen(pnode) + 1;
                auto ichRCurly = context->lsExtension->RCurly(pnode) + 1;

                // The function might not have a curly (such as it is the last function in the file and the user hasn't finished typing it in yet).
                // In this situation just grab the ichLim which will most likely extend to the end of the file.
                if (ichRCurly < ichRParenMin) ichRCurly = pnode->ichLim;

                // Only add a region for the function if it has a positive length.
                // This will exclude functions with no actual region (e.g. default constructors)
                auto length = ichRCurly - ichRParenMin;
                if (length > 0) context->regions->Add(ichRParenMin, length);
            }
            else if (pnode->nop == knopClassDecl)
            {
                auto ichLCurly = context->lsExtension->LCurly(pnode);
                auto ichRCurly = context->lsExtension->RCurly(pnode) + 1;

                if (ichLCurly < pnode->ichMin) ichLCurly = pnode->sxClass.pnodeBlock->ichMin;
                if (ichRCurly < ichLCurly) ichRCurly = pnode->ichLim;

                context->regions->Add(ichLCurly, ichRCurly - ichLCurly);
            }
            else if (pnode->nop == knopStrTemplate)
            {
                context->regions->Add(pnode->ichMin, pnode->ichLim - pnode->ichMin);
            }
            else if (pnode->nop == knopBlock || pnode->nop == knopObject || pnode->nop == knopSwitch)
            {
                bool isSyntheticBlock = (pnode->nop == knopBlock) && ((pnode->grfpn & fpnSyntheticNode) != 0);

                if (!isSyntheticBlock)
                {
                    auto ichLCurly = context->lsExtension->LCurly(pnode);
                    auto ichRCurly = context->lsExtension->RCurly(pnode) + 1;

                    // TODO (andrewau) what the existing corrections are doing?
                    context->regions->Add(ichLCurly, ichRCurly - ichLCurly);
                }
            }
            else if (pnode->nop == knopArray)
            {
                auto ichLBrack = context->lsExtension->LBrack(pnode);
                auto ichRBrack = context->lsExtension->RBrack(pnode) + 1;

                context->regions->Add(ichLBrack, ichRBrack - ichLBrack);
            }

            return true;
        }
    };

    STDMETHODIMP FileAuthoring::GetTaskComments(AuthorTaskCommentPrefix* prefixes, ULONG count, IAuthorTaskCommentSet **result)
    {
        return DebugApiWrapper([&]
        {
            TaskCommentSet *taskComments = NULL;
            m_abortCalled = false;

            STDMETHOD_PREFIX;

            ValidateArg(prefixes);
            
            for (unsigned int i = 0; i < count; i++)
            {
                ValidateArg(prefixes[i].taskCommentPrefixText)
            }

            ValidateArg(result);

            OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetTaskComments Started\n");

            IfFailGo(UpdatePrimary(false));

#ifdef DEBUG
            IfFailGo(TestHasActiveCursors());
#endif

            taskComments = new TaskCommentSet(m_rootScriptContext->GetThreadContext()->GetPageAllocator());

            Js::CharClassifier* charClassifier = this->GetLSCharClassifier();

            const unsigned int todoTagLength = _countof(Names::jsdoc_todo_tag) - 1;
            for (CommentTableIterator* commentIterator = this->GetPrimaryFile()->GetCommentTable()->GetIterator(/* groupAdjacentComments = */false); commentIterator->HasNext(); commentIterator->MoveNext())
            {
                int min, lim;
                CommentBuffer* comment = commentIterator->GetCurrentComment(this->GetPrimaryFile()->Text(), &min, &lim);

                int size = lim - min;

                OUTPUT_TRACE(Js::TaskParsingPhase, L"%ls", comment->Sz());

                const WCHAR* buffer = comment->GetBuffer();
                const WCHAR* cursor = buffer;

                const int STATE_LINE_START = 0;
                const int STATE_IN_TODO_LINE = 1;
                const int STATE_NOT_IN_TODO_LINE = 2;

                AuthorTaskCommentPriority priority = AuthorTaskCommentPriority::low;

                int state = STATE_LINE_START;
                int begin = -1;
                for (int i = 0; i < size; i++)
                {
                    if (state == STATE_LINE_START)
                    {
                        BOOL isWhiteSpace = charClassifier->IsWhiteSpace(*cursor);
                        if (!isWhiteSpace && *cursor != '*')
                        {
                            state = STATE_NOT_IN_TODO_LINE;
                            if (wcsncmp(Names::jsdoc_todo_tag, cursor, todoTagLength) == 0 && comment->GetCommentType() == CommentType::commenttypeJSDoc)
                            {
                                begin = i;
                                state = STATE_IN_TODO_LINE;
                                priority = AuthorTaskCommentPriority::normal;
                            }
                            else
                            {
                                for (unsigned int j = 0; j < count; j++)
                                {
                                    if (_wcsnicmp(prefixes[j].taskCommentPrefixText, cursor, prefixes[j].taskCommentPrefixLength) == 0)
                                    {
                                        begin = i;
                                        state = STATE_IN_TODO_LINE;
                                        priority = prefixes[j].priority;
                                    }
                                }
                            }
                        }
                    }
                    else if (state == STATE_IN_TODO_LINE)
                    {
                        if (*cursor == '\r' || *cursor == '\n')
                        {
                            // At this point buffer[i] = '\r' (or '\n'), buffer[begin] = 'T', so the span should start with begin and 
                            // the length is given by i - begin
                            // 
                            // For example, consider
                            // 
                            // // TODO: Hello\r
                            // 0123456789ABCDE
                            //    ^          ^
                            // 
                            // offset = 3
                            // length = 14 - 3 = 11
                            // 
                            // Of course, begin is the offset with respect to the comment, we need to return the offset 
                            // with respect to the document.
                            // 
                            // The 2 is due to the fact that comment table actually remove the /* or // from the buffer.
                            // 
                            taskComments->Add(min + begin + 2, i - begin, priority);
                            state = STATE_LINE_START;
                        }
                    }
                    else if (state == STATE_NOT_IN_TODO_LINE)
                    {
                        if (*cursor == '\r' || *cursor == '\n')
                        {
                            state = STATE_LINE_START;
                        }
                    }

                    cursor++;
                }
            }

            *result = taskComments;
            taskComments = NULL;

            STDMETHOD_POSTFIX_CLEAN_START;
            ReleasePtr(taskComments);
            Phase(afpDormant);
            DecommitUnusedPages();
            OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetTaskComments Ended\n");
            STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    STDMETHODIMP FileAuthoring::GetRegions(IAuthorRegionSet **result)
    {
        return DebugApiWrapper([&]
        {
        RegionSet *regions = NULL;
        m_abortCalled = false;

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetRegions Started\n");

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetRegionsBegin);

        ValidateArg(result);

        IfFailGo(UpdatePrimary(false));

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif

        regions = new RegionSet(m_rootScriptContext->GetThreadContext()->GetPageAllocator());
        ParseNodeVisitor<RegionPreorderAddPolicy> visitor;
        RegionPreorderAddContext context(regions, m_primaryTree.LanguageServiceExtension());
        visitor.Visit(m_primaryTree.TreeRoot(), &context);
        
        for (CommentTableIterator* commentIterator = this->GetPrimaryFile()->GetCommentTable()->GetIterator(/* groupAdjacentComments = */true); commentIterator->HasNext(); commentIterator->MoveNext())
        {
            int min, lim;
            commentIterator->GetCurrentCommentSpan(&min, &lim);
            regions->Add(min, lim - min);
        }

        *result = regions;
        regions = NULL;

        STDMETHOD_POSTFIX_CLEAN_START;
        ReleasePtr(regions);
        Phase(afpDormant);
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetRegionsEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetRegions Ended\n");
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    LanguageServiceExtension::CompletionRangeMode FileAuthoring::GetCompletionRangeMode(uint offset)
    {
        LanguageServiceExtension* lsExtension = m_primaryTree.LanguageServiceExtension();
        Assert(lsExtension);
        JsUtil::List<LanguageServiceExtension::CompletionRange*, ArenaAllocator>* completionRanges = lsExtension->CompletionRanges();
        Assert(completionRanges);

        #if DEBUG
            // Ensure the list is sorted
            for (int i = 1; i < completionRanges->Count(); i++)
            {
                auto previous = completionRanges->Item(i - 1);
                auto current = completionRanges->Item(i);
                Assert(previous->ichLim < current->ichMin);
            }
        #endif

        auto entryIndex = ListHelpers::BinarySearch<LanguageServiceExtension::CompletionRange*>(*completionRanges, [&](LanguageServiceExtension::CompletionRange* entry) -> int
        {
            if (entry->ichMin > offset)
                return 1;

            if (entry->ichLim < offset)
                return -1;

            // The offset is in the min..max range
            return 0;
        });

        // make sure we found the correct element
        Assert(entryIndex < 0 ||  (completionRanges->Item(entryIndex)->ichMin <= offset && completionRanges->Item(entryIndex)->ichLim >= offset));

        return entryIndex >= 0 ? completionRanges->Item(entryIndex)->mode : LanguageServiceExtension::CompletionRangeMode::None;
    }

    bool FileAuthoring::IsCursorInComment(ParseNodeCursor* cursor)
    {
        Assert(cursor);

        // Check if the offset is in a comment
        CommentTable* commentTable = m_primaryFile->GetCommentTable();
        Assert(commentTable);
        if (commentTable->OffsetInComment(cursor->Offset()))
        {
            return true;
        }

        return false;
    }

    bool FileAuthoring::IsCompletionSupported(ParseNodeCursor* cursor, LanguageServiceExtension::CompletionRangeMode* completionRangeMode)
    {
        Assert(cursor != nullptr);
        Assert(completionRangeMode != nullptr);

        ParseNodePtr current = cursor->Current();

        if (current == nullptr)
        {
            return true;
        }

        // Check if the offset is in a comment.
        if (IsCursorInComment(cursor))
        {
            return false;
        }

        // Check if the offset is in a dead range set by the parser
        *completionRangeMode = this->GetCompletionRangeMode(cursor->Offset());
        if (*completionRangeMode == LanguageServiceExtension::CompletionRangeMode::Others)
        {
            return false;
        }

        switch (current->nop)
        {
            
        case knopStr:
        case knopInt:
        case knopFlt:
        case knopRegExp:
            // Do not enable completions inside literals
        case knopGetMember:
        case knopSetMember:
           return false;
        case knopDot:
            // Check if the dot operator is following a numerical literal or an erroneous expression something 
            // like "while().", which results in an operator dot, with an error node at the left side.  
            if (current->sxBin.pnode1 && 
                (cursor->IsErrorNode(current->sxBin.pnode1) || cursor->IsNumericLiteral(current->sxBin.pnode1)))
                return false;
            // intended fall through
        default:
            {
                ParseNodePtr parent = cursor->Parent();
                // Something like "while().", which results in an operator dot, with an error node at the left side.  
                if(parent && parent->nop == knopDot && 
                    parent->sxBin.pnode1 != nullptr && cursor->IsErrorNode(parent->sxBin.pnode1))
                {
                    return false;
                }
            }
            break;
        }

        return true;
    }

    bool FileAuthoring::IsImplicitFunctionHelpSupported(ParseNodeCursor* cursor)
    {
        Assert(cursor);

        ParseNodePtr current = cursor->Current();

        if (current == nullptr)
        {
            return true;
        }

        // Do not include token ichMin to support function help in a case like:  f(|'').  
        if (current->ichMin == cursor->Offset())
        {
            return true;
        }

        // Do not show function help on error nodes
        if (cursor->IsErrorNode(current))
        {
            return false;
        }

        // Check if the offset is in a comment.
        if (IsCursorInComment(cursor))
        {
            return false;
        }

        switch (current->nop)
        {
        case knopStr:
        case knopInt:
        case knopFlt:
        case knopRegExp:
            return false;
        }

        return true;
    }

    STDMETHODIMP FileAuthoring::GetCompletions(long offset, AuthorCompletionFlags flags, IAuthorCompletionSet **result)
    {
        return DebugApiWrapper([&]
        {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::HiResTimer timer;
            double start = timer.Now();
#endif
        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetCompletions Started, offset : %d started @ absolute time %10.3f\n", offset, g_timer.Now());

        AutoSetAuthorFileContext autoSetAuthorFileContext(this->m_rootScriptContext, this->m_fileContext);

        ResetHurryAndAbort();

        RefCountedPtr<Completions> completions;

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetCompletionsBegin);

        // Completion lists always create a snapshot copy via FileAuthoring::CreateSnapshot which they subscribe to.
        Assert(_completionLists.Count() == 0);

        IfFailGo(EnsureContext());
        if (CONFIG_FLAG(InvalidateSolutionContextsForGetStructure))
        {
            Assert(m_scriptContextPath);
            m_scriptContextPath->ApplyUsageFlagsUp(ScriptContextUsageFlags::UsedForCompletions);
        }

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif
        
        {
            // Create a bit vector to ensure we don't return duplicates
            ArenaAllocator localArena(L"ls: Completions", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
            BVSparse<ArenaAllocator> ids(&localArena);

            ValidateArg(result);

            IfFailGo(UpdatePrimary(true));

            ParseNodeCursor cursor(&localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);
            ParseNodePtr current = cursor.Current();

            LanguageServiceExtension::CompletionRangeMode completionRangeMode;
            if (IsCompletionSupported(&cursor, &completionRangeMode))
            {
                IfFailGo(EnsureContext());

                Js::Var symbolValue = nullptr;

                completions.TakeOwnership(Completions::New(this));

                if (current && IdentifierLikeNode(current))
                    UpdateExtent(completions, current, m_primaryTree.LanguageServiceExtension());
                else
                    completions->SetExtent(offset, 0);
                
                bool isObjectLiteral = (cursor.Current() != nullptr && cursor.Current()->nop == knopObject) || completionRangeMode == LanguageServiceExtension::CompletionRangeMode::ObjectLiteralNames;

                if (flags & acfImplicitRequest)
                {
                    if (!cursor.RightOfDot() && !isObjectLiteral)
                    {
                        *result = NULL;
                        goto Error;
                    }
                }

                // Add file identifiers
                // This must be done prior to adding mutating the tree below to prevent the mutated tree contributing to the identifier list.
                if (flags & acfFileIdentifiersFilter)
                {
                    IfFailGo(Update());
                    Assert(m_scriptContext);
                    IdentifierContext identifierContext(m_scriptContext, completions, completions->Alloc(), m_primaryFile->FileId(), this);
                    ParseNodeVisitor<IdentifierPreorderAddPolicy> visitor;
                    ResetHurryAndAbort();
                    Phase(afpExecuting);
                    visitor.Visit(m_primaryTree.TreeRoot(), &identifierContext);
                    Phase(afpDormant);
                }

                if (flags & acfSyntaxElementsFilter)
                {
                    if (!cursor.RightOfDot() && !isObjectLiteral)
                    {
                        IfFailGo(AddReservedWordsFor(&localArena, offset, completions));
                    }
                }

                bool literalCompletionUnavailable = false;
                if (flags & acfMembersFilter)
                {
                    if (cursor.InAJumpStatement())
                    {
                        IfFailGo(GetLabelCompletions(offset, &localArena, completions));
                    }
                    else
                    {
                        if (isObjectLiteral)
                        {
                            OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetCompletions - Fetching literal completions\n");

                            bool objectLiteralCompletionAvailable = false;
                            IfFailGo(GetObjectLiteralCompletions(offset, &localArena, completions, &objectLiteralCompletionAvailable));
                            if (!objectLiteralCompletionAvailable)
                            {
                                literalCompletionUnavailable = true;
                            }
                        }
                        else
                        {
                            IfFailGo(GetSymbolCompletions(offset, &localArena, completions, &symbolValue));
                        }
                    }
                }

                if (literalCompletionUnavailable)
                {
                    completions.ReleaseAndNull();
                }
                else
                {
                    if (m_scriptContextPath->WasContextFileHalted())
                    {
                        completions->SetDiagnosticFlags(acdfContextFileHalted);
                    }

                    // Avoid calling the extensibility callback if the caller is just asking for the identifier list or just the syntax elements
                    if (!m_parseOnlyScriptContext && flags != acfFileIdentifiersFilter && flags != acfSyntaxElementsFilter)
                    {
                        Assert(IsScriptContextValid());

                        // The previous cursor might not be valid because one of the above calls could cause a 
                        // reparse. This can happen, for example, when the primary file invokes a script loader.
                        ParseNodeCursor cursor(&localArena, &m_primaryTree);
                        cursor.SeekToOffset(offset);

                        LangSvcExtensibility extensibility(completions->Alloc(), m_scriptContext, this);
                        extensibility.FireOnCompletion(completions, offset, symbolValue, cursor.LeftOfDotIdentifier());
                    }
                }
            }
        }

        if(completions)
        {
            completions->ExtendLifetime();
        }

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Output::Print(L"Primary instruction executed: %d\n", InstructionsExecuted(m_scriptContext));
#endif
#endif
        
        // Define FORCE_LS_RECYCLING to diagnose garbage collection related issues in the language service. This forces garbage collection to occur 
        // when all the lifetimes have been extended.
#ifdef FORCE_LS_RECYCLING
        m_scriptContext->GetThreadContext()->GetRecycler()->CollectNow<CollectionFlags::CollectNowForceInThread>();
#endif
        *result = completions.Detach();
        
        STDMETHOD_POSTFIX_CLEAN_START;
        Phase(afpDormant);
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetCompletionsEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetCompletions Ended\n");
        OUTPUT_TRACE(Js::JSLSStatsPhase, L"GetCompletions time %8.3f, ended @ absolute time %10.3f\n", timer.Now() - start, g_timer.Now());
        OUTPUT_FLUSH();

        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    STDMETHODIMP FileAuthoring::GetCompletionHint(long completionCookie, BSTR *result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        hr = E_NOTIMPL;
        *result = NULL;

Error:
        return hr;
    }

    STDMETHODIMP FileAuthoring::GetQuickInfo(long offset, AuthorFileRegion *extent, IAuthorSymbolHelp **result)
    {
        return DebugApiWrapper([&]
        {
        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetQuickInfo Started, offset : %d\n", offset);

        AutoSetAuthorFileContext autoSetAuthorFileContext(this->m_rootScriptContext, this->m_fileContext);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetQuickInfoBegin);

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif

        ResetHurryAndAbort();

        *result = nullptr;

        Js::Var value = nullptr;
        Js::RecyclableObject* parentObject = nullptr;

        ValidateArg(result);

        if (m_polluted)
            ReleasePrimaryContext();

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));

        {
            ArenaAllocator localArena(L"ls: QuickInfo", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            ParseNodeCursor cursor(&localArena, &m_primaryTree);
            auto parseNode = cursor.SeekToOffset(offset);
            if((parseNode == nullptr) || (parseNode->nop != knopName && parseNode->nop != knopThis))
                goto Error;

            if (extent)
            {
                if (IdentifierLikeNode(parseNode))
                {
                    extent->offset = parseNode->ichMin;
                    extent->length = parseNode->ichLim - parseNode->ichMin;
                }
                else
                {
                    extent->offset = offset;
                    extent->length = 0;
                }
            }

            Js::InternalString* name = nullptr;
            switch(parseNode->nop)
            {
            case knopName:
            case knopStr:
                name = AllocInternalString(&localArena, parseNode->sxPid.pid, true);
                break;
            case knopThis:
                name = AllocInternalString(&localArena, Names::_this);
                break;
            }
            if(!name)
                goto Error;

            int fileId = -1;
            charcount_t declarationPos = 0;
            AuthorScope scope = ascopeUnknown;
            bool found = false;
            auto fileAuthoring = this;
            IfFailGo(GetSingleCompletion(&localArena, fileAuthoring, offset, name, [&](Completions::InternalCompletion* completion) 
            {
                if (completion && completion->hintInfo)
                {
                    completion->FetchPropertyValue(fileAuthoring);
                    value = completion->value;
                    parentObject = completion->parentObject;
                    if(completion->hintInfo->fileId >= 0)
                    {
                        fileId = completion->hintInfo->fileId;
                        declarationPos = completion->hintInfo->funcSourceMin;
                    }
                    scope = completion->hintInfo->scope;
                    found = true;
                }
            }));

            if (!found)
                goto Error;

            Assert(IsScriptContextValid());

            IfFailGo(CreateSymbolHelp(m_scriptContext, parentObject, value, name->GetBuffer(), scope, atUnknown, fileId, declarationPos, result));
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        Phase(afpDormant);
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetQuickInfoEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetQuickInfo Ended\n");
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    STDMETHODIMP FileAuthoring::HurryImpl(int phaseId, bool isAbort)
    {
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendHurryCalled);

        if (m_progress && m_hurrySkipped < HURRY_SKIP_LIMIT)
        {
            // Using an intellisense extension the executing code indicates that it is actually making progress and is not
            // stuck in an infinite loop. We should give it more time to execute. Record that we saw the progress and then 
            // continue as if hurry wasn't called. We will eventually ignore progress calls as the code might be wrong about 
            // making progress.
            m_hurrySkipped++;
            m_progress = false;
            return S_OK;
        }

        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::Hurry Called,  @ absolute time %10.3f\n", g_timer.Now());

        m_hurryCalled = true;

        // Protecting this clause with a critical section. This check and access of m_currentProbe is a race condition,
        // because Hurry() is called from a separate thread. 
        AutoCriticalSection autocs(&hurryCriticalSection);
        if (m_currentProbe && m_executingContext && (phaseId == 0 || phaseId == m_phaseId))
        {
            OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::Hurry Called : Enabling Async break, starting @ absolute time %10.2f\n", Js::HiResTimer::GetSystemTime());
            OUTPUT_FLUSH();
            // Cause a break of execution. Since we probably didn't hit the probe before this call, this will
            // cause a re-execution using a rewritten tree that forces evaluation to the caret.
            m_currentProbe->EnableAsyncBreak(m_executingContext, isAbort);
        }

        return S_OK;
    }

    STDMETHODIMP FileAuthoring::Hurry(int phaseId)
    {
        return DebugApiWrapper([&]
        {
            return this->HurryImpl(phaseId, /* isAbort = */false);
        });
    }

    STDMETHODIMP FileAuthoring::Abort()
    {
        return DebugApiWrapper([&]
        {
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendAbortCalled);

        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::Abort Called,  @ absolute time %10.3f\n", g_timer.Now());
        m_abortCalled = true;
        return HurryImpl(m_phaseId, /* isAbort = */true);
        });
    }

    STDMETHODIMP FileAuthoring::GetASTAsJSON(BSTR *result)
    {
        return DebugApiWrapper([&]
        {
        ResetHurryAndAbort();

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetASTAsJSON Started\n");

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTBegin);

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif

        *result = NULL;
        TextBuffer *buffer = NULL;

        ArenaAllocator localArena(L"ls: ASTJSON", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), NULL);
        IfFailGo(Update());
        buffer = TextBuffer::New(&localArena);
        Assert(m_primaryFile);

        IfFailGo(SerializeTreeIntoJSON(buffer, &localArena));

        Phase(afpDormant);

        *result = buffer->ToBSTR();

        STDMETHOD_POSTFIX_CLEAN_START;
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetASTAsJSON Ended\n");
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    STDMETHODIMP FileAuthoring::GetASTCursor(__out IAuthorParseNodeCursor **result)
    {
        return DebugApiWrapper([&]
        {
        ResetHurryAndAbort();

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetASTCursor Started\n");

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTCursorBegin);

        *result = NULL;

        CreateASTCursor(result);

        STDMETHOD_POSTFIX_CLEAN_START;
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetASTCursorEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetASTCursor Ended\n");
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    HRESULT FileAuthoring::GetFunctionValueAt(long offset, ArenaAllocator *alloc, Js::Var& callTargetValue, Js::RecyclableObject*& parentObject)
    {
        METHOD_PREFIX;

        callTargetValue = nullptr;
        parentObject = nullptr;

        IfFailGo(ApplyContext());

        IfFailGo(UpdatePrimary(true));

        // Introduce scope
        { 
            // Find the node that contains the cursor.
            ParseNodeCursor cursor(alloc, &m_primaryTree);
            cursor.SeekToOffset(offset);

            ParseNode* callNode = cursor.Up(IsInCallArgs(offset, m_primaryTree.LanguageServiceExtension(), /* members = */nullptr, /* usedProperties =*/nullptr),
                InFuncBodyOrObject(offset, m_primaryTree.LanguageServiceExtension(), /* stopOnObjectLiteral = */false));

            Assert(callNode);

            ParseNode* target = callNode->sxCall.pnodeTarget;
            Assert(target);

            // In case of a member function, we need the value of the object in order to initialize parentObject output value.
            if(target->nop == knopDot && target->sxBin.pnode1 && IsNameNode(target->sxBin.pnode2))
            {
                auto leftOfDotNode = target->sxBin.pnode1;

                // Perform the execution to get the value
                Js::Var leftOfDotValue = nullptr;
                IfFailGo(GetValueOf(alloc, leftOfDotNode, nullptr, leftOfDotValue));
                Js::RecyclableObject* leftOfDotObj = nullptr;
                leftOfDotValue = UnwrapUndefined(leftOfDotValue);

                Assert(IsScriptContextValid());

                // AsLeftOfDotObject converts TaggedNumber to NumberPrototype and returns null for undefined.
                if(AsLeftOfDotObject(m_scriptContext, leftOfDotValue, leftOfDotObj))
                {
                    IdentPtr name = target->sxBin.pnode2->sxPid.pid;
                    Js::PropertyId id = m_scriptContext->GetOrAddPropertyIdTracked(name->Psz(), name->Cch());
                    IfFailGo(GetPropertyOf(leftOfDotObj, id, m_scriptContext, &callTargetValue));
                    parentObject = leftOfDotObj;
                }
            }
            else
            {
                // Do not force register load of 'super'. Super will be loaded into a register regardless and adding a property access
                // to super before the target call operation, will result in the incorrect register being recorded for 'super()' scenarios.
                if (callNode->nop == knopCall && callNode->sxCall.pnodeTarget->nop != knopSuper)
                {
                    // Ensure that the call target expression is always evaluated.
                    ForceRegisterLoad(callNode, m_primaryTree.GetParser());

                    // Indicate the tree was mutated
                    m_primaryTree.SetMutated(ParseNodeTree::mutForceRegisterLoad);
                }

                // Perform the execution to get the value
                IfFailGo(GetValueOf(alloc, target, nullptr, callTargetValue));
            }
        }

        METHOD_POSTFIX;
    }

    // Gets function help information
    STDMETHODIMP FileAuthoring::GetFunctionHelp(
        __in long offset,
        __in AuthorFunctionHelpFlags flags,
        __out_ecount(1) DWORD *currentParameterIndex, 
        __inout_opt AuthorFileRegion *extent,
        __out AuthorDiagStatus *diagStatus,
        __out IAuthorFunctionHelp **result 
        )
    {
        return DebugApiWrapper([&]
        {
        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetFunctionHelp Started, offset %d, @ absolute time %10.3f\n", offset, g_timer.Now());

        AutoSetAuthorFileContext autoSetAuthorFileContext(this->m_rootScriptContext, this->m_fileContext);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetFunctionHelpBegin);

        ValidateArg(result);
        ValidateArg(currentParameterIndex);

        *result = NULL;
        *currentParameterIndex = 0;
        *diagStatus  = adsSuccess;

        ResetHurryAndAbort();

        if (m_polluted)
        {
            ReleasePrimaryContext();
        }

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));

#ifdef DEBUG
        IfFailGo(TestHasActiveCursors());
#endif

        ArenaAllocator localArena(L"ls:GetFunctionHelp", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

        // Introduce scope
        { 
            // Find the node that contains the cursor.
            ParseNodeCursor cursor(&localArena, &m_primaryTree);
            cursor.SeekToOffset(offset);

            if ((flags & afhfImplicitRequest) && !IsImplicitFunctionHelpSupported(&cursor))
            {
                *diagStatus = adsOperationNotApplicableAtLocation;
                return S_OK;
            }

            ParseNode* callNode = cursor.Up(IsInCallArgs(offset, m_primaryTree.LanguageServiceExtension(), /* members = */nullptr, /* usedProperties =*/nullptr),
                InFuncBodyOrObject(offset, m_primaryTree.LanguageServiceExtension(), /* stopOnObjectLiteral = */true));
            if(!callNode)
            {
                // No parent knopCall/knopNew node
                *diagStatus = adsOperationNotApplicableAtLocation;
                return S_OK;
            }

            ParseNode* funcOrProgNode = cursor.Up(IsFuncDeclOrProg); 
            if(!funcOrProgNode)
            {
                // No parent function / program
                *diagStatus = adsInvalidAST;
                return S_OK;
            }

            // Get call target node
            ParseNode* target = callNode->sxCall.pnodeTarget;
            if(!target)
            {
                *diagStatus = adsInvalidAST;
                return S_OK;
            }

            charcount_t extentMin = callNode->ichMin;
            charcount_t extentLim = callNode->ichLim;
            LPCWSTR funcAlias = nullptr;
            switch(target->nop)
            {
            case knopName:
                {
                    if(target->sxPid.pid != nullptr)
                    {
                        // When calling via an identifier use its name as the function name
                        funcAlias = target->sxPid.pid->Psz();
                    }
                    break;
                }
            case knopDot:
                {
                    // When calling a method, use its name as the function name
                    auto rhs = target->sxBin.pnode2;
                    if((rhs != nullptr) && (rhs->nop == knopName) && (rhs->sxPid.pid != nullptr))
                    {
                        funcAlias = rhs->sxPid.pid->Psz();
                        // In case if object.method(|) the extent should be:
                        //                   ^-------^
                        extentMin = rhs->ichMin;
                    }
                    break;
                }
            }

            if (InternalName(funcAlias))
            {
                *diagStatus = adsFunctionDeclarationUnavailable;
                return S_OK;
            }

            // The parse tree might be invalidated during execution, keeping a local copy of the funcAlias
            wchar_t* localFuncAlias = String::Copy(&localArena, funcAlias);

            if (extent != NULL)
            {
                // Set the extent
                extent->offset = extentMin;
                extent->length = extentLim - extentMin;
            }

            // Determine the index of the argument at the location. Do it prior to operations which mutate the tree.
            *currentParameterIndex = GetArgumentIndex(callNode, offset);

            Js::Var callTargetValue = nullptr;
            Js::RecyclableObject* parentObject = nullptr;

            // Executing the primary file may result in detecting new asynchrony script requests, which means that the context is 
            // not complete. If detected, retry.
            hr = EnsureQuiescentContext([&]()->HRESULT 
            {
                INTERNALMETHOD_PREFIX;

                IfFailGoInternal(GetFunctionValueAt(offset, &localArena, callTargetValue, parentObject));

                INTERNALMETHOD_POSTFIX;
            });
            IfFailGo(hr);

            callTargetValue = UnwrapUndefined(callTargetValue);

            if (!callTargetValue)
            {
                *diagStatus = adsNoEvaluationResult; 
                return S_OK;
            }

            Js::JavascriptFunction* funcObj = nullptr;
            if(!Convert::FromVar(nullptr, callTargetValue, funcObj))
            {
                // We're expecting a function
                *diagStatus = adsUnexpectedEvaluationResult;
                return S_OK;
            }

            IfFailGo(TestAbort());

            IfFailGo(GetFunctionHelp(localFuncAlias, funcObj, parentObject, diagStatus, result));
            
            // No function help returned
            if(!(*result)) 
                return S_OK;

            Assert(IsScriptContextValid());
            
            LangSvcExtensibility extensibility(&localArena, m_scriptContext, this);
            extensibility.FireOnParameterHelp(m_rootScriptContext->GetThreadContext()->GetPageAllocator(), 
                *result, offset, Js::JavascriptFunction::FromVar(callTargetValue), parentObject);
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        Phase(afpDormant);
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetFunctionHelpEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetFunctionHelp Ended, @ absolute time %10.3f\n", g_timer.Now());
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    bool FileAuthoring::TryToGetLocationFromFieldDefinition(ArenaAllocator* arena, Js::RecyclableObject* parentObject, Js::InternalString* fieldName, int* fileId, long* sourceOffset)
    {
        // if the user was referencing through a field,
        // first look for a field def to go to where the field was defined
        if (parentObject)
        {
            Assert(fieldName);

            // Try to see if there is a field doc member in the parent object.
            auto fieldDocObj = JsValueDoc::GetFieldDoc(arena, parentObject, fieldName->GetBuffer(), m_scriptContext);
            if(fieldDocObj)
            {
                // If the field documentation is for field that was annotated, we want the original not the annotation.
                fieldDocObj = fieldDocObj->Original();
                *fileId = fieldDocObj->fileId;
                *sourceOffset = fieldDocObj->pos;
                return true;
            }
        }

        return false;
    }

    bool FileAuthoring::TryToGetLocationFromObjectDefinitionLocation(ArenaAllocator* arena, Js::RecyclableObject* instance, int* fileId, long* sourceOffset)
    {
        // if object has a defined location use that
        if (instance)
        {
            // See if there is a location definition for the object.
            auto location = JsHelpers::GetProperty<Js::RecyclableObject *>(instance, Names::_defLoc, arena, m_scriptContext);
            if (location && location->GetTypeId() == Js::TypeIds_Object)
            {
                *sourceOffset = JsHelpers::GetProperty<int>(location, Names::offset, arena, m_scriptContext);
                *fileId = JsHelpers::GetProperty<int>(location, Names::fileId, arena, m_scriptContext);
                return true;
            }
        }

        return false;
    }

    bool FileAuthoring::TryToGetLocationInformationFromFunctionParseInformation(ArenaAllocator* arena, Js::RecyclableObject* functionObject, int* fileId, long* sourceOffset)
    {
        if (Js::JavascriptFunction::Is(functionObject))
        {
            Assert(IsScriptContextValid());

            // look for a function redirection
            auto redirect = JsHelpers::GetProperty<Js::RecyclableObject*>(functionObject, Names::def, arena, m_scriptContext);
            if (redirect && Js::JavascriptFunction::Is(redirect))
            {
                functionObject = redirect;
            }

            auto function = Js::JavascriptFunction::FromVar(functionObject);
            auto body = function->GetParseableFunctionInfo();

            // Built-in functions won't have a body
            if (body)
            {
                *sourceOffset = (long)body->StartInDocument();
                *fileId = this->GetFileIdOf(body->GetSourceIndex());

                // Parse the function to see if we can find a better location for the name.
            
                AuthoringFileHandle* handle = GetAuthoringFileById(*fileId);
                if (handle)
                {
                    auto functionNameLocation = handle->GetFunctionNameLocation(arena, m_scriptContext, function);
                    if (functionNameLocation)
                    {
                        *sourceOffset = functionNameLocation;
                    }
                }

                return true;
            }
        }

        // Not a function or Built-in function
        return false;
    }

    STDMETHODIMP FileAuthoring::GetDefinitionLocation(long offset, __out IAuthorFileHandle **fileHandle, __out long *location)
    {
        return DebugApiWrapper([&]
        {
        ResetHurryAndAbort();

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetDefinitionLocation Started, offset %d\n", offset);

        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetDefinitionLocationBegin);

        ValidateArg(fileHandle);
        ValidateArg(location);

        *fileHandle = NULL;
        *location = 0;

        if (m_polluted)
        {
            ReleasePrimaryContext();
        }

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));

        {
            ArenaAllocator localArena(L"ls:GetDefinitionLocation", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            ParseNodeCursor cursor(&localArena, &m_primaryTree);
            auto parseNode = cursor.SeekToOffset(offset);
            if (!parseNode)
                goto Error;

            // Special case break and continue
            if ((parseNode->nop == knopBreak || parseNode->nop == knopContinue) && parseNode->sxJump.pnodeTarget)
            {
                *location = parseNode->sxJump.pnodeTarget->ichMin;
                *fileHandle = m_primaryFile;
                m_primaryFile->AddRef();
                goto Error;
            }

            if(parseNode->nop != knopName || !parseNode->sxPid.pid)
                goto Error;

            Js::Var value = nullptr;
            Js::RecyclableObject* parentObject = nullptr;
            int fileId = -1;
            long sourceOffset = 0;
            auto name = AllocInternalString(&localArena, parseNode->sxPid.pid, true);
            bool found = false;

            IfFailGo(GetSingleCompletion(&localArena, this, offset, name, [&](Completions::InternalCompletion* completion) 
            {
                if (completion && completion->hintInfo)
                {
                    found = true;
                    value = completion->value;
                    parentObject = completion->parentObject;
                    if(completion->hintInfo->fileId > 0)
                    {
                        fileId = completion->hintInfo->fileId;
                        sourceOffset = (long)completion->hintInfo->funcSourceMin;
                    }
                }
            }));

            if (found)
            {
                if (fileId <= 0 &&
                    !TryToGetLocationFromFieldDefinition(&localArena, parentObject, name, &fileId, &sourceOffset) &&
                    value && 
                    Js::RecyclableObject::Is(value))
                {               
                    auto instance = Js::RecyclableObject::FromVar(value);

                    if (!TryToGetLocationFromObjectDefinitionLocation(&localArena, instance, &fileId, &sourceOffset) &&
                        !TryToGetLocationInformationFromFunctionParseInformation(&localArena, instance, &fileId, &sourceOffset))
                    {
                        // we need to exit because it is a builtin function or the object has no defined location
                        // never try to goto def on a built in
                        goto Error;
                    }
                }

                AuthoringFileHandle *handle = GetAuthoringFileById(fileId);

                if (!handle || handle->IsDisableRewrite()) goto Error;

                // If we are in the primary file, try to find a better offset of the identifier by finding the original node.
                if (handle == m_primaryFile)
                {
                    ParseNodeCursor cursor(&localArena, &m_primaryTree);
                    auto definingNode = cursor.SeekToOffset(sourceOffset);
                    if (definingNode)
                    {
                        switch (definingNode->nop)
                        {
                        case knopVarDecl:
                        case knopLetDecl:
                        case knopConstDecl:
                            {
                                auto newOffset = m_primaryTree.LanguageServiceExtension()->IdentMin(definingNode);
                                if (newOffset != 0) sourceOffset = newOffset;
                                break;
                            }
                        case knopName:
                            sourceOffset = definingNode->ichMin;
                            break;
                        case knopDot:
                            if (definingNode->sxBin.pnode2 && definingNode->sxBin.pnode2->nop == knopName)
                                sourceOffset = definingNode->sxBin.pnode2->ichMin;
                            break;
                        }
                    }
                }

                *location = sourceOffset;
                *fileHandle = handle;
                handle->AddRef();
            }
        }

        STDMETHOD_POSTFIX_CLEAN_START;
        DecommitUnusedPages();
        CODEMARKER(Microsoft::Internal::Performance::CodeMarkerEvent::perfBrowserTools_LanguageServiceBackendGetDefinitionLocationEnd);
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetDefinitionLocation Ended, location %d\n", *location);
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    STDMETHODIMP FileAuthoring::GetReferences(long offset, __out IAuthorReferenceSet **result)
    {
        return DebugApiWrapper([&]
        {
        ReferenceSet *references = nullptr;

        ResetHurryAndAbort();

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetReferences Started, offset %d\n", offset);

        ValidateArg(result);

        IfFailGo(UpdatePrimary(false));

        references = new ReferenceSet(m_scriptContext->GetThreadContext()->GetPageAllocator());

        IdentPtr symbol = nullptr;
        if (m_primaryTree.CollectReferences(offset, references->GetReferenceList(), symbol) && symbol)
            references->SetIdentifer(symbol);

        *result = references;
        references = nullptr;

        STDMETHOD_POSTFIX_CLEAN_START;

        if (references) references->Release();

        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetReferences Ended\n", offset);
        STDMETHOD_POSTFIX_CLEAN_END;
        });
    }

    HRESULT FileAuthoring::GetDiagnostics(__out IAuthorDiagnostics **result)
    {
        return DebugApiWrapper([&]
        {
        STDMETHOD_PREFIX;
#ifdef DEBUG
        ValidateArg(result);
        auto authorDiagnostics = AuthorDiagnostics::CreateInstance(m_rootScriptContext);
        ValidateAlloc(authorDiagnostics);
        *result = authorDiagnostics;
#else
        hr = E_NOTIMPL;
#endif
        STDMETHOD_POSTFIX;
        });
    }

    HRESULT FileAuthoring::GetStructure(__out IAuthorStructure **result)
    {
        return DebugApiWrapper([&]
        {
            return GetStructureImpl(result);
        });
    }

#pragma optimize( "", off )
    HRESULT FileAuthoring::GetStructureImpl(__out IAuthorStructure **result)
    {
        ResetHurryAndAbort();
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        Js::HiResTimer timer;
        double start = timer.Now();
#endif

        ArenaAllocator local(L"ls:Local:Structure", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

        STDMETHOD_PREFIX;
        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetStructure Started\n");

        AutoSetAuthorFileContext autoSetAuthorFileContext(this->m_rootScriptContext, this->m_fileContext);

        ValidateArg(result);

        IfFailGo(ApplyContext());
        IfFailGo(UpdatePrimary(true));
        if (CONFIG_FLAG(InvalidateSolutionContextsForGetStructure))
        {
            Assert(m_scriptContextPath);
            m_scriptContextPath->ApplyUsageFlagsUp(ScriptContextUsageFlags::UsedForStructure);
        }

        IfFailGo(TestAbort());

        IAuthorStructure *structure;
        *result = structure = NewStructure(this);

        // Collect the static structure before any modifications are made for execution.
        AddStaticStructure(structure, &local, m_primaryTree.TreeRoot());
        
        // Execute the primary script and then collect the results of the execution.
        Assert(IsScriptContextValid());

        {
            TemporaryAssignment<bool> t(m_shouldCopyOnGet, true);

            ArenaAllocator localArena(L"ls:GetFunctionHelp", m_rootScriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            // Insert a value at the end of the file that we can find that will normally be findable.
            ASTBuilder<> ast(m_primaryTree.GetParser(), m_primaryTree.TreeRoot());
            auto lim = m_primaryTree.TreeRoot()->ichLim;
            ast.SetExtent(lim, lim);
            auto valueNode = ast.Object(nullptr);
            ast.Append(ast.If(ast.Boolean(false), ast.Assign(ast.Name(L"_$tmp"), valueNode), nullptr));
            Js::Var value;
            IfFailGo(GetValueOf(&localArena, valueNode, nullptr, value));
        }

        bool aborted = false;
        try
        {
            Phase(AuthorFileAuthoringPhase::afpExecuting);
            AddDynamicStructure(structure, &local, m_scriptContext, m_primaryFile, &m_primaryTree);
        }
        catch (ExecutionStop)
        {
            aborted = true;
        }
        Phase(AuthorFileAuthoringPhase::afpDormant);

        if (CONFIG_FLAG(InvalidateSolutionContextsForGetStructure))
        {
            Assert(m_scriptContextPath);
            // Invalidate script contexts created for structure -- this has perf impact as we no longer cache some solution files,
            // but what's more important for us is that memory is not growing significantly when opening a solution file.
            m_scriptContextPath->InvalidateUpForStructure();
        }

        // Invalidate leaf script context -- we don't need it anymore.
        m_leafScriptContext->Invalidate();

        OUTPUT_TRACE(Js::JSLSPhase, L"FileAuthoring::GetStructure Ended\n");
        OUTPUT_TRACE(Js::JSLSStatsPhase, L"GetStructure time %8.3f\n", timer.Now() - start);
        IfFailGo(aborted ? E_ABORT : S_OK);
        STDMETHOD_POSTFIX;
    }
#pragma optimize( "", on )

    HRESULT FileAuthoring::SetCompletionDiagnosticCallback(__in IAuthorCompletionDiagnosticCallback *acdCallback)
    {
        return DebugApiWrapper([&]
        {
        if (m_diagCallback)
        {
            m_diagCallback->Release();
        }
        m_diagCallback = acdCallback;
        if (m_diagCallback)
        {
            m_diagCallback->AddRef();
        }

        return S_OK;
        });
    }

    // Ensure the execution does not result in a non-quiescent context.
    // The context is deemed non-quiescent if while execution an asynchronous request for a file that is not in the context
    // was detected. This is marked by the HRESULT value of hresNonQuiescentContext.
    // Upon detecting that the context is not quiescent, ForgetContext is triggered and the current routine is called again.
    // The routine will not be allowed to terminate as long as the context is missing files, and abort was not triggered.
    //
    // NOTE: This method assumes the routine will check for the context and reapply it as appropriate.
    template<typename TRoutine>
    HRESULT FileAuthoring::EnsureQuiescentContext(TRoutine routine)
    {
        HRESULT hr = S_OK;
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        int i = 0;
#endif
        while (true)
        {
            hr = TestAbort();
            if (FAILED(hr))
                return hr;

            hr = routine();

            // Make sure the context is quiescent
            if (hr == hresNonQuiescentContext)
            {
                // A new script was added to the context as we were precessing context files.
                // Forget the context and retry.
                ForgetContext();

                OUTPUT_TRACE(Js::JSLSPhase, L"EnsureQuiescentContext : Trying %d\n", ++i);

                continue;
            }

            return hr;
        }
    }

    template<typename TDoneHandler>
    HRESULT FileAuthoring::GetSingleCompletion(ArenaAllocator* alloc, FileAuthoring* fileAuthoring, long offset, Js::InternalString* itemName, TDoneHandler doneHandler)
    {
        RefCountedPtr<Completions> completions;
        Js::Var value            = nullptr;
        METHOD_PREFIX;
        Assert(alloc);
        Assert(itemName);
        completions.TakeOwnership(Completions::New(this));
        IfFailGo(GetSymbolCompletions(offset, alloc, completions, &value));
        // Find the symbol in the completions list
        auto completion = completions->Find(itemName);
        if (completion)
        {
            doneHandler(completion);
        }
        METHOD_POSTFIX;
    }

    void FileAuthoring::AddCompletionList(Completions* completions)
    {
        Assert(completions);
        _completionLists.Add(completions);
    }

    void FileAuthoring::RemoveCompletionList(Completions* completions)
    {
        Assert(completions);
        _completionLists.Remove(completions);
    }

    FileAuthoring* FileAuthoring::CreateSnapshot()
    {
        auto snapshot = new FileAuthoring(m_factory, m_authoringServicesInfo, m_rootScriptContext, m_fileContext);
        auto alloc = snapshot->Alloc();
        if (m_leafScriptContext)
        {
            snapshot->m_leafScriptContext = m_leafScriptContext->CreateSnapshot(snapshot->m_contextAlloc, snapshot);
            snapshot->m_leafScriptContext->GetScriptContext(snapshot, &snapshot->m_scriptContext);
        }
        snapshot->m_scriptContextPath.Assign(
            m_scriptContextPath->CreateSnapshot(alloc));
        snapshot->m_primaryFile.TakeOwnership(m_primaryFile->CreateSnapshot());
        snapshot->m_contextChanged = false;
        snapshot->m_scriptContextPathComplete = true;
        return snapshot;
    }

    bool FileAuthoring::IsHurryCalled()
    {
        AutoCriticalSection acs(&(this->hurryLock));
        return this->m_hurryCalled;
    }

    void FileAuthoring::SetHurryCalled(bool hurryCalled)
    {
        AutoCriticalSection acs(&(this->hurryLock));
        this->m_hurryCalled = hurryCalled;
    }
}
