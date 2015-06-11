//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#pragma warning(disable:4100)

namespace DevTests 
{
    namespace Repros
    {
        namespace Performance
        {
            class RefClass :
                public Microsoft::WRL::RuntimeClass<__IRefClassPublicNonVirtuals,
                                                    IFoo,
                                                    IRef>
            {
                InspectableClass(L"DevTests.Repros.Performance.RefClass", BaseTrust);

                unsigned int retrievableStringsLength;
                HSTRING * retrievableStrings;


                void ClearRetrievableStrings()
                {
                    for(unsigned int i=0; i<retrievableStringsLength; ++i)
                    {
                        WindowsDeleteString(retrievableStrings[i]);
                    }
                    retrievableStringsLength = 0;
                    CoTaskMemFree(retrievableStrings);
                    retrievableStrings = nullptr;
                }

                HRESULT CopyIntoRetrievableStrings(unsigned int length, HSTRING * source)
                {
                    ClearRetrievableStrings();
                    retrievableStrings = (HSTRING*)CoTaskMemAlloc(sizeof(HSTRING) * length);
                    retrievableStringsLength = length;
                    for(unsigned int i = 0; i<length; ++i)
                    {
                        WindowsDuplicateString(source[i], &retrievableStrings[i]);
                    }
                    return S_OK;
                }

                HRESULT CopyFromRetrievableStrings(unsigned int * length, HSTRING ** destination)
                {
                    *destination = (HSTRING*)CoTaskMemAlloc(sizeof(HSTRING) * retrievableStringsLength);
                    *length = retrievableStringsLength;
                    for(unsigned int i = 0; i<retrievableStringsLength; ++i)
                    {
                        WindowsDuplicateString(retrievableStrings[i], &(*destination)[i]);
                    }
                    return S_OK;
                }

            public:
                RefClass() 
                {
                    
                    retrievableStrings = (HSTRING*)CoTaskMemAlloc(0);
                    retrievableStringsLength = 0;


                };
                ~RefClass() 
                { 
                    ClearRetrievableStrings();
                };

                HRESULT STDMETHODCALLTYPE Initialize(int,int,int*) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE Calc(int,int,int*) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE OneIntParamsFunc(int i1) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE arrayParamsFunc(UINT32 length, int * intArray) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE manyIntParamsFunc(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE noParamsFunc() override { return S_OK; }
                HRESULT STDMETHODCALLTYPE paramsFunc(int i1, int i2) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE refArrayParamsFunc(UINT32 length, __IRefClassPublicNonVirtuals ** classArray) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE refClassParamFunc(__IRefClassPublicNonVirtuals * refClass) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnArrayFunc(UINT32 *length, int **result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnArrayOfOneFunc(UINT32 *length, int **result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnIntFunc(int * result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnRefArrayFunc(UINT32 *length, __IRefClassPublicNonVirtuals *** result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnRefClassFunc(__IRefClassPublicNonVirtuals ** result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnStringArrayFunc(UINT32 *length, HSTRING **result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE returnStringFunc(HSTRING * result) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE stringArrayParamsFunc(UINT32 length, HSTRING * classArray) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE stringParamFunc(HSTRING str) override { return S_OK; }
                HRESULT STDMETHODCALLTYPE passStringWithDelegate( 
                    /* [in] */ __RPC__in HSTRING str,
                    /* [in] */ __RPC__in_opt DevTests::Repros::Performance::ISomeDelegate *callme) 
                {
                    callme->Invoke();
                    return S_OK;
                }

                HRESULT STDMETHODCALLTYPE passRetrievableStringArray( 
                    /* [in] */ UINT32 length,
                    /* [in][size_is] */ __RPC__in_ecount_full(length) HSTRING *stringArray)
                {
                    return CopyIntoRetrievableStrings(length, stringArray);
                }
                    
                HRESULT STDMETHODCALLTYPE retrievePassedStringArray( 
                    /* [out] */ __RPC__out UINT32 *length,
                    /* [out][retval][size_is][size_is] */ __RPC__deref_out_ecount_full_opt(*length) HSTRING **result)
                {
                    return CopyFromRetrievableStrings(length, result);
                }
            };
        }
    }
}