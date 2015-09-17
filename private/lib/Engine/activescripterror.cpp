/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "EnginePch.h"
#include "errstr.h"

// === ActiveScriptError ==
ActiveScriptError::ActiveScriptError() : m_cRef(1),
        m_dwSourceContext(0), m_ulLineNumber(0), m_lCharacterPosition(0),
        m_bstrSourceLine(nullptr), m_errInfo(nullptr), m_fHasDispatchedToDebugger(FALSE), m_fIsFirstChance(FALSE), m_fIsExceptionCaughtInNonUserCode(FALSE),
        m_scriptDebugDocument(nullptr), thrownObject(nullptr), recycler(nullptr), m_wasRooted(FALSE)
{
    memset(&m_ei, 0, sizeof(m_ei));
    memset(&m_restrictedStr, 0, sizeof(m_restrictedStr));
    DLLAddRef();
}


void ActiveScriptError::Free()
{
    FreeExcepInfo(&m_ei.exceptionInfo);
    CoTaskMemFree((LPVOID)m_ei.errorType.typeText);
    FreeStackTrace(m_ei.callStack);
    m_ei.errorType.typeText = nullptr;
    SysFreeString(m_bstrSourceLine);
    m_dwSourceContext = 0;
    m_ulLineNumber = 0;
    m_lCharacterPosition = -1;
    if (m_restrictedStr.referenceStr)
    {
        SysFreeString(m_restrictedStr.referenceStr);
        m_restrictedStr.referenceStr = nullptr;
    }
    if (m_restrictedStr.restrictedErrStr)
    {
        SysFreeString(m_restrictedStr.restrictedErrStr);
        m_restrictedStr.restrictedErrStr = nullptr;
    }
    if (m_restrictedStr.capabilitySid)
    {
        SysFreeString(m_restrictedStr.capabilitySid);
        m_restrictedStr.capabilitySid = nullptr;
    }
    if (m_errInfo != nullptr)
    {
        m_errInfo->Release();
        m_errInfo = nullptr;
    }
    if (thrownObject != nullptr && Js::RecyclableObject::Is(this->thrownObject) && m_wasRooted)
    {
#if DBG
        if (recycler->IsValidObject(thrownObject))
#endif
        {
            recycler->RootRelease(thrownObject);
        }
        thrownObject = nullptr;
    }
}

ActiveScriptError::~ActiveScriptError()
{
    Free();
    DLLRelease();
}


// === IUnknown ===
STDMETHODIMP ActiveScriptError::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_POINTER(ppv);

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (IUnknown *) IACTIVESCRIPTERROR64 this;

    else if (IsEqualIID(riid, IID_IActiveScriptError))
        *ppv = (IActiveScriptError *) IACTIVESCRIPTERROR64  this;

#if _WIN64 || USE_32_OR_64_BIT
    else if (IsEqualIID(riid, IID_IActiveScriptError64))
        *ppv = (IActiveScriptError64 *)this;
#endif // _WIN64 || USE_32_OR_64_BIT

    else if (IsEqualIID(riid, __uuidof(IActiveScriptErrorEx)))
    {
        *ppv = (IActiveScriptErrorEx *) this;
    }

    else if (IsEqualIID(riid, __uuidof(IActiveScriptErrorDebug)))
        *ppv = (IActiveScriptErrorDebug *)this;

    else if (IsEqualIID(riid, __uuidof(IActiveScriptWinRTErrorDebug)))
    {
        *ppv = (IActiveScriptWinRTErrorDebug *)this;
    }
    else if (IsEqualIID(riid, __uuidof(IActiveScriptErrorDebug110)))
    {
        *ppv = (IActiveScriptErrorDebug110 *)this;
    }
    else if (IsEqualIID(riid, __uuidof(IRemoteDebugCriticalErrorEvent110)))
    {
        *ppv = (IRemoteDebugCriticalErrorEvent110 *)this;
    }
    else
    {
        *ppv = nullptr;
        return HR(E_NOINTERFACE);
    }
    AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) ActiveScriptError::AddRef(void)
{
    return InterlockedIncrement((long *)&m_cRef);
}

STDMETHODIMP_(ULONG) ActiveScriptError::Release(void)
{
    LONG cRef;

    cRef = InterlockedDecrement((long *)&m_cRef);
    if (cRef)
        return cRef;

    delete this;
    return 0;
}


// === IActiveScriptError ===
STDMETHODIMP ActiveScriptError::GetExceptionInfo(EXCEPINFO *excepInfo)
{
    CHECK_POINTER(excepInfo);
    *excepInfo = NoException;

    // Some of our client's don't know enough to do this.
    if (nullptr != m_ei.exceptionInfo.pfnDeferredFillIn)
    {
        m_ei.exceptionInfo.pfnDeferredFillIn(&m_ei.exceptionInfo);
        m_ei.exceptionInfo.pfnDeferredFillIn = nullptr;
    }

    CopyException(excepInfo, &m_ei.exceptionInfo);
    return NOERROR;
}

STDMETHODIMP ActiveScriptError::GetSourcePosition(DWORD *pdwSourceContext,
        ULONG *pulLineNumber, LONG *plCharacterPosition)
{
    if (nullptr != pdwSourceContext)
        *pdwSourceContext = (DWORD)m_dwSourceContext;
    if (nullptr != pulLineNumber)
        *pulLineNumber = m_ulLineNumber;
    if (nullptr != plCharacterPosition)
        *plCharacterPosition = m_lCharacterPosition;
    return NOERROR;
}

#if _WIN64 || USE_32_OR_64_BIT
STDMETHODIMP ActiveScriptError::GetSourcePosition64(DWORDLONG *pdwSourceContext,
        ULONG *pulLineNumber, LONG *plCharacterPosition)
{
    if (nullptr != pdwSourceContext)
        *pdwSourceContext = (DWORDLONG)m_dwSourceContext;
    if (nullptr != pulLineNumber)
        *pulLineNumber = m_ulLineNumber;
    if (nullptr != plCharacterPosition)
        *plCharacterPosition = m_lCharacterPosition;
    return NOERROR;
}
#endif // _WIN64 || USE_32_OR_64_BIT

STDMETHODIMP ActiveScriptError::GetSourceLineText(BSTR *pbstrSourceLine)
{
    CHECK_POINTER(pbstrSourceLine);
    if (nullptr == m_bstrSourceLine)
        return HR(E_FAIL);
    *pbstrSourceLine = SysAllocString(m_bstrSourceLine);
    if (nullptr == *pbstrSourceLine)
        return HR(E_OUTOFMEMORY);
    return NOERROR;
}

STDMETHODIMP ActiveScriptError::GetDocumentContext(IDebugDocumentContext **ppDocContext)
{
    HRESULT hr = S_OK;
    CComPtr<IDebugCodeContext> pCodeContext;

    CHECK_POINTER(ppDocContext);
    *ppDocContext = nullptr;
    if (!m_pStackFrame)
    {
        return E_FAIL;
    }
    hr = m_pStackFrame->GetCodeContext(&pCodeContext);
    if (!SUCCEEDED(hr))
    {
        return hr;
    }
    return pCodeContext->GetDocumentContext(ppDocContext);
}

STDMETHODIMP ActiveScriptError::GetStackFrame(IDebugStackFrame **ppStackFrame)
{
    CHECK_POINTER(ppStackFrame);
    *ppStackFrame = nullptr;
    if (!m_pStackFrame)
    {
        return E_FAIL;
    }
    *ppStackFrame = m_pStackFrame;
    (*ppStackFrame)->AddRef();
    return S_OK;
}

HRESULT ActiveScriptError::GetExtendedExceptionInfo(ExtendedExceptionInfo *excepInfo)
{
    if (!Js::Configuration::Global.flags.WERExceptionSupport)
    {
        return E_NOTIMPL;
    }

    CHECK_POINTER(excepInfo);
    memset(excepInfo, 0, sizeof(ExtendedExceptionInfo));
    HRESULT hr = FillText(excepInfo->errorType.typeText, m_ei.errorType.typeText);
    IfFailedReturn(hr);
    excepInfo->errorType.typeNumber = m_ei.errorType.typeNumber;
    CopyException(&excepInfo->exceptionInfo, &m_ei.exceptionInfo);
    excepInfo->flags = m_ei.flags;
    CopyStackTrace(excepInfo->callStack, m_ei.callStack);
    return S_OK;
}

HRESULT ActiveScriptError::HasDispatchedToDebugger(BOOL *pfHasDispatched)
{
    if (pfHasDispatched == nullptr)
    {
        return E_POINTER;
    }

    *pfHasDispatched = ((m_fHasDispatchedToDebugger != FALSE) ? TRUE : FALSE);

    return S_OK;
}

HRESULT ActiveScriptError::GetThrownObject(Var* thrownObject)
{
    if (thrownObject == nullptr)
    {
        return E_POINTER;
    }
    // Note that the thrownObject can be nullptr in case of compile error reported to host.
    *thrownObject = this->thrownObject;
    return NOERROR;
}

HRESULT ActiveScriptError::GetExceptionThrownKind(SCRIPT_ERROR_DEBUG_EXCEPTION_THROWN_KIND *pExceptionKind)
{
    if (pExceptionKind != nullptr)
    {
        if (m_fIsExceptionCaughtInNonUserCode)
        {
            *pExceptionKind = ETK_USER_UNHANDLED;
        }
        else if (m_fIsFirstChance)
        {
            *pExceptionKind = ETK_FIRST_CHANCE;
        }
        else
        {
            *pExceptionKind = ETK_UNHANDLED;
        }

        return S_OK;
    }

    return E_INVALIDARG;
}

// Info:        Get WinRT Restricted Error string, if available
// Parameters:  errorString - restricted error string
//                  value is nullptr if no restricted string available
//                  caller is responsible for freeing the string
// Returns:     S_OK
HRESULT ActiveScriptError::GetRestrictedErrorString(__deref_out BSTR * errorString)
{
    CHECK_POINTER(errorString);
    *errorString = nullptr;
    if (m_restrictedStr.restrictedErrStr != nullptr)
    {
        *errorString = SysAllocString(m_restrictedStr.restrictedErrStr);
        if (nullptr == *errorString)
        {
            return HR(E_OUTOFMEMORY);
        }
    }
    return S_OK;
}

// Info:        Get WinRT Restricted Error reference string, if available
// Parameters:  referenceString - restricted error reference string
//                  value is nullptr if no reference string available
//                  caller is responsible for freeing the string
// Returns:     S_OK
HRESULT ActiveScriptError::GetRestrictedErrorReference(__deref_out BSTR * referenceString)
{
    CHECK_POINTER(referenceString);
    *referenceString = nullptr;
    if (m_restrictedStr.referenceStr != nullptr)
    {
        *referenceString = SysAllocString(m_restrictedStr.referenceStr);
        if (nullptr == *referenceString)
        {
            return HR(E_OUTOFMEMORY);
        }
    }
    return S_OK;
}

// Info:        Get CapabilitySid for the WinRT Error, if available
// Parameters:  capabilitySid - capability SID associated with the error
//                  value is nullptr if no capability SID available
//                  caller is responsible for freeing the string
// Returns:     S_OK
HRESULT ActiveScriptError::GetCapabilitySid(__deref_out BSTR * capabilitySid)
{
    CHECK_POINTER(capabilitySid);
    *capabilitySid = nullptr;
    if (m_restrictedStr.capabilitySid != nullptr)
    {
        *capabilitySid = SysAllocString(m_restrictedStr.capabilitySid);
        if (nullptr == *capabilitySid)
        {
            return HR(E_OUTOFMEMORY);
        }
    }
    return S_OK;
}

// Critical error - syntax errors
HRESULT ActiveScriptError::GetErrorInfo(BSTR* pbstrSource,
        int* pMessageId,
        BSTR* pbstrMessage,
        IDebugDocumentContext** ppDebugDocumentContext)
{
    // source is always "SCRIPT", not localized
    *pbstrSource = ::SysAllocString(L"SCRIPT");
    if(*pbstrSource == nullptr)
    {
        return HR(E_OUTOFMEMORY);
    }

    // message id from LOWORD of scode
    *pMessageId = LOWORD(this->m_ei.exceptionInfo.scode);

    Assert(this->m_ei.exceptionInfo.bstrDescription != nullptr);
    *pbstrMessage = ::SysAllocString(this->m_ei.exceptionInfo.bstrDescription);
    if(*pbstrMessage == nullptr)
    {
        return HR(E_OUTOFMEMORY);
    }

    // doc context is optional
    if (ppDebugDocumentContext != nullptr && m_scriptDebugDocument != nullptr)
    {
        // get doc context for error position
        ULONG characterOffset = (ULONG)this->m_ichMin;
        ULONG characterCount = (ULONG)this->m_ichLim - this->m_ichMin;

        HRESULT hr = m_scriptDebugDocument->GetDocumentContext(characterOffset, characterCount, ppDebugDocumentContext);
        Assert(hr == S_OK);
    }

    return S_OK;
}

HRESULT ActiveScriptError::GetCompileErrorInfo(_Out_ BSTR* description, _Out_ ULONG* line, _Out_ ULONG* column)
{
    *description = ::SysAllocString(this->m_ei.exceptionInfo.bstrDescription);
    if (*description == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    *line = m_ulLineNumber;
    *column = static_cast<ULONG>(m_lCharacterPosition);
    return S_OK;
}

void ActiveScriptError::FillParseErrorInfo(ExtendedExceptionInfo &exInfo)
{
    // Fake error type. Only for WWA. It can not be thrown.
    FillText(exInfo.errorType.typeText, L"Parse error");
    exInfo.errorType.typeNumber = 
        ErrorTypeHelper::MapToExternal(Js::JavascriptError::MapParseError(exInfo.exceptionInfo.scode));
    exInfo.flags = ExtendedExceptionInfo_Available;
    exInfo.callStack.frameCount = 0;
    exInfo.callStack.frames = nullptr;
}

HRESULT ActiveScriptError::CreateCompileError(const SRCINFO * psi, CompileScriptException * pcse, Js::Utf8SourceInfo* sourceInfo, ActiveScriptError **ppase)
{
    AssertMem(ppase);

    ActiveScriptError* pase = new ActiveScriptError; // The constructor sets the reference-count to 1
    if (nullptr == pase)
        return HR(E_OUTOFMEMORY);
    
    CopyException(&pase->m_ei.exceptionInfo, &pcse->ei);
    FillParseErrorInfo(pase->m_ei);
    
    if(sourceInfo)
    {
        if (sourceInfo->HasDebugDocument())
        {
            pase->m_scriptDebugDocument = static_cast<ScriptDebugDocument*>(sourceInfo->GetDebugDocument());
        }
    }

    if (pcse->hasLineNumberInfo)
    {
        if ((ULONG)pcse->ichMin < psi->ichLimHost && (ULONG)pcse->ichLim <= psi->ichLimHost)
        {
            pase->m_ichMin = pcse->ichMin - psi->ichMinHost;
            pase->m_ichLim = pcse->ichLim - psi->ichMinHost;
        }
        else
        {
            // if min > limhost, the parser scanned to the end of the script
            // point to the last host character - in bounds of the script (required for doc context resolution)
            pase->m_ichMin = psi->ichLimHost - psi->ichMinHost - 1;
            pase->m_ichLim = psi->ichLimHost - psi->ichMinHost;
        }

        pase->m_ulLineNumber = psi->dlnHost;
        pase->m_dwSourceContext = psi->sourceContextInfo->dwHostSourceContext;
        if (pcse->line >= (long)psi->lnMinHost)
        {
            pase->m_ulLineNumber += pcse->line - psi->lnMinHost;
            if (pcse->ichMin > pcse->ichMinLine)
                pase->m_lCharacterPosition = pcse->ichMin - pcse->ichMinLine;
        }

        if (pase->m_ulLineNumber == psi->dlnHost)
        {
            pase->m_lCharacterPosition = pase->m_lCharacterPosition + psi->ulColumnHost;
        }
    }
    pase->m_bstrSourceLine = SysAllocStringLen(pcse->bstrLine,
            SysStringLen(pcse->bstrLine));


    *ppase = pase;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if(Js::Configuration::Global.flags.IsEnabled(Js::ReportErrorsFlag))
    {
        Output::Print(L"Error: %s (%d, %d)\n", pase->m_ei.exceptionInfo.bstrDescription, pase->m_ulLineNumber + 1, pase->m_lCharacterPosition + 1);
        Output::Print(L"Error code: %x\n", pase->m_ei.exceptionInfo.wCode);
        Output::Print(L"Source Line: %s\n", pase->m_bstrSourceLine);
        Output::Flush();
    }
#endif //ENABLE_DEBUG_CONFIG_OPTIONS

    return S_OK;
}

HRESULT ActiveScriptError::CreateRuntimeError(Js::JavascriptExceptionObject * exceptionObject, HRESULT * pHrError, IDebugStackFrame* pStackFrame, Js::ScriptContext* requestContext, ActiveScriptError **ppase)
{
    HRESULT hr = NOERROR;

    ActiveScriptError* pase = new ActiveScriptError;// The constructor sets the reference-count to 1
    if (nullptr == pase)
    {
        return E_OUTOFMEMORY;
    }

    if (requestContext != nullptr) 
    {
        pase->recycler = requestContext->GetThreadContext()->GetRecycler();
        BEGIN_TRANSLATE_OOM_TO_HRESULT_NESTED
        {
            pase->thrownObject = exceptionObject->GetThrownObject(requestContext);
            if (pase->thrownObject != nullptr && Js::RecyclableObject::Is(pase->thrownObject))
            {
                // AddRef at creation time, and Release at last refcount. This is similar to 
                // normal JavascriptDispatch.
#if DBG
                if (pase->recycler->IsValidObject(pase->thrownObject))
#endif
                {
                    pase->recycler->RootAddRef(pase->thrownObject);
                }
            }
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        if (FAILED(hr))
        {
            delete pase;
            *ppase = nullptr;
            return hr;
        }
        pase->m_wasRooted = TRUE;
    }

    *pHrError = FillExcepInfo(exceptionObject, nullptr, &pase->m_ei, &pase->m_restrictedStr);
    StoreErrorInfo(exceptionObject, &pase->m_errInfo);

    pase->m_fHasDispatchedToDebugger = (exceptionObject->HasDebuggerLogged() ? TRUE : FALSE);
    pase->m_fIsFirstChance = (exceptionObject->IsFirstChanceException() ? TRUE : FALSE);
    pase->m_fIsExceptionCaughtInNonUserCode = (exceptionObject->IsExceptionCaughtInNonUserCode() ? TRUE : FALSE);

    Js::FunctionBody * funcBody = exceptionObject->GetFunctionBody();

    // In cases of OOM we will have not functionBody to report
    if (funcBody)
    {
        // If no debugger is attached we need to get to the correct line.
        // As far as I can tell we do not have enough info absent the debugger
        // to properly resolve to character position.

        uint32 offset = exceptionObject->GetByteCodeOffset();
        pase->m_dwSourceContext = funcBody->GetHostSourceContext();
        if (funcBody->GetUtf8SourceInfo()->GetIsLibraryCode() || !funcBody->GetLineCharOffset(offset, &pase->m_ulLineNumber, &pase->m_lCharacterPosition))
        {
            pase->m_ulLineNumber = 0;
            pase->m_lCharacterPosition = 0;
        }
    }
    else
    {
        pase->m_dwSourceContext = Js::Constants::NoHostSourceContext;
    }
    pase->m_pStackFrame = pStackFrame;

    *ppase = pase;

    return S_OK;
}

HRESULT ActiveScriptError::FillExcepInfo(Js::JavascriptExceptionObject* exceptionObject, EXCEPINFO* excepInfo, ExtendedExceptionInfo* extendedExcepInfo, Js::RestrictedErrorStrings* restrictedErrorString)
{
    Assert(excepInfo == nullptr && extendedExcepInfo != nullptr || excepInfo != nullptr && extendedExcepInfo == nullptr);
    if (! excepInfo)
    {
        excepInfo = &extendedExcepInfo->exceptionInfo;
        extendedExcepInfo->flags = ExtendedExceptionInfo_None;
    }

    HRESULT hr;
    // exceptionObject->scriptContext is nullptr for OOM exception object - hence, get the threadContext using
    // the static helper
    if(exceptionObject == ThreadContext::GetContextForCurrentThread()->GetPendingOOMErrorObject())
    {
        FillExcepInfo(VBSERR_OutOfMemory, nullptr, excepInfo);
        if (extendedExcepInfo)
        {
            // Ignore OOM here. We'll just not get the full info
            IfFailedReturn(FillErrorType(JavascriptError, *extendedExcepInfo));
            FillStackTrace(VBSERR_OutOfMemory, *exceptionObject, *extendedExcepInfo);
        }
        return E_OUTOFMEMORY;
    }

    HRESULT exceptionHR = JSERR_UncaughtException;  // default to uncaught exception (for non-Error object exception)
    wchar_t const * messageSz = nullptr;
    BEGIN_TRANSLATE_EXCEPTION_TO_HRESULT
    {
        JsErrorType errorTypeNumber = CustomError;
        // Pass in nullptr to get the original thrown object instead of a dispatch
        Var errorObject = exceptionObject->GetThrownObject(nullptr);
        if (Js::JavascriptError::Is(errorObject) ||
            Js::JavascriptError::IsRemoteError(errorObject))
        {
            exceptionHR = Js::JavascriptError::GetRuntimeErrorWithScriptEnter(Js::RecyclableObject::FromVar(errorObject), &messageSz);
            // We can't get at the type for remote error objects, so they will just be returned as default of CustomError
            if (extendedExcepInfo && Js::JavascriptError::Is(errorObject))
            {
                errorTypeNumber = ErrorTypeHelper::MapToExternal(Js::JavascriptError::FromVar(errorObject)->GetErrorType());
            }

            if (restrictedErrorString && Js::JavascriptErrorDebug::Is(errorObject))
            {
                Js::JavascriptErrorDebug * errDebug = Js::JavascriptErrorDebug::FromVar(errorObject);

                restrictedErrorString->restrictedErrStr = errDebug->GetRestrictedErrorString();

                restrictedErrorString->referenceStr = errDebug->GetRestrictedErrorReference();

                restrictedErrorString->capabilitySid = errDebug->GetCapabilitySid();
            }
        }
        else
        {
            Js::ScriptContext* scriptContext = exceptionObject->GetScriptContext();
           
            // Calling GetSz() is fine because underlying string is allocated using recycler
            // as separate allocation from it's wrapping JavascriptString.
            messageSz = Js::JavascriptExternalConversion::ToString(errorObject, scriptContext)->GetSz();            
        }
        if (extendedExcepInfo)
        {
            IfFailedReturn(FillErrorType(errorTypeNumber, *extendedExcepInfo));
        }
    }
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(exceptionHR)
    catch(Js::JavascriptExceptionObject *)
    {
        exceptionHR = E_FAIL; // don't recursively get error code

        // Technically cleanup below isn't necessary because of order of potential failures
        // above, but in case anything changes, this will ensure we don't end up with a
        // mismatched hr and text.
        messageSz = nullptr;
        CoTaskMemFree((LPVOID)extendedExcepInfo->errorType.typeText);
        extendedExcepInfo->errorType.typeText = nullptr;
    } 
    CATCH_UNHANDLED_EXCEPTION(hr)

    FillExcepInfo(exceptionHR, messageSz, excepInfo);
    if (extendedExcepInfo)
    {
        // Even if failed above, the stack trace still will be valid, so try
        // to copy what we can.
        FillStackTrace(exceptionHR, *exceptionObject, *extendedExcepInfo);
    }
    return exceptionHR;
}


void ActiveScriptError::FillExcepInfo(HRESULT hr, wchar_t const * messageSz, EXCEPINFO *excepInfo)
{
    BSTR bstrError = nullptr;
    if (messageSz == nullptr)
    {
        if (FACILITY_CONTROL == HRESULT_FACILITY(hr))
        {
            bstrError = BstrGetResourceString(HRESULT_CODE(hr));
        }
    }
    else
    {
         // Ignore out of memory here or string too long
        size_t len = wcslen(messageSz);
        if (len <= UINT_MAX)
        {
            bstrError = SysAllocStringLen(messageSz, (uint)len);
        }
    }

    excepInfo->scode = (SCODE)hr;
    excepInfo->bstrSource = BstrGetResourceString(IDS_RUNTIME_ERROR_SOURCE);
    excepInfo->bstrDescription = bstrError;
    excepInfo->pfnDeferredFillIn = nullptr;
}

HRESULT ActiveScriptError::FillErrorType(JsErrorType typeNumber, ExtendedExceptionInfo& extendedExcepInfo)
{
    HRESULT hr;
    IfFailedReturn(FillText(extendedExcepInfo.errorType.typeText, ErrorTypeHelper::MapExternalToErrorText(typeNumber)));
    extendedExcepInfo.errorType.typeNumber = typeNumber;
    extendedExcepInfo.flags = ExtendedExceptionInfo_Available;
    return S_OK;
}

HRESULT ActiveScriptError::FillText(LPCWSTR& target, LPCWSTR source)
{
    if (source == nullptr)
    {
        target = nullptr;
        return S_OK;
    }
    size_t allocLen = wcslen(source) + 1;
    target = (LPWSTR)CoTaskMemAlloc(allocLen * sizeof(source[0]));
    IfNullReturnError(target, E_OUTOFMEMORY);
    wcscpy_s((LPWSTR)(target), allocLen, source);
    return S_OK;
}

// Allocate space for the external stack trace representation. If we are reporting an SO, determine if will truncate
// from top or bottom and add text indicating that.
ActiveScriptError::ExternalStackTrace::ExternalStackTrace(HRESULT exceptionHR, Js::JavascriptExceptionObject& exceptionObject) :
    m_maxStackFramesOnSO(100), m_startOffset(0), m_numFramesToCopy(0), m_totalFrameCount(0), m_framesToFillOffset(0), m_frames(nullptr)
{
    Js::JavascriptExceptionContext::StackTrace* stackTrace = exceptionObject.GetExceptionContext()->GetStackTrace();
    Js::JavascriptExceptionContext::StackTrace* originalStackTrace = exceptionObject.GetExceptionContext()->GetOriginalStackTrace();
    // If we are not in stack overflow, we'll combine the two stacks; however if the final stack is SO, we'll only get the trimmed
    // current stack.
    if (exceptionHR != VBSERR_OutOfStack)
    {
        // This is the one last time the stack will be created before process termination. We can generate a long stack.
        m_totalFrameCount = m_numFramesToCopy = stackTrace->Count();
        if (originalStackTrace) 
            m_totalFrameCount += originalStackTrace->Count() + 1;
        m_numFramesToCopy = m_totalFrameCount;
    }
    else
    {
        m_startOffset = StackTraceStartOffsetOnSO(exceptionObject);
        if (m_startOffset == 0)
        {
            // we are truncating from the bottom
            m_numFramesToCopy = min(stackTrace->Count(), (int)m_maxStackFramesOnSO);
        }
        else
        {
            m_numFramesToCopy = stackTrace->Count() - m_startOffset;
            m_framesToFillOffset = 1;    // Put stack truncated message at offset 0
        }
        m_totalFrameCount = m_numFramesToCopy + 1; // +1 for stack truncated message
    }
    m_frames = (CallStackFrame*)CoTaskMemAlloc(m_totalFrameCount * sizeof(CallStackFrame));
    if (m_frames == nullptr)
    {
        m_numFramesToCopy = m_totalFrameCount = 0;
    }
    else
    {
        memset(m_frames, 0, m_totalFrameCount * sizeof(CallStackFrame));
        if (exceptionHR == VBSERR_OutOfStack)
        {
            int indexOfTruncateMessage = m_framesToFillOffset == 1 ? 0 : m_totalFrameCount -1;
            memset(&m_frames[indexOfTruncateMessage], 0, sizeof(m_frames[0]));
            // The following is explicilty not localized for WER error reporting, as with ErrorTypeHelper
            // Ignore OOM here. Just don't get the text
            FillText(m_frames[indexOfTruncateMessage].functionName, L"Recurring functions on call stack truncated due to stack overflow");
        }
    }
}

//
// For SO, report only the portion of the stack up to a certain number of iterations of the throwing function.
//
int ActiveScriptError::ExternalStackTrace::StackTraceStartOffsetOnSO(Js::JavascriptExceptionObject& exceptionObject)
{
    const uint numThrowingFunctionOccurencesToInclude = 10;

    Js::FunctionBody* throwingFunction = exceptionObject.GetFunctionBody();
    Js::JavascriptExceptionContext::StackTrace* stackTrace = exceptionObject.GetExceptionContext()->GetStackTrace();
    int throwingFunctionOccurences = 0;
    for (uint j = 0, i=stackTrace->Count()-1; i > 0 && j < m_maxStackFramesOnSO; i--, j++)
    {
        if (stackTrace->Item(i).GetFunctionBody() == throwingFunction && ++throwingFunctionOccurences == numThrowingFunctionOccurencesToInclude)
        {
            return i;
        }
    }
    return 0;
}

void ActiveScriptError::ExternalStackTrace::Dump()
{
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    if (! Js::Configuration::Global.flags.Dump.IsEnabled(Js::ExtendedExceptionInfoStackTracePhase))
    {
        return;
    }
    Output::Print(L"\nExtendedExceptionInfo stack trace for thrown exception, count is %d\n", TotalFrameCount());
    CallStackFrame* framesOut = AllFrames();
    for (int i = 0; i < TotalFrameCount(); i++)
    {        
        Output::Print(L"    %3d: %s (%d, %d)\n", i, framesOut[i].functionName, framesOut[i].lineNumber, framesOut[i].characterPosition);
    }
    Output::Flush();
#endif
}

//
// Fill in as much of the stack trace as we can. If get an OOM/failure at any point, then just bail and return what
// we've got so far
//
void ActiveScriptError::FillStackTrace(HRESULT exceptionHR, Js::JavascriptExceptionObject& exceptionObject, ExtendedExceptionInfo& excepInfo)
{
    if (! exceptionObject.GetExceptionContext()->HasStackTrace())
    {
        Assert(exceptionObject.GetExceptionContext()->GetOriginalStackTrace() == nullptr);
        return;
    }

    ExternalStackTrace est(exceptionHR, exceptionObject);
    CallStackFrame* framesOut = est.FramesToFill();
    if (framesOut == nullptr)
    {
        return;
    }

    HRESULT hr = S_OK;
    int numFramesToCopy = est.NumFramesToCopy();
    Js::JavascriptExceptionContext::StackTrace* stackTrace = exceptionObject.GetExceptionContext()->GetStackTrace();
    Js::JavascriptExceptionContext::StackTrace* originalStackTrace = exceptionObject.GetExceptionContext()->GetOriginalStackTrace();
    CallStackFrameHelper callStackFrameHelper(framesOut, numFramesToCopy, hr);

    auto FillOneFrame = [&](CallStackFrame& currentOutFrame, Js::JavascriptExceptionContext::StackFrame& currentFrame)-> HRESULT {
        LPCWSTR functionName = nullptr;
        HRESULT hr = NO_ERROR;
        IfFailedReturn(currentFrame.GetFunctionNameWithArguments(&functionName));
        IfFailedReturn(FillText(currentOutFrame.functionName, functionName));
        Js::FunctionBody* functionBody = currentFrame.GetFunctionBody();
        currentOutFrame.sourceContext = functionBody ? functionBody->GetHostSourceContext() : Js::Constants::NoHostSourceContext;
        if (!functionBody // native library function
            || functionBody->GetUtf8SourceInfo()->GetIsLibraryCode() // script library function
            || !functionBody->GetLineCharOffset(currentFrame.GetByteCodeOffset(), &currentOutFrame.lineNumber, &currentOutFrame.characterPosition))
        {
            currentOutFrame.lineNumber = 0;
            currentOutFrame.characterPosition = 0;
        }
        currentOutFrame.activeScriptDirect = functionBody ? functionBody->GetScriptContext()->GetActiveScriptDirect() : nullptr;
        if (currentOutFrame.activeScriptDirect != nullptr)
        {
            currentOutFrame.activeScriptDirect->AddRef();
        }
        return hr;
    };

    auto DumpOneStack = [&](int numFramesToCopy, Js::JavascriptExceptionContext::StackTrace* stackTraceToDump ) ->HRESULT {
        BEGIN_TRANSLATE_OOM_TO_HRESULT  // Getting native library function name could allocate and OOM
        {
            int stackTraceStartOffset = est.StartOffset();
            int i = 0;
            for ( ; i < numFramesToCopy; i++)
            {
                Js::JavascriptExceptionContext::StackFrame currentFrame = stackTraceToDump->Item(i + stackTraceStartOffset);
                CallStackFrame currentOutFrame = {};
                IfFailedReturn(FillOneFrame(currentOutFrame, currentFrame));
                callStackFrameHelper.AppendFrame(currentOutFrame);
            }
        }
        END_TRANSLATE_OOM_TO_HRESULT(hr);
        return hr;
    };

    // In SO case we only have one stack.
    if (exceptionHR == VBSERR_OutOfStack)
    {
        hr = DumpOneStack(numFramesToCopy, stackTrace);
    }
    else
    {
        if (originalStackTrace != nullptr) 
        {
            hr = DumpOneStack(originalStackTrace->Count(), originalStackTrace);
            if (SUCCEEDED(hr)) 
            {
                CallStackFrame currentOutFrame = {};
                hr = FillText(currentOutFrame.functionName, CallStackFrameHelper::DelimiterString);
                if (SUCCEEDED(hr))
                {
                    callStackFrameHelper.AppendFrame(currentOutFrame);
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = DumpOneStack(stackTrace->Count(), stackTrace);
        }
    }

    excepInfo.callStack.frames = est.AllFrames();
    excepInfo.callStack.frameCount = est.TotalFrameCount();

    if (FAILED(hr))
    {
        FreeStackTrace(excepInfo.callStack);
    } 

    est.Dump();
}

void ActiveScriptError::CopyStackTrace(CallStack& callStackOut, CallStack& callStackIn)
{
    if (callStackIn.frameCount == 0)
    {
        callStackOut.frameCount = 0;
        callStackOut.frames = nullptr;
        return;
    }

    CallStackFrame* framesOut = (CallStackFrame*)CoTaskMemAlloc(callStackIn.frameCount * sizeof(CallStackFrame));
    if (framesOut == nullptr)
    {
        return;
    }

    uint i = 0;
    CallStackFrame *framesIn = callStackIn.frames;
    for ( ; i < callStackIn.frameCount; i++)
    {
        framesOut[i] = framesIn[i];
        if (framesOut[i].activeScriptDirect != nullptr)
        {
            framesOut[i].activeScriptDirect->AddRef();
        }
        if (FAILED(FillText(framesOut[i].functionName, framesIn[i].functionName)))
        {
            // if OOM, then just return what were able to copy so far
            break;
        }

    }
    callStackOut.frames = framesOut;
    callStackOut.frameCount = i;
}

void ActiveScriptError::FreeStackTrace(CallStack& callStack)
{
    for (uint i=0; i < callStack.frameCount; i++)
    {
        if (callStack.frames[i].activeScriptDirect != nullptr) 
        {
            callStack.frames[i].activeScriptDirect->Release();
        }
        CoTaskMemFree((LPVOID)(callStack.frames[i].functionName));
    }
    if (callStack.frames)
    {
        CoTaskMemFree((LPVOID)(callStack.frames));
    }
    callStack.frames = nullptr;
    callStack.frameCount = 0;
}

// Info:        Gets an IErrorInfo contained in the exception object, if any.
// Parameters:  exceptionObject - the exception from with to obtain the error info
//              errInfo - the resulting IErrorInfo
void ActiveScriptError::StoreErrorInfo(Js::JavascriptExceptionObject * exceptionObject, IErrorInfo ** errInfo)
{
    if (errInfo)
    {
        *errInfo = nullptr;
        Var errorObject = exceptionObject->GetThrownObject(nullptr);
        if (errorObject && Js::JavascriptErrorDebug::Is(errorObject))
        {
            Js::JavascriptErrorDebug * errDebug = Js::JavascriptErrorDebug::FromVar(errorObject);
            if (errDebug)
            {
                *errInfo = errDebug->GetRestrictedErrorInfo();
                (*errInfo)->AddRef();
            }
        }
    }
}


HRESULT ActiveScriptError::CanHandleException(Js::ScriptContext * scriptContext, Js::JavascriptExceptionObject * exceptionObject, IServiceProvider * pspCaller)
{
    HRESULT hr = E_FAIL;
    IUnknown * pIU = pspCaller;
    pIU->AddRef();
    IServiceProvider * pIS = pspCaller;
    pIS->AddRef();
    EXCEPINFO ei = { 0 };
    ActiveScriptError::FillExcepInfo(exceptionObject, &ei, nullptr, nullptr);
    IUnknown* jsCaller = nullptr;
    if (SUCCEEDED(pIU->QueryInterface(IID_IJavascriptLocalProxy, (void**)&jsCaller)))
    {
        pIS->Release();
        jsCaller->Release();
        Assert(scriptContext->GetThreadContext()->IsInScript());
        // Save it for the previous call root frames
        scriptContext->RecordException(exceptionObject);
        hr = SCRIPT_E_RECORDED;
        FreeExcepInfo(&ei);
        pIU->Release();
        return hr;
    }

    do
    {
        //Check if caller can handle exception
        ICanHandleException * pIC;
        hr = pIU->QueryInterface(IID_ICanHandleException, (void **)&pIC);
        pIU->Release();
        pIU = nullptr;

        Var errorObject;
        if (SUCCEEDED(hr) && ((errorObject = exceptionObject->GetThrownObject(nullptr)) != nullptr))
        {
            VARIANT variantValue;
            VariantInit(&variantValue);

            hr = DispatchHelper::MarshalJsVarToVariantNoThrow(errorObject, &variantValue, scriptContext);
            if (SUCCEEDED(hr))
            {
                hr = pIC->CanHandleException(&ei, &variantValue);
                VariantClear(&variantValue);
            }
            pIC->Release();
            if (SUCCEEDED(hr))
            {
                if (pIS != nullptr)
                {
                    pIS->Release();
                }
                hr = SCRIPT_E_PROPAGATE;
                break;
            }
        }

        // no more next caller without IServiceProvider
        if (pIS == nullptr)
        {
            break;
        }

        // Move on to caller's caller
        hr = pIS->QueryService(SID_GetCaller, IID_IUnknown, (void **)&pIU);
        pIS->Release();

        if (FAILED(hr))
        {
            break;
        }
        if (pIU == nullptr)
        {
            hr = E_FAIL;
            break;
        }
        hr = pIU->QueryInterface(__uuidof(IServiceProvider), (void **)&pIS);
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(pIU->QueryInterface(IID_IJavascriptLocalProxy, (void**)&jsCaller)))
            {
                pIS->Release();
                jsCaller->Release();
                Assert(scriptContext->GetThreadContext()->IsInScript());
                // Save it for the previous call root frames
                scriptContext->RecordException(exceptionObject);
                pIU->Release();
                hr = SCRIPT_E_RECORDED;
                break;
            }
        }
        else
        {
            pIS = nullptr;
        }
    }
    while (true);

    FreeExcepInfo(&ei);
    return hr;
}

LPCWSTR CallStackFrameHelper::DelimiterString = L"[Throw Stack Follows]";
CallStackFrameHelper::CallStackFrameHelper(CallStackFrame* callStackFrame, ULONG totalFrameCount, HRESULT& hr) :
    callStackFrames(callStackFrame), frameCount(totalFrameCount), currentFrame(0), hResult(hr)
{
}

CallStackFrameHelper::~CallStackFrameHelper()
{
    Assert(currentFrame == frameCount || hResult != NOERROR);
}

void CallStackFrameHelper::AppendFrame(CallStackFrame newFrame)
{
    Assert(currentFrame < frameCount);
    // DiD
    if (currentFrame < frameCount )
    {
        callStackFrames[currentFrame] = newFrame;
        currentFrame++;
    }
}
