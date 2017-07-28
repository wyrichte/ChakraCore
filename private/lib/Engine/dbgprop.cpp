//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

#define END_TRANSLATE_EXCEPTION_AND_DBG_HANDLE_ERROBJECT(hr) \
    END_TRANSLATE_KNOWN_EXCEPTION_TO_HRESULT(hr) \
    catch(const Js::JavascriptException& err) \
    { \
        err.GetAndClear(); \
        hr = E_FAIL; \
    } \
    CATCH_UNHANDLED_EXCEPTION(hr)



DebugProperty::DebugProperty(WeakArenaReference<Js::IDiagObjectModelDisplay>* pModel,
                             IDebugApplicationThread* _pApplicationThread,
                             ScriptEngine* scriptEngine,
                             __in_opt IUnknown * punkParent,
                             bool isInReturnValueHierarchy,
                             DWORD setDbgPropInfoAttributes)
    : m_refCount(1),
      pModelWeakRef(pModel),
      m_pApplicationThread(_pApplicationThread),
      pScriptEngine(scriptEngine),
      spunkParent(punkParent),
      m_isInReturnValueHierarchy(isInReturnValueHierarchy),
      m_setDbgPropInfoAttributes(setDbgPropInfoAttributes)
{
}

DebugProperty::~DebugProperty()
{
    if (pModelWeakRef)
    {
        HeapDelete(pModelWeakRef);
    }
}

ULONG DebugProperty::AddRef(void)
{
    return (ulong)InterlockedIncrement(&m_refCount);
}

ULONG DebugProperty::Release(void)
{
    ulong refCount = (ulong)InterlockedDecrement(&m_refCount);

    if (0 == refCount)
    {
        HeapDelete(this);
    }

    return refCount;
}

HRESULT DebugProperty::QueryInterface(REFIID iid, void ** ppv)
{
    if(!ppv)
    {
        return E_INVALIDARG;
    }

    if (__uuidof(IUnknown) == iid)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IDebugProperty*>(this));
    }
    else if (__uuidof(IDebugProperty) == iid)
    {
        *ppv = static_cast<IDebugProperty*>(this);
    }
    else if (__uuidof(IDebugThreadCall) == iid)
    {
        *ppv = static_cast<IDebugThreadCall*>(this);
    }
#ifdef ENABLE_MUTATION_BREAKPOINT
    else if (__uuidof(IDebugPropertyObjectMutation) == iid)
    {
        *ppv = static_cast<IDebugPropertyObjectMutation*>(this);
    }
#endif
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT DebugProperty::GetPropertyInfo(
        /* [in] */ DBGPROP_INFO_FLAGS dwFieldSpec,
        /* [in] */ UINT nRadix,
        /* [out] */ __RPC__out DebugPropertyInfo *pPropertyInfo)
{
    HRESULT hr = S_OK;

    if (!pPropertyInfo)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    if (!m_pApplicationThread)
    {
        hr = E_FAIL;
        goto Error;
    }

    if (S_OK == m_pApplicationThread->QueryIsCurrentThread())
    {
        Js::IDiagObjectModelDisplay* pModel = pModelWeakRef->GetStrongReference();
        if (!pModel)
        {
            return E_FAIL;
        }

        if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
        {
            return hr;
        }

        pPropertyInfo->m_dwValidFields = 0;

        BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
        {
            // Fill the debug prop first so that we will have the debug prop filled even if there is an exception
            if (dwFieldSpec & DBGPROP_INFO_DEBUGPROP)
            {
                pPropertyInfo->m_pDebugProp = this;
                AddRef();
                pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_DEBUGPROP;
            }

            if (dwFieldSpec & DBGPROP_INFO_NAME)
            {
                LPCWSTR name = pModel->Name();
                if (name)
                {
                    pPropertyInfo->m_bstrName = SysAllocString(name);
                    pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_NAME;
                }
            }

            if (dwFieldSpec & DBGPROP_INFO_TYPE)
            {
                LPCWSTR type = pModel->Type();
                if (type)
                {
                    pPropertyInfo->m_bstrType = SysAllocString(type);
                    pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_TYPE;
                }
            }

            if (dwFieldSpec & DBGPROP_INFO_VALUE)
            {
                // The engine did not restricted itself of 10 and 16 for radix value, so no need to put restriction at this place.
                LPCWSTR value = pModel->Value((int)nRadix);
                if (value)
                {
                    pPropertyInfo->m_bstrValue = SysAllocString(value);
                    pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_VALUE;
                }
            }

            if (dwFieldSpec & DBGPROP_INFO_FULLNAME)
            {
                if (SUCCEEDED(hr = GetFullName(&pPropertyInfo->m_bstrFullName)))
                {
                    pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_FULLNAME;
                }
            }

            if (dwFieldSpec & DBGPROP_INFO_ATTRIBUTES)
            {
                pPropertyInfo->m_dwAttrib = m_setDbgPropInfoAttributes;

                if (pModel->HasChildren())
                {
                    pPropertyInfo->m_dwAttrib |= DBGPROP_ATTRIB_VALUE_IS_EXPANDABLE;
                }

                pPropertyInfo->m_dwAttrib |= pModel->GetTypeAttribute();

                if (pPropertyInfo->m_dwAttrib & DBGPROP_ATTRIB_VALUE_IS_RETURN_VALUE)
                {
                    m_isInReturnValueHierarchy = true;
                }
                // Mark current attribute the fake attributes if we are in hierarchy of the return value, that way we will not allow the Add to watch happen
                pPropertyInfo->m_dwAttrib |= (m_isInReturnValueHierarchy ? DBGPROP_ATTRIB_VALUE_IS_FAKE : 0);

                pPropertyInfo->m_dwValidFields |= DBGPROP_INFO_ATTRIBUTES;
            }
        }
        END_TRANSLATE_EXCEPTION_AND_DBG_HANDLE_ERROBJECT(hr);

        pModelWeakRef->ReleaseStrongReference();
    }
    else
    {
        GetPropertyInfoArgs args;
        args.dwFieldSpec = dwFieldSpec;
        args.nRadix = nRadix;
        args.pPropertyInfo = pPropertyInfo;

        hr = m_pApplicationThread->SynchronousCallIntoThread(
                            static_cast<IDebugThreadCall*>(this),
                            xthread_GetPropertyInfo,
                            (DWORD_PTR)&args,
                            0);
    }

// TODO: , DBGPROP_INFO_BEAUTIFY, DBGPROP_INFO_CALLTOSTRING, DBGPROP_INFO_AUTOEXPAND

Error:
    return hr;
}

HRESULT DebugProperty::GetExtendedInfo( 
    /* [in] */ ULONG cInfos,
    /* [size_is][in] */ __RPC__in_ecount_full(cInfos) GUID *rgguidExtendedInfo,
    /* [size_is][out] */ __RPC__out_ecount_full(cInfos) VARIANT *rgvar)
{
    return E_NOTIMPL;
}

/*static*/
HRESULT DebugProperty::BuildExprAndEval(BSTR bstrFullName,
                                    DebugProperty *dbgProp,
                                    Js::IDiagObjectAddress* address,
                                    Js::ScriptContext *scriptContext,
                                    Js::DiagStackFrame* frame,
                                    LPCOLESTR pszValue)
{
    Assert(scriptContext);
    Assert(frame);

    Var result = NULL;
    HRESULT hr = E_FAIL;
    if (address != NULL)
    {
        //
        // Attempt to get the value out of right hand side of the expression, if the value can be setable, ie. if the lhs has any address. 

        Js::ScriptFunction* func = frame->TryGetFunctionForEval(scriptContext, pszValue, wcslen(pszValue));
        if (!func)
        {
            // Clean this exception, as we are not going to report this error.
            scriptContext->GetThreadContext()->SetRecordedException(nullptr);
        }
        else
        {
            result = frame->DoEval(func);
            if (result != NULL)
            {
                hr = S_OK;
            }

            // Clean this exception, as we are not going to report this error.
            scriptContext->GetThreadContext()->SetRecordedException(nullptr);
        }
    }

    Var prevValue = address ? address->GetValue(FALSE) : NULL;

    if (hr != S_OK || result == NULL || address == NULL || !address->Set(result))
    {
        CComBSTR bstrFullString = NULL;
        if (dbgProp != NULL)
        {
            Assert(bstrFullName == NULL);
            hr = dbgProp->GetFullName(&bstrFullString);
        }
        else
        {
            Assert(dbgProp == NULL);
            bstrFullString = bstrFullName; 
        }

        if ((hr = bstrFullString.Append(_u(" = "))) == S_OK
            && (hr = bstrFullString.Append(pszValue)) == S_OK)
        {
            Js::ScriptFunction* updatedFunc = frame->TryGetFunctionForEval(scriptContext, bstrFullString, bstrFullString.Length());
            if (updatedFunc != NULL)
            {
                frame->DoEval(updatedFunc);
                hr = S_OK;
            }
        }
    }

    if (hr == S_OK && (prevValue != NULL && address->GetValue(TRUE) == prevValue))
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT DebugProperty::SetValueAsString( 
    /* [in] */ __RPC__in LPCOLESTR pszValue,
    /* [in] */ UINT nRadix)
{
    HRESULT hr = S_OK;

    if (!m_pApplicationThread)
    {
        hr = E_FAIL;
        goto Error;
    }

    if (S_OK == m_pApplicationThread->QueryIsCurrentThread())
    {
        if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
        {
            return hr;
        }

        Js::IDiagObjectModelDisplay* pModel = pModelWeakRef->GetStrongReference();
        if (!pModel)
        {
            return E_FAIL;
        }

        Js::ScriptContext* scriptContext = pScriptEngine->GetScriptSiteHolder()->GetScriptSiteContext();

        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, true)
        {
            ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
            Js::WeakDiagStack* weakStack = scriptContext->GetDebugContext()->GetProbeContainer()->GetFramePointers();
            Assert(weakStack);
            Js::DiagStack* stack = weakStack->GetStrongReference();
            Js::DiagStackFrame* leafFrame = stack->Peek(0);

            weakStack->ReleaseStrongReference();        
            HeapDelete(weakStack);

            hr = DebugProperty::BuildExprAndEval(NULL, this, pModel->GetDiagAddress(), scriptContext, leafFrame, pszValue);
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)

        pModelWeakRef->ReleaseStrongReference();
    }
    else
    {
        SetValueAsStringArgs args;
        args.pszValue = pszValue;
        args.nRadix = nRadix;

        hr = m_pApplicationThread->SynchronousCallIntoThread(
                                    static_cast<IDebugThreadCall*>(this),
                                    xthread_SetValueAsString,
                                    (DWORD_PTR)&args,
                                    0);
    }

Error:
    return hr;
}

HRESULT DebugProperty::GetFullName(_Out_ BSTR * pbstrFullName)
{
    Assert(pbstrFullName);
    Assert(S_OK == m_pApplicationThread->QueryIsCurrentThread());

    HRESULT hr = S_OK;

    DebugProperty* currentDebugProperty = this;
    *pbstrFullName = nullptr;

    while (currentDebugProperty != nullptr)
    {
        Js::IDiagObjectModelDisplay* currentDisplay = currentDebugProperty->pModelWeakRef->GetStrongReference();
        IfNullReturnError(currentDisplay, E_FAIL);

        bool isFake = currentDisplay->IsFake();
        bool isSymbolProperty = currentDisplay->IsSymbolProperty();
        bool isLiteralProperty = currentDisplay->IsLiteralProperty();
        Js::DiagObjectModelDisplayType displayType = currentDisplay->GetType();
        
        currentDebugProperty->pModelWeakRef->ReleaseStrongReference();
        
        if (isSymbolProperty)
        {
            // We can't use a full name string to fetch the value when the property is keyed by a symbol.
            // For example, if we have:
            //
            // var o = {};
            // o[Symbol('sym')] = 1;
            // // Add o to the watch window here.
            //
            // In this case we cannot use any string value to retrieve the value stored at property Symbol('sym')
            // since that property is symbol-keyed. There is no way to get the symbol itself from the fullname 
            // string and something like o.Symbol(sym), o['Symbol(sym)'], or o['sym'] will all return undefined.

            // Return failure hr, we don't want this property to be marked with a full name.
            hr = E_FAIL;
            goto Error;
        }
        else if (!isFake)
        {
            // Don't include fake nodes as part of the full name
            // ("[Methods]", "[Scope]", etc.).

            // Grab the debug info with the name filled in.
            DebugPropertyInfo debugPropertyInfo;
            IfFailGo(currentDebugProperty->GetPropertyInfo(DBGPROP_INFO_NAME, 16, &debugPropertyInfo));

            BSTR currentName =
                (debugPropertyInfo.m_dwValidFields & DBGPROP_INFO_NAME) == DBGPROP_INFO_NAME
              ? debugPropertyInfo.m_bstrName
              : nullptr;

            // PrependParentNameToChildFullName will allocate a new fullname so save the location prior to
            // combining so we can free it after.
            BSTR composedName = *pbstrFullName;
            if (currentName != nullptr && SysStringLen(currentName) == 0)
            {
                CComBSTR emptyName(_u("[\"\"]"));
                IfFailGo(PrependParentNameToChildFullName(emptyName, composedName, isLiteralProperty, pbstrFullName));
            }
            else
            {
                IfFailGo(PrependParentNameToChildFullName(currentName, composedName, isLiteralProperty, pbstrFullName));
            }
            SysFreeString(composedName);
        }
        else if (displayType == Js::DiagObjectModelDisplayType_RecyclableCollectionObjectDisplay)
        {
            // We need to special case maps, sets, weakmaps, and weaksets.  In these cases,
            // we don't want to allow a full name beyond the root container object since
            // the nodes leading down to the key/value types are fake and there's no
            // guaranteed representation for accessing the data in these containers.
            // For example, if we have:
            // 
            // var m = new Map();
            // {
            //     let a = {};
            //     m.set(a, 1);
            // }
            // // Add m to the watch window here.
            // 
            // In this case, there is no full name expression that can retrieve 'a' or its value,
            // as they were lost when the inner scope was left.

            // Chop the total string we've been building back down to nothing.
            // The parent (which is the map variable name) will be added on next iteration.
            SysFreeString(*pbstrFullName);
            *pbstrFullName = SysAllocString(_u(""));
            IfNullReturnError(*pbstrFullName, E_OUTOFMEMORY);
        }
        else if (displayType == Js::DiagObjectModelDisplayType_RecyclableSimdDisplay)
        {
            SysFreeString(*pbstrFullName);
            *pbstrFullName = SysAllocString(_u("SIMD"));
            IfNullReturnError(*pbstrFullName, E_OUTOFMEMORY);
        }
        else if (*pbstrFullName == nullptr)
        {
            // So we wanted to get the fullname for fake node itself.
            // Since this is a fake node, we need not give any fullname to it.
            return E_FAIL;
        }

        // Move to the next parent.
        CComPtr<IDebugProperty> parentDebugProperty;
        IfFailGo(currentDebugProperty->GetParent(&parentDebugProperty));
        currentDebugProperty = reinterpret_cast<DebugProperty*>(parentDebugProperty.p);
    }

    if (*pbstrFullName == nullptr)
    {
        // Need to check for the case where no string was appended (everything was fake).
        // This can happen in the case of "[Globals]" so we'll just add the short name
        // as the full name in this case.
        Js::IDiagObjectModelDisplay* display = this->pModelWeakRef->GetStrongReference();
        IfNullReturnError(display, E_FAIL);

        *pbstrFullName = SysAllocString(display->Name());
        this->pModelWeakRef->ReleaseStrongReference();
        IfNullReturnError(*pbstrFullName, E_OUTOFMEMORY);
    }

Error:
    if (FAILED(hr))
    {
        SysFreeString(*pbstrFullName);
        *pbstrFullName = nullptr;
    }

    return hr;
}

HRESULT DebugProperty::PrependParentNameToChildFullName(
    _In_z_ BSTR parentName,
    _In_z_ BSTR childFullName,
    _In_ bool isParentNameLiteralProperty,
    _Out_ BSTR* combinedFullName)
{
    Assert(combinedFullName);

    uint parentNameLength = parentName == nullptr ? 0u : SysStringLen(parentName);
    uint childFullNameLength = childFullName == nullptr ? 0u : SysStringLen(childFullName);

    if (parentNameLength == 0 && childFullNameLength == 0)
    {
        *combinedFullName = SysAllocString(_u(""));
        IfNullReturnError(*combinedFullName, E_OUTOFMEMORY);
        return S_OK;
    }
    else if (parentNameLength == 0)
    {
        *combinedFullName = SysAllocString(childFullName);
        IfNullReturnError(*combinedFullName, E_OUTOFMEMORY);
        return S_OK;
    }

    if (isParentNameLiteralProperty)
    {
        return PrependParentNameToChildFullName(parentName, parentNameLength, childFullName, childFullNameLength, combinedFullName);
    }
    else
    {
        return PrependNonLiteralPropertyNameToChildFullName(parentName, parentNameLength, childFullName, childFullNameLength, combinedFullName);
    }
}

HRESULT DebugProperty::PrependParentNameToChildFullName(
    _In_z_ BSTR parentName,
    _In_ uint parentNameLength,
    _In_z_ BSTR childFullName,
    _In_ uint childFullNameLength,
    _Out_ BSTR* combinedFullName)
{
    const bool fullNameWasArrayElement = childFullNameLength > 0 ? childFullName[0] == _u('[') : false;
    if (fullNameWasArrayElement)
    {
        return PrependParentArrayNameToChildFullName(parentName, parentNameLength, childFullName, childFullNameLength, combinedFullName);
    }
    else
    {
        return PrependDottedElementNameToChildFullName(parentName, parentNameLength, childFullName, childFullNameLength, combinedFullName);
    }
}

// Enumerates all characters in a string and checks for characters needing escape in full name.
template<typename Function>
void DebugProperty::EnumEscapeCharacters(_In_reads_z_(strLength) PCWSTR str, uint strLength, Function function)
{
    for (uint i = 0; i < strLength; ++i)
    {
        WCHAR currentCharacter = str[i];

        PCWCHAR escaped;
        uint escapedLength;

        switch (currentCharacter)
        {
        case _u('\''):
            {
                static const WCHAR ESCAPE[] = _u("\\\'");      // '  =>  \'
                escaped = ESCAPE;
                escapedLength = _countof(ESCAPE) - 1;
            }
            break;

        case _u('\\'):
            {
                static const WCHAR ESCAPE[] = _u("\\\\");      // \  => \\  
                escaped = ESCAPE;
                escapedLength = _countof(ESCAPE) - 1;
            }
            break;

        case _u('\0'):
            {
                static const WCHAR ESCAPE[] = _u("\\u0000");   // \0  => \u0000
                escaped = ESCAPE;
                escapedLength = _countof(ESCAPE) - 1;
            }
            break;

        default:                                            // no escape needed
            escaped = &currentCharacter;
            escapedLength = 1;
            break;
        }

        if (!function(escaped, escapedLength)) // "function" returns false to stop enumeration
        {
            break;
        }
    }
}

// Gets the count of characters in escaped string
HRESULT DebugProperty::GetEscapedCharacterCount(_In_reads_z_(strLength) PCWSTR str, uint strLength, _Out_ uint* count)
{
    HRESULT hr = S_OK;
    *count = 0;

    EnumEscapeCharacters(str, strLength, [&](_In_reads_(escapedLength) PCWCHAR escaped, uint escapedLength) -> bool
    {
        Assert(SUCCEEDED(hr));

        hr = UIntAdd(*count, escapedLength, count);
        return SUCCEEDED(hr);
    });

    return hr;
}

// Copies all characters from a source string into a destination and escapes all characters
// needed in full name, so that we can evaluate the full name expression properly.
// For example:
// var reg = RegExp();
// reg.constructor.$' will be full name expanded to reg.constructor['$\''].
HRESULT DebugProperty::EscapeCharacters(_Out_writes_all_(destinationLength) PWCHAR destination, uint destinationLength, _In_reads_z_(sourceLength) PCWSTR source, uint sourceLength)
{
    PCWCHAR destinationEnd = destination + destinationLength;
    EnumEscapeCharacters(source, sourceLength, [&](_In_reads_(escapedLength) PCWCHAR escaped, uint escapedLength) -> bool
    {
        if (destination + escapedLength <= destinationEnd) // has enough buffer
        {
            while (escapedLength--)
            {
                *destination++ = *escaped++;
            }
            return true;
        }

        AssertMsg(false, "Buffer length mismatch?");
        return false;
    });

    AssertMsg(destination == destinationEnd, "Must have filled destination buffer");
    return destination == destinationEnd ? S_OK : E_UNEXPECTED;
}

HRESULT DebugProperty::PrependNonLiteralPropertyNameToChildFullName(
    _In_z_ BSTR parentName,
    _In_ uint parentNameLength,
    _In_z_ BSTR childFullName,
    _In_ uint childFullNameLength,
    _Out_ BSTR* combinedFullName)
{
    Assert(parentName);
    Assert(combinedFullName);

    HRESULT hr = S_OK;
    BSTR adjustedName = nullptr;

    // Get required length after escaping property name.
    uint escapedNameLength;
    IfFailGo(GetEscapedCharacterCount(parentName, parentNameLength, &escapedNameLength));
    const bool noEscapeCharacters = escapedNameLength == parentNameLength;

    // Allocate room for appending [' and ']
    uint adjustedNameLength;
    IfFailGo(UIntAdd(escapedNameLength, 4, &adjustedNameLength));

    adjustedName = SysAllocStringLen(nullptr, adjustedNameLength);
    IfNullReturnError(adjustedName, E_OUTOFMEMORY);

    PWCHAR currentAdjustedNameElement = adjustedName;

    // Ex. "a" and "-1" become "a['-1']".
    *currentAdjustedNameElement++ = _u('[');
    *currentAdjustedNameElement++ = _u('\'');
    if (noEscapeCharacters)
    {
        js_memcpy_s(currentAdjustedNameElement, sizeof(OLECHAR) * parentNameLength, parentName, sizeof(OLECHAR) * parentNameLength);
    }
    else
    {
        IfFailGo(EscapeCharacters(currentAdjustedNameElement, escapedNameLength, parentName, parentNameLength));
    }
    currentAdjustedNameElement += escapedNameLength;

    *currentAdjustedNameElement++ = _u('\'');
    *currentAdjustedNameElement++ = _u(']');

    IfFailGo(PrependParentNameToChildFullName(adjustedName, adjustedNameLength, childFullName, childFullNameLength, combinedFullName));

Error:
    SysFreeString(adjustedName);
    return hr;
}

HRESULT DebugProperty::PrependParentArrayNameToChildFullName(
    _In_z_ BSTR parentName,
    _In_ uint parentNameLength,
    _In_z_ BSTR childFullName,
    _In_ uint childFullNameLength,
    _Out_ BSTR* combinedFullName)
{
    Assert(parentName);
    Assert(childFullName);
    Assert(combinedFullName);
    Assert(childFullName[0] == _u('['));

    HRESULT hr = S_OK;

    uint length;
    IfFailGo(UIntAdd(parentNameLength, childFullNameLength, &length));

    *combinedFullName = SysAllocStringLen(nullptr, length);
    IfNullReturnError(*combinedFullName, E_OUTOFMEMORY);

    // Ex. 'a' and '[0]' become 'a[0]'.
    js_memcpy_s(*combinedFullName, sizeof(OLECHAR) * parentNameLength, parentName, sizeof(OLECHAR) * parentNameLength);
    js_memcpy_s(*combinedFullName + parentNameLength, sizeof(OLECHAR) * childFullNameLength, childFullName, sizeof(OLECHAR) * childFullNameLength);

Error:
    return hr;
}

HRESULT DebugProperty::PrependDottedElementNameToChildFullName(
    _In_z_ BSTR parentName,
    _In_ uint parentNameLength,
    _In_z_ BSTR childFullName,
    _In_ uint childFullNameLength,
    _Out_ BSTR* combinedFullName)
{
    Assert(parentName);
    Assert(combinedFullName);
    Assert(parentNameLength > 0);

    if (childFullNameLength == 0)
    {
        *combinedFullName = SysAllocStringLen(parentName, parentNameLength);
        IfNullReturnError(*combinedFullName, E_OUTOFMEMORY);
        return S_OK;
    }

    HRESULT hr = S_OK;

    // Allocate room for the '.' in-between the strings.
    uint length;
    IfFailGo(UIntAdd(parentNameLength, childFullNameLength, &length));
    IfFailGo(UIntAdd(length, 1, &length));

    *combinedFullName = SysAllocStringLen(nullptr, length);
    IfNullReturnError(*combinedFullName, E_OUTOFMEMORY);

    BSTR dottedFullName = *combinedFullName;

    // Ex. 'a' and 'b' become 'a.b'.
    js_memcpy_s(dottedFullName, sizeof(OLECHAR) * parentNameLength, parentName, sizeof(OLECHAR) * parentNameLength);
    dottedFullName += parentNameLength;
    *dottedFullName++ = _u('.');
    js_memcpy_s(dottedFullName, sizeof(OLECHAR) * childFullNameLength, childFullName, sizeof(OLECHAR) * childFullNameLength);

Error:
    return hr;
}

DebugProperty::EnumMembersArgs::EnumMembersArgs(DWORD _dwFieldSpec,
                                                UINT _nRadix,
                                                REFIID _refiid,
                                                IEnumDebugPropertyInfo** _ppepi)
    :dwFieldSpec(_dwFieldSpec),
     nRadix(_nRadix),
     refiid(_refiid),
     ppepi(_ppepi)
{
}
#ifdef ENABLE_MUTATION_BREAKPOINT
DebugProperty::SetMutationBreakpointArgs::SetMutationBreakpointArgs(BOOL setOnObject, MutationType type, IMutationBreakpoint **mutationBreakpoint)
    : setOnObject(setOnObject)
    , type(type)
    , mutationBreakpoint(mutationBreakpoint)

{}

DebugProperty::CanSetMutationBreakpointArgs::CanSetMutationBreakpointArgs(BOOL setOnObject, MutationType type, BOOL *canSet) 
    : setOnObject(setOnObject)
    ,type(type)
    , canSet(canSet)

{}
#endif

HRESULT DebugProperty::EnumMembers( 
    /* [in] */ DBGPROP_INFO_FLAGS dwFieldSpec,
    /* [in] */ UINT nRadix,
    /* [in] */ __RPC__in REFIID refiid,
    /* [out] */ __RPC__deref_out_opt IEnumDebugPropertyInfo **ppepi)
{
    HRESULT hr = S_OK;

    if (!ppepi)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    if (!m_pApplicationThread)
    {
        hr = E_FAIL;
        goto Error;
    }

    if (S_OK == m_pApplicationThread->QueryIsCurrentThread())
    {
        if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
        {
            return hr;
        }

        Js::IDiagObjectModelDisplay* pModel = pModelWeakRef->GetStrongReference();
        if (!pModel)
        {
            return E_FAIL;
        }

        WeakArenaReference<Js::IDiagObjectModelWalkerBase>* pObjectModelWalker = nullptr;
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_OOM_TO_HRESULT(pScriptEngine->scriptContext, /*doCleanup*/ false, /*hasCaller*/ false)
        {
            pObjectModelWalker = pModel->CreateWalker();
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_OOM_TO_HRESULT(hr);
        if (SUCCEEDED(hr))
        {
            if (pObjectModelWalker != nullptr)
            {
                CComPtr<IUnknown> spunk = NULL;
                if (!pModel->IsLocalsAsRoot())
                {
                    this->QueryInterface(__uuidof(IUnknown), (void **)&spunk);
                }

                EnumDebugPropertyInfo* pEnumPropInfo = new EnumDebugPropertyInfo(pObjectModelWalker, m_pApplicationThread, pScriptEngine, spunk, dwFieldSpec, nRadix, m_isInReturnValueHierarchy);
                *ppepi = pEnumPropInfo;
            }
            else
            {
                hr = E_FAIL;
            }
        }
        pModelWeakRef->ReleaseStrongReference();
    }
    else
    {
        EnumMembersArgs args(dwFieldSpec ,nRadix, refiid, ppepi);

        hr = m_pApplicationThread->SynchronousCallIntoThread(
                                    static_cast<IDebugThreadCall*>(this),
                                    xthread_EnumMembers,
                                    (DWORD_PTR)&args,
                                    0);
    }

Error:
    return hr;
}

HRESULT DebugProperty::GetParent( 
    /* [out] */ __RPC__deref_out_opt IDebugProperty **ppDebugProp)
{
    if (!ppDebugProp)
    {
        return E_POINTER;
    }

    if (spunkParent == NULL)
    {
        *ppDebugProp = NULL;
        return S_OK;
    }

    return spunkParent.QueryInterface(ppDebugProp);
}

HRESULT DebugProperty::ThreadCallHandler(
    /* [in] */ DWORD_PTR dwParam1,
    /* [in] */ DWORD_PTR dwParam2,
    /* [in] */ DWORD_PTR dwParam3)
{
    return DebugApiWrapper( [=] {
        switch (dwParam1)
        {
            case xthread_GetPropertyInfo:
            {
                GetPropertyInfoArgs* pArgs = (GetPropertyInfoArgs*)dwParam2;
                return GetPropertyInfo(pArgs->dwFieldSpec, pArgs->nRadix, pArgs->pPropertyInfo);
            }
            case xthread_EnumMembers:
            {
                EnumMembersArgs* pArgs = (EnumMembersArgs*)dwParam2;
                return EnumMembers(pArgs->dwFieldSpec, pArgs->nRadix, pArgs->refiid, pArgs->ppepi);
            }
            case xthread_SetValueAsString:
            {
                SetValueAsStringArgs* pArgs = (SetValueAsStringArgs*)dwParam2;
                return SetValueAsString(pArgs->pszValue, pArgs->nRadix);
            }
#ifdef ENABLE_MUTATION_BREAKPOINT
            case xthread_CanSetMutationBreakpoint:
            {
                CanSetMutationBreakpointArgs* pArgs = (CanSetMutationBreakpointArgs*)dwParam2;
                return CanSetMutationBreakpoint(pArgs->setOnObject, pArgs->type, pArgs->canSet);
            }
            case xthread_SetMutationBreakpoint:
            {
                SetMutationBreakpointArgs* pArgs = (SetMutationBreakpointArgs*)dwParam2;
                return SetMutationBreakpoint(pArgs->setOnObject, pArgs->type, pArgs->mutationBreakpoint);
            }
#endif
        }
        return E_INVALIDARG;
    });
}

// IDebugPropertyObjectMutation Methods Helpers
// Extracts Var from IDiagObjectModelDisplay
Var DebugProperty::GetVarFromModelDisplayWeakRef()
{
    if (pModelWeakRef == nullptr)
    {
        return nullptr;
    }

    Js::IDiagObjectModelDisplay *pModel = pModelWeakRef->GetStrongReference();
    Var object = nullptr;
    if (pModel)
    {
        object = pModel->GetVarValue(false);
    }
    pModelWeakRef->ReleaseStrongReference();
    return object;
}

#ifdef ENABLE_MUTATION_BREAKPOINT
STDMETHODIMP DebugProperty::CanSetMutationBreakpoint(
    /* [in] */ BOOL setOnObject,
    /* [in] */ MutationType type,
    /* [out] */ __RPC__out BOOL *canSet)
{
    if (!canSet)
    {
        return E_INVALIDARG;
    }

    // Thread check
    if (!m_pApplicationThread)
    {
        return E_FAIL;
    }

    if (S_OK != m_pApplicationThread->QueryIsCurrentThread())
    {
        CanSetMutationBreakpointArgs args(setOnObject, type, canSet);

        return m_pApplicationThread->SynchronousCallIntoThread(
            static_cast<IDebugThreadCall*>(this),
            xthread_CanSetMutationBreakpoint,
            (DWORD_PTR)&args,
            0);
    }

    // Check if script engine is closed
    HRESULT hr = S_OK;
    if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
    {
        return hr;
    }

    DebugProperty *parentProperty = this;
    CComPtr<DebugProperty> parentPropertyRef;           // Use this to keep track of the ref count of the parent for !setOnObject
    if (!setOnObject)
    {
        // Get parent debug property, if we can't get parent property we can't set OMBP
        IfFailedReturn(this->GetParent(reinterpret_cast<IDebugProperty **>(&parentPropertyRef)));
        if (parentPropertyRef == nullptr)
        {
            *canSet = false;
            return S_OK;
        }
        parentProperty = parentPropertyRef;
    }

    Var var = parentProperty->GetVarFromModelDisplayWeakRef();
    IfNullReturnError(var, E_FAIL);

    *canSet = Js::MutationBreakpoint::CanSet(var);

    return S_OK;
}

STDMETHODIMP DebugProperty::SetMutationBreakpoint(
    /* [in] */ BOOL setOnObject,
    /* [in] */ MutationType type,
    /* [out] */ __RPC__deref_out_opt IMutationBreakpoint **mutationBreakpoint)
{
    IfNullReturnError(mutationBreakpoint, E_INVALIDARG);
    
    // Thread check
    if (!m_pApplicationThread)
    {
        return E_FAIL;
    }

    if (S_OK != m_pApplicationThread->QueryIsCurrentThread())
    {
        SetMutationBreakpointArgs args(setOnObject, type, mutationBreakpoint);

        return m_pApplicationThread->SynchronousCallIntoThread(
            static_cast<IDebugThreadCall*>(this),
            xthread_SetMutationBreakpoint,
            (DWORD_PTR)&args,
            0);
    }

    // Check if script engine is closed
    HRESULT hr = S_OK;
    if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
    {
        return hr;
    }
    
    // Check if script context exists 
    Js::ScriptContext *scriptContext = pScriptEngine->GetScriptContext();
    IfNullReturnError(scriptContext, E_FAIL);

    DebugProperty *parentProperty = this;
    CComPtr<DebugProperty> parentPropertyRef;           // Use this to keep track of the ref count of the parent for !setOnObject
    PropertyId parentPid = Js::Constants::NoProperty;
    PropertyId pid = Js::Constants::NoProperty;
    if (!setOnObject)
    {
        // Get parent debug property, if we can't get parent property we can't set OMBP
        IfFailedReturn(this->GetParent(reinterpret_cast<IDebugProperty **>(&parentPropertyRef)));
        IfNullReturnError(parentPropertyRef, E_FAIL);
        parentProperty = parentPropertyRef;

        // Get property Id
        Js::RecyclableObjectDisplay *pModel = static_cast<Js::RecyclableObjectDisplay *>(this->pModelWeakRef->GetStrongReference());
        pid = pModel->GetPropertyId();
        this->pModelWeakRef->ReleaseStrongReference();
    }

    // Get Property ID of object on which Mutation will be set
    Js::RecyclableObjectDisplay *pModel = static_cast<Js::RecyclableObjectDisplay *>(parentProperty->pModelWeakRef->GetStrongReference());
    parentPid = pModel->GetPropertyId();
    parentProperty->pModelWeakRef->ReleaseStrongReference();

    // Get object
    Var obj = parentProperty->GetVarFromModelDisplayWeakRef();
    Assert(obj);
    
    // Set/update mutation breakpoint
    BEGIN_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT_NESTED
    {
        Assert(!setOnObject || pid == Js::Constants::NoProperty);
        *mutationBreakpoint = Js::MutationBreakpoint::Set(scriptContext, obj, setOnObject, type, parentPid, pid);
    }
    END_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    
    // Translated error/exception
    if (hr != S_OK)
    {
        return hr;
    }
    // Some how creation of delegate failed but no exception
    else if (*mutationBreakpoint == nullptr)
    {
        return E_FAIL;
    }
    return S_OK;
}
#endif
DebugPropertySetValueCallback::DebugPropertySetValueCallback(IDebugApplicationThread* _pApplicationThread,
                             ScriptEngine* scriptEngine)
    : pApplicationThread(_pApplicationThread),
      pScriptEngine(scriptEngine),
      pAddress(NULL),
      m_refCount(1)
{
}

ULONG DebugPropertySetValueCallback::AddRef(void)
{
    return (ulong)InterlockedIncrement(&m_refCount);
}

ULONG DebugPropertySetValueCallback::Release(void)
{
    ulong refCount = (ulong)InterlockedDecrement(&m_refCount);

    if (0 == refCount)
    {
        delete this;
    }

    return refCount;
}

HRESULT DebugPropertySetValueCallback::QueryInterface(REFIID iid, void ** ppv)
{
    if(!ppv)
    {
        return E_INVALIDARG;
    }

    if (__uuidof(IUnknown) == iid)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IDebugSetValueCallback*>(this));
    }
    else if (__uuidof(IDebugSetValueCallback) == iid)
    {
        *ppv = static_cast<IDebugSetValueCallback*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

void DebugPropertySetValueCallback::SetDiagAddress(Js::IDiagObjectAddress* _pAddress)
{
    if (_pAddress)
    {
        pAddress = _pAddress;
    }
}

HRESULT STDMETHODCALLTYPE DebugPropertySetValueCallback::SetValue(VARIANT *pvarNode,
        DISPID dispid,
        ULONG clIndicies,
        LONG *prglIndicies,
        LPCOLESTR pszValue,
        UINT nRadix,
        BSTR *pbstrError)
{
    if (pApplicationThread->QueryIsCurrentThread() != S_OK)
    {
        // This callback will be called from the debug manager, it is expected to be called pApplicationThread.
        Assert(FALSE);
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    Js::ScriptContext* scriptContext = NULL;
    ReferencedArenaAdapter* pRefArena = NULL;

    if (pAddress == NULL)
    {
        hr = E_UNEXPECTED;
        goto Error;
    }

    if (NULL == pvarNode || NULL == pszValue || pvarNode->vt != VT_BSTR)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    if (pbstrError)
    {
        *pbstrError = NULL;
    }

    if (DebugHelper::IsScriptEngineClosed(pScriptEngine, &hr))
    {
        return hr;
    }

    scriptContext = pScriptEngine->GetScriptSiteHolder()->GetScriptSiteContext();

    pRefArena = scriptContext->GetThreadContext()->GetDebugManager()->GetDiagnosticArena();

    if (!pRefArena)
    {
        hr = E_FAIL;
        goto Error;
    }

    // scope
    {
        Js::WeakDiagStack* weakStack = nullptr;
        Js::DiagStackFrame* leafFrame = nullptr;
        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, true)
        {
            ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
            weakStack = scriptContext->GetDebugContext()->GetProbeContainer()->GetFramePointers();

            Assert(weakStack);
            Js::DiagStack* stack = weakStack->GetStrongReference();

            leafFrame = stack->Peek(0);

            weakStack->ReleaseStrongReference();
            HeapDelete(weakStack);

            hr = DebugProperty::BuildExprAndEval(pvarNode->bstrVal, NULL, pAddress, scriptContext, leafFrame, pszValue);
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr);
    }

Error:
    return hr;
}

EnumDebugPropertyInfo::EnumDebugPropertyInfo(WeakArenaReference<Js::IDiagObjectModelWalkerBase>* pWalker,
                                             IDebugApplicationThread* _pApplicationThread,
                                             ScriptEngine* scriptEngine,
                                             __in_opt IUnknown * punkDbgProp,
                                             DBGPROP_INFO_FLAGS dwFieldSpec,
                                             UINT nRadix,
                                             bool isInReturnValueHierarchy)
    : m_refCount(1),
     current(0),
     pWalkerWeakRef(pWalker),
     m_pApplicationThread(_pApplicationThread),
     m_pScriptEngine(scriptEngine),
     spunkDbgProp(punkDbgProp),
     m_dwFieldSpec(dwFieldSpec),
     m_nRadix(nRadix),
     m_isInReturnValueHierarchy(isInReturnValueHierarchy)
{
}

EnumDebugPropertyInfo::~EnumDebugPropertyInfo()
{
    if (pWalkerWeakRef)
    {
        HeapDelete(pWalkerWeakRef);
    }
}

ULONG EnumDebugPropertyInfo::AddRef(void)
{
    return (ulong)InterlockedIncrement(&m_refCount);
}

ULONG EnumDebugPropertyInfo::Release(void)
{
    ulong refCount = (ulong)InterlockedDecrement(&m_refCount);

    if (0 == refCount)
    {
        delete this;
    }
    return refCount;
}

HRESULT EnumDebugPropertyInfo::QueryInterface(REFIID iid, void ** ppv)
{
    if(!ppv)
    {
        return E_INVALIDARG;
    }

    if (__uuidof(IUnknown) == iid)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IEnumDebugPropertyInfo*>(this));
    }
    else if (__uuidof(IEnumDebugPropertyInfo) == iid)
    {
        *ppv = static_cast<IEnumDebugPropertyInfo*>(this);
    }
    else if (__uuidof(IDebugThreadCall) == iid)
    {
        *ppv = static_cast<IDebugThreadCall*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT EnumDebugPropertyInfo::NextInternal(ULONG celt, DebugPropertyInfo *pi, ULONG *pcEltsfetched)
{
    Assert(S_OK == m_pApplicationThread->QueryIsCurrentThread());
    Assert(pi != nullptr && pcEltsfetched != nullptr);
    Assert(m_pApplicationThread != nullptr);

    HRESULT hr = S_OK;
    if (DebugHelper::IsScriptEngineClosed(m_pScriptEngine, &hr))
    {
        return hr;
    }
    Js::ScriptContext* scriptContext = m_pScriptEngine->GetScriptSiteHolder()->GetScriptSiteContext();

    Js::IDiagObjectModelWalkerBase* pWalker = pWalkerWeakRef->GetStrongReference();
    if (!pWalker)
    {
        return E_FAIL;
    }

    *pcEltsfetched = 0;
    int start = current;
    for (ulong i = 0; i < celt; i++)
    {
        Js::ResolvedObject resolvedObj;
        BOOL ret = FALSE;
        try
        {
            BEGIN_JS_RUNTIME_CALL(scriptContext)
            {
                ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
                ret = pWalker->Get((int)(i+start), &resolvedObj);
            }
            END_JS_RUNTIME_CALL(scriptContext)
        }
        catch(const Js::JavascriptException& err)
        {
            Js::JavascriptExceptionObject* exception = err.GetAndClear();
            Var error = exception->GetThrownObject(scriptContext);
            resolvedObj.obj = error;
            resolvedObj.address = NULL;
            resolvedObj.scriptContext = exception->GetScriptContext();
            resolvedObj.typeId = Js::JavascriptOperators::GetTypeId(error);
            resolvedObj.name = _u("{error}");
            resolvedObj.propId = Js::Constants::NoProperty;
            ret = TRUE;
        }
        if (!ret)
        {
            pi[i].m_dwValidFields = 0;
            pi[i].m_pDebugProp = NULL;
            hr = E_FAIL;
            goto Error;
        }

        if (DebugHelper::IsScriptEngineClosed(m_pScriptEngine, &hr))
        {
            goto Error;
        }

        if (resolvedObj.typeId != Js::TypeIds_HostDispatch)
        {                               
            CComPtr<IDebugProperty> property;
            BEGIN_TRANSLATE_OOM_TO_HRESULT
            {
                AutoPtr<WeakArenaReference<Js::IDiagObjectModelDisplay>> pDisplay  = resolvedObj.GetObjectDisplay();
                property.Attach(HeapNew(DebugProperty, pDisplay, m_pApplicationThread, m_pScriptEngine, spunkDbgProp, m_isInReturnValueHierarchy));
                pDisplay.Detach();
            }
            END_TRANSLATE_OOM_TO_HRESULT(hr)

            if (FAILED(hr))
            {
                goto Error;
            }

            property->GetPropertyInfo(m_dwFieldSpec | DBGPROP_INFO_DEBUGPROP,
                m_nRadix,
                pi + i);         
        }
        else
        {
            HostDispatch* pHostDisp = (HostDispatch*)resolvedObj.obj;
            IDispatch* pDisp = nullptr;
            BEGIN_JS_RUNTIME_CALL_EX(scriptContext, false)
            {
                ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
                pDisp = pHostDisp->GetDispatch();
            }
            END_JS_RUNTIME_CALL(scriptContext)

            if (nullptr == pDisp)
            {
                hr = E_FAIL;
                goto Error;
            }
            CComVariant variant = pDisp;
            pDisp->Release();

            CComPtr<IDebugProperty> spDbgProp;

            DebugPropertySetValueCallback* pPropSetValue = new DebugPropertySetValueCallback(m_pApplicationThread, m_pScriptEngine);

            if (pPropSetValue == nullptr)
            {
                hr = E_OUTOFMEMORY;
                goto Error;
            }

            pPropSetValue->SetDiagAddress(resolvedObj.address);

            if (S_OK != m_pScriptEngine->DbgCreateBrowserFromProperty(&variant, pPropSetValue, resolvedObj.name, &spDbgProp))
            {
                pi[i].m_dwValidFields = 0;
                pi[i].m_pDebugProp = nullptr;
                delete pPropSetValue;
                hr = E_FAIL;
                goto Error;
            }

            spDbgProp->GetPropertyInfo(m_dwFieldSpec | DBGPROP_INFO_DEBUGPROP,
                m_nRadix,
                pi + i);       
        }

        (*pcEltsfetched)++;
        current++;
    }

Error:
    pWalkerWeakRef->ReleaseStrongReference();
    return hr;
}

HRESULT EnumDebugPropertyInfo::Next( 
    /* [in] */ ULONG celt,
    /* [out] */ DebugPropertyInfo *pi,
    /* [out] */ ULONG *pcEltsfetched)
{
    HRESULT hr = S_OK;

    if (!pi || !pcEltsfetched)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    if (!m_pApplicationThread)
    {
        hr = E_FAIL;
        goto Error;
    }

    if (S_OK == m_pApplicationThread->QueryIsCurrentThread())
    {
        hr = NextInternal(celt, pi, pcEltsfetched);
    }
    else
    {
        NextArgs args;
        args.celt = celt;
        args.pi = pi;
        args.pCeltsfetched = pcEltsfetched;

        hr = m_pApplicationThread->SynchronousCallIntoThread(
                static_cast<IDebugThreadCall*>(this),
                xthread_Next,
                (DWORD_PTR)&args,
                0);
    }

Error:
    return hr;
}

HRESULT EnumDebugPropertyInfo::Skip( 
    /* [in] */ ULONG celt)
{
    Js::IDiagObjectModelWalkerBase* pWalker = pWalkerWeakRef->GetStrongReference();
    if (!pWalker)
    {
        return E_FAIL;
    }

    current += celt;

    pWalkerWeakRef->ReleaseStrongReference();
    return S_OK;
}

HRESULT EnumDebugPropertyInfo::Reset( void)
{
    Js::IDiagObjectModelWalkerBase* pWalker = pWalkerWeakRef->GetStrongReference();
    if (!pWalker)
    {
        return E_FAIL;
    }

    current = 0;

    pWalkerWeakRef->ReleaseStrongReference();
    return S_OK;
}

HRESULT EnumDebugPropertyInfo::Clone( 
    /* [out] */ __RPC__deref_out_opt IEnumDebugPropertyInfo **ppepi)
{
    return E_NOTIMPL;
}

HRESULT EnumDebugPropertyInfo::GetCount( 
    /* [out] */ __RPC__out ULONG *pcelt)
{
    HRESULT hr = S_OK;

    if (!pcelt)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

    if (!m_pApplicationThread)
    {
        hr = E_FAIL;
        goto Error;
    }

    if (S_OK == m_pApplicationThread->QueryIsCurrentThread())
    {
        if (DebugHelper::IsScriptEngineClosed(m_pScriptEngine, &hr))
        {
            return hr;
        }

        Js::IDiagObjectModelWalkerBase* pWalker = pWalkerWeakRef->GetStrongReference();
        if (!pWalker)
        {
            return E_FAIL;
        }

        Js::ScriptContext* scriptContext = m_pScriptEngine->GetScriptSiteHolder()->GetScriptSiteContext();

        BEGIN_JS_RUNTIME_CALL_EX_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(scriptContext, true)
        {
            ENFORCE_ENTRYEXITRECORD_HASCALLER(scriptContext);
           *pcelt= pWalker->GetChildrenCount();
        }
        END_JS_RUNTIME_CALL_AND_TRANSLATE_EXCEPTION_AND_ERROROBJECT_TO_HRESULT(hr)
        
        pWalkerWeakRef->ReleaseStrongReference();
    }
    else
    {
        hr = m_pApplicationThread->SynchronousCallIntoThread(
                static_cast<IDebugThreadCall*>(this),
                xthread_GetCount,
                (DWORD_PTR)pcelt,
                0);
    }

Error:
    return hr;
}

HRESULT EnumDebugPropertyInfo::ThreadCallHandler(
    /* [in] */ DWORD_PTR dwParam1,
    /* [in] */ DWORD_PTR dwParam2,
    /* [in] */ DWORD_PTR dwParam3)
{
    return DebugApiWrapper( [=] {
        switch (dwParam1)
        {
        case xthread_GetCount:
            return GetCount((ULONG *)dwParam2);

        case xthread_Next:
            NextArgs* pArgs = (NextArgs*)dwParam2;
            return Next(pArgs->celt,pArgs->pi,pArgs->pCeltsfetched);
        }
        return E_INVALIDARG;
    });
}
