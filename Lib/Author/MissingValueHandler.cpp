#include "stdafx.h"

namespace Authoring
{
    static const wchar_t fnValueMethod[] = L"_$value";
    static const wchar_t fnElementValueMethod[] = L"_$elementValue";
    static const wchar_t fnParamValue[] = L"_$paramValue";

    static Js::Var CallGlobalFunctionInternal(Js::ScriptContext *scriptContext, LPCWSTR functionName, Js::Var arg0, Js::Var arg1)
    {
        auto globalObject = scriptContext->GetGlobalObject();
        auto func = Convert::FromVar<Js::JavascriptFunction>(JsHelpers::GetPropertyVar(globalObject, functionName, scriptContext));
        if (func)
        {
            Js::Var result = nullptr;
            JsHelpers::WithArguments([&](Js::Arguments& arguments) {
                result = func->CallFunction(arguments);
            }, scriptContext, globalObject, arg0, arg1);
            return result;
        }

        return nullptr;
    }

    static Js::Var CallGlobalFunction(Js::ScriptContext *scriptContext, LPCWSTR functionName, Js::Var arg0 = nullptr, Js::Var arg1 = nullptr)
    {
        if (scriptContext->GetThreadContext()->IsScriptActive())
        {
            return CallGlobalFunctionInternal(scriptContext, functionName, arg0, arg1);
        }
        else
        {
            BEGIN_JS_RUNTIME_CALL(scriptContext)
            {
                return CallGlobalFunctionInternal(scriptContext, functionName, arg0, arg1);
            }
            END_JS_RUNTIME_CALL(scriptContext)
        }
    }

    MissingValueHandler::MissingValueHandler(Js::ScriptContext *scriptContext, ScriptContextPath *contextPath, PhaseReporter *phaseReporter, FileAuthoring *fileAuthoring): contextPath(contextPath), 
        fileAuthoring(fileAuthoring), defaultParameters(nullptr), sendParsePhaseChange(true), inMissingValueHandler(false), isCallGraphEnabled(false), indent(0), lastEvalLength(0)
#if DEBUG
        , scriptContext(scriptContext)
#endif
    {
        Assert(contextPath);
        Assert(scriptContext);

        auto alloc = scriptContext->GetGuestArena();
        this->missingValueMap = Anew(alloc, InstanceMap, alloc, 1);
        this->functionParameterMap = Anew(alloc, FunctionMap, alloc, 1);
        this->manager = phaseReporter->GetScriptContextManager();
    }

    Js::RecyclableObject *NormalMissingValue(Js::ScriptContext *scriptContext, Js::TypeId typeId)
    {
        if (typeId == Js::TypeIds_Null)
            return scriptContext->GetLibrary()->GetNull();
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Js::RecyclableObject *MissingValueHandler::RecordTrackingValue(Js::Var value, Js::ScriptContext *scriptContext, Js::TypeId typeId)
    {
        if (value)
        {
            if (JsHelpers::IsNullOrUndefined(value))
            {
                // unwrap tracking values
                auto trackingKey = Js::RecyclableObject::FromVar(value);
                value = this->missingValueMap->Lookup(trackingKey, value);
            }

            if (!JsHelpers::IsNullOrUndefined(value))
            {
                auto recycler = scriptContext->GetThreadContext()->GetRecycler();
                auto nullValue = scriptContext->GetLibrary()->GetNull();
                Assert(typeId == Js::TypeIds_Undefined || typeId == Js::TypeIds_Null);
                auto trackingKey = RecyclerNew(recycler, Js::RecyclableObject, Js::StaticType::New(scriptContext, typeId, nullValue, null));
                this->missingValueMap->Add(trackingKey, value);
                return trackingKey;
            }
        }
        return NormalMissingValue(scriptContext, typeId);
    }

    Js::RecyclableObject *MissingValueHandler::GetMissingPropertyResult(Js::ScriptContext *scriptContext, Js::RecyclableObject *instance, Js::PropertyId id, Js::TypeId typeId) 
    {
        Assert(this->scriptContext == scriptContext);

        auto name = scriptContext->GetPropertyName(id);

        if (!InternalName(name->GetBuffer()) && name->GetLength() > 0 && !this->inMissingValueHandler && this->fileAuthoring->IsValidMissingValueContext(scriptContext))
        {
            TemporaryAssignment<bool> a(this->inMissingValueHandler, true);

            if (typeId == Js::TypeIds_GlobalObject)
            {
                // This is with the typeId as TypeIds_GlobalObject when the global object is referenced in a way that
                // generates an exception. Return an exception object directly instead of raising the exception as this
                // can cause a cascade of exceptions that can significantly slow down execution. 
                auto globalObject = scriptContext->GetGlobalObject();
                if (instance == globalObject && this->missingValueMap)
                {
                    auto exceptObjectId = GetOrAddPropertyIdFromLiteral(scriptContext, L"_$isExceptionObject");
                    auto newValue = scriptContext->GetLibrary()->CreateObject();
                    newValue->SetProperty(exceptObjectId, scriptContext->GetLibrary()->GetTrue(), Js::PropertyOperation_None, nullptr);                    
                    return newValue;
                }
                typeId = Js::TypeIds_Undefined;
            }

            uint32 index;
            if (Js::JavascriptOperators::TryConvertToUInt32(name->GetBuffer(), name->GetLength(), &index) && (index != Js::JavascriptArray::InvalidIndex))
                return this->GetMissingItemResult(scriptContext, instance, index, typeId);

            ArenaAllocator local(L"ls:MissingValueHandler", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);

            auto docObj = JsValueDoc::GetFieldDocObj(&local, instance, name->GetBuffer(), scriptContext);
            if (docObj)
            {
                auto docReference = JsValueDoc::FromDocRef(&local, docObj, scriptContext);
                if (docReference)
                {
                    JsValueDoc* resolvedDoc;
                    AuthoringFileHandle* file;
                    HRESULT hr = /* static */ FileAuthoring::ResolveDocCommentRef(&local, fileAuthoring, contextPath, docReference, scriptContext->GetGlobalObject() == instance ? ascopeGlobal : ascopeMember, name->GetBuffer(), resolvedDoc, file);
                    if (SUCCEEDED(hr) && resolvedDoc)
                    {
                        docObj = resolvedDoc->ToRecyclableObject(scriptContext);
                    }
                    else
                        return NormalMissingValue(scriptContext, typeId);
                }

                return RecordTrackingValue(CallGlobalFunction(scriptContext, fnValueMethod, docObj), scriptContext, typeId);
            }
        }

        return NormalMissingValue(scriptContext, typeId);
    }

    // This function generates tracking undefined values for parameters that have a type declared using a <param /> tag and whose values are
    // not supplied by the caller. This is called by the execution engine when a call is encountered to a function that has more formal
    // parameters declared than actual parameters supplied. 
    Js::RecyclableObject *MissingValueHandler::GetMissingParameterValue(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function, uint32 paramIndex) 
    {
        Assert(this->scriptContext == scriptContext);

        if (paramIndex < PARAMETER_LIMIT && this->fileAuthoring && !this->inMissingValueHandler && 
            this->fileAuthoring->IsValidMissingValueContext(scriptContext))
        {
            TemporaryAssignment<bool> a(this->inMissingValueHandler, true);

            Js::RecyclableObject **parameterValues;
            if (!this->functionParameterMap->TryGetValue(function, &parameterValues))
            {
                parameterValues = this->defaultParameters;

                auto body = function->GetParseableFunctionInfo();
                if (body) 
                {
                    auto file = this->fileAuthoring->GetAuthoringFile(body);
                    if (file)
                    {
                        ArenaAllocator local(L"ls:MissingValueHandler", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
                        TemporaryAssignment<bool> l(this->sendParsePhaseChange, false);

                        CommentBuffer* functionComments = file->GetFunctionComments(&local, scriptContext, function, commenttypeAnyDoc);
                        auto functionCommentsText = functionComments == nullptr ? nullptr : functionComments->Sz();
                        if ((functionCommentsText != nullptr) && HasRelevantTags(functionCommentsText))
                        {
                            FunctionDocComments* funcDoc = nullptr;

                            // This looks like it has a <param/> tag, parse the comments and populate the parameter array.
                            if (SUCCEEDED(Authoring::ParseFuncDocComments(&local, functionCommentsText, functionComments->GetCommentType(), &funcDoc)) && funcDoc)
                            {
                                auto signature = funcDoc->FirstSignature();
                                if (signature)
                                {
                                    // Enumerate the parameters in declaration order and find any relavant param declarations
                                    FunctionDocComments::Param* params[PARAMETER_LIMIT];
                                    for (int i = 0; i < PARAMETER_LIMIT; i++) params[i] = nullptr;
                                    int index = 0;
                                    bool found = false;

                                    file->ForEachArgument(&local, scriptContext, function, [&](ParseNodePtr node, bool isRest)
                                    {
                                        if (index < PARAMETER_LIMIT)
                                        {
                                            auto pid = node->sxVar.pid;
                                            if (pid)
                                            {
                                                auto param = signature->FindParam(pid->Psz());
                                                if (param && (param->type || (param->elementType && IsSafeTypeExpression(param->elementType))))
                                                {
                                                    params[index] = param;
                                                    found = true;
                                                }
                                            }
                                        }
                                        index++;
                                    });

                                    // If we found any, create a new parameter array an initialize it with the values from the parameter types
                                    if (found)
                                    {
                                        parameterValues = NewParameterValueArray(scriptContext);
                                        for (int i = 0; i < PARAMETER_LIMIT; i++)
                                        {
                                            auto param = params[i];
                                            if (param)
                                            {
                                                Js::Var typeName;
                                                Js::Var isElement;

                                                // We don't have to handle param->value here because it is already handled in DocCommentRewrite and will
                                                // generate a conditional parameter initialization even if the completion offset is not inside the function.
                                                if (param->type)
                                                {
                                                    typeName = Js::JavascriptString::NewCopyBuffer(param->type, wcslen(param->type), scriptContext);
                                                    isElement = scriptContext->GetLibrary()->GetFalse();
                                                }
                                                else
                                                {
                                                    typeName = Js::JavascriptString::NewCopyBuffer(param->elementType, wcslen(param->elementType), scriptContext);
                                                    isElement = scriptContext->GetLibrary()->GetTrue();
                                                }
                                                parameterValues[i] = RecordTrackingValue(CallGlobalFunction(scriptContext, fnParamValue, typeName, isElement), scriptContext, Js::TypeIds_Undefined);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (!parameterValues)
                {
                    // The only way parameterValues can be null is if it came from a null value in defaultParameters. Verify this assumption is correct.
                    Assert(!this->defaultParameters);

                    // Defer create the default parameter array because it is slightly more efficient if we never need it
                    // and the library is not set up yet when we get constructed (hence the library's undefined object we
                    // need isn't created yet) so we can't do this in the constructor.
                    parameterValues = this->defaultParameters = NewParameterValueArray(scriptContext);
                }

                this->functionParameterMap->AddNew(function, parameterValues);
            }

            return parameterValues[paramIndex];
        }

        return NormalMissingValue(scriptContext, Js::TypeIds_Undefined);
    }

    Js::RecyclableObject *MissingValueHandler::GetMissingItemResult(Js::ScriptContext *scriptContext, Js::RecyclableObject *instance, uint32 index, Js::TypeId typeId)
    {
        Assert(this->scriptContext == scriptContext);

        if (!this->inMissingValueHandler && this->fileAuthoring->IsValidMissingValueContext(scriptContext)) 
        {
            TemporaryAssignment<bool> a(this->inMissingValueHandler, true);

            ArenaAllocator local(L"ls:MissingValueHandler", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
            auto docObj = JsValueDoc::GetDocObj(&local, instance, scriptContext);
            if (docObj)
            {
                auto elementValue = CallGlobalFunction(scriptContext, fnElementValueMethod, docObj);
                if (elementValue && !JsHelpers::IsNullOrUndefined(elementValue))
                    return RecordTrackingValue(elementValue, scriptContext, typeId);
            }

            Js::Var element;
            if (index != 0 && instance->GetItem(instance, 0, &element, scriptContext))
            {
                return RecordTrackingValue(element, scriptContext, typeId);
            }
        }
        return NormalMissingValue(scriptContext, typeId);
    }

    Js::RecyclableObject *MissingValueHandler::GetTrackingKey(Js::ScriptContext *scriptContext, Js::Var value, Js::TypeId typeId)
    {
        Assert(this->scriptContext == scriptContext);
        Assert(typeId == Js::TypeIds_Undefined || typeId == Js::TypeIds_Null);

        return RecordTrackingValue(value, scriptContext, typeId);
    }

    Js::Var MissingValueHandler::GetCallerName(Js::ScriptContext *scriptContext, int fileId, int offset)
    {
        Assert(this->scriptContext == scriptContext);

        auto primaryFile = this->fileAuthoring->GetPrimaryFile();
        auto primaryTree = this->fileAuthoring->GetPrimaryTree();
       
        if (primaryFile->FileId() == fileId)
        {
            ArenaAllocator local(L"ls:getCallerName", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
            ParseNodeCursor cursor(&local, primaryTree);
            if (cursor.SeekToOffset(offset))
            {
                auto current = cursor.Current();
                IdentPtr name = nullptr;

                while (current)
                {
                    switch (current->nop)
                    {
                    case knopBlock:
                    case knopFncDecl:
                    case knopProg:
                        return scriptContext->GetLibrary()->GetUndefined();
                    case knopLetDecl:
                    case knopConstDecl:
                    case knopVarDecl:
                        {
                            name = current->sxVar.pid;
                            break;
                        }
                    case knopAsg:
                        {
                            auto target = current->sxBin.pnode1;
                            if (target)
                            {
                                switch (target->nop)
                                {
                                case knopName:
                                    {
                                        name = target->sxPid.pid;
                                        break;
                                    }
                                case knopDot:
                                    {
                                        auto dottedName = target->sxBin.pnode2;
                                        if (dottedName && dottedName->nop == knopName)
                                            name = dottedName->sxPid.pid;
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }

                    if (name)
                    {
                        return Js::JavascriptString::NewCopyBuffer(name->Psz(), name->Cch(), scriptContext);
                    }

                    current = cursor.Up();
                }
            }
        }

        return scriptContext->GetLibrary()->GetUndefined();
    }

    bool MissingValueHandler::HasThisStmt(Js::ScriptContext *scriptContext, Js::JavascriptFunction *function)
    {
        if (this->fileAuthoring && function->GetFunctionInfo() && !function->GetFunctionInfo()->IsDeferredDeserializeFunction())
        {
            auto info = function->GetParseableFunctionInfo();

            if (info)
            {
                auto offset = info->StartInDocument();
                auto sourceIndex = info->GetSourceIndex();
                auto primaryFile = this->fileAuthoring->GetPrimaryFile();
                auto primaryTree = this->fileAuthoring->GetPrimaryTree();

                if (primaryFile->IsSourceAtIndex(scriptContext, sourceIndex))
                {
                    ArenaAllocator local(L"ls:hasThisStmt", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory);
                    ParseNodeCursor cursor(&local, primaryTree);
                    if (cursor.SeekToOffset(offset))
                    {
                        auto node = cursor.Current();
                        return node->nop == knopFncDecl && node->sxFnc.HasThisStmt();
                    }
                }
            }
        }

        return false;
    }

    Js::Var MissingValueHandler::GetExecutingScriptFileName(Js::ScriptContext *scriptContext)
    {
        Assert(this->fileAuthoring);

        CComBSTR name;
        IAuthorFileHandle* handle = this->fileAuthoring->GetExecutionFile();
        
        if (handle)
        {
            AuthoringFileHandle* internalHandle = nullptr;
            this->fileAuthoring->GetInternalHandle(handle, internalHandle);
            internalHandle->GetDisplayName(&name);
        }

        return name ? Js::JavascriptString::NewCopyBuffer(name, name.Length(), scriptContext) :
            scriptContext->GetLibrary()->GetUndefined();
    }

    Js::Var MissingValueHandler::GetTrackingValue(Js::ScriptContext *scriptContext, Js::RecyclableObject *value) 
    {
        Assert(this->scriptContext == scriptContext);

        return this->missingValueMap->Lookup(value, value);
    }

    PhaseReporter* MissingValueHandler::GetActivePhaseReporter()
    {
        if (manager)
        {
            return manager->GetActivePhaseReporter();
        }
        return nullptr;
    }

    void MissingValueHandler::ReportPhase(AuthorFileAuthoringPhase phase)
    {
        auto reporter = GetActivePhaseReporter();
        if (reporter)
        {
            reporter->Phase(phase);
        }
    }

    void MissingValueHandler::Parsing()
    {
        if (sendParsePhaseChange)
            ReportPhase(afpParsing);
    }

    void MissingValueHandler::GeneratingByteCode()
    {
        ReportPhase(afpPreparing);
    }

    void MissingValueHandler::Executing()
    {
        ReportPhase(afpExecuting);
    }

    void MissingValueHandler::Progress()
    {
        auto reporter = GetActivePhaseReporter();
        if (reporter)
        {
            reporter->Progress();
        }
    }

    void MissingValueHandler::PreparingEval(charcount_t length)
    {
        // If we get an eval that is the same length assume we are in a loop and don't send the 
        // phase changes for this eval. This prevents a loop containing eval from disabling hurry.
        if (length == this->lastEvalLength)
        {
            auto reporter = GetActivePhaseReporter();
            if (reporter)
            {
                reporter->InPrepareEval();
            }
        }
        this->lastEvalLength = length;
    }

    void MissingValueHandler::SetScriptContextPath(ScriptContextPath *scriptContextPath)
    {
        contextPath = scriptContextPath;
    }

    void MissingValueHandler::SetFileAuthoring(FileAuthoring *fileAuthoring)
    {
        this->fileAuthoring = fileAuthoring;
    }

    Js::RecyclableObject **MissingValueHandler::NewParameterValueArray(Js::ScriptContext *scriptContext)
    {
        auto result = AnewArray(scriptContext->GetGuestArena(), Js::RecyclableObject *, PARAMETER_LIMIT);
        auto undefined = scriptContext->GetLibrary()->GetUndefined();

        for (int i = 0; i < PARAMETER_LIMIT; i++)
        {
            result[i] = undefined;
        }

        return result;
    }

    int MissingValueHandler::GetFileIdOfSourceIndex(Js::ScriptContext *scriptContext, int sourceIndex)
    {
        return fileAuthoring->GetFileIdOf(sourceIndex, scriptContext);
    }

    DWORD_PTR MissingValueHandler::GetAuthorSource(int sourceIndex, Js::ScriptContext *scriptContext)
    {
        return (DWORD_PTR)fileAuthoring->GetAuthoringFile(sourceIndex, scriptContext);
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS

    void MissingValueHandler::LogFunctionStart(Js::ScriptContext *scriptContext, Js::FunctionBody * functionBody)
    {
        const wchar_t* name = functionBody->GetDisplayName();
        AuthoringFileHandle *fileHandle = (AuthoringFileHandle *)GetAuthorSource(functionBody->GetSourceIndex(), scriptContext);
        CComBSTR sourceName;
        if (fileHandle)
        {
            fileHandle->GetDisplayName(&sourceName);
        }
        int currentIndent = (this->indent > 0 ? this->indent : 0) * 2;
        int length = currentIndent + wcslen(name) + sourceName.Length() + 7; // 7 for rest of the string.
        BSTR text = SysAllocStringLen(L"", length);
        for (int i = 0; i < currentIndent; i++)
        {
            text[i] = L' ';
        }

        swprintf_s(text + currentIndent, length - currentIndent, L"%ls (%ls) {\n", functionBody->GetDisplayName(), sourceName.m_str ? sourceName.m_str : L"");

        Assert(scriptContext->GetThreadContext()->GetAuthoringContext() != nullptr);
        scriptContext->GetThreadContext()->GetAuthoringContext()->LogMessage(text);
        ::SysFreeString(text);
        this->indent++;
    }

    void MissingValueHandler::LogFunctionEnd(Js::ScriptContext *scriptContext)
    {
        this->indent--;
        int currentIndent = (this->indent > 0 ? this->indent : 0) * 2;
        int length = currentIndent + 4;
        BSTR text = SysAllocStringLen(L"", length);
        for (int i = 0; i < currentIndent; i++)
        {
            text[i] = L' ';
        }
        swprintf_s(text + currentIndent, length - currentIndent, L"}\n");
        Assert(scriptContext->GetThreadContext()->GetAuthoringContext() != nullptr);
        scriptContext->GetThreadContext()->GetAuthoringContext()->LogMessage(text);
        ::SysFreeString(text);
    }
#endif

}
