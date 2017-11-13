//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "AnimalServer.h"


using namespace Microsoft::WRL;
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;

//Methods with Multiple [out] parameters (all basic types)
IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutBool(t_Bool a, t_Bool b, __out t_Bool* reta, __out t_Bool* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}	 

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutUInt8(t_UInt8 a, t_UInt8 b, __out t_UInt8* reta, __out t_UInt8* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutInt32(t_Int32 a, t_Int32 b, __out t_Int32* reta, __out t_Int32* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutUInt32(t_UInt32 a, t_UInt32 b, __out t_UInt32* reta, __out t_UInt32* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutInt64(t_Int64 a, t_Int64 b, __out t_Int64* reta, __out t_Int64* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutUInt64(t_UInt64 a, t_UInt64 b, __out t_UInt64* reta, __out t_UInt64* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutSingle(t_Single a, t_Single b, __out t_Single* reta, __out t_Single* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutDouble(t_Double a, t_Double b, __out t_Double* reta, __out t_Double* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutChar16(t_Char16 a, t_Char16 b, __out t_Char16* reta, __out t_Char16* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutHSTRING(HSTRING a, HSTRING b, __out HSTRING* reta, __out HSTRING* retb)
{
    WindowsDuplicateString(a,reta);
    WindowsDuplicateString(b,retb);
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutPhylum(Phylum a, Phylum b, __out Phylum* reta, __out Phylum* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutDimensions(Dimensions a, Dimensions b, __out Dimensions* reta, __out Dimensions* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutIFish(IFish* a, IFish* b, __out IFish** reta, __out IFish** retb)
{
    if (nullptr == a)
    {
        *reta = nullptr;
    }
    else
    {
        *reta = a;
        (*reta)->AddRef();
    }

    if (nullptr == b)
    {
        *retb = nullptr;
    }
    else
    {
        *retb = b;
        (*retb)->AddRef();
    }

    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultipleOutFish(IFish* a, IFish* b, __out IFish** reta, __out IFish** retb)
{
    if (nullptr == a)
    {
        *reta = nullptr;
    }
    else
    {
        *reta = a;
        (*reta)->AddRef();
    }

    if (nullptr == b)
    {
        *retb = nullptr;
    }
    else
    {
        *retb = b;
        (*retb)->AddRef();
    }

    return S_OK;
}   

//Methods with interspersed [in] and [out] parameters (all basic types)
IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutBool(__in t_Bool a, __out t_Bool* reta, __in t_Bool b, __out t_Bool* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutUInt8(__in t_UInt8 a, __out t_UInt8* reta, __in t_UInt8 b, __out t_UInt8* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutInt32(__in t_Int32 a, __out t_Int32* reta, __in t_Int32 b, __out t_Int32* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutUInt32(__in t_UInt32 a, __out t_UInt32* reta, __in t_UInt32 b, __out t_UInt32* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutInt64(__in t_Int64 a, __out t_Int64* reta, __in t_Int64 b, __out t_Int64* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutUInt64(__in t_UInt64 a, __out t_UInt64* reta, __in t_UInt64 b, __out t_UInt64* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutSingle(__in t_Single a, __out t_Single* reta, __in t_Single b, __out t_Single* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutDouble(__in t_Double a, __out t_Double* reta, __in t_Double b, __out t_Double* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutChar16(__in t_Char16 a, __out t_Char16* reta, __in t_Char16 b, __out t_Char16* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutHSTRING(HSTRING a, HSTRING b, __out HSTRING* reta, __out HSTRING* retb)
{
    WindowsDuplicateString(a,reta);
    WindowsDuplicateString(b,retb);
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutPhylum(Phylum a, __out Phylum* reta, Phylum b, __out Phylum* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutDimensions(Dimensions a, __out Dimensions* reta, Dimensions b, __out Dimensions* retb)
{
    *reta = a;
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutIFish(IFish* a, __out IFish** reta, IFish* b, __out IFish** retb)
{
    if (nullptr == a)
    {
        *reta = nullptr;
    }
    else
    {
        *reta = a;
        (*reta)->AddRef();
    }

    if (nullptr == b)
    {
        *retb = nullptr;
    }
    else
    {
        *retb = b;
        (*retb)->AddRef();
    }

    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::InterspersedInOutFish(IFish* a, __out IFish** reta, IFish* b, __out IFish** retb)
{
    if (nullptr == a)
    {
        *reta = nullptr;
    }
    else
    {
        *reta = a;
        (*reta)->AddRef();
    }

    if (nullptr == b)
    {
        *retb = nullptr;
    }
    else
    {
        *retb = b;
        (*retb)->AddRef();
    }

    return S_OK;
}   

//Method to ensure layout is correct for with multiple or different alignment members
IFACEMETHODIMP
    Animals::AnimalServer::LayoutOfManyMembers(t_UInt8 a, t_Int32 b, t_UInt8 c, t_Double d, t_UInt8 e, t_UInt8 f, t_Double g, t_Int32 h,  t_Double i, 
    __out t_UInt8* reta, __out t_Int32* retb, __out t_UInt8* retc, __out t_Double* retd, __out t_UInt8* rete, __out t_UInt8* retf, __out t_Double* retg, __out t_Int32* reth,  __out t_Double* reti)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    *retd = d;
    *rete = e;
    *retf = f;
    *retg = g;
    *reth = h;
    *reti = i;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::LayoutStructs(InnerStruct a, Dimensions b, OuterStruct c, Names d, PhylumChange e, __out InnerStruct* reta, __out Dimensions* retb, __out OuterStruct* retc, __out Names* retd, __out PhylumChange* rete)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    *retd = d;
    *rete = e;
    WindowsDuplicateString(d.Common,&retd->Common);
    WindowsDuplicateString(d.Scientific,&retd->Scientific);
    WindowsDuplicateString(d.AlsoKnownAs,&retd->AlsoKnownAs);

    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::LayoutBasicWithStructs(t_UInt8 a, InnerStruct b, t_Int32 c, t_Double d, Names e, t_UInt8 f, t_UInt8 g, Dimensions h, t_Int32 i, 
    __out t_UInt8* reta, __out InnerStruct* retb, __out t_Int32* retc, __out t_Double* retd, __out Names* rete, __out t_UInt8* retf, __out t_UInt8* retg, __out Dimensions* reth, __out t_Int32* reti)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    *retd = d;
    *rete = e;
    *retf = f;
    *retg = g;
    *reth = h;
    *reti = i;
    WindowsDuplicateString(e.Common,&rete->Common);
    WindowsDuplicateString(e.Scientific,&rete->Scientific);
    WindowsDuplicateString(e.AlsoKnownAs,&rete->AlsoKnownAs);

    return S_OK;
}

//Methods with multiple float/double parameters
IFACEMETHODIMP
    Animals::AnimalServer::MultiFloat3(t_Single a, t_Single b, t_Single c, __out t_Single* reta, __out t_Single* retb, __out t_Single* retc)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultiFloat4(t_Single a, t_Single b, t_Single c, t_Single d, __out t_Single* reta, __out t_Single* retb, __out t_Single* retc, __out t_Single* retd)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    *retd = d; 
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultiDouble3(t_Double a, t_Double b, t_Double c, __out t_Double* reta, __out t_Double* retb, __out t_Double* retc)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::MultiDouble4(t_Double a, t_Double b, t_Double c, t_Double d, __out t_Double* reta, __out t_Double* retb, __out t_Double* retc, __out t_Double* retd)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    *retd = d; 
    return S_OK;
}

//Methods with float/double parameters at different offsets
IFACEMETHODIMP
    Animals::AnimalServer::FloatOffsetChar(t_Char16 a, t_Single b, __out t_Char16* reta, __out t_Single* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::FloatOffsetByte(t_UInt8 a, t_Single b, __out t_UInt8* reta, __out t_Single* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::FloatOffsetInt(t_Int32 a, t_Single b, __out t_Int32* reta, __out t_Single* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::FloatOffsetInt64(t_Int64 a, t_Single b, __out t_Int64* reta, __out t_Single* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::FloatOffset2Int(t_Int32 a, t_Int32 b, t_Single c, __out t_Int32* reta, __out t_Int32* retb, __out t_Single* retc)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::FloatOffsetStruct(Names a, t_Single b, __out Names* reta, __out t_Single* retb)
{
    *reta = a; 
    *retb = b;
    WindowsDuplicateString(a.Common,&reta->Common);
    WindowsDuplicateString(a.Scientific,&reta->Scientific);
    WindowsDuplicateString(a.AlsoKnownAs,&reta->AlsoKnownAs);
    return S_OK;
}


IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffsetChar(t_Char16 a, t_Double b, __out t_Char16* reta, __out t_Double* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffsetByte(t_UInt8 a, t_Double b, __out t_UInt8* reta, __out t_Double* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffsetInt(t_Int32 a, t_Double b, __out t_Int32* reta, __out t_Double* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffsetInt64(t_Int64 a, t_Double b, __out t_Int64* reta, __out t_Double* retb)
{
    *reta = a; 
    *retb = b;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffset2Int(t_Int32 a, t_Int32 b, t_Double c, __out t_Int32* reta, __out t_Int32* retb, __out t_Double* retc)
{
    *reta = a; 
    *retb = b;
    *retc = c;
    return S_OK;
}

IFACEMETHODIMP
    Animals::AnimalServer::DoubleOffsetStruct(Names a, t_Double b, __out Names* reta, __out t_Double* retb)
{
    *reta = a; 
    *retb = b;
    WindowsDuplicateString(a.Common,&reta->Common);
    WindowsDuplicateString(a.Scientific,&reta->Scientific);
    WindowsDuplicateString(a.AlsoKnownAs,&reta->AlsoKnownAs);

    return S_OK;
}

//Method to return given int as HRESULT (for error testing)
IFACEMETHODIMP
    Animals::AnimalServer::TestError(HRESULT hr)
{
    return hr;
}

template <class typeName>
HRESULT Animals::AnimalServer::GetIReferenceValue(Windows::Foundation::IReference<typeName>* inReference, Windows::Foundation::PropertyType inPropertyType, __out typeName* outValue)
{
    if (nullptr == inReference || nullptr == outValue)
    {
        return E_INVALIDARG;
    }
    HRESULT hr;
    ComPtr<IReference<typeName>> referenceT;
    ComPtr<IPropertyValue> propertyValue;
    PropertyType propertyType;

    hr = inReference->QueryInterface(__uuidof(IReference<typeName>), (LPVOID*)&referenceT);
    IfFailedReturn(hr);

    hr = inReference->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
    IfFailedReturn(hr);

    hr = propertyValue->get_Type(&propertyType);
    IfFailedReturn(hr);

    if (propertyType != inPropertyType)
    {
        return E_INVALIDARG;
    }

    return referenceT->get_Value(outValue);
}

HRESULT Animals::AnimalServer::GetBooleanIReferenceValue(Windows::Foundation::IReference<bool>* inReference, __out boolean* outValue)
{
    if (nullptr == inReference || nullptr == outValue)
    {
        return E_INVALIDARG;
    }
    HRESULT hr;
    ComPtr<IReference<bool>> referenceT;
    ComPtr<IPropertyValue> propertyValue;
    PropertyType propertyType;

    hr = inReference->QueryInterface(__uuidof(IReference<bool>), (LPVOID*)&referenceT);
    IfFailedReturn(hr);

    hr = inReference->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
    IfFailedReturn(hr);

    hr = propertyValue->get_Type(&propertyType);
    IfFailedReturn(hr);

    if (propertyType != PropertyType_Boolean)
    {
        return E_INVALIDARG;
    }

    return referenceT->get_Value(outValue);
}

HRESULT Animals::AnimalServer::TestInSimpleIRefStruct( SimpleIRefStruct simpleIRefStruct)
{
    HRESULT hr = S_OK;
    if (simpleIRefStruct.field1 == nullptr && simpleIRefStruct.field2 != nullptr)
    {
        return E_FAIL;
    }
    if (simpleIRefStruct.field1 != nullptr && simpleIRefStruct.field2 == nullptr)
    {
        return E_FAIL;
    }

    if (simpleIRefStruct.field1 == nullptr && simpleIRefStruct.field2 == nullptr)
    {
        return S_OK;
    }

    int field1;
    short field2;
    hr = GetIReferenceValue<int>(simpleIRefStruct.field1, PropertyType_Int32, &field1);
    IfFailedReturn(hr);

    hr = GetIReferenceValue<short>(simpleIRefStruct.field2, Windows::Foundation::PropertyType::PropertyType_Int16, &field2);
    IfFailedReturn(hr);

    if (field1 == field2)
    {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT Animals::AnimalServer::TestOutSimpleIRefStruct(int seedValue, SimpleIRefStruct* simpleIRefStruct)
{
    HRESULT hr;
    if (seedValue == 0)
    {
        simpleIRefStruct->field1 = nullptr;
        simpleIRefStruct->field2 = nullptr;
        return S_OK;
    }
    ComPtr<IPropertyValue> propertyValue2;
    ComPtr<IPropertyValue> propertyValue1;
    hr = spPropertyValueFactory->CreateInt32(seedValue, (IInspectable **)&propertyValue1);
    IfFailedReturn(hr);
    hr = propertyValue1.Get()->QueryInterface(__uuidof(IReference<int>), (void**)&(simpleIRefStruct->field1));
    IfFailedReturn(hr);

    hr = spPropertyValueFactory->CreateInt16((short)seedValue, (IInspectable **)&propertyValue2);
    IfFailedReturn(hr);
    hr = propertyValue2.Get()->QueryInterface(__uuidof(IReference<short>), (void**)&(simpleIRefStruct->field2));
    return hr;
}

HRESULT Animals::AnimalServer::TestInMixIRefStruct( SimpleMixIRefStruct mixedIRefStruct)
{
    HRESULT hr = S_OK;
    if (mixedIRefStruct.field1 == nullptr && mixedIRefStruct.field2 != nullptr)
    {
        return E_FAIL;
    }
    if (mixedIRefStruct.field1 != nullptr && mixedIRefStruct.field2 == nullptr)
    {
        return E_FAIL;
    }

    if (mixedIRefStruct.field1 == nullptr && mixedIRefStruct.field2 == nullptr)
    {
        return S_OK;
    }

    int field1;
    short field2;
    hr = GetIReferenceValue<int>(mixedIRefStruct.field1, PropertyType_Int32, &field1);
    IfFailedReturn(hr);

    hr = GetIReferenceValue<short>(mixedIRefStruct.field2, Windows::Foundation::PropertyType::PropertyType_Int16, &field2);
    IfFailedReturn(hr);

    if (field1 != field2)
    {
        return E_FAIL;
    }
    if ((field1 > 0 && field1 == mixedIRefStruct.field3) ||
        (field1 < 0 && -field1 == mixedIRefStruct.field3))
    {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT Animals::AnimalServer::TestOutMixIRefStruct(int seedValue, SimpleMixIRefStruct* mixedIRefStruct)
{
    HRESULT hr;
    if (seedValue == 0)
    {
        mixedIRefStruct->field1 = nullptr;
        mixedIRefStruct->field2 = nullptr;
        mixedIRefStruct->field3 = 0;
        return S_OK;
    }
    ComPtr<IPropertyValue> propertyValue2;
    ComPtr<IPropertyValue> propertyValue1;
    hr = spPropertyValueFactory->CreateInt32(seedValue, (IInspectable **)&propertyValue1);
    IfFailedReturn(hr);
    hr = propertyValue1.Get()->QueryInterface(__uuidof(IReference<int>), (void**)&(mixedIRefStruct->field1));
    IfFailedReturn(hr);

    hr = spPropertyValueFactory->CreateInt16((short)seedValue, (IInspectable **)&propertyValue2);
    IfFailedReturn(hr);
    hr = propertyValue2.Get()->QueryInterface(__uuidof(IReference<short>), (void**)&(mixedIRefStruct->field2));
    mixedIRefStruct->field3 = (byte)seedValue;
    return hr;
}

HRESULT Animals::AnimalServer::TestInNestedIRefStruct( NestedIRefStruct nestedIRefStruct)
{
    nestedIRefStruct;
    return E_NOTIMPL;
}

HRESULT Animals::AnimalServer::TestOutNestedIRefStruct(int seedValue, NestedIRefStruct* nestedIRefStruct)
{
    nestedIRefStruct;seedValue;
    return E_NOTIMPL;
}

HRESULT Animals::AnimalServer::TestInNestedIRefNestedStruct( NestedIRefNestedStruct nestedIRefStruct)
{
    HRESULT hr = S_OK;
    if (nestedIRefStruct.field1 == nullptr && nestedIRefStruct.dimensions != nullptr)
    {
        return E_FAIL;
    }
    if (nestedIRefStruct.field1 != nullptr && nestedIRefStruct.dimensions == nullptr)
    {
        return E_FAIL;
    }

    if (nestedIRefStruct.field1 == nullptr && nestedIRefStruct.dimensions == nullptr)
    {
        return S_OK;
    }

    if (m_dimensionsRef != nullptr)
    {
        m_dimensionsRef->Release();
    }
    m_dimensionsRef = nestedIRefStruct.dimensions;
    m_dimensionsRef->AddRef();
    int field1;
    hr = GetIReferenceValue<int>(nestedIRefStruct.field1, PropertyType_Int32, &field1);
    IfFailedReturn(hr);

    Dimensions dimensions;
    hr = GetIReferenceValue<Dimensions>(nestedIRefStruct.dimensions, Windows::Foundation::PropertyType::PropertyType_OtherType, &dimensions);
    IfFailedReturn(hr);

    if (field1 != dimensions.Length || field1 != dimensions.Width)
    {
        return E_FAIL;
    }

    return S_OK;
}

HRESULT Animals::AnimalServer::TestOutNestedIRefNestedStruct(int seedValue, NestedIRefNestedStruct* nestedIRefStruct)
{
    HRESULT hr;
    if (seedValue == 0)
    {
        nestedIRefStruct->field1 = nullptr;
        nestedIRefStruct->dimensions = nullptr;
        return S_OK;
    }
    ComPtr<IPropertyValue> propertyValue2;
    ComPtr<IPropertyValue> propertyValue1;
    hr = spPropertyValueFactory->CreateInt32(seedValue, (IInspectable **)&propertyValue1);
    IfFailedReturn(hr);
    hr = propertyValue1.Get()->QueryInterface(__uuidof(IReference<int>), (void**)&(nestedIRefStruct->field1));
    IfFailedReturn(hr);

    nestedIRefStruct->dimensions = m_dimensionsRef;
    if (m_dimensionsRef)
    {
        m_dimensionsRef->AddRef();
    }
    nestedIRefStruct->field3 = (BYTE)seedValue;
    return hr;
}

HRESULT Animals::AnimalServer::TestInAllIRefStruct( AllIRefStruct allIRefStruct)
{
    ComPtr<IPropertyValue> propertyValue;
    WCHAR wcharField = 0;
    byte byteField = 0;
    INT16 int16Field = 0;
    INT32 int32Field = 0;
    UINT16 uint16Field = 0;
    UINT32 uint32Field = 0;
    boolean booleanField = 0;
    float floatField = 0;
    double doubleField = 0;
    BOOL hasNULLField = false;
    BOOL hasNonNULLField = false;
    HRESULT hr = S_OK;

    if (allIRefStruct.wcharField)
    {
        hr = GetIReferenceValue<WCHAR>(allIRefStruct.wcharField, PropertyType_Char16, &wcharField);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }


    if (allIRefStruct.byteField)
    {
        GetIReferenceValue<byte>(allIRefStruct.byteField, PropertyType_UInt8, &byteField);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.int16Field)
    {
        GetIReferenceValue<INT16>(allIRefStruct.int16Field, PropertyType_Int16, &int16Field);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.int32Field)
    {
        hr = GetIReferenceValue<INT32>(allIRefStruct.int32Field, PropertyType_Int32, &int32Field);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.uint16Field)
    {
        hr = GetIReferenceValue<UINT16>(allIRefStruct.uint16Field, PropertyType_UInt16, &uint16Field);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.uint32Field)
    {
        hr = GetIReferenceValue<UINT32>(allIRefStruct.uint32Field, PropertyType_UInt32, &uint32Field);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.booleanField)
    {
        hr = GetBooleanIReferenceValue(allIRefStruct.booleanField,&booleanField);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.floatField)
    {
        hr = GetIReferenceValue<float>(allIRefStruct.floatField, PropertyType_Single, &floatField);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (allIRefStruct.floatField)
    {
        hr = GetIReferenceValue<double>(allIRefStruct.doubleField, PropertyType_Double, &doubleField);
        IfFailedReturn(hr);
        hasNonNULLField = true;
    }
    else
    {
        hasNULLField = true;
    }

    if (hasNULLField && hasNonNULLField)
    {
        return E_FAIL;
    }

    if (hasNULLField)
    {
        return S_OK;
    }

    if ((floatField != doubleField) || (floatField != (float)uint32Field) ||
        (floatField != (float)int32Field) || (floatField != (float)int16Field) ||
        (floatField != (float)uint16Field) || (floatField != (float)byteField))
    {
        return E_FAIL;
    }
    if ((floatField != 0) ^ booleanField)
    {
        return E_FAIL;
    }
    return S_OK;
}

HRESULT Animals::AnimalServer::TestOutAllIRefStruct(int seedValue, AllIRefStruct* allIRefStruct)
{
    if (seedValue == 0)
    {
        memset(allIRefStruct, 0, sizeof(AllIRefStruct));
        return S_OK;
    }
    ComPtr<IPropertyValue> propertyValue;
    HRESULT hr;

    hr = spPropertyValueFactory->CreateInt16((short)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<short>), (void**)&(allIRefStruct->int16Field));

    hr = spPropertyValueFactory->CreateInt32((int)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<int>), (void**)&(allIRefStruct->int32Field));

    hr = spPropertyValueFactory->CreateUInt16((UINT16)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<UINT16>), (void**)&(allIRefStruct->uint16Field));

    hr = spPropertyValueFactory->CreateUInt32((UINT32)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<UINT32>), (void**)&(allIRefStruct->uint32Field));

    hr = spPropertyValueFactory->CreateUInt8((byte)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<byte>), (void**)&(allIRefStruct->byteField));

    hr = spPropertyValueFactory->CreateChar16((WCHAR)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<WCHAR>), (void**)&(allIRefStruct->wcharField));

    hr = spPropertyValueFactory->CreateSingle((float)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<float>), (void**)&(allIRefStruct->floatField));

    hr = spPropertyValueFactory->CreateDouble((double)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<double>), (void**)&(allIRefStruct->doubleField));

    hr = spPropertyValueFactory->CreateBoolean((boolean)seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<bool>), (void**)&(allIRefStruct->booleanField));

    return hr;
}

// Bug 258665
HRESULT Animals::AnimalServer::TestOutBug258665_HttpProgress(__in HSTRING url, __out Bug258665_HttpProgress* structHttpProgress)
{
	memset(structHttpProgress, 0, sizeof(Bug258665_HttpProgress));

	(void)url;
	
    HRESULT hr;
    ComPtr<IPropertyValue> propertyValue;
	UINT64 seedValue = 1234;

	// test values
	LPCWSTR sz = L"Stage test";
    WindowsCreateString(sz, (UINT32)wcslen(sz), &(structHttpProgress->Stage));
	structHttpProgress->BytesSent = 123;
    hr = spPropertyValueFactory->CreateUInt64(seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<UINT64>), (void**)&(structHttpProgress->TotalBytesToSend));
    IfFailedReturn(hr);
	structHttpProgress->BytesReceived = 321;
    hr = spPropertyValueFactory->CreateUInt64(seedValue, (IInspectable **)&propertyValue);
    IfFailedReturn(hr);
    hr = propertyValue.Get()->QueryInterface(__uuidof(IReference<UINT64>), (void**)&(structHttpProgress->TotalBytesToReceive));
    IfFailedReturn(hr);
	structHttpProgress->Retries = 2;
	
	return hr;
}

HRESULT Animals::AnimalServer::TestOutBug258665_HttpProgressAsOptEmpty(__out_opt Windows::Foundation::IReference<Bug258665_HttpProgress>** optionalStructHttpProgress)
{
    HRESULT hr;
	
	*optionalStructHttpProgress = nullptr;
	hr = S_OK;
	
	return hr;
}

HRESULT Animals::AnimalServer::TestOutBug258665_HttpProgressAsOptIntEmpty(__out_opt Windows::Foundation::IReference<UINT64>** optionalUInt64)
{
    HRESULT hr;
	
	*optionalUInt64 = nullptr;
	hr = S_OK;
	
	return hr;
}
// /Bug 258665
