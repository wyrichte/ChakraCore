// Copyright (C) Microsoft. All rights reserved. 
// Tests to verify typed APIs exposed to hosts

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"
#include "ieisos.h"

int ClampInteger(int val, int minVal, int maxVal)
{
    if (val > maxVal)
    {
        return maxVal;
    }
    if (val < minVal)
    {
        return minVal;
    }
    return val;
}

HRESULT TestOneTypedArray(IActiveScriptDirect* activeScriptDirect, Var typedArray, uint sizeData, TypedArrayType arrayType)
{
    CComPtr<ITypeOperations> defaultTypeOperations;
    HRESULT hr;
    IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultTypeOperations));
    Var index;
    BOOL wasSet;
    int maxItemCount = sizeData > 0x10000 ? 0x10000 : sizeData;
    for (int i = 0; i < maxItemCount  ; i++)
    {
        IfFailedReturn(activeScriptDirect->IntToVar(i, &index));
        hr = defaultTypeOperations->SetItem(activeScriptDirect, typedArray, index, index, &wasSet);
        if (FAILED(hr) || !wasSet)
        {
            return E_FAIL;
        }
    }
    Var value;
    BOOL wasPresented;
    for (int i = 0; i < maxItemCount; i++)
    {
        IfFailedReturn(activeScriptDirect->IntToVar(i, &index));
        hr = defaultTypeOperations->GetOwnItem(activeScriptDirect, typedArray, index, &value, &wasPresented);
        if (FAILED(hr) || !wasPresented)
        {
            return E_FAIL;
        }
        int outValue;
        hr = activeScriptDirect->VarToInt(value, &outValue);
        if (FAILED(hr))
        {
            return hr;
        }
        if (outValue != i)
        {
            switch(arrayType)
            {
                case Int8Array:
                    if ((char)outValue == (char)i)
                        continue;
                    break;
                case Uint8Array:
                    if ((byte)outValue == (byte)i)
                        continue;
                    break;
                case Uint8ClampedArray:
                    if ((byte)outValue == (byte)ClampInteger(i, 0, 0xff))
                        continue;
                    break;
                case Int16Array:
                    if ((short)outValue == (short)i)
                        continue;
                    break;
                case Uint16Array:
                    if ((unsigned short)outValue == (unsigned short)i)
                        continue;
                    break;
                case Int32Array:
                    if (outValue == i)
                        continue;
                    break;
                case Uint32Array:
                    if ((uint)outValue == (uint)i)
                        continue;
                    break;
                case Float32Array:
                case Float64Array:
                    {
                        double dbVal;
                        hr = activeScriptDirect->VarToDouble(value, &dbVal);
                        if (FAILED(hr))
                        {
                            return hr;
                        }
                        if (dbVal == i)
                            continue;
                    }
            }
            return E_FAIL;
        }
    }
    return NOERROR;
}



template<int count>
HRESULT TestTypedArray(
    IActiveScriptDirect* activeScriptDirect,
    MyScriptDirectTests* myTests, 
    TypedArrayType* arrayTypes, 
    int countOfArrayTypes, 
    uint* ElementSize, 
    Var typedArrays[][count],
    Var typedArrays2[][count],
    uint dataSize[count]
    )
{
    HRESULT hr = NOERROR;
    Var arrayBuffer[count];

    StartTest(myTests, "Test TypedArray functionality...");

    for (int i = 0; i < countOfArrayTypes; i++)
    {
        for (int j = 0; j < count; j++)
        {
            printf("testing testArrays type %d, size %d\n", arrayTypes[i],dataSize[j]);

            IfFailGoto(activeScriptDirect->CreateTypedArray(arrayTypes[i], NULL, dataSize[j], &typedArrays[i][j]), LReturn);
            IfFailGoto(TestOneTypedArray(activeScriptDirect, typedArrays[i][j], dataSize[j], arrayTypes[i]), LReturn);
        }
    }

    for (uint i = 0; i < count; i++)
    {
        IfFailGoto(activeScriptDirect->CreateArrayBuffer(dataSize[i], &(arrayBuffer[i])), LReturn);
    }

    for (int i = 0; i < countOfArrayTypes; i++)
    {
        for (int j = 0; j < count; j++)
        {
            printf("testing testArrays type %d, size %d\n", arrayTypes[i],dataSize[j]);
            hr = activeScriptDirect->CreateTypedArray(arrayTypes[i], arrayBuffer[j], 0, &typedArrays2[i][j]);
            if (FAILED(hr))
            {
                if (dataSize[j] > ElementSize[i])
                {
                    goto LReturn;
                }
                else
                {
                    continue;
                }
            }

            IfFailGoto(TestOneTypedArray(activeScriptDirect, typedArrays2[i][j], dataSize[j]/ElementSize[i], arrayTypes[i]), LReturn);
        }
    }

    BYTE* outBufferSrc = NULL, *outBufferDst = NULL;
    UINT bufferLengthSrc, bufferLengthDst;
    TypedArrayType typedArrayTypeSrc, typedArrayTypeDst;
    INT elementSizeSrc, elementSizeDst;
    for (int i = 0; i < countOfArrayTypes; i++)
    {
        for (uint j = 0; j < count; j++)
        {
            printf("test GetArrayBuffer %d %d\n", arrayTypes[i],dataSize[j]);
            fflush(stdout);

            IfFailGoto(activeScriptDirect->GetTypedArrayBuffer(arrayBuffer[j], &outBufferSrc, &bufferLengthSrc, &typedArrayTypeSrc, &elementSizeSrc), LReturn);

            if (SUCCEEDED(hr))
            {
                hr = activeScriptDirect->GetTypedArrayBuffer(typedArrays2[i][j], &outBufferDst, &bufferLengthDst, &typedArrayTypeDst, &elementSizeDst);
                if (SUCCEEDED(hr))
                {
                    if (outBufferSrc != outBufferDst || 
                        bufferLengthSrc != bufferLengthDst )
                    {
                        hr = E_FAIL;
                    }
                }
                else
                {
                    if (dataSize[j] > ElementSize[i])
                    {
                        goto LReturn;
                    }
                    else
                    {
                        continue;
                    }
                }

            }
        }
    }

LReturn:
    FinishTest(myTests, "Done", hr);
    return hr;
}

template<int count>
HRESULT TestES5TypedArrays(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests, uint dataSize[count])
{
    TypedArrayType arrayTypes[] = {
        Int8Array,
        Uint8Array,
        Int16Array,
        Uint16Array,
        Int32Array,
        Uint32Array,
        Float32Array,
        Float64Array
    };
    uint ElementSize[] = {
        1,
        1,
        2,
        2,
        4,
        4,
        4,
        8
    };
    
    

    Var typedArrays[MYCOUNTOF(arrayTypes)][count];
    Var typedArrays2[MYCOUNTOF(arrayTypes)][count];
    memset(typedArrays, 0, sizeof(typedArrays));
    memset(typedArrays2, 0, sizeof(typedArrays2));

    return TestTypedArray<count>(activeScriptDirect, myTests, arrayTypes, MYCOUNTOF(arrayTypes), ElementSize, typedArrays, typedArrays2, dataSize);
}

template<int count>
HRESULT TestES6TypedArrays(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests, uint dataSize[count])
{
    TypedArrayType arrayTypes[] = {
        Int8Array,
        Uint8Array,
        Uint8ClampedArray,
        Int16Array,
        Uint16Array,
        Int32Array,
        Uint32Array,
        Float32Array,
        Float64Array
    };
    uint ElementSize[] = {
        1,
        1,
        1,
        2,
        2,
        4,
        4,
        4,
        8
    };
    
    Var typedArrays[MYCOUNTOF(arrayTypes)][count];
    Var typedArrays2[MYCOUNTOF(arrayTypes)][count];
    memset(typedArrays, 0, sizeof(typedArrays));
    memset(typedArrays2, 0, sizeof(typedArrays2));

    HRESULT hr = TestTypedArray<count>(activeScriptDirect, myTests, arrayTypes, MYCOUNTOF(arrayTypes), ElementSize, typedArrays, typedArrays2, dataSize);
    IfFailedReturn(hr);

    // Verify detachtypedarray buffer
    printf("Test DetachTypedArrayBuffer functionality...\n");
    BYTE* buffer;
    UINT byteLength;
    TypedArrayType arrayType;
    TypedArrayBufferAllocationType allocationType;
    hr = activeScriptDirect->DetachTypedArrayBuffer(typedArrays[0][0], &buffer, &byteLength, &allocationType, &arrayType, /*elementLength*/ nullptr);

    return hr;
}

HRESULT TestArrayErrors(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests)
{
    StartTest(myTests, "Test TypedArray Error cases...");

    HRESULT hr = S_OK;
    Var var;
    const UINT TOO_BIG_LENGTH = 0x80000000;

    Print("CreatePixelArray is expected to fail with length 3");
    // This triggers a TypeError
    if (activeScriptDirect->CreatePixelArray(3, &var) == NOERROR)
    {
        hr = E_FAIL;
        goto LReturn;
    }

    Print("CreatePixelArray is expected to fail with large length");
    // This triggers a TypeError
    if (activeScriptDirect->CreatePixelArray(TOO_BIG_LENGTH, &var) == NOERROR)
    {
        hr = E_FAIL;
        goto LReturn;
    }

    Print("CreateArrayBuffer is expected to fail with large length");
    // This triggers a TypeError
    if (activeScriptDirect->CreateArrayBuffer(TOO_BIG_LENGTH, &var) == NOERROR)
    {
        hr = E_FAIL;
    }

LReturn:
    FinishTest(myTests, "Done", hr);
    return hr;
}

HRESULT TestGetTypedArrayBuffer(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests, const LPWSTR* getBufferTests, int countOfGetBufferTests)
{
    StartTest(myTests, "Test GetTypedArrayBuffer functionality...");

    HRESULT hr = S_OK;
    Var var, func;
    for (int i = 0; i < countOfGetBufferTests && SUCCEEDED(hr); i++)
    {
        hr = activeScriptDirect->Parse(getBufferTests[i], &func);
        if (SUCCEEDED(hr))
        {
            CallInfo callInfo = {0, CallFlags_None};
            hr = activeScriptDirect->Execute(func, callInfo, NULL, NULL, &var);
            if (SUCCEEDED(hr))
            {
                byte* buffer;
                UINT length;
                TypedArrayType typedArrayType;
                INT elementSize;
                hr = activeScriptDirect->GetTypedArrayBuffer(var, &buffer, &length, &typedArrayType, &elementSize);
                if (SUCCEEDED(hr) && (buffer != NULL))
                {
                    printf("testcase %d, buffer length is %d, arrayType %d, elementSize %d\n", i, length, typedArrayType, elementSize);
                    for (UINT j = 0; j < length; j++)
                    {
                        printf("%d, ", buffer[j]);
                    }
                    printf("\n");
                }
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        Print("GetTypedArrayBuffer test succeeded");
    }
    else
    {
        Print("GetTypedArrayBuffer failed");
    }

    FinishTest(myTests, "Done", hr);
    return hr;
}

HRESULT TestGetES5TypedArrayBuffer(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests)
{
    LPWSTR const getBufferTests[] = {
        L"(function() {var testArray = new ArrayBuffer(40); return testArray;})(); ",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int8Array(testArray, 4, 8); myresult[0] = 20; return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint8Array(testArray, 4, 8); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int16Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint16Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int32Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint32Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new Float32Array(testArray, 8, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new Float64Array(testArray, 8, 2); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new DataView(testArray);return myresult;})();",
    };

    return TestGetTypedArrayBuffer(activeScriptDirect, myTests, getBufferTests, MYCOUNTOF(getBufferTests));
}

HRESULT TestGetES6TypedArrayBuffer(IActiveScriptDirect* activeScriptDirect, MyScriptDirectTests* myTests)
{
    LPWSTR const getBufferTests[] = {
        L"(function() {var testArray = new ArrayBuffer(40); return testArray;})(); ",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int8Array(testArray, 4, 8); myresult[0] = 20; return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint8Array(testArray, 4, 8); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint8ClampedArray(testArray, 4, 8); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int16Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint16Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Int32Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(40); var myresult = new Uint32Array(testArray, 4, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new Float32Array(testArray, 8, 4); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new Float64Array(testArray, 8, 2); myresult[0] = 20;return myresult;})();",
        L"(function() {var testArray = new ArrayBuffer(80); var myresult = new DataView(testArray);return myresult;})();",
    };

    return TestGetTypedArrayBuffer(activeScriptDirect, myTests, getBufferTests, MYCOUNTOF(getBufferTests));
}

void RunTypedArrayTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        if (IsOs_OneCoreUAP())
        {
            // Due to reduced memory available on Windows Phone, this test OOMs when array size = 0x1000000 (16.7M).
            // When this test is built for Windows Phone, we need to omit the largest array size.
            uint dataSize[] = { 0, 1, 4, 16, 32, 100000 };
            TestES5TypedArrays<MYCOUNTOF(dataSize)>(mytest->GetScriptDirectNoRef(), mytest, dataSize);
        }
        else
        {
            uint dataSize[] = { 0, 1, 4, 16, 32, 100000, 0x1000000 };
            TestES5TypedArrays<MYCOUNTOF(dataSize)>(mytest->GetScriptDirectNoRef(), mytest, dataSize);
        }

        TestArrayErrors(mytest->GetScriptDirectNoRef(), mytest);
        TestGetES5TypedArrayBuffer(mytest->GetScriptDirectNoRef(), mytest);
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
}

void RunTypedArrayES6Tests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        if (IsOs_OneCoreUAP())
        {
            // Due to reduced memory available on Windows Phone, this test OOMs when array size = 0x1000000 (16.7M).
            // When this test is built for Windows Phone, we need to omit the largest array size.
            uint dataSize[] = { 0, 1, 4, 16, 32, 100000 };
            TestES6TypedArrays<MYCOUNTOF(dataSize)>(mytest->GetScriptDirectNoRef(), mytest, dataSize);
        }
        else
        {
            uint dataSize[] = { 0, 1, 4, 16, 32, 100000, 0x1000000 };
            TestES6TypedArrays<MYCOUNTOF(dataSize)>(mytest->GetScriptDirectNoRef(), mytest, dataSize);
        }
        TestGetES6TypedArrayBuffer(mytest->GetScriptDirectNoRef(), mytest);
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
}