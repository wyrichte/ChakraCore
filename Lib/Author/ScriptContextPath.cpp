//
//    Copyright (C) Microsoft.  All rights reserved.
//
#include "stdafx.h"

namespace Authoring
{
    class JavascriptLibraryAccessor
    {
    public:
        template <size_t N>
        static Js::JavascriptFunction * AddFunctionToLibraryObjectWithPropertyName(Js::JavascriptLibrary* library, Js::DynamicObject* object, const wchar_t(&propertyName)[N], Js::FunctionInfo * functionInfo, int length)
        {
            Assert(library != nullptr);
            return library->AddFunctionToLibraryObjectWithPropertyName(object, propertyName, functionInfo, length);
        }
    };
    namespace Names
    {
        const wchar_t getCallerLocation[] = L"_$getCallerLocation";
        const wchar_t getCallerName[] = L"_$getCallerName";
        const wchar_t getTrackingUndefined[] = L"_$getTrackingUndefined";
        const wchar_t getTrackingNull[] = L"_$getTrackingNull";
        const wchar_t isProxyObject[] = L"_$isProxyObject";
        const wchar_t hasThisStmt[] = L"_$hasThisStmt";
        const wchar_t progress[] = L"_$progress";
        const wchar_t getExecutingScriptFileName[] = L"_$getExecutingScriptFileName";
        const wchar_t trace[] = L"_$trace";
        const wchar_t createProxyWithoutTarget[] = L"_$createProxyWithoutTarget";
        const wchar_t updateProxyTarget[] = L"_$updateProxyTarget";
        const wchar_t logStack[] = L"_$logStack";
        const wchar_t enableCallGraph[] = L"_$enableCallGraph";
    }

    TYPE_STATS(ScriptContextPath, L"ScriptContextPath")
    TYPE_STATS(LeafScriptContext, L"LeafScriptContext")

    void ScriptContextPath::AddLeaf(LeafScriptContext *leaf)
    {
        leaf->m_nextLeaf = m_activeLeaves;
        m_activeLeaves = leaf;
    }

    void ScriptContextPath::RemoveLeaf(LeafScriptContext *leaf)
    {
        LeafScriptContext **current = &m_activeLeaves;
        while (*current)
        {
            if (*current == leaf)
            {
                *current = leaf->m_nextLeaf;
                break;
            }
            else
                current = &(*current)->m_nextLeaf;
        }
    }

    ScriptContextPath::~ScriptContextPath()
    {
        Assert(m_nexts.Count() == 0); // Ensure we are not released when one of the paths is still active.
        if (m_parent)
        {
            m_parent->m_nexts.Remove(m_handle);
            m_parent.ReleaseAndNull();
        }
        if (m_handle)
        {
            m_handle->RemoveContext(this);
            m_handle.ReleaseAndNull();
        }
    }

    static Js::Var GetTrackingUndefined(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto authoringData = scriptContext->authoringData;
        if (authoringData && authoringData->Callbacks() && callInfo.Count >= 2)
        {
            ARGUMENTS(args, callInfo);
            Js::Var result = nullptr;
            HRESULT hr = S_OK;
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                result = authoringData->Callbacks()->GetTrackingKey(scriptContext, args[1], Js::TypeIds_Undefined);
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
            if (SUCCEEDED(hr))
                return result;
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var GetTrackingNull(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto authoringData = scriptContext->authoringData;
        if (authoringData && authoringData->Callbacks() && callInfo.Count >= 2)
        {
            ARGUMENTS(args, callInfo);
            Js::Var result = nullptr;
            HRESULT hr = S_OK;
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                result = authoringData->Callbacks()->GetTrackingKey(scriptContext, args[1], Js::TypeIds_Null);
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
            if (SUCCEEDED(hr))
                return result;
        }

        return scriptContext->GetLibrary()->GetNull();
    }

    static Js::Var IsProxyObject(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        ARGUMENTS(args, callInfo);
        Js::Var result = nullptr;
        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            if (args.Info.Count >= 2)
            {
                auto param = args[1];
                if (Js::DynamicType::Is(Js::JavascriptOperators::GetTypeId(param)))
                {
                    auto object = Js::DynamicObject::FromVar(args[1]);
                    if (object->IsCopyOnWriteProxy())
                        return scriptContext->GetLibrary()->GetTrue();
                }
            }
            return scriptContext->GetLibrary()->GetFalse();
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        if (SUCCEEDED(hr) && result)
            return result;
        return scriptContext->GetLibrary()->GetUndefined();
    }

    static void LogMessage(Js::ScriptContext *scriptContext, const wchar_t* text)
    {
        scriptContext->GetThreadContext()->GetAuthoringContext()->LogMessage(CComBSTR(text));
    }

    static Js::Var Trace(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (PHASE_TRACE1(Js::JSLSScriptTracePhase))
        {
            ARGUMENTS(args, callInfo);
            HRESULT hr = S_OK;
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                if (args.Info.Count >= 2)
                {
                    if (Js::JavascriptString::Is(args[1]))
                    {
                        auto object = Js::JavascriptString::FromVar(args[1]);
                        if (object->GetSz())
                        {
                            bool shouldLogMessageToHost = (args.Info.Count >= 3 && Js::JavascriptBoolean::Is(args[2]) && Js::JavascriptBoolean::FromVar(args[2])->GetValue());
                            if (!shouldLogMessageToHost)
                            {
                                Output::Print(object->GetSz());
                                Output::Flush();
                            }
                            else if (scriptContext->GetThreadContext()->GetAuthoringContext() != nullptr)
                            {
                                LogMessage(scriptContext, object->GetSz());
                            }
                        }
                    }
                }
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        }
#endif
        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var LogStack(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        ARGUMENTS(args, callInfo);
        HRESULT hr = S_OK;

        if (scriptContext->GetThreadContext()->GetAuthoringContext() != nullptr && args.Info.Count >= 2)
        {
            // I am enabling this even on fre build. If needed we could put this under ENABLE_DEBUG_CONFIG_OPTIONS
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT_NESTED
            {
                BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"LSLogStack");

                Assert(Js::TaggedInt::Is(args[1]));
                ushort maxFrameCount = Js::TaggedInt::ToUInt16(args[1]);

                TextBuffer stackText(tempAllocator);
                stackText.Add(L"Stack trace :\n");

                Js::JavascriptStackWalker walker(scriptContext);
                walker.WalkUntil((ushort)maxFrameCount, [&](Js::JavascriptFunction* function, ushort frameIndex) -> bool
                {
                    ULONG lineNumber = 0;
                    LONG columnNumber = 0;
                    WCHAR frameName[512];
                    ZeroMemory(frameName, _countof(frameName));
                    if (function->IsScriptFunction() && !function->IsLibraryCode())
                    {
                        Js::FunctionBody * functionBody = function->GetFunctionBody();
                        functionBody->GetLineCharOffset(walker.GetByteCodeOffset(), &lineNumber, &columnNumber);
                        AuthoringFileHandle *fileHandle = nullptr;
                        if (scriptContext->authoringData && scriptContext->authoringData->Callbacks())
                        {
                            fileHandle = (AuthoringFileHandle *)scriptContext->authoringData->Callbacks()->GetAuthorSource(functionBody->GetSourceIndex(), scriptContext);
                        }
                        if (fileHandle && !fileHandle->IsLibraryCode())
                        {
                            CComBSTR name;
                            fileHandle->GetDisplayName(&name);
                            swprintf_s(frameName, _countof(frameName), L"\tat %ls (%ls:%d:%d)\n",
                                functionBody->GetExternalDisplayName(),
                                name ? name.m_str : L"<no name>",
                                lineNumber + 1,
                                columnNumber + 1);
                            stackText.Add(frameName);
                        }
                    }

                    return false;
                });

                LogMessage(scriptContext, stackText.Sz());
                END_TEMP_ALLOCATOR(tempAllocator, scriptContext);
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var EnableCallGraph(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        ARGUMENTS(args, callInfo);
        if (args.Info.Count >= 2 && Js::JavascriptBoolean::Is(args[1])
            && scriptContext->authoringData && scriptContext->authoringData->Callbacks())
        {
            bool toEnable = !!Js::JavascriptBoolean::FromVar(args[1])->GetValue();
            scriptContext->authoringData->Callbacks()->SetCallGraph(toEnable);

            LogMessage(scriptContext, toEnable ? L"Call graph logging is enabled\n" : L"Call graph logging is disabled\n");
        }
#endif
        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var ForceCatchException(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        ARGUMENTS(args, callInfo);
        Assert(args.Info.Count >= 2 && Js::JavascriptError::Is(args[1]));

        Js::JavascriptExceptionOperators::ThrowForceCatchException(Js::JavascriptError::FromVar(args[1]), scriptContext);

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Js::FunctionInfo forceCatchExceptionInfo(ForceCatchException);

    static Js::Var CreateProxyWithoutTarget(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto recycler = scriptContext->GetRecycler();
        return RecyclerNew(recycler, Js::JavascriptProxy, scriptContext->GetLibrary()->GetProxyType());
    }

    Js::FunctionInfo createProxyWithoutTargetInfo(CreateProxyWithoutTarget);

    static Js::Var UpdateProxyTarget(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        ARGUMENTS(args, callInfo);

        if (args.Info.Count >= 3)
        {
            AssertMsg(Js::JavascriptProxy::Is(args[1]), "The 1st argument to updateProxyTarget is expected to be an lazy instance created by createProxyWithoutTarget");
            AssertMsg(Js::RecyclableObject::Is(args[2]), "The 2nd argument to updateProxyTarget is expected to be a RecyclableObject");
            if (Js::JavascriptProxy::Is(args[1]))
            {
                // For primitives - we will never need the blank proxy - so no need to update that at all.
                if (Js::JavascriptOperators::IsObjectType(Js::JavascriptOperators::GetTypeId(args[2])))
                {
                    auto proxy = Js::JavascriptProxy::FromVar(args[1]);
                    auto target = Js::RecyclableObject::FromVar(args[2]);
                    proxy->SneakyUpdateTargetByLanguageService(target);
                }
            }
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    Js::FunctionInfo updateProxyTargetInfo(UpdateProxyTarget);

    static Js::Var Progress(Js::RecyclableObject* jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        if (scriptContext->authoringData)
            scriptContext->authoringData->Callbacks()->Progress();
        return scriptContext->GetLibrary()->GetUndefined();
    }

    template< size_t N >
    void SetProperty(Js::ScriptContext *scriptContext, Js::DynamicObject *object, const wchar_t (&propertyNameLiteral)[N], Js::Var value)
    {
        auto id = GetOrAddPropertyIdFromLiteral(scriptContext, propertyNameLiteral);
        object->SetProperty(id, value, Js::PropertyOperation_None, nullptr);
    }

    static bool WalkUpNJavascriptFrames(Js::JavascriptStackWalker &walker, int count)
    {
        while (count > 0)
        {
            if (!walker.Walk()) break;
            if (walker.IsJavascriptFrame() && --count <= 0) return true;
        }
        return false;
    }

    static bool GetCallerFileIdAndOffset(Js::ScriptContext *scriptContext, int *fileId, int *offset)
    {
        Assert(scriptContext);
        Assert(fileId);
        Assert(offset);

        Js::JavascriptStackWalker walker(scriptContext);

        // This is called from a helper and we want the helper's caller's caller, so we want the function call 3 frames up.
        if (WalkUpNJavascriptFrames(walker, 3))
        {
            auto function = walker.GetCurrentFunction();
            if (function && function->GetFunctionInfo() && function->GetFunctionProxy()->IsFunctionBody())
            {
                auto body = function->GetFunctionBody();
                if (body)
                {
                    auto byteCodeOffset = walker.GetByteCodeOffset();
                    auto sourceMap = body->GetMatchingStatementMapFromByteCode(byteCodeOffset, true);
                    if (sourceMap)
                    {
                        *offset = sourceMap->sourceSpan.Begin();
                        auto sourceIndex = body->GetSourceIndex();
                        *fileId = scriptContext->authoringData->Callbacks()->GetFileIdOfSourceIndex(scriptContext, sourceIndex);

                        return true;
                    }
                }
            }
        }

        return false;
    }

    static Js::Var GetCallerLocation(Js::RecyclableObject *jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        ARGUMENTS(args, callInfo);
        Js::Var result = nullptr;
        HRESULT hr = S_OK;
        BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
        {
            int fileId;
            int offset;

            if (GetCallerFileIdAndOffset(scriptContext, &fileId, &offset))
            {
                auto resultObject = scriptContext->GetLibrary()->CreateObject();
                SetProperty(scriptContext, resultObject, Names::offset, Js::TaggedInt::ToVarUnchecked(offset));
                SetProperty(scriptContext, resultObject, Names::fileId, Js::TaggedInt::ToVarUnchecked(fileId));
                result = resultObject;
            }
        }
        END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        if (SUCCEEDED(hr) && result)
            return result;
        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var GetCallerName(Js::RecyclableObject *jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto authoringData = scriptContext->authoringData;

        ARGUMENTS(args, callInfo);
        Js::Var result = nullptr;
        HRESULT hr = S_OK;

        if (authoringData && authoringData->Callbacks())
        {
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                int fileId;
                int offset;

                if (GetCallerFileIdAndOffset(scriptContext, &fileId, &offset))
                {
                    result = authoringData->Callbacks()->GetCallerName(scriptContext, fileId, offset);
                }
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
            if (SUCCEEDED(hr) && result)
                return result;
        }
        return scriptContext->GetLibrary()->GetUndefined();
    }

    static Js::Var HasThisStmt(Js::RecyclableObject *jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto authoringData = scriptContext->authoringData;

        ARGUMENTS(args, callInfo);

        HRESULT hr = S_OK;

        if (authoringData && authoringData->Callbacks())
        {
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                // Ensure there is at least one real argument (other than the implicit this).
                if (args.Info.Count >= 2)
                {
                    auto param = args[1];
                    if (Js::JavascriptFunction::Is(param))
                    {
                        auto function = Js::JavascriptFunction::FromVar(param);
                        if (authoringData->Callbacks()->HasThisStmt(scriptContext, function))
                            return scriptContext->GetLibrary()->GetTrue();
                    }
                }
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
        }

        return scriptContext->GetLibrary()->GetFalse();
    }

    static Js::Var GetExecutingScriptFileName(Js::RecyclableObject *jsFuncVar, Js::CallInfo callInfo, ...)
    {
        auto scriptContext = jsFuncVar->GetScriptContext();
        auto authoringData = scriptContext->authoringData;

        ARGUMENTS(args, callInfo);
        Js::Var result = nullptr;
        HRESULT hr = S_OK;

        if (authoringData && authoringData->Callbacks())
        {
            BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
            {
                result = authoringData->Callbacks()->GetExecutingScriptFileName(scriptContext);
            }
            END_TRANSLATE_EXCEPTION_TO_HRESULT(hr);
            if (SUCCEEDED(hr) && result)
                return result;
        }
        return scriptContext->GetLibrary()->GetUndefined();
    }

    template< size_t N >
    void SetFunctionProperty(Js::ScriptContext *scriptContext, Js::DynamicObject *object, const wchar_t (&propertyNameLiteral)[N], Js::JavascriptMethod entryPoint)
    {
        auto id = GetOrAddPropertyIdFromLiteral(scriptContext, propertyNameLiteral);
        auto value = scriptContext->GetLibrary()->CreateExternalFunction(entryPoint, id, nullptr);
        object->SetProperty(id, value, Js::PropertyOperation_None, nullptr);
    }

    static void InjectCallbacks(Js::ScriptContext *scriptContext)
    {
        auto globalObject = scriptContext->GetGlobalObject();

        SetFunctionProperty(scriptContext, globalObject, Names::getTrackingUndefined, GetTrackingUndefined);
        SetFunctionProperty(scriptContext, globalObject, Names::getTrackingNull, GetTrackingNull);
        SetFunctionProperty(scriptContext, globalObject, Names::isProxyObject, IsProxyObject);
        SetFunctionProperty(scriptContext, globalObject, Names::getCallerLocation, GetCallerLocation);
        SetFunctionProperty(scriptContext, globalObject, Names::getCallerName, GetCallerName);
        SetFunctionProperty(scriptContext, globalObject, Names::hasThisStmt, HasThisStmt);
        SetFunctionProperty(scriptContext, globalObject, Names::progress, Progress);
        SetFunctionProperty(scriptContext, globalObject, Names::getExecutingScriptFileName, GetExecutingScriptFileName);
        SetFunctionProperty(scriptContext, globalObject, Names::trace, Trace);
        SetFunctionProperty(scriptContext, globalObject, Names::logStack, LogStack);
        SetFunctionProperty(scriptContext, globalObject, Names::enableCallGraph, EnableCallGraph);

        JavascriptLibraryAccessor::AddFunctionToLibraryObjectWithPropertyName(scriptContext->GetLibrary(), globalObject, Names::createProxyWithoutTarget, &createProxyWithoutTargetInfo, 0);
        JavascriptLibraryAccessor::AddFunctionToLibraryObjectWithPropertyName(scriptContext->GetLibrary(), globalObject, Names::updateProxyTarget, &updateProxyTargetInfo, 0);
        JavascriptLibraryAccessor::AddFunctionToLibraryObjectWithPropertyName(scriptContext->GetLibrary(), globalObject, Names::forceCatchException, &forceCatchExceptionInfo, 1);
    }

    Js::ScriptContext *ScriptContextPath::CreateEmptyScriptContext(Js::HostType hostType)
    {
        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
        AutoPtr<Js::ScriptContext> scriptContext(Js::ScriptContext::New(threadContext));

        Assert(scriptContext != nullptr);

        scriptContext->ForceNoNative();
        scriptContext->SetInDebugMode();
        scriptContext->SetHostType(hostType);
        scriptContext->SetCanOptimizeGlobalLookupFlag(false);
        scriptContext->Initialize();

        InjectCallbacks(scriptContext);

        return scriptContext.Detach();
    }

    struct InitializeScriptContextContext
    {
        PhaseReporter *reporter;
        FileAuthoring *authoring;
        ScriptContextPath *scriptContextPath;
        InitializeScriptContextContext(PhaseReporter *reporter, ScriptContextPath *scriptContextPath): reporter(reporter), 
            authoring(nullptr), scriptContextPath(scriptContextPath) { }
        InitializeScriptContextContext(FileAuthoring *authoring, ScriptContextPath *scriptContextPath): reporter(authoring), 
            authoring(authoring), scriptContextPath(scriptContextPath) { }
    };

    static void InitializeScriptContextCopy(void *initContext, Js::ScriptContext *scriptContext)
    {
        InitializeScriptContextContext *context = (InitializeScriptContextContext *)initContext;
        context->scriptContextPath->InstallAuthoringCallback(scriptContext, context->reporter, context->authoring);
    }

    HRESULT ScriptContextPath::EnsureScriptContext(FileAuthoring *fileAuthoring, Js::ScriptContext **result)
    {
        Assert(fileAuthoring && result);

        METHOD_PREFIX;

        if (!m_scriptContext)
        {
            m_contextFileHalted = false;

            // Don't start anything expensive if we have been aborted.
            IfFailGo(fileAuthoring->WasAbortCalled() ? E_ABORT : S_OK);

            if (m_parent)
            {
                Assert(m_handle); // This is also enforced in the constructor.

                Js::ScriptContext *parentContext; 
                IfFailGo(m_parent->EnsureScriptContext(fileAuthoring, &parentContext));
                size_t scriptContextSize = 0;

                // Record if a previous context file was halted.
                m_contextFileHalted = m_parent->WasContextFileHalted();

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
                Output::Print(L">>> Get scriptContext for file -- [%s]\n", this->GetScriptContextFileName());
#endif
#endif

                // Check if we can take over the parent scriptContext or we need to create a copy
                if (m_parent->CanTakeoverScriptContext())
                {
                    m_parent->TakeoverScriptContext(m_scriptContext);
                    // Update the deferred parsing callbacks to point to the current scriptContextPath instead of the parent
                    RefreshAuthoringCallback(m_scriptContext, fileAuthoring);
                    scriptContextSize = m_parent->GetScriptContextWeight();
                }
                else
                {
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
                    Output::Print(L">>>   Create a copy of parent context -- [%s]\n", m_parent->GetScriptContextFileName());
                    Output::Print(L">>>        parent weight = %i\n", m_parent->GetScriptContextWeight());
#endif
#endif

                    AutoPtr<Js::ScriptContext> scriptContext(nullptr);

                    // Create a copy-on-write copy of the parent script context and
                    // apply the script associated to the handle to it to form
                    // the state of the context after all scripts have been executed up to
                    // the end of the script referenced by the handle.
                    BEGIN_ENTER_SCRIPT(parentContext, false, false, false);
                    {
                        InitializeScriptContextContext initContext(fileAuthoring, this);
                        scriptContext = parentContext->CopyOnWriteCopy(&initContext, InitializeScriptContextCopy);
                    }
                    END_ENTER_SCRIPT;

                    m_scriptContext.TakeOwnership(RefCountedScriptContext::Create(scriptContext.Detach()));

                    // Record that this child copied its scriptContext from the parent
                    m_scriptContextCopiedFromParent = true;
                }

                IfFailGo(fileAuthoring->WasAbortCalled() ? E_ABORT : S_OK);

                IfFailGo(m_handle->Apply(fileAuthoring, m_scriptContext, this, m_parent, /* isPrimaryFile = */(fileAuthoring->GetPrimaryFile() == m_handle)));

                if (this->m_contextFileHalted)
                {
                    IfFailGo(E_ABORT);
                }

                // update the weight of the scriptContext to include the length of the text
                scriptContextSize += m_handle->Text()->Length();
                m_scriptContextSize = scriptContextSize;
            }
            else
            {
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
                Output::Print(L">>> Create scriptContext for root\n");
#endif
#endif
                // For the root context, create a fresh blank context.
                AutoPtr<Js::ScriptContext> scriptContext(nullptr);
                scriptContext = CreateEmptyScriptContext(m_hostType);
                ValidateAlloc(scriptContext);
                m_scriptContext.TakeOwnership(RefCountedScriptContext::Create(scriptContext.Detach()));
                m_scriptContextSize = 0;
            }
        }
        *result = m_scriptContext;

        METHOD_POSTFIX_CLEAN_START;
        if (FAILED(hr))
        {
            m_scriptContext.ReleaseAndNull();
        }
        METHOD_POSTFIX_CLEAN_END;
    }

    bool ScriptContextPath::CanTakeoverScriptContext()
    {
        // Do not allow children to takeover scriptContext if:
        // 1. This is the root context (does not have a parent)
        // 2. The weight of the scriptContext is more than the limit
        // 3. The scriptContext is already copied by another child
        // 4. Has active leaves

        if (m_parent && GetScriptContextWeight() < SCRIPTCONTEXT_TAKEOVER_LIMIT)
        {
            bool copiedByAChild = false;

            // check if any of the children copied the scriptContext
            if (m_nexts.Count() > 0)
            {
                m_nexts.Map([&](AuthoringFileHandle*, ScriptContextPath * current)
                {
                    Assert(current);
                    copiedByAChild |= current->m_scriptContextCopiedFromParent;
                });
            }

            // check for active leaves
            for (LeafScriptContext *current = m_activeLeaves; current && !copiedByAChild; current = current->m_nextLeaf)
            {
                copiedByAChild |= current->IsValid();
            }

            return !copiedByAChild;
        }
        return false;
    }

    void ScriptContextPath::TakeoverScriptContext(ScriptContextAutoPtr& newScriptContext)
    {
        Assert(this->CanTakeoverScriptContext());
        Assert(!newScriptContext);      // make sure we are not deleting an existing script context object

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
        Output::Print(L">>>   taking ownership of parent scriptContext -- [%s]\n", this->GetScriptContextFileName());
        Output::Print(L">>>        parent script size = %i\n", m_scriptContextSize);
        Output::Print(L">>>        number of children = %i\n", m_nexts.Count());
        Output::Print(L">>>        has leaves = %s\n", m_activeLeaves != nullptr ? L"true" : L"false");
        Output::Print(L">>>        new size = %i\n",GetScriptContextWeight());
#endif
#endif

        newScriptContext.Assign(m_scriptContext);
        m_scriptContext.ReleaseAndNull();
    }

    size_t ScriptContextPath::GetScriptContextWeight()
    {
        Assert(m_nexts.Count() > 0);

        return m_scriptContextSize * m_nexts.Count();
    }

    // A path is incomplete if it doesn't reflect all the files requested dynamically by
    // the execution of this context. This call ensures the host knows about all the 
    // context files this file requested during a previous call to Apply().
    HRESULT ScriptContextPath::EnsureCompletePath(PhaseReporter *reporter)
    {
        METHOD_PREFIX;

        bool quiet = true;

        if (m_parent)
        {
            hr = m_parent->EnsureCompletePath(reporter);
            if (hr == hresNonQuiescentContext)
            {
                // Suppress the error for now. We want all the paths to request their async requests
                // so we are likely to let the host know all the files that are needed but we still
                // want to report this as a non-quiet path so this is turned back into a non-quiescent 
                // result after we call the handles EnsureAsyncRequests().
                quiet = false;
                hr = S_OK;
            }
        }

        if (m_dependentAsyncRequests && m_handle)
        {
            IfFailGo(m_handle->EnsureAsyncRequests(reporter, m_dependentAsyncRequests));
        }

        if (!quiet) 
        {
            // Restore suppressed non-quiescent results.
            hr = hresNonQuiescentContext;
        }

        METHOD_POSTFIX;
    }

    struct RuntimeParsingContext
    {
        RuntimeParsingContext(ScriptContextPath* path, AuthoringFileHandle* primaryFile) : m_path(path), m_primaryFile(primaryFile) { }
        ScriptContextPath* m_path;
        AuthoringFileHandle* m_primaryFile;
    };

    void ScriptContextPath::RuntimeParsingCallback(void *context, Parser *parser, LPCUTF8 pszSrc, size_t offset, size_t length, ParseNodePtr pnodeProg)
    {
        Assert(context);
        RuntimeParsingContext* runtimeParsingContext = (RuntimeParsingContext*)context;
        runtimeParsingContext->m_path->ProcessRuntimeParseTree(parser, pszSrc, offset, length, pnodeProg, runtimeParsingContext->m_primaryFile);
    }

    void ScriptContextPath::InstallAuthoringCallback(Js::ScriptContext *scriptContext, PhaseReporter *reporter, FileAuthoring *fileAuthoring)
    {
        Assert(scriptContext);
        auto alloc = scriptContext->GeneralAllocator();
        auto missingValueHandler = Anew(alloc, MissingValueHandler, scriptContext, this, reporter, fileAuthoring);
        auto runtimeParsingContext = Anew(alloc, RuntimeParsingContext, this, fileAuthoring->GetPrimaryFile());
        scriptContext->authoringData = Anew(alloc, RuntimeParseCallbackData, missingValueHandler, runtimeParsingContext, ScriptContextPath::RuntimeParsingCallback);
    }

    void ScriptContextPath::RefreshAuthoringCallback(Js::ScriptContext *scriptContext, FileAuthoring *fileAuthoring)
    {
        Assert(scriptContext);
        Assert(scriptContext->authoringData);
        Assert(scriptContext->authoringData->Callbacks());

        RuntimeParseCallbackData* authoringData = ((RuntimeParseCallbackData*)scriptContext->authoringData);
        Assert(authoringData && authoringData->dataType == RUNTIME_PARSE_CALLBACK_TYPE);

        if (authoringData && authoringData->dataType == RUNTIME_PARSE_CALLBACK_TYPE)
        {
            // update the context object
            RuntimeParsingContext* runtimeParsingContext = (RuntimeParsingContext*)authoringData->context;
            runtimeParsingContext->m_path = this;
            runtimeParsingContext->m_primaryFile = fileAuthoring->GetPrimaryFile();

            // update the missingValueHandler context
            MissingValueHandler* callbacks = (MissingValueHandler*)(authoringData->Callbacks());
            callbacks->SetScriptContextPath(this);
            callbacks->SetFileAuthoring(fileAuthoring);
        }
    }

    void ScriptContextPath::ProcessRuntimeParseTree(Parser *parser, LPCUTF8 pszSrc, size_t offset, size_t length, ParseNodePtr pnodeProg, AuthoringFileHandle* primaryFile)
    {
        auto handle = GetAuthoringFileByBufferLocation(pszSrc);
        if (handle)
            handle->ProcessDeferredParseTree(parser->GetScriptContext(), m_scriptContextManager->GetActivePhaseReporter(), parser, pnodeProg, /* isPrimaryFile = */(handle == primaryFile));
        else
            // If there is no handle assume the parsing came from an eval().
            AuthoringFileHandle::ProcessEvalTree(parser->GetScriptContext(), m_scriptContextManager->GetActivePhaseReporter(), parser, pnodeProg);
    }

    void ScriptContextPath::Invalidate()
    {
        if (m_scriptContext || m_nexts.Count() > 0 || m_activeLeaves)
        {
#ifdef DEBUG
#ifdef VERBOSE_LOGGING
            Output::Print(L">>>  Invalidating script context for file \n");
            Output::Print(L">>>        number of children = %i\n", m_nexts.Count());
            Output::Print(L">>>        has leaves = %s\n", m_activeLeaves != nullptr ? L"true" : L"false");
            Output::Print(L">>>        current size = %i\n",m_scriptContextSize);
#endif
#endif

            // If this context is invalidated then all contexts that are based should also be invalidated.
            // It shouldn't matter but it is cleaner to discard the script contexts that are based on this
            // one prior to discarding it.
            if (m_nexts.Count() > 0)
            {
                m_nexts.Map([](AuthoringFileHandle*, ScriptContextPath * current)
                {
                    Assert(current);
                    current->Invalidate();
                }
                );
            }

            // Invalidate leaves
            for (LeafScriptContext *current = m_activeLeaves; current; current = current->m_nextLeaf)
            {
                current->Invalidate();
            }

            m_version++;

            m_scriptContext.ReleaseAndNull();
            m_dependentAsyncRequests.ReleaseAndNull();
            m_scriptContextSize = 0;
            m_scriptContextCopiedFromParent = false;
        }
    }

    void ScriptContextPath::InstallInlineBreakpointProbes(Js::HaltCallback *pProbe)
    {
        if (m_scriptContext)
            m_scriptContext->diagProbesContainer.InitializeInlineBreakEngine(pProbe);
        if (m_parent)
            m_parent->InstallInlineBreakpointProbes(pProbe);
    }

    void ScriptContextPath::UninstallInlineBreakpointProbes(Js::HaltCallback *pProbe)
    {
        if (m_scriptContext)
            m_scriptContext->diagProbesContainer.UninstallInlineBreakpointProbe(pProbe);
        if (m_parent)
            m_parent->UninstallInlineBreakpointProbes(pProbe);
    }

    ScriptContextPath* ScriptContextPath::Next(AuthoringFileHandle *handle)
    {
        ScriptContextPath *nextPath;

        if (!m_nexts.TryGetValue(handle, &nextPath))
        {
            AutoPtr<ScriptContextPath> newPath = ScriptContextPath::Create(m_alloc, this, handle, m_scriptContextManager);
            m_nexts.Add(handle, newPath);
            handle->AddContext(newPath);
            nextPath = newPath.Detach();
        }
        return nextPath;
    }

    AuthoringFileHandle* ScriptContextPath::GetAuthoringFileById(int fileId)
    {
        ScriptContextPath *current = this;

        while (current)
        {
            if (current->m_handle && current->m_handle->FileId() == fileId)
                return current->m_handle;
            current = current->m_parent;
        }

        return nullptr;
    }

    AuthoringFileHandle* ScriptContextPath::GetAuthoringFileBySourceInfo(Js::Utf8SourceInfo * sourceInfo)
    {
        if (sourceInfo)
        {
            LPCUTF8 sourceBuffer = sourceInfo->GetSource(L"ScriptContextPath::GetAuthoringFileBySourceInfo");
            return GetAuthoringFileByBufferLocation(sourceBuffer);
        }
        return nullptr;
    }

    AuthoringFileHandle* ScriptContextPath::GetAuthoringFileByIndex(Js::ScriptContext *scriptContext, uint sourceIndex)
    {
        return GetAuthoringFileBySourceInfo(scriptContext->GetSource(sourceIndex));
    }

    AuthoringFileHandle* ScriptContextPath::GetAuthoringFileByBufferLocation(LPCUTF8 sourceBuffer)
    {
        if (sourceBuffer) 
        {
            ScriptContextPath *current = this;
            while (current)
            {
                if (current->m_handle && current->m_handle->Text() && current->m_handle->Text()->Buffer() == sourceBuffer)
                    return current->m_handle;
                current = current->m_parent;
            }
        }

        return nullptr;
    }

    LeafScriptContext::LeafScriptContext(ArenaAllocator *alloc, ScriptContextPath *path, ParseNodeTree *dependentTree)
        : m_path(path),
        m_alloc(alloc),
        m_dependentTree(dependentTree),
        m_scriptContext(nullptr),
        m_nextLeaf(nullptr),
        m_fileAuthoring(nullptr),
        m_sourceInfoList(nullptr)
    {
        if (m_path)
        {
            m_path->AddRef();
            m_path->AddLeaf(this);
        }
#if TRACK_TYPE_STATS
        _typeStats.IncrementInstances();
#endif
    }

    LeafScriptContext::~LeafScriptContext()
    {
        RemoveUtf8SourceInfo();

        if (m_path)
        {
            m_path->RemoveLeaf(this);
            m_path->Release();
        }
#if TRACK_TYPE_STATS
        _typeStats.DecrementInstances();
#endif
    }

    HRESULT LeafScriptContext::GetScriptContext(FileAuthoring *fileAuthoring, Js::ScriptContext **result)
    {
        Assert(fileAuthoring && result);
        METHOD_PREFIX;
        if (!m_scriptContext)
        {
            Assert(!m_fileAuthoring);
            m_fileAuthoring = fileAuthoring;

            Js::ScriptContext *parentContext;
            IfFailGo(m_path->EnsureScriptContext(fileAuthoring, &parentContext));

#ifdef DEBUG
#ifdef VERBOSE_LOGGING
            Output::Print(L">>>  Get leaf script context .. Copy parent context -- [%s]\n", m_path->GetScriptContextFileName());
#endif
#endif

            OUTPUT_TRACE(Js::JSLSPhase, L"LeafScriptContext::GetScriptContext called \n");

            BEGIN_ENTER_SCRIPT(parentContext, false, false, false);
            {
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
                Js::HiResTimer timer;
                double start = timer.Now();
#endif

                InitializeScriptContextContext initContext(fileAuthoring, m_path);
                m_scriptContext.TakeOwnership(RefCountedScriptContext::Create(parentContext->CopyOnWriteCopy(&initContext, InitializeScriptContextCopy)));
                PopulateUtf8SourceInfo();
                OUTPUT_TRACE(Js::JSLSStatsPhase, L"LeafScriptContext::GetScriptContext time %8.3f\n", timer.Now() - start);
                OUTPUT_FLUSH();
            }
            END_ENTER_SCRIPT;
        }
        *result = m_scriptContext;
        METHOD_POSTFIX;
    }

    void LeafScriptContext::Invalidate()
    {
        if (m_dependentTree)
        {
            m_dependentTree->ForgetTree();
        }

        RemoveUtf8SourceInfo();
        m_fileAuthoring = nullptr;
        m_scriptContext.ReleaseAndNull();
    }

    void LeafScriptContext::PopulateUtf8SourceInfo()
    {
        if (m_scriptContext)
        {
            m_sourceInfoList = JsUtil::List<Js::Utf8SourceInfo *>::New(m_scriptContext->GetRecycler());
            m_scriptContext->GetRecycler()->RootAddRef(m_sourceInfoList);
            m_scriptContext->GetSourceList()->MapUntil([=](int i, RecyclerWeakReference<Js::Utf8SourceInfo>* sourceInfoWeakRef) -> bool {
                Js::Utf8SourceInfo* sourceInfo = sourceInfoWeakRef->Get();
                if (sourceInfo)
                {
                    m_sourceInfoList->Add(sourceInfo);
                }
                return false;
            });

        }
    }
    void LeafScriptContext::RemoveUtf8SourceInfo()
    {
        if (m_sourceInfoList) // Parser only script context will not have sourceInfoList populated.
        {
            m_sourceInfoList->ClearAndZero();
            if (m_scriptContext)
            {
                m_scriptContext->GetRecycler()->RootRelease(m_sourceInfoList);
            }
            m_sourceInfoList = nullptr;
        }
    }


    LeafScriptContext *LeafScriptContext::CreateSnapshot(ArenaAllocator *alloc, FileAuthoring *fileAuthoring)
    {
        auto result = Anew(alloc, LeafScriptContext, alloc, nullptr, nullptr);
        result->m_scriptContext.Assign(m_scriptContext);
        result->m_fileAuthoring = fileAuthoring;
        result->PopulateUtf8SourceInfo();
        return result;
    }

    LeafScriptContext *ScriptContextPath::CreateLeafScriptContext(ArenaAllocator *alloc, ParseNodeTree *dependantTree)
    {
        return Anew(alloc, LeafScriptContext, alloc, this, dependantTree);
    }

    ScriptContextPath* ScriptContextPath::CreateSnapshot(ArenaAllocator *alloc)
    {
        Assert(alloc);
        ScriptContextPath* snapshot = Anew(alloc, ScriptContextPath, alloc, m_hostType, m_scriptContextManager);
        snapshot->m_scriptContext.Assign(m_scriptContext);
        if (m_handle)
            snapshot->m_handle.TakeOwnership(m_handle->CreateSnapshot());
        if (m_parent)
            snapshot->m_parent.Assign(m_parent->CreateSnapshot(alloc));
        if (m_dependentAsyncRequests)
            snapshot->m_dependentAsyncRequests.Assign(m_dependentAsyncRequests);
        return snapshot;
    }

    void ScriptContextPath::RecordDependentAsyncRequest(AsyncRequest *asyncRequest)
    {
        Assert(asyncRequest);

        if (!m_dependentAsyncRequests) 
        {
            // If this is the first time we use the list, create it
            m_dependentAsyncRequests.Assign(AsyncRequestList::New(Alloc()->GetPageAllocator()));
        }

        Assert(m_dependentAsyncRequests);

        m_dependentAsyncRequests->Add(asyncRequest);
    }

    void ScriptContextPath::ClearDependentAsyncRequest()
    {
        m_dependentAsyncRequests.ReleaseAndNull();
    }

    void ScriptContextPath::CleanupScriptPropertyCaches()
    {
        if (m_scriptContext)
        {
            auto propertyStringMap = m_scriptContext->GetLibrary()->GetPropertyStringMap();

            if (propertyStringMap)
            {
                typedef JsUtil::SimpleDictionaryEntry<Js::PropertyId, RecyclerWeakReference<Js::PropertyString>*> EntryType;
                propertyStringMap->MapAndRemoveIf([](EntryType entry) {
                    return entry.Value() == null || entry.Value()->Get() == null;
                });
            }

        }

        this->m_nexts.Map([](AuthoringFileHandle*, ScriptContextPath * current) {
            current->CleanupScriptPropertyCaches();
        });
    }

    void ScriptContextPath::ApplyUsageFlagsUp(ScriptContextUsageFlags flags)
    {
        ScriptContextPath* node = this;
        while (node)
        {
            if ((node->GetUsageFlags() & flags) == flags)
            {
                // Optimization: we have already applied these flags earlier. Since we only apply them up, 
                // all parent nodes should already have them, so we don't have to go further up from here.
                break;
            }

            node->OrUsageFlags(flags);
            node = node->m_parent;
        }
    }

    void ScriptContextPath::InvalidateUpForStructure()
    {
        // Starting from current and going up, find last node that was used only for structure, also keep the parent just in case.
        // Intereting scenarios:
        // 1) Main references (helpers.js, etc) are used for completions prior to 1st GetStructure call.
        //    Thus, even in simple case of linear tree, we will have these as non-invalidated (i.e. cached).
        // 2) We are going up from one branch and there is another branch used for completions.
        //    We will invalidate all nodes up to and not including common parent, thus we will not invalidate
        //    the branch used for completions, which is good.
        ScriptContextPath* node = this;
        ScriptContextPath* last = nullptr;
        while (node && node->m_parent && node->GetUsageFlags() == ScriptContextUsageFlags::UsedForStructure)
        {
            last = node;
            node = node->m_parent;
        }

        if (last)
        {
            last->Invalidate(); // This will invalidate the node and all children.
        }
    }

#if DEBUG
    void DumpCopyOnWriteTableSize(Js::ScriptContext *scriptContext, int id) 
    {
        Output::Print(L">>> ID: %d, Size %d\n", id, scriptContext->CopyOnWriteTableSize());
    }

    void ScriptContextPath::DumpCopyOnWriteTableSizes()
    {
        if (this->m_parent)
            this->m_parent->DumpCopyOnWriteTableSizes();
        if (this->m_scriptContext && this->m_handle)
            DumpCopyOnWriteTableSize(this->m_scriptContext, this->m_handle->FileId());
    }

    void LeafScriptContext::DumpCopyOnWriteTableSizes()
    {
        if (this->m_path)
            this->m_path->DumpCopyOnWriteTableSizes();
        if (this->m_scriptContext)
            DumpCopyOnWriteTableSize(this->m_scriptContext, 0);
    }

    LPCWSTR ScriptContextPath::GetScriptContextFileName()
    {
        CComBSTR name;
        if (m_handle)
        {
            m_handle->GetDisplayName(&name);
            if (!name) name = L"<no name>";
        }
        else 
            name = L"root";
        return name;
    }

#endif
}
