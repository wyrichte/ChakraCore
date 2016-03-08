/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "SCALookup.h"
#include "hostsysinfo.h"

HTYPE SCA::htypeImageData = NULL;
CComPtr<ITypeOperations> SCA::s_pImageDataTypeOperations;

HRESULT SCA::Initialize(IActiveScript* pActiveScript)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(pActiveScript));

    // Add SCA
    Var SCA;
    IfFailGo(pScriptDirect->CreateObject(&SCA));

    IfFailGo(pScriptDirect.AddMethod(SCA, _u("serialize"), &Serialize));
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("deserialize"), &Deserialize));
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("lookup"), &Lookup));
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("lookupEx"), &LookupEx));//With Dense Arrays
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("dataToKey"), &DataToKey));
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("makeInt64"), &MakeInt64));
    IfFailGo(pScriptDirect.AddMethod(SCA, _u("makeUint64"), &MakeUint64));

    Var global;
    IfFailGo(pScriptDirect->GetGlobalObject(&global));
    IfFailGo(pScriptDirect.AddProperty(global, _u("SCA"), SCA));

    // Add constructor ImageData
    PropertyId namePropertyId;
    Var imageData;
    IfFailGo(pScriptDirect->GetOrAddPropertyId(_u("ImageData"), &namePropertyId));
    IfFailGo(pScriptDirect->CreateConstructor(NULL, &ImageDataConstructor, namePropertyId, FALSE, &imageData));
    IfFailGo(pScriptDirect.SetProperty(global, namePropertyId, imageData));

Error:
    return hr;
}

//
// Get optional SCA options object "arg[2]" with the following optional properties:
//  context: a string for SCAContextType, e.g.: "persist", "crossthread".
//  target:  the global object for target script context.
// Also check for optional filename string.
//
HRESULT SCA::GetOptions(ScriptDirect& pScriptDirect, CallInfo callInfo, Var* args,
    ISCAContext** ppSCAContext, _Outptr_opt_result_maybenull_ BSTR* pFileName, unsigned int argIndex)
{
    HRESULT hr = S_OK;

    CComPtr<MockSCAContext> pSCAContext;
    SCAContextType context = SCAContext_Persist; // default
    CComPtr<IUnknown> pTarget;

    IfFailGo(ComObject<MockSCAContext>::CreateInstance(&pSCAContext));

    // Read optional options object
    if (callInfo.Count > argIndex)
    {
        Var options = args[argIndex];

        ScriptType scriptType;
        IfFailGo(pScriptDirect->GetScriptType(options, &scriptType));
        if (scriptType == ScriptType_Object)
        {
            argIndex++; // Yes, "options" object provided

            BOOL hasContext;
            IfFailGo(pScriptDirect.HasProperty(options, _u("context"), &hasContext));
            if (hasContext)
            {
                CComBSTR contextStr;
                IfFailGo(pScriptDirect.GetOwnProperty(options, _u("context"), &contextStr));
                if (_wcsicmp(contextStr, _u("samethread")) == 0)
                {
                    context = SCAContext_SameThread;
                }
                else if (_wcsicmp(contextStr, _u("crossthread")) == 0)
                {
                    context = SCAContext_CrossThread;
                }
                else if (_wcsicmp(contextStr, _u("crossprocess")) == 0)
                {
                    context = SCAContext_CrossProcess;
                }
            }

            BOOL hasTarget;
            IfFailGo(pScriptDirect.HasProperty(options, _u("target"), &hasTarget));
            if (hasTarget)
            {
                CComPtr<IActiveScriptDirect> pTargetScriptDirect;
                Var targetVar;
                IfFailGo(pScriptDirect.GetOwnProperty(options, _u("target"), &targetVar));
                IfFailGo(ScriptDirect::JsVarToScriptDirect(targetVar, &pTargetScriptDirect));
                IfFailGo(((IUnknown*)pTargetScriptDirect)->QueryInterface(&pTarget));
            }
        }
    }

    // Read optional filename string
    if (pFileName != NULL)
    {
        *pFileName = NULL;
        if (callInfo.Count > argIndex)
        {
            Var filename = args[argIndex];

            ScriptType scriptType;
            IfFailGo(pScriptDirect->GetScriptType(filename, &scriptType));
            if (scriptType == ScriptType_String)
            {
                IfFailGo(pScriptDirect->VarToString(filename, pFileName));
            }
        }
    }

    IfFailGo(pSCAContext->Init(context, pTarget));
    IfFailGo(((IUnknown*)pSCAContext)->QueryInterface(ppSCAContext));

Error:
    return hr;
}

//
// SCA.serialize(root, [options], [filename], [transferVars])
//  root:           The root object to serialize
//  options:        Optional options object
//  filename:       Optional output filename
//  transferVars:   The array (map) of objects which should be transferred instead of cloned.
//
Var SCA::Serialize(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    CComPtr<IStream> pStream;
    CComPtr<ISCAContext> pSCAContext;
    CComBSTR filename;

    IfFailGo(pScriptDirect.From(function));
    IfFailGo(GetOptions(pScriptDirect, callInfo, args, &pSCAContext, &filename));

    if (callInfo.Count >= 2)
    {
        Var obj = args[1];
        bool isSerializeToFile = filename.Length() > 0;

        if (isSerializeToFile)
        {
            AssertMsg(callInfo.Count < 5, "Transfer isn't supported with serialization to file.");
            if (HostSystemInfo::SupportsOnlyMultiThreadedCOM())
            {
                IfFailGo(SHCreateStreamOnFile(filename,
                    STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, &pStream));
            }
            else
            {
                IfFailGo(SHCreateStreamOnFileEx(filename,
                    STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &pStream));
            }

            IServiceProvider* pServiceProvider = NULL;
            IfFailGo(pScriptDirect->GetServiceProviderOfCaller(&pServiceProvider));
            hr = pScriptDirect->Serialize(pSCAContext, obj, nullptr, 0, pStream, pServiceProvider);
            pScriptDirect->ReleaseServiceProviderOfCaller(pServiceProvider);
            IfFailGo(hr);
        }
        else
        {
            BOOL boolResult = false;
            int arrayLength = 0;
            Var* transferVarsArray = nullptr;

            if (callInfo.Count >= 5)
            {
                Var transferVars = args[4];

                IfFailGo(pScriptDirect->IsArrayObject(transferVars, &boolResult));

                if (!boolResult)
                {
                    IfFailGo(E_INVALIDARG);
                }

                IfFailGo(pScriptDirect.GetProperty(transferVars, _u("length"), &arrayLength));

                if (arrayLength > 0)
                {
                    transferVarsArray = new Var[arrayLength];
                    if (transferVarsArray == nullptr)
                    {
                        IfFailGo(hr = E_OUTOFMEMORY);
                    }

                    int validArrayIndex = 0;
                    for (int i = 0; i < arrayLength; i++)
                    {
                        Var index = nullptr;
                        IfFailGo(pScriptDirect->IntToVar(i, &index));
                        Var value = nullptr;
                        JavascriptTypeId typeId;
                        IfFailGo(pScriptDirect.GetItem(transferVars, index, &value));
                        IfFailGo(pScriptDirect->GetTypeIdForVar(value, &typeId));                        
                        if (typeId != TypeIds_ArrayBuffer)
                        {
                            continue;
                        }
                        *(transferVarsArray + validArrayIndex) = value;
                        validArrayIndex++;
                    }
                    arrayLength = validArrayIndex;
                }
            }

            // Serialize to a mem stream
            IfFailGo(CreateMemStream(&pStream));

            IServiceProvider* pServiceProvider = NULL;
            IfFailGo(pScriptDirect->GetServiceProviderOfCaller(&pServiceProvider));
            hr = pScriptDirect->Serialize(pSCAContext, obj, transferVarsArray, arrayLength, pStream, pServiceProvider);
            pScriptDirect->ReleaseServiceProviderOfCaller(pServiceProvider);
            IfFailGo(hr);

            IUnknown *outDependentObject = nullptr;
            IfFailGo(pSCAContext->GetDependentObject(&outDependentObject));//AddRef's, we are aware of this

            UINT streamLen;
            {
                LARGE_INTEGER li = {0};
                ULARGE_INTEGER uli = {0};

                // Get stream length (only supports maximum MAXINT)
                IfFailGo(pStream->Seek(li, SEEK_CUR, &uli));
                LONG len = uli.LowPart;
                IfFailGo(LongToUInt(len, &streamLen));

                // Rewind stream to the beginning
                IfFailGo(pStream->Seek(li, SEEK_SET, /*plibNewPosition*/NULL));
            }

            // Return a Uint8Array containing the stream content
            Var uint8Array;
            BYTE* pb;
            UINT len;
            IfFailGo(pScriptDirect->CreateTypedArray(Uint8Array, /*sourceVar*/NULL, streamLen, &uint8Array));
            IfFailGo(pScriptDirect->GetTypedArrayBuffer(uint8Array, &pb, &len, NULL, NULL));
            Assert(len == streamLen);

            IfFailGo(pStream->Read(pb, len, /*pcbRead*/NULL));

            if (outDependentObject != nullptr)
            {
                Var pointer = nullptr;
                pScriptDirect->Int64ToVar((long long)outDependentObject, &pointer);
             
                //This isn't the best approach, but its test code
                IfFailGo(pScriptDirect.AddProperty(uint8Array, _u("__state__"), (Var)pointer));
            }
            return uint8Array;
        }
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}

//
// SCA.deserialize(data, [options])
//  data:           The data or filename to deserialize
//  options:        Optional options object
//
Var SCA::Deserialize(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    CComPtr<IStream> pStream;
    CComPtr<ISCAContext> pSCAContext;

    IfFailGo(pScriptDirect.From(function));
    IfFailGo(GetOptions(pScriptDirect, callInfo, args, &pSCAContext));

    if (callInfo.Count >= 2)
    {
        Var data = args[1];
        IfFailGo(GetInStream(pScriptDirect, data, &pStream));

        Var obj;
        IServiceProvider* pServiceProvider = NULL;
        IfFailGo(pScriptDirect->GetServiceProviderOfCaller(&pServiceProvider));

        BOOL boolResult = false;
        IfFailGo(pScriptDirect.HasProperty(data, _u("__state__"), &boolResult));

        if (boolResult)
        {
            Var pointerValue = nullptr;
            IfFailGo(pScriptDirect.GetProperty(data, _u("__state__"), &pointerValue));
            long long outInt64 = 0;
            pScriptDirect->VarToInt64(pointerValue, &outInt64);

            IUnknown *dependentObject = (IUnknown *)outInt64;
            pSCAContext->SetDependentObject(dependentObject);
            // One release for the pSCAContext->GetDependentObject's AddRef
            dependentObject->Release();
        }

        hr = pScriptDirect->Deserialize(pSCAContext, pStream, pServiceProvider, &obj);
        pScriptDirect->ReleaseServiceProviderOfCaller(pServiceProvider);
        IfFailGo(hr);

        return obj;
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}

//
// Get an input stream from data. Create a byte stream if data is an int array.
// Create an input file stream if data is a string for filename.
//
HRESULT SCA::GetInStream(ScriptDirect& pScriptDirect, Var data, IStream** ppStream)
{
    HRESULT hr = S_OK;
    CComPtr<IStream> pStream;

    BYTE* pb;
    UINT len;

    // If data is a Uint8Array, read its buffer directly (note this reads the whole ArrayBuffer)
    if (SUCCEEDED(pScriptDirect->GetTypedArrayBuffer(data, &pb, &len, NULL, NULL)))
    {
        IfFailGo(CreateMemStream(pb, len, &pStream));
    }
    else
    {
        ScriptType dataScriptType;
        IfFailGo(pScriptDirect->GetScriptType(data, &dataScriptType));

        // If data is a string, use it as input file name
        if (dataScriptType == ScriptType_String)
        {
            CComBSTR filename;
            IfFailGo(pScriptDirect->VarToString(data, &filename));
            if (HostSystemInfo::SupportsOnlyMultiThreadedCOM())
            {
                IfFailGo(SHCreateStreamOnFile(filename,
                    STGM_READ | STGM_SHARE_EXCLUSIVE, &pStream));
            }
            else
            {
                IfFailGo(SHCreateStreamOnFileEx(filename,
                    STGM_READ | STGM_SHARE_EXCLUSIVE, 0, FALSE, NULL, &pStream));
            }
        }
        else
        {
            IfFailGo(CreateMemStream(&pStream));

            // Transfer array-like input into stream
            {
                ByteStreamContainer byteStream(pStream);
                IfFailGo(pScriptDirect.ReadArray(data, &byteStream));
            }

            // Move stream back to beginning
            {
                LARGE_INTEGER offset = {0};
                IfFailGo(pStream->Seek(offset, SEEK_SET, /*plibNewPosition*/NULL));
            }
        }
    }

    IfFailGo(((IUnknown*)pStream)->QueryInterface(ppStream));

Error:
    return hr;
}

//
// Create an in-memory stream.
//
HRESULT SCA::CreateMemStream(_In_reads_bytes_opt_(len) const BYTE* pb, UINT len, IStream** ppStream)
{
    *ppStream = SHCreateMemStream(pb, len);
    return *ppStream != NULL ? S_OK : E_OUTOFMEMORY;
}

HRESULT SCA::CreateMemStream(IStream** ppStream)
{
    return CreateMemStream(NULL, 0, ppStream);
}

HRESULT SCA::VariantToVar(_In_ ScriptDirect& pScriptDirect, _In_ const VARIANT &value, _Out_ Var *outVar)
{
    HRESULT hr = S_OK;

    Assert(V_VT(&value) != (VT_ARRAY | VT_VARIANT));

    switch (V_VT(&value))
    {
    case VT_EMPTY:
        IfFailGo(pScriptDirect.StringToVar(_u("NotIndexable"), outVar));
        break;

    case VT_NULL:
        IfFailGo(pScriptDirect->GetNull(outVar));
        break;

    case VT_I4:
        IfFailGo(pScriptDirect->IntToVar(V_I4(&value), outVar));
        break;

    case VT_I8:
        IfFailGo(pScriptDirect->Int64ToVar(V_I8(&value), outVar));
        break;

    case VT_UI8:
        IfFailGo(pScriptDirect->UInt64ToVar(V_UI8(&value), outVar));
        break;

    case VT_R8:
        IfFailGo(pScriptDirect->DoubleToVar(V_R8(&value), outVar));
        break;

    case VT_DATE:
        IfFailGo(pScriptDirect.NewDate(V_DATE(&value), outVar));
        break;

    case VT_BSTR:
        {
            int len = static_cast<int>(::SysStringLen(V_BSTR(&value)));
            IfFailGo(pScriptDirect->StringToVar(V_BSTR(&value), len, outVar));
        }
        break;
    default:
        {
            const size_t bufsize = 256;
            static WCHAR buf[bufsize];

            IfFailGo(StringCchPrintf(buf, bufsize, _u("Unexpected VT: %d"), V_VT(&value)));
            IfFailGo(pScriptDirect.StringToVar(buf, outVar));
        }
        break;
    }

Error:
    return hr;
}

HRESULT SCA::VariantArrayToVar(_In_ ScriptDirect& pScriptDirect, _In_ const VARIANT &value, _Out_ Var *outVar)
{
    HRESULT hr = S_OK;

    Assert(V_VT(&value) == (VT_ARRAY | VT_VARIANT));

    Var jsArrayList = nullptr;
    int jsArrayListLength = 0;
    pScriptDirect->CreateArrayObject(0, &jsArrayList);
    list<SAFEARRAY*> safeArrayList;
    list<int> indexesList;
    
    unsigned int currentIndex = 0;
    Var currentVarArray = nullptr, indexVar = nullptr;
    CComSafeArray<VARIANT> currentSafeArray;
    currentSafeArray.Attach(value.parray);

    //We don't want to have a recusion here if we want to enable testing of very deeply nested arrays
LPushNewArray:
    IfFailGo(pScriptDirect->CreateArrayObject(currentSafeArray.GetCount(), &currentVarArray));
    IfFailGo(pScriptDirect.SetItem(jsArrayList, jsArrayListLength, currentVarArray));

    jsArrayListLength++;
    safeArrayList.push_back(currentSafeArray);
    indexesList.push_back(0);

LItemsLoop:
    currentIndex = indexesList.back();
    indexesList.pop_back();//We will need to push it back on below if we need to save the value
    
    while (currentIndex < currentSafeArray.GetCount())
    {
        const VARIANT& currentVARIANT = currentSafeArray.GetAt(currentIndex);
        currentIndex++;
        if ((V_VT(&currentVARIANT) == (VT_ARRAY | VT_VARIANT)))
        {
            indexesList.push_back(currentIndex);
            currentSafeArray.Detach();
            currentSafeArray.Attach(currentVARIANT.parray);
            goto LPushNewArray;
        }
        else
        {
            Var var = nullptr;
            IfFailGo(VariantToVar(pScriptDirect, currentVARIANT, &var));
            IfFailGo(pScriptDirect->IntToVar(currentIndex - 1, &indexVar));
            IfFailGo(pScriptDirect.SetItem(currentVarArray, indexVar, var));
        }
    }
    if (indexesList.size() > 0)
    {
        //Current are still in the lists
        //Soft "pop"
        jsArrayListLength--;
        safeArrayList.pop_back();

        //Here we need to set the current JS array on parent JS array
        IfFailGo(pScriptDirect->IntToVar(indexesList.back() - 1 /* previous index (post incrementation)*/, &indexVar));
        Var lastItem = nullptr;
        IfFailGo(pScriptDirect.GetItem(jsArrayList, jsArrayListLength - 1 /* last after soft "pop", not the one we popped */, &lastItem));
        IfFailGo(pScriptDirect.SetItem(lastItem, indexVar, currentVarArray));

        //We can detach from the current safe array
        currentSafeArray.Detach();
        
        //Reset the state
        currentVarArray = lastItem;
        currentSafeArray.Attach(safeArrayList.back());

        goto LItemsLoop;
    }
    else
    {
        *outVar = currentVarArray;
    }

Error:
    if (currentSafeArray != nullptr)
    {
        currentSafeArray.Detach();
    }
    return hr;
}

template <class APIFuncType>
Var SCA::LookupHelper(_In_ APIFuncType ReadIndexableProperty /* In order to avoid updating baseline of a UT that captures call to apiFunc() */, _In_ Var function, _In_ CallInfo callInfo, __in_xcount(callInfo.Count) Var* args)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(function));

    if (callInfo.Count >= 3)
    {
        Var data = args[1];
        CComPtr<IStream> pStream;
        IfFailGo(GetInStream(pScriptDirect, data, &pStream));

        Var names = args[2];
        BSTRContainer bstrs;
        IfFailGo(pScriptDirect.ReadArray(names, &bstrs));

        CComVariant value;
        bool canPropertyBeAdded = false;
        IfFailGo(ReadIndexableProperty(pStream, bstrs.Count(), bstrs.Items(), &value, &canPropertyBeAdded));

        Var var;

        if (hr == S_FALSE)
        {
            IfFailGo(pScriptDirect.StringToVar(canPropertyBeAdded ? _u("NoProperty, CanAdd") : _u("NoProperty"), &var));
            return var;
        }

        auto fn = (V_VT(&value) == (VT_ARRAY | VT_VARIANT)) ? VariantArrayToVar : VariantToVar;

        IfFailGo(fn(pScriptDirect, value, &var));

        return var;
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}

//
// Lookup tests ReadIndexablePropertyEx. Return the result value, or a string to represent
// lookup results: NoProperty, NotIndexable, or Unexpected VT
//

Var SCA::LookupEx(_In_ Var function, _In_ CallInfo callInfo, Var* args)
{
    return LookupHelper(ReadIndexablePropertyEx, function, callInfo, args);
}


//
// Lookup tests ReadIndexableProperty. Return the result value, or a string to represent
// lookup results: NoProperty, NotIndexable, or Unexpected VT
//
Var SCA::Lookup(_In_ Var function, _In_ CallInfo callInfo, Var* args)
{
    return LookupHelper(ReadIndexableProperty, function, callInfo, args);
}

//
// DataToKey tests KeyFromStreamToVariant. Return the result value, or undefined if not a valid key.
//
Var SCA::DataToKey(_In_ Var function, _In_ CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;
    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(function));

    if (callInfo.Count >= 2)
    {
        Var data = args[1];
        CComPtr<IStream> pStream;
        IfFailGo(GetInStream(pScriptDirect, data, &pStream));

        CComVariant value;
        IfFailGo(KeyFromStreamToVariant(pStream, &value));

        if (value.vt == VT_EMPTY || value.vt == VT_NULL)
        {
            return pScriptDirect.GetUndefined();
        }
        
        Var var;
        auto fn = (V_VT(&value) == (VT_ARRAY | VT_VARIANT)) ? VariantArrayToVar : VariantToVar;

        IfFailGo(fn(pScriptDirect, value, &var));

        return var;
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}


template<class Func>
Var MakeInt64Common(Var function, CallInfo callInfo, Var args[], Func newVar)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(function));

    if (callInfo.Count >= 3)
    {
        int low, high;
        IfFailGo(pScriptDirect->VarToInt(args[1], &low));
        IfFailGo(pScriptDirect->VarToInt(args[2], &high));

        UINT64 value = (UINT64)((DWORD)low) | ((UINT64)((DWORD)high)) << 32;
        Var var;
        IfFailGo(newVar(pScriptDirect, value, &var));
        return var;
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}

Var SCA::MakeInt64(Var function, CallInfo callInfo, Var* args)
{
    return MakeInt64Common(function, callInfo, args,
        [](ScriptDirect& pScriptDirect, UINT64 value, Var* pVar) {
            return pScriptDirect->Int64ToVar((INT64)value, pVar);
    });
}

Var SCA::MakeUint64(Var function, CallInfo callInfo, Var* args)
{
    return MakeInt64Common(function, callInfo, args,
        [](ScriptDirect& pScriptDirect, UINT64 value, Var* pVar) {
            return pScriptDirect->UInt64ToVar(value, pVar);
    });
}

Var SCA::ImageDataConstructor(Var function, CallInfo callInfo, Var* args)
{
    HRESULT hr = S_OK;

    ScriptDirect pScriptDirect;
    IfFailGo(pScriptDirect.From(function));

    if (callInfo.Count >= 2)
    {
        Var options = args[1];

        Var instance;
        CComPtr<MockImageData> pImageData;
        IfFailGo(CreateImageDataObject(pScriptDirect, &instance, &pImageData));
        IfFailGo(pImageData->InitFrom(options));

        return instance;
    }

Error:
    pScriptDirect.ThrowIfFailed(hr);
    return pScriptDirect.GetUndefined();
}

HRESULT SCA::CreateImageDataObject(ScriptDirect& pScriptDirect, Var* value, MockImageData** ppImageData)
{
    HRESULT hr = S_OK;

    CComPtr<MockImageData> pImageData;
    IfFailGo(ComObject<MockImageData>::CreateInstance(&pImageData));

    Var instance;
    IfFailGo(EnsureImageDataType(pScriptDirect));
    IfFailGo(pScriptDirect->CreateTypedObject(htypeImageData, sizeof(IUnknown*), FALSE, &instance));

    IfFailGo(pImageData->Init(instance));
    IfFailGo(MockTypeOperations::SetInstanceUnknown(instance, static_cast<ISCASerializable*>(pImageData)));

    *value = instance;
    if (ppImageData)
    {
        *ppImageData = pImageData;
        static_cast<ISCASerializable*>(pImageData)->AddRef();
    }

Error:
    return hr;
}

HRESULT SCA::EnsureImageDataType(ScriptDirect& pScriptDirect)
{
    HRESULT hr = S_OK;

    if (!htypeImageData)
    {
        CComPtr<MockTypeOperations> pTypeOperations;
        IfFailGo(ComObject<MockTypeOperations>::CreateInstance(&pTypeOperations));

        // ScriptEngine does not AddRef on operations. Keep a reference myself.
        IfFailGo(((IUnknown*)pTypeOperations)->QueryInterface(&s_pImageDataTypeOperations));

        JavascriptTypeId typeId;
        PropertyId nameId;
        IfFailGo(pScriptDirect->ReserveTypeIds(1, &typeId));
        IfFailGo(pScriptDirect->GetOrAddPropertyId(_u("ImageData"), &nameId));
        IfFailGo(pScriptDirect->CreateType(typeId, NULL, 0, NULL, NULL, pTypeOperations, FALSE, nameId, TRUE, &htypeImageData));
    }

Error:
    return hr;
}

//
// Add a new byte value into the stream.
//
HRESULT ByteStreamContainer::Add(ScriptDirect& pScriptDirect, Var value)
{
    HRESULT hr = S_OK;

    int byte;
    IfFailGo(pScriptDirect->VarToInt(value, &byte));
    IfFailGo(m_pStream->Write(&byte, 1, NULL));
Error:
    return hr;
}

BSTRContainer::~BSTRContainer()
{
    // Releases all contained BSTRs
    std::for_each(m_bstrArr.begin(), m_bstrArr.end(), &::SysFreeString);
}

//
// Add a new JavaScript string.
//
HRESULT BSTRContainer::Add(ScriptDirect& pScriptDirect, Var value)
{
    HRESULT hr = S_OK;

    BSTR bstr;
    IfFailGo(pScriptDirect->VarToString(value, &bstr));

    try
    {
        m_bstrArr.push_back(bstr);
    }
    catch(const std::bad_alloc&)
    {
        hr = E_OUTOFMEMORY;
        ::SysFreeString(bstr);
    }

Error:
    return hr;
}
