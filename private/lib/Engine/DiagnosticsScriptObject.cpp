//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    FunctionInfo DiagnosticsScriptObject::EntryInfo::GetStackTrace(FORCE_NO_WRITE_BARRIER_TAG(DiagnosticsScriptObject::EntryGetStackTrace), FunctionInfo::ErrorOnNew);
    FunctionInfo DiagnosticsScriptObject::EntryInfo::DebugEval(FORCE_NO_WRITE_BARRIER_TAG(DiagnosticsScriptObject::EntryDebugEval), FunctionInfo::ErrorOnNew);
    FunctionInfo DiagnosticsScriptObject::EntryInfo::GetConsoleScopeObject(FORCE_NO_WRITE_BARRIER_TAG(DiagnosticsScriptObject::EntryGetConsoleScopeObject), FunctionInfo::ErrorOnNew);

#ifdef EDIT_AND_CONTINUE
    FunctionInfo DiagnosticsScriptObject::EntryInfo::EditSource(FORCE_NO_WRITE_BARRIER_TAG(DiagnosticsScriptObject::EntryEditSource), FunctionInfo::ErrorOnNew);
#endif

    DiagnosticsScriptObject::DiagnosticsScriptObject(DynamicType * type)
        : DynamicObject(type)
    {
        ScriptContext* scriptContext = type->GetScriptContext();

        Assert(scriptContext != nullptr);
        Assert(scriptContext->IsDiagnosticsScriptContext());

        functionNameId = scriptContext->GetOrAddPropertyIdTracked(_u("functionName"));
        urlId = scriptContext->GetOrAddPropertyIdTracked(_u("documentUrl"));
        documentId = scriptContext->GetOrAddPropertyIdTracked(_u("documentID"));
        lineId = scriptContext->GetOrAddPropertyIdTracked(_u("line"));
        columnId = scriptContext->GetOrAddPropertyIdTracked(_u("column"));
    }

    /*static*/
    DiagnosticsScriptObject * DiagnosticsScriptObject::New(Recycler * recycler, DynamicType * type)
    {
        return NewObject<DiagnosticsScriptObject>(recycler, type);
    }

    template <size_t N>
    void DiagnosticsScriptObject::SetPropertyStatic(RecyclableObject* obj, const char16(&propertyName)[N], Var value,
        ScriptContext* scriptContext)
    {
        const PropertyRecord* propertyRecord;
        scriptContext->GetOrAddPropertyRecord(propertyName, &propertyRecord);
        obj->SetProperty(propertyRecord->GetPropertyId(), value, PropertyOperation_Force, nullptr);
    }

    template <size_t N>
    void DiagnosticsScriptObject::SetPropertyStatic(RecyclableObject* obj, const char16(&propertyName)[N], BSTR value,
        ScriptContext* scriptContext)
    {
        SetPropertyStatic(obj, propertyName, JavascriptString::NewCopyBuffer(value, SysStringLen(value), scriptContext), scriptContext);
    }

    /*static*/
    Var DiagnosticsScriptObject::EntryGetStackTrace(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (!scriptContext->IsDiagnosticsScriptContext())
        {
            // This must be called from the diagnostics scriptengine
            Assert(false);
            return scriptContext->GetLibrary()->GetUndefined();
        }

        if (!scriptContext->IsRunningScript())
        {
            // Not on script execution, early bailout.
            return scriptContext->GetLibrary()->GetUndefined();
        }

        ushort maxFrameCount = USHORT_MAX;

        if (args.Info.Count > 1 && Js::TaggedInt::Is(args[1]) && TaggedInt::ToUInt32(args[1]) < USHORT_MAX)
        {
            maxFrameCount = TaggedInt::ToUInt16(args[1]);
        }

        Js::JavascriptStackWalker walker(scriptContext);
        Js::JavascriptArray * arrayObject = scriptContext->GetLibrary()->CreateArray();

        ScriptSite* scriptSite = ScriptSite::FromScriptContext(scriptContext);
        Js::DiagnosticsScriptObject *scriptObject = scriptSite->GetActiveScriptExternalLibrary()->GetDiagnosticsScriptObect();
        Assert(scriptObject != nullptr);

        ushort frameCount = walker.WalkUntil((ushort)maxFrameCount, [&](Js::JavascriptFunction* function, ushort frameIndex) -> bool
        {
            ULONG lineNumber = 0;
            LONG columnNumber = 0;
            const char16* name = nullptr;
            Js::Var urlVar = scriptContext->GetLibrary()->GetUndefined();
            Var documentIdVar = nullptr;

            if (function->IsScriptFunction() && !function->IsLibraryCode())
            {
                Js::FunctionBody * functionBody = function->GetFunctionBody();
                name = functionBody->GetExternalDisplayName();
                functionBody->GetLineCharOffset(walker.GetByteCodeOffset(), &lineNumber, &columnNumber);

                BSTR sourceName;
                bool ret = functionBody->GetExternalDisplaySourceName(&sourceName);
                Assert(ret == true);

                urlVar = Js::JavascriptString::NewCopyBuffer(sourceName, ::SysStringLen(sourceName), scriptContext);
                ::SysFreeString(sourceName);

                if (functionBody->GetUtf8SourceInfo()->HasDebugDocument() && functionBody->GetUtf8SourceInfo()->GetDebugDocument()->HasDocumentText())
                {
                    documentIdVar = Js::JavascriptNumber::ToVar((uint64)functionBody->GetUtf8SourceInfo()->GetDebugDocument()->GetDocumentText(), scriptContext);
                }
                else
                {
                    documentIdVar = Js::JavascriptNumber::ToVar(0, scriptContext);
                }
            }
            else
            {
                name = function->IsScriptFunction() ? function->GetFunctionBody()->GetExternalDisplayName()
                                                    : walker.GetCurrentNativeLibraryEntryName();

                documentIdVar = Js::JavascriptNumber::ToVar(0, scriptContext);
            }
            Js::DynamicObject * object = scriptContext->GetLibrary()->CreateObject();

            // Populate object with fields functionName, url, documentId (IDebugDocumentText), line and column
            // TODO : in future we could avoid this type transition by creating an object with a type which has these properties already defined, so that 
            // we could avoid the type transition.

            object->SetProperty(scriptObject->GetFunctionNameId(),
                scriptContext->GetLibrary()->CreateStringObject(name, wcslen(name)),
                (Js::PropertyOperationFlags)PropertyDynamicTypeDefaults, nullptr/*PropertyValueInfo*/);

            object->SetProperty(scriptObject->GetUrlId(), urlVar, (Js::PropertyOperationFlags)PropertyDynamicTypeDefaults, nullptr/*PropertyValueInfo*/);

            object->SetProperty(scriptObject->GetDocumentId(), documentIdVar, (Js::PropertyOperationFlags)PropertyDynamicTypeDefaults, nullptr/*PropertyValueInfo*/);

            object->SetProperty(scriptObject->GetLineId(),
                Js::JavascriptNumber::ToVar((int32)lineNumber, scriptContext),
                (Js::PropertyOperationFlags)PropertyDynamicTypeDefaults,
                nullptr/*PropertyValueInfo*/);

            object->SetProperty(scriptObject->GetColumnId(),
                Js::JavascriptNumber::ToVar((int32)columnNumber, scriptContext),
                (Js::PropertyOperationFlags)PropertyDynamicTypeDefaults,
                nullptr/*PropertyValueInfo*/);

            arrayObject->SetItem(frameIndex, object, Js::PropertyOperationFlags::PropertyOperation_None);

            return false;
        }, false /*onlyondebugmode*/, true/*filter the diagnostics scriptengine*/);

        if (frameCount == 0)
        {
            return scriptContext->GetLibrary()->GetUndefined();
        }
        Assert(frameCount == (ushort)arrayObject->GetLength());
        return arrayObject;
    }

    Var DiagnosticsScriptObject::EntryGetConsoleScopeObject(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* diagnosticScriptContext = function->GetScriptContext();
        PROBE_STACK(diagnosticScriptContext, Js::Constants::MinStackDefault);
        Assert(!(callInfo.Flags & CallFlags_New));

        RUNTIME_ARGUMENTS(args, callInfo);

        Var consoleScopeObj = function->GetLibrary()->GetUndefined();
        if (diagnosticScriptContext->IsDiagnosticsScriptContext() && RecyclableObject::Is(args[0]))
        {
            // Use args[0] as the this object
            ScriptContext* targetScriptContext = RecyclableObject::FromVar(args[0])->GetScriptContext();
            if (!targetScriptContext->IsClosed())
            {
                DebugManager* debugManager = targetScriptContext->GetDebugContext()->GetProbeContainer()->GetDebugManager();
                consoleScopeObj = debugManager->GetConsoleScope(targetScriptContext);
            }
        }

        return consoleScopeObj;
    }

    // 
    // diagnosticsScriptObject.debugEval(sourceText, isNonUserCode, shouldRegisterDocument).
    // Parameters:
    //   sourceText: the text to eval.
    //   isNonUserCode: whether we should treat this as external to script code, like library code.
    //                  Non-user code will not show up in file picker, stack trace, etc.
    //   shouldRegisterDocument: Should this text be registered with PDM,
    //                  text entered by user in console is not library code and should not be registered

    // -----------------------------------------------------------------------------------------
    //     isNonUserCode | shouldRegisterDocument |                  Action
    // -----------------------------------------------------------------------------------------
    //        true       |     true or false      | Not registered - not visible in file picker
    //                                            | Not visible in stack
    // -----------------------------------------------------------------------------------------
    //        false      |        false           | Not registered - not visible in file picker
    //                                            | if break happens in this code we will register - CheckAndRegisterFuncToDiag call in ProbeContainer::Dispatch*
    //                                            | Visible in stack
    // -----------------------------------------------------------------------------------------
    //        false      |        true            | Registered - Visible in file picker
    //                                            | Visible in stack
    // -----------------------------------------------------------------------------------------

    Var DiagnosticsScriptObject::EntryDebugEval(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* debugEvalScriptContext = function->GetScriptContext();
        PROBE_STACK(debugEvalScriptContext, Js::Constants::MinStackDefault);
        Assert(!(callInfo.Flags & CallFlags_New));
        AssertMsg(!(callInfo.Flags & CallFlags_Eval), "CallFlags_Eval (presense of frame display) is not supported by debugEval (for simplicity).");

        RUNTIME_ARGUMENTS(args, callInfo);

        // Environment is not there (frameDisplay as in case of regular eval) => we can treat any arg as Var.
        if (args.Info.Count < 3)
        {
            // isNonUserCode is required parameter.
            return function->GetLibrary()->GetUndefined();
        }

        // Execute in the context of "this" which is expected to be a (global) object from the engine debugEval need to run in.
        if (!RecyclableObject::Is(args[0]))
        {
            // Failed to deduct the global object.
            return function->GetLibrary()->GetUndefined();
        }

        // Arguments: this, evalString, isNonUserCode, shouldRegisterDocument.

        JavascriptLibrary* targetLibrary;
        ScriptContext* targetScriptContext;

        RecyclableObject* thisObj = RecyclableObject::FromVar(args[0]);
        Assert(thisObj->GetType());
        targetLibrary = thisObj->GetLibrary();
        targetScriptContext = targetLibrary->GetScriptContext();

        // Note: scriptContext here can be from main engine, or diagnostics (this) engine.
        // In case it's from main engine, make sure it's alive.
        targetScriptContext->VerifyAlive(!function->IsExternal());

        Var isNonUserCodeArg = args[2];
        if (JavascriptOperators::GetTypeId(isNonUserCodeArg) != TypeIds_Boolean)
        {
            // Wrong value passed to isNonUserCode argument.
            return function->GetLibrary()->GetUndefined();
        }

        bool isLibraryCode = !!JavascriptBoolean::FromVar(isNonUserCodeArg)->GetValue();

        bool registerDocument = !isLibraryCode; // LibraryCode should not be registered
        
        // Treating this has optional for not breaking existing use of debugEval
        if (args.Info.Count > 3) 
        {
            if (!isLibraryCode) // shouldRegisterDocument is no-op if it is LibraryCode
            {
                Var shouldRegisterDocument = args[3];
                if (JavascriptOperators::GetTypeId(shouldRegisterDocument) != TypeIds_Boolean)
                {
                    // Wrong value passed to shouldRegisterDocument argument.
                    return function->GetLibrary()->GetUndefined();
                }
                registerDocument = !!JavascriptBoolean::FromVar(shouldRegisterDocument)->GetValue();
            }
            --args.Info.Count; // Remove registerDocument argument
        }

        --args.Info.Count;  // Remove isNonUserCodeArg argument

        bool isStrictMode = false;
        Js::DynamicObject * emptyTopMostScope = targetScriptContext->GetLibrary()->CreateActivationObject();
        emptyTopMostScope->SetPrototype(targetScriptContext->GetLibrary()->GetNull());
        DynamicObject* activeScopeObject = GetActiveScopeObject(targetScriptContext, &isStrictMode);
        Js::DebugManager* debugManager = targetScriptContext->GetDebugContext()->GetProbeContainer()->GetDebugManager();
        FrameDisplay* environment = debugManager->GetFrameDisplay(targetScriptContext, emptyTopMostScope, activeScopeObject);

        JavascriptExceptionObject* reThrownEx = nullptr;
        try
        {
            OUTPUT_TRACE(Js::ConsoleScopePhase, _u("EntryDebugEval strictMode = %d, isLibraryCode = %d, source = '%s'\n"), isStrictMode, isLibraryCode, JavascriptString::FromVar(args[1])->GetSz());
            Var value = GlobalObject::VEval(targetLibrary, environment, kmodGlobal, isStrictMode, /*isIndirect=*/ false, args, isLibraryCode, registerDocument, fscrConsoleScopeEval);
            value = CrossSite::MarshalVar(debugEvalScriptContext, value);  // MarshalVar if needed.
            Assert(!CrossSite::NeedMarshalVar(value, debugEvalScriptContext));
            debugManager->UpdateConsoleScope(emptyTopMostScope, targetScriptContext);
            return value;
        }
        catch (const Js::JavascriptException& err)
        {
            // Rethrowing inside catch will blow the stack up since the stack is not unwound while in the C++ catch. Save the exception and rethrow outside the catch block.
            reThrownEx = err.GetAndClear();
        }

        AssertMsg(reThrownEx, "How come we don't have an exception object here?");
        Var rethrownObject = reThrownEx->GetThrownObject(targetScriptContext);
        if (rethrownObject && RecyclableObject::Is(rethrownObject) && CrossSite::NeedMarshalVar(rethrownObject, debugEvalScriptContext))
        {
            // DebugEval runs in diagnostics context with different CMDID_SCRIPTSITE_SID then user page
            if (JavascriptError::Is(rethrownObject))
            {
                // Exception (JavaScript error) thrown from user page will fail the ScriptSite::CheckCrossDomainScriptContext check resulting in a Permission Denied error message
                // Create a new error, copy basic stuff (line no. and message) and throw that instead
                JavascriptError* jsErrorObject = JavascriptError::FromVar(rethrownObject);
                JavascriptError* jsNewErrorObject = jsErrorObject->CloneErrorMsgAndNumber(debugEvalScriptContext->GetLibrary());
                AssertMsg(jsNewErrorObject != nullptr, "Error shouldn't have been null");
                reThrownEx->ReplaceThrownObject(jsNewErrorObject);
            }
            else
            {
                // If it is a object we can't throw it, just get the toString value of the object and throw that instead
                JavascriptString* stringValue = JavascriptConversion::ToString(rethrownObject, targetScriptContext);
                AssertMsg(stringValue != nullptr, "Failed to get string value of thrown object");
                RecyclableObject* stringObj = stringValue->CloneToScriptContext(debugEvalScriptContext);
                reThrownEx->ReplaceThrownObject(stringObj);
            }
        }

        debugManager->UpdateConsoleScope(emptyTopMostScope, targetScriptContext);
        JavascriptExceptionOperators::DoThrow(reThrownEx, targetScriptContext);
    }

    // When in break state, uses debugger scopes to populate locals from stack frames.
    // Otherwise returns empty environment.
    // static
    DynamicObject* DiagnosticsScriptObject::GetActiveScopeObject(ScriptContext* targetScriptContext, bool *isStrictMode)
    {
        Assert(targetScriptContext);

        DynamicObject* activeScopeObject = nullptr;
        *isStrictMode = false;

        // In break state F12 needs to be able to see user variables defined in stack frames below e.g. in console.
        // In non-break state F12 doesn't need that.
        // The good thing is that in break state we can take advantage of debugger scopes and populate (captured) local using them.
        // In non-break state we can't but it's not needed.
        if (targetScriptContext->IsScriptContextInDebugMode() &&
            targetScriptContext->GetThreadContext()->GetDebugManager()->IsAtDispatchHalt())
        {
            Assert(targetScriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena());
            ArenaAllocator* diagArena = targetScriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena()->Arena();
            DiagStackFrame* frm = nullptr;

            // Get the top-most frame matching targetScriptContext.
            JavascriptStackWalker walker(targetScriptContext, false);
            walker.WalkUntil([&](JavascriptFunction* func, ushort frameIndex) -> bool
            {
                if (!func->IsLibraryCode())
                {
                    InterpreterStackFrame *interpreterFrame = walker.GetCurrentInterpreterFrame();
                    ScriptContext* frameScriptContext = walker.GetCurrentScriptContext();
                    Assert(frameScriptContext);

                    if (frameScriptContext == targetScriptContext)
                    {
                        if (interpreterFrame)
                        {
                            frm = Anew(diagArena, DiagInterpreterStackFrame, interpreterFrame);
                            
                        }
                        else if (func->IsScriptFunction())
                        {
                            frm = Anew(diagArena, DiagNativeStackFrame,
                                ScriptFunction::FromVar(walker.GetCurrentFunction()), walker.GetByteCodeOffset(), walker.GetCurrentArgv(), walker.GetCurrentCodeAddr());
                        }
                    }
                }

                if (frm)
                {
                    *isStrictMode = !!func->IsStrictMode();

                    if (targetScriptContext->GetThreadContext()->GetDebugManager()->IsMatchTopFrameStackAddress(frm))
                    {
                        frm->SetIsTopFrame();
                    }
                }

                return frm != nullptr;
            });

            // Use LocalsWalker to populate frame display with variables.
            if (frm)
            {
                LocalsWalker* localsWalker = Anew(diagArena, Js::LocalsWalker, frm, 
                    Js::FrameWalkerFlags::FW_EnumWithScopeAlso | Js::FrameWalkerFlags::FW_AllowLexicalThis | Js::FrameWalkerFlags::FW_AllowSuperReference | Js::FrameWalkerFlags::FW_DontAddGlobalsDirectly);
                activeScopeObject = localsWalker->CreateAndPopulateActivationObject(targetScriptContext, [](Js::ResolvedObject& resolveObject){});
            }
        }

        return activeScopeObject;
    }

#ifdef EDIT_AND_CONTINUE
    /*static*/
    Var DiagnosticsScriptObject::EntryEditSource(RecyclableObject* function, CallInfo callInfo, ...)
    {
        ScriptContext* scriptContext = function->GetScriptContext();
        PROBE_STACK(scriptContext, Js::Constants::MinStackDefault);
        RUNTIME_ARGUMENTS(args, callInfo);

        if (args.Info.Count < 4
            || !RecyclableObject::Is(args[1])
            || !JavascriptString::Is(args[2])
            || !JavascriptString::Is(args[3]))
        {
            return function->GetLibrary()->GetUndefined();
        }

        // args[1]: target global object
        RecyclableObject* targetObj = RecyclableObject::FromVar(args[1]);
        ScriptContext* targetScriptContext = targetObj->GetScriptContext();
        IActiveScriptDirect* pActiveScriptDirect = targetScriptContext->GetActiveScriptDirect();

        // args[2]: longDocumentId
        IDebugDocumentText* debugDocumentText = reinterpret_cast<IDebugDocumentText*>(
            _wcstoui64(JavascriptString::FromVar(args[2])->GetSz(), nullptr, 10));

        // args[3]: newSource
        JavascriptString* newSource = JavascriptString::FromVar(args[3]);

        // Result object
        RecyclableObject* ret = scriptContext->GetLibrary()->CreateObject();

        HRESULT hr = S_OK;
        AutoScriptEditResult result;
        BEGIN_LEAVE_SCRIPT(scriptContext)
        {
            CComPtr<IScriptEditQuery> pScriptEditQuery;

            CComPtr<IActiveScriptEdit> pActiveScriptEdit;
            IfFailGo(pActiveScriptDirect->QueryInterface(&pActiveScriptEdit));
            {
                ULONG length;
                IfFailGo(debugDocumentText->GetSize(nullptr, &length));

                // Query single edit
                ScriptEditRequest request = { debugDocumentText, { /*startOffset*/0, length }, newSource->GetString(), newSource->GetLength() };
                IfFailGo(pActiveScriptEdit->QueryEdit(&request, 1, &pScriptEditQuery));
            }

            BOOL canApply;
            IfFailGo(pScriptEditQuery->CanApply(&canApply));
            if (canApply)
            {
                IfFailGo(pScriptEditQuery->CommitEdit());
            }

            OUTPUT_TRACE(Phase::ENCPhase, _u("EnC: Cannot apply edit\n"));

            ULONG count;
            IfFailGo(pScriptEditQuery->GetResultCount(&count));
            if (count != 1)
            {
                IfFailGo(E_UNEXPECTED);
            }
            IfFailGo(pScriptEditQuery->GetResult(0, &result));
        Error:
            ;
        }
        END_LEAVE_SCRIPT(scriptContext);

        if (FAILED(hr))
        {
            JavascriptError::MapAndThrowError(scriptContext, hr);
        }

        if (result.newDebugDocumentText)
        {
            WCHAR buf[32];
            _ui64tow_s(reinterpret_cast<UINT64>(result.newDebugDocumentText), buf, _countof(buf), 10);
            SetPropertyStatic(ret, _u("newDocId"), JavascriptString::NewCopySz(buf, scriptContext), scriptContext);
        }
        if (result.message)
        {
            OUTPUT_TRACE(Phase::ENCPhase, _u("EnC: Compile error: %ls, line %d, column %d\n"), result.message, result.line, result.column);
            OUTPUT_FLUSH();

            RecyclableObject* error = scriptContext->GetLibrary()->CreateObject();
            SetPropertyStatic(error, _u("message"), result.message, scriptContext);
            SetPropertyStatic(error, _u("line"), JavascriptNumber::ToVar(static_cast<uint32>(result.line), scriptContext), scriptContext);
            SetPropertyStatic(error, _u("column"), JavascriptNumber::ToVar(static_cast<uint32>(result.column), scriptContext), scriptContext);

            SetPropertyStatic(ret, _u("error"), error, scriptContext);
        }

        return ret;
    }
#endif

} // namespace Js
