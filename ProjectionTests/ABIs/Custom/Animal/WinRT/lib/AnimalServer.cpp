//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "AnimalServer.h"
#include "DinoServer.h"
#include "PropertyValueTests.h"
#include "CollectionsServer.h"

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;


#define MarshalMethodImp(type) IFACEMETHODIMP Animals::AnimalServer::Marshal##type(t_##type _in, __out t_##type* _out) \
{ \
    *_out = _in; \
    return S_OK; \
}

using namespace Microsoft::WRL;

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneVector(__out Windows::Foundation::Collections::IVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<int>> sp;

    hr = Vector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 4; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVal);
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetNullAsVector(__out Windows::Foundation::Collections::IVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneObservableVector(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = NULL;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ObservableVector<int>> sp;

    hr = ObservableVector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 5; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVal);
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetNullAsObservableVector(__out Windows::Foundation::Collections::IObservableVector<int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneAnimal(__out Animals::IAnimal **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
    spAnimal.CopyTo(outVal);

    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetNullAsAnimal(__out Animals::IAnimal **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneMap(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    Microsoft::WRL::ComPtr<HashMap<HSTRING, int>> spMap;
    HRESULT hr = HashMap<HSTRING, int>::Make(&spMap); 
    IfFailedReturn(hr);

    boolean fReplaced;
    HSTRING hString = nullptr;

    WindowsCreateString(L"Hundred", 7, &hString);
    spMap->Insert(hString, 7, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"by", 2, &hString);
    spMap->Insert(hString, 2, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"Hundred And Fifty", 17, &hString);
    spMap->Insert(hString, 17, &fReplaced);
    WindowsDeleteString(hString);

    spMap.CopyTo(outVal);
    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetNullAsMap(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOnePropertyValue(__out Windows::Foundation::IPropertyValue **outVal)
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    Microsoft::WRL::ComPtr<Windows::Foundation::IPropertyValueStatics> spPropertyValueFactory;
    Windows::Internal::StringReference strFactory(L"Windows.Foundation.PropertyValue");
    Windows::Foundation::GetActivationFactory(strFactory.Get(), &spPropertyValueFactory);

    IInspectable *inspectable = nullptr;
    HRESULT hr = spPropertyValueFactory->CreateDouble(10.5, &inspectable);
    if (SUCCEEDED(hr))
    {
        hr = inspectable->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **) outVal);
        inspectable->Release();
    }

    return hr;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetNullAsPropertyValue(__out Windows::Foundation::IPropertyValue **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneEmptyGRCNInterface(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;

    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyGRCNInterface>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneEmptyGRCNNull(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    
    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyGRCN>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

IFACEMETHODIMP Animals::CFastSigInterfaceStatic::GetOneEmptyGRCNFail(__out Animals::IEmptyGRCN **outVal) 
{
    IfNullReturnError(outVal, E_POINTER);
    *outVal = nullptr;
    
    ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyFailingGRCNString>();
    spDuplicate.CopyTo(outVal);
    return S_OK;
}

Animals::AnimalServer::AnimalServer() : m_Weight(50), m_NumLegs(20), mother(nullptr), m_array(nullptr), m_arrayHSTRING(nullptr), m_Vector(nullptr), m_Iterable(nullptr)
{
    InnerStruct s;
    s.a = 100;
    
    m_Dimensions.Length = 180;
    m_Dimensions.Width = 360;
    m_OuterStruct.Inner = s;
    m_Phylum = Phylum_Acoelomorpha;
    
    WindowsCreateString(L"Hello", 5, &m_Greeting);
    WindowsCreateString(L"Wolverine", 9, &m_Names.Common);
    WindowsCreateString(L"Gulo gulo", 9, &m_Names.Scientific);
    WindowsCreateString(L"Skunk Bear", 10, &m_Names.AlsoKnownAs);

    m_arrayLength = 0;
    m_arraySize = 0;
    m_arrayLengthHSTRING = 0;
    m_arraySizeHSTRING = 0;

    Windows::Internal::StringReference strFactory(L"Windows.Foundation.PropertyValue");
    Windows::Foundation::GetActivationFactory(strFactory.Get(), &spPropertyValueFactory);
    m_dimensionsRef = nullptr;
}

Animals::AnimalServer::~AnimalServer()
{
    if (NULL != m_array)
    {
        // Clear the array
        CoTaskMemFree(m_array);
        m_array = nullptr;
        m_arraySize = 0;
        m_arrayLength = 0;
    }

    if (NULL != m_arrayHSTRING)
    {
        for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
        {
            WindowsDeleteString(m_arrayHSTRING[iIndex]);
        }

        // Clear the array
        CoTaskMemFree(m_arrayHSTRING);
        m_arrayHSTRING = nullptr;
        m_arraySizeHSTRING = 0;
        m_arrayLengthHSTRING = 0;
    }
    WindowsDeleteString(m_Greeting);
    WindowsDeleteString(m_Names.Common);
    WindowsDeleteString(m_Names.Scientific);
    WindowsDeleteString(m_Names.AlsoKnownAs);
    

    ReleasePtr(m_Vector);
    ReleasePtr(m_Iterable);
    ReleasePtr(mother);
    ReleasePtr(m_dimensionsRef);
}

IFACEMETHODIMP Animals::AnimalServer::IsHungry(__out boolean* hungry)
{
     *hungry = false;
     return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::isSleepy(__out boolean* sleepy)
{
     *sleepy = false;
     return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::GetNames(__out Names *names)
{
    *names = m_Names;
    WindowsDuplicateString(m_Names.Common,&names->Common);
    WindowsDuplicateString(m_Names.Scientific,&names->Scientific);
    WindowsDuplicateString(m_Names.AlsoKnownAs,&names->AlsoKnownAs);
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::GetDimensions(__out Dimensions *dimensions)
{
    if ((dimensions->Length != NULL) || (dimensions->Width != NULL))
    {
        return E_INVALIDARG;
    }

    *dimensions = m_Dimensions;
    return S_OK;
}

void CopyVectorTo(Animals::Vector8 source, Animals::Vector8* target)
{
    target->Value1 = source.Value1;
    target->Value2 = source.Value2;
    target->Value3 = source.Value3;
    target->Value4 = source.Value4;
    target->Value5 = source.Value5;
    target->Value6 = source.Value6;
    target->Value7 = source.Value7;
    target->Value8 = source.Value8;
}

IFACEMETHODIMP Animals::AnimalServer::PassIDByRef(__in const GUID* value)
{
    return this->put_ID(*value);
}
IFACEMETHODIMP Animals::AnimalServer::FillDimensions(__in const Dimensions* dimensions)
{
    this->m_Dimensions.Length = dimensions->Length;
    this->m_Dimensions.Width = dimensions->Width;
    return S_OK;
}
IFACEMETHODIMP Animals::AnimalServer::CheckMother(__in const IAnimal* value)
{
    return this->mother == value ? S_OK : E_INVALIDARG;
}
IFACEMETHODIMP Animals::AnimalServer::CheckMotherConcrete(__in const IAnimal* animal)
{
    return this->mother == animal ? S_OK : E_INVALIDARG;
}
IFACEMETHODIMP Animals::AnimalServer::DelegateByRef_Struct(__in const Animals::IDelegateWithByRefParameter_Struct* value, __in const Dimensions* dimensions)
{
    return ((IDelegateWithByRefParameter_Struct*)value)->Invoke(dimensions);
}
IFACEMETHODIMP Animals::AnimalServer::DelegateByRef_GUID(__in const Animals::IDelegateWithByRefParameter_GUID* value, __in const GUID* guid)
{
    return ((IDelegateWithByRefParameter_GUID*)value)->Invoke(guid);
}
IFACEMETHODIMP Animals::AnimalServer::DelegateByRef_Interface(__in const Animals::IDelegateWithByRefParameter_Interface* value, __in const IAnimal* animal)
{
    boolean success = false;
    HRESULT hr = ((IDelegateWithByRefParameter_Interface*)value)->Invoke(animal, &success);
    if (hr == S_OK)
    {
        return success ? S_OK : E_INVALIDARG;
    }
    return hr;
}
IFACEMETHODIMP Animals::AnimalServer::DelegateByRef_Class(__in const Animals::IDelegateWithByRefParameter_Class* value, __in const IAnimal* animal)
{
    boolean success = false;
    HRESULT hr = ((IDelegateWithByRefParameter_Class*)value)->Invoke(animal, &success);
    if (hr == S_OK)
    {
        return success ? S_OK : E_INVALIDARG;
    }
    return hr;
}
IFACEMETHODIMP Animals::AnimalServer::DelegateByRef_Delegate(__in const Animals::IDelegateWithByRefParameter_Delegate* value, __in const Animals::IDelegateWithByRefParameter_Struct* del, __in const Dimensions* dimensions)
{
    return ((IDelegateWithByRefParameter_Delegate*)value)->Invoke(del, dimensions);
}
IFACEMETHODIMP Animals::AnimalServer::IsStructModified(__in const IDelegateWithByRefParameter_Struct* value, __in const Dimensions* dimensions, __out boolean *wasModified)
{
    if (wasModified == nullptr)
    {
        return E_INVALIDARG;
    }

    Dimensions localVersion(*dimensions);
    HRESULT hr = ((IDelegateWithByRefParameter_Struct*)value)->Invoke(dimensions);
    *wasModified = dimensions->Length != localVersion.Length || dimensions->Width != localVersion.Width;
    return hr;
}
IFACEMETHODIMP Animals::AnimalServer::AreDimensionPointersEqual(__in const Dimensions* one, __in const Dimensions* two, __out boolean *areEqual)
{
    if (areEqual == nullptr)
    {
        return E_INVALIDARG;
    }
    *areEqual = one == two;
    return S_OK;
}
IFACEMETHODIMP Animals::AnimalServer::AcceptKiloStruct(__in const KiloStruct* kiloStruct)
{
    if (kiloStruct == nullptr)
    {
        return E_INVALIDARG;
    }
    return S_OK;
}
IFACEMETHODIMP Animals::AnimalServer::CheckByRefStruct(__in Windows::Foundation::IReference<MixedStruct>* one, __in const MixedStruct* two, __out MixedStructResult* result)
{
    if (one == nullptr || result == nullptr)
    {
        return E_INVALIDARG;
    }
    MixedStruct localVersion;
    one->get_Value(&localVersion);
    result->StructPointerEqual = false;
    result->AStringPointerEqual = localVersion.AString == two->AString;

    result->MatrixRefPointerEqual = localVersion.MatrixRef == two->MatrixRef;
    result->IntRefPointerEqual = localVersion.IntRef == two->IntRef;
    CopyVectorTo(two->Matrix.Vector2, &localVersion.Matrix.Vector1);
    CopyVectorTo(localVersion.Matrix.Vector2, (Vector8 *)&two->Matrix.Vector1);
    localVersion.AnInt = 1;
    ((MixedStruct *)two)->AnInt = 2;
    localVersion.MatrixRef->Release();
    localVersion.IntRef->Release();

    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::get_Weight(__out int* _weight)
{
    *_weight = m_Weight;
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::put_Weight(int _weight)
{
    if (_weight < 110)
    {
        m_Weight = _weight;
        return S_OK;
    }
    return E_FAIL;
}

IFACEMETHODIMP
Animals::AnimalServer::SetNumLegs(int numberOfLegs)
{
    m_NumLegs = numberOfLegs;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::GetNumLegs(__out int* numberOfLegs)
{
    *numberOfLegs = m_NumLegs;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::SetGreeting(HSTRING greeting)
{
    if(m_Greeting)
        WindowsDeleteString(m_Greeting);
    return WindowsDuplicateString(greeting, &m_Greeting);
}

IFACEMETHODIMP                   
Animals::AnimalServer::GetGreeting(__out HSTRING* greeting)
{
   return WindowsDuplicateString(m_Greeting, greeting);
}

IFACEMETHODIMP
Animals::AnimalServer::AddInts(int val1, int val2, __out int* result)
{
    *result = val1 + val2;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::GetOuterStruct(__out OuterStruct* strct)
{
    *strct = m_OuterStruct;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::MarshalPhylum(Phylum _in, __out Phylum* _out)
{
    *_out = _in;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::MarshalPhylumChange(PhylumChange _in, __out PhylumChange* _out)
{
    *_out = _in;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::MarshalNames(Names _in, __out Names* _out)
{
    *_out = _in;
    WindowsDuplicateString(_in.Common,&_out->Common);
    WindowsDuplicateString(_in.Scientific,&_out->Scientific);
    WindowsDuplicateString(_in.AlsoKnownAs,&_out->AlsoKnownAs);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::MarshalHSTRING(HSTRING _in, __out HSTRING* _out)
{
    return WindowsDuplicateString(_in,_out);
}


IFACEMETHODIMP
Animals::AnimalServer::GetVector(__out Windows::Foundation::Collections::IVector<int> ** uniqueNumbersVector)
{
    if (NULL == uniqueNumbersVector)
    {
        return E_POINTER;
    }

    *uniqueNumbersVector = NULL;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<int>> sp;

    hr = Vector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 10; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(uniqueNumbersVector);
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::CopyVector(Windows::Foundation::Collections::IVector<int> * inVector, __out Windows::Foundation::Collections::IVector<int> ** outVector)
{
    if (NULL == inVector)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<int>> sp;

    hr = Vector<int>::Make(&sp);
    if (FAILED(hr))
    {
        return hr;
    }
    
    unsigned int cCount;
    hr = inVector->get_Size(&cCount);
    for (int i = 0; SUCCEEDED(hr) && i < (int)cCount; i++)
    {
        int iValue;
        hr = inVector->GetAt(i, &iValue);
        if (SUCCEEDED(hr))
        {
            hr = sp->Append(iValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVector);
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::GetStringVector(__out Windows::Foundation::Collections::IVector<HSTRING> ** outVector)
{
    if (NULL == outVector)
    {
        return E_POINTER;
    }

    *outVector = NULL;

    Microsoft::WRL::ComPtr<Vector<HSTRING>> sp;

    IfFailedReturn(Vector<HSTRING>::Make(&sp));

    auto spAppend = [&](PCNZWCH str) -> HRESULT{
        HSTRING sValue;
        IfFailedReturn(WindowsCreateString(str, (UINT32)wcslen(str), &sValue));
        HRESULT hr = sp->Append(sValue);
        IfFailedReturn(WindowsDeleteString(sValue));
        return hr;
    };

    IfFailedReturn(spAppend(L"Blue"));
    IfFailedReturn(spAppend(L"Red"));
    IfFailedReturn(spAppend(L"Yellow"));
    IfFailedReturn(spAppend(L"Green"));
    IfFailedReturn(spAppend(L"Pink"));
    IfFailedReturn(spAppend(L"Black"));
    IfFailedReturn(spAppend(L"White"));
    IfFailedReturn(spAppend(L"Tan"));
    IfFailedReturn(spAppend(L"Magenta"));
    IfFailedReturn(spAppend(L"Orange"));

    IfFailedReturn(sp.CopyTo(outVector));

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::CopyStringVector(Windows::Foundation::Collections::IVector<HSTRING> * inVector, __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector)
{
    if (NULL == inVector)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<HSTRING>> sp;

    hr = Vector<HSTRING>::Make(&sp);
    if (FAILED(hr))
    {
        return hr;
    }
    
    unsigned int cCount;
    hr = inVector->get_Size(&cCount);
    for (int i = 0; SUCCEEDED(hr) && i < (int)cCount; i++)
    {
        HSTRING outStringValue;
        hr = inVector->GetAt(i, &outStringValue);
        if (SUCCEEDED(hr))
        {
            hr = sp->Append(outStringValue);
            WindowsDeleteString(outStringValue);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = sp.CopyTo(outVector);
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::GetMap(Windows::Foundation::Collections::IVector<int> * /* uniqueNumbersVector */, __out Windows::Foundation::Collections::IMap<int, HSTRING> ** /* uniqueNumbersMap */)
{

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::LikesChef(__out Fabrikam::Kitchen::IChef **chef)
{
    if (0 == chef)
    {
        return E_INVALIDARG;
    }

    HSTRING typeName = NULL;
    IActivationFactory *pActivationFactory = NULL;
    IInspectable* pInspectableKitchen = NULL;
    Fabrikam::Kitchen::IKitchen *pKitchen = NULL;
    HSTRING chefName = NULL;
    HSTRING chefFactoryTypeName = NULL;
    Fabrikam::Kitchen::IChefFactory *pChefFactory = NULL;
    Fabrikam::Kitchen::IChef *pChef = NULL;

    // Create Kitchen
    HRESULT hr = WindowsCreateString(L"Fabrikam.Kitchen.Kitchen", (UINT32)wcslen(L"Fabrikam.Kitchen.Kitchen"), &typeName);
    IfFailedGo(hr);

    hr = RoGetActivationFactory(typeName, __uuidof(IActivationFactory), (LPVOID *)&pActivationFactory);
    IfFailedGo(hr);

    hr = pActivationFactory->ActivateInstance(&pInspectableKitchen);
    IfFailedGo(hr);

    hr = pInspectableKitchen->QueryInterface(__uuidof(Fabrikam::Kitchen::IKitchen), (LPVOID *)&pKitchen);
    IfFailedGo(hr);

    // Create Chef name
    hr = WindowsCreateString(L"Aarti Sequeira", (UINT32)wcslen(L"Aarti Sequeira"), &chefName);
    IfFailedGo(hr);

    // Create Chef
    hr = WindowsCreateString(L"Fabrikam.Kitchen.Chef", (UINT32)wcslen(L"Fabrikam.Kitchen.Chef"), &chefFactoryTypeName);
    IfFailedGo(hr);

    hr = RoGetActivationFactory(chefFactoryTypeName, __uuidof(Fabrikam::Kitchen::IChefFactory), (LPVOID *)&pChefFactory);
    IfFailedGo(hr);

    hr =  pChefFactory->CreateChef(chefName, pKitchen, &pChef);
    IfFailedGo(hr);

    *chef = pChef;

LReturn:
    if (typeName)
    {
        WindowsDeleteString(typeName);
    }

    if (pActivationFactory)
    {
        pActivationFactory->Release();
    }

    if (pInspectableKitchen)
    {
        pInspectableKitchen->Release();
    }

    if (pKitchen)
    {
        pKitchen->Release();
    }

    if (chefName)
    {
        WindowsDeleteString(chefName);
    }

    if (chefFactoryTypeName)
    {
        WindowsDeleteString(chefFactoryTypeName);
    }

    if (pChefFactory)
    {
        pChefFactory->Release();
    }

    return hr;
}

HRESULT
Animals::AnimalServer::GetArray(__out Windows::Foundation::Collections::IVector<int> **outVector, __in int from, __in int to)
{
    // Make vector to send back so that we can check if returned array is marshalled correctly
    Microsoft::WRL::ComPtr<Vector<int>> sp;
    HRESULT hr = Vector<int>::Make(&sp);
    if (FAILED(hr))
    {
        return hr;
    }
    
    for (int i = from; SUCCEEDED(hr) && i < (int)m_arrayLength && i < to; i++)
    {
        hr = sp->Append(m_array[i]);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVector);
    }

    return hr;
}

HRESULT Animals::AnimalServer::PassArrayCore(__in UINT32 length, __RPC__in_ecount_part(length, lengthValue) int *value, __in UINT32 lengthValue)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue > length)
    {
        return E_INVALIDARG;
    }

    if (m_array)
    {
        CoTaskMemFree(m_array);
        m_array = nullptr;
        m_arraySize = 0;
        m_arrayLength = 0;
    }
    
    m_arraySize = length;
    if (m_arraySize > 0)
    {
        m_array = (int*)CoTaskMemAlloc(m_arraySize * sizeof(int));
        if (m_array == NULL)
        {
            return E_OUTOFMEMORY;
        }

        m_arrayLength = lengthValue;
        // read in the values
        for(size_t i = 0; i < m_arrayLength; i++)
        {
            m_array[i] = *value;
            value += 1;
        }
    }

    return S_OK;
}
        
IFACEMETHODIMP
Animals::AnimalServer::PurePassArray(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) int *value)
{
    return PassArrayCore(length, value, length);
}

IFACEMETHODIMP
Animals::AnimalServer::PassArray(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) int *value,
    __out Windows::Foundation::Collections::IVector<int> ** outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }
    
    HRESULT hr =  PurePassArray(length, value);
    IfFailedReturn(hr);

    return GetArray(outVector, 0, length);
}

HRESULT Animals::AnimalServer::PassArrayWithInLength(
    __in UINT32 length, 
    __RPC__in_ecount_part(length, lengthValue) int *value, 
    __in UINT32 lengthValue)
{
    return  PassArrayCore(length, value, lengthValue);
}
            
HRESULT Animals::AnimalServer::PassArrayWithOutLength( 
    __in UINT32 length,
    __RPC__in_ecount_part(length, *lengthValue) int *value,
    __RPC__out UINT32 *lengthValue)
{
    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }
    
    HRESULT hr =  PurePassArray(length, value);
    IfFailedReturn(hr);

    *lengthValue = m_arrayLength;
    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::PureFillArray(
    __in UINT32 length, 
    __RPC__out_ecount_full(length) int *value)
{
    if (NULL == value && length != 0)
    {
        return E_POINTER;
    }

    // write the values into value array
    for(size_t i = 0; i < length && i < m_arrayLength; i++)
    {
        value[i] = m_array[i];
    }

    for(size_t i = m_arrayLength; i < length; i++)
    {
        value[i] = 0;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::FillArray(
    __in UINT32 length, 
    __RPC__out_ecount_full(length) int *value,
    __out Windows::Foundation::Collections::IVector<int> ** outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = PureFillArray(length, value);
    IfFailedReturn(hr);

    return GetArray(outVector, 0, length);
}

HRESULT Animals::AnimalServer::FillArrayWithInLength( 
    __in UINT32 length,
    __RPC__out_ecount_part(length, lengthValue) int *value,
    __in UINT32 lengthValue)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue > length)
    {
        return E_INVALIDARG;
    }

    // write the values into value array
    size_t i = 0;
    for(; i < lengthValue && i < m_arrayLength; i++)
    {
        value[i] = m_array[i];
    }

    for (; i < lengthValue; i++)
    {
        value[i] = 0;
    }

    return S_OK;
}
            
HRESULT Animals::AnimalServer::FillArrayWithOutLength( 
    __in UINT32 length,
    __RPC__out_ecount_part(length, *lengthValue) int *value,
    __RPC__out UINT32 *lengthValue)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }

    *lengthValue = 0;

    // write the values into value array
    for(size_t i = 0; i < length && i < m_arrayLength; i++)
    {
        value[i] = m_array[i];
        (*lengthValue)++;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::PureReceiveArray(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) int **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    *value = (int *)CoTaskMemAlloc(m_arraySize * sizeof(int));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLength; i++)
    {
        (*value)[i] = m_array[i];
    }

    for (size_t i = m_arrayLength; i < m_arraySize; i++)
    {
        (*value)[i] = 0;
    }

    *length = m_arraySize;

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::put_MyArrayProp(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) int *value)
{
    return PurePassArray(length, value);
}

IFACEMETHODIMP
Animals::AnimalServer::get_MyArrayProp(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) int **value)
{
    return PureReceiveArray(length, value);
}

IFACEMETHODIMP
Animals::AnimalServer::put_MyArrayPropHSTRING(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) HSTRING *value)
{
    return PassArrayHSTRINGCore(length, value, length);
}

IFACEMETHODIMP
Animals::AnimalServer::get_MyArrayPropHSTRING(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
{
    if (nullptr == length || nullptr == value)
    {
        return E_POINTER;
    }

    *value = (HSTRING *)CoTaskMemAlloc(m_arraySizeHSTRING * sizeof(HSTRING));
    if (*value == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        (*value)[i] = item;
    }

    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        (*value)[i] = nullptr;
    }

    *length = m_arraySizeHSTRING;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::ReceiveArray(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) int **value,
    __out Windows::Foundation::Collections::IVector<int> ** outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = PureReceiveArray(length, value);
    IfFailedReturn(hr);

    return GetArray(outVector, 0, *length);
}

HRESULT Animals::AnimalServer::ReceiveArrayWithInLength( 
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_part_opt(*length, lengthValue) int **value,
    __in UINT32 lengthValue)
{
    if (nullptr == length || nullptr == value)
    {
        return E_POINTER;
    }

    UINT32 finalSize = (lengthValue > m_arraySize) ? lengthValue : m_arraySize;

    *value = (int *)CoTaskMemAlloc(finalSize * sizeof(int));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    size_t i = 0;
    for(; i < lengthValue && i < m_arrayLength; i++)
    {
        (*value)[i] = m_array[i];
    }

    for (; i < lengthValue; i++)
    {
        (*value)[i] = 0;
    }

    *length = finalSize;
    return S_OK;
}
            
HRESULT Animals::AnimalServer::ReceiveArrayWithOutLength( 
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_part_opt(*length, *lengthValue) int **value,
    __RPC__out UINT32 *lengthValue)
{
   if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }

    *value = (int *)CoTaskMemAlloc(m_arraySize * sizeof(int));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLength; i++)
    {
        (*value)[i] = m_array[i];
    }
    *lengthValue = m_arrayLength;
    *length = m_arraySize;

    return S_OK;
}

HRESULT
Animals::AnimalServer::CallDelegatePassArray(__in IDelegateWithInParam_Array* delegatePassArray)
{
    UINT32 arraySize = m_arraySize;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    memcpy_s(pArray, arraySize * sizeof(int), m_array, m_arraySize * sizeof(int));
    for (size_t i = m_arrayLength; i < m_arraySize; i++)
    {
        pArray[i] = 0;
    }
    
    HRESULT hr = delegatePassArray->Invoke(this, arraySize, pArray);
    CoTaskMemFree(pArray);

    return hr;
}

HRESULT
Animals::AnimalServer::CallDelegateFillArray(__in IDelegateWithInOutParam_Array* delegateFillArray)
{
    UINT32 arraySize = m_arraySize;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = delegateFillArray->Invoke(this, arraySize, pArray);

    if (SUCCEEDED(hr))
    {
        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arraySize;
        m_arraySize = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT
Animals::AnimalServer::CallDelegateReceiveArray(__in IDelegateWithOutParam_Array* delegateReceiveArray)
{
    UINT32 arraySize = 0;
    int *pArray = NULL;

    HRESULT hr = delegateReceiveArray->Invoke(this, &arraySize, &pArray);
    if (SUCCEEDED(hr))
    {
        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arraySize;
        m_arraySize = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithInLength(__in IDelegatePassArrayWithInLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = m_arrayLength;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    memcpy_s(pArray, arraySize * sizeof(int), m_array, m_arrayLength * sizeof(int));
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, arrayLength);
    CoTaskMemFree(pArray);

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLength(__in IDelegatePassArrayWithOutLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    memcpy_s(pArray, arraySize * sizeof(int), m_array, m_arraySize * sizeof(int));
    for (size_t i = m_arrayLength; i < m_arraySize; i++)
    {
        pArray[i] = 0;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLengthWithRetValLength(__in IDelegatePassArrayWithOutLengthWithRetValLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    memcpy_s(pArray, arraySize * sizeof(int), m_array, m_arraySize * sizeof(int));
    for (size_t i = m_arrayLength; i < m_arraySize; i++)
    {
        pArray[i] = 0;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLengthWithRetValRandomParam(__in IDelegatePassArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    memcpy_s(pArray, arraySize * sizeof(int), m_array, m_arraySize * sizeof(int));
    for (size_t i = m_arrayLength; i < m_arraySize; i++)
    {
        pArray[i] = 0;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength, randomRetVal);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithInLength(__in IDelegateFillArrayWithInLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = m_arrayLength;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, arrayLength);

    if (SUCCEEDED(hr))
    {
        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLength(__in IDelegateFillArrayWithOutLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLengthWithRetValLength(__in IDelegateFillArrayWithOutLengthWithRetValLength *delegateIn)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLengthWithRetValRandomParam(__in IDelegateFillArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal)
{
    UINT32 arraySize = m_arraySize;
    UINT32 arrayLength = 0;
    int *pArray = (int *)CoTaskMemAlloc(arraySize * sizeof(int));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength, randomRetVal);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithInLength(__in IDelegateReceiveArrayWithInLength *delegateIn)
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = m_arrayLength;
    int *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLength(__in IDelegateReceiveArrayWithOutLength *delegateIn) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    int *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLengthWithRetValLength(__in IDelegateReceiveArrayWithOutLengthWithRetValLength *delegateIn) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    int *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLengthWithRetValRandomParam(__in IDelegateReceiveArrayWithOutLengthWithRetValRandomParam *delegateIn, __out int *randomRetVal) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    int *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength, randomRetVal);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_array)
        {
            CoTaskMemFree(m_array);
        }

        m_array = pArray;
        m_arrayLength = arrayLength;
        m_arraySize = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalFactory::MethodWithInParam_BigStruct(__in CollectionChangedEventArgs inParam, __out HSTRING *objectId, __out CollectionChangeType *eType, __out UINT32 *index, __out UINT32 *previousIndex)
{
    if (objectId == nullptr || eType == nullptr || index == nullptr || previousIndex == nullptr)
    {
        return E_POINTER;
    }

    WindowsDuplicateString(inParam.objectId, objectId);
    *eType = inParam.eType;
    *index = inParam.index;
    *previousIndex = inParam.previousIndex;

    return S_OK;
}

HRESULT Animals::AnimalFactory::MethodWithOutParam_BigStruct(__in HSTRING objectId, __in CollectionChangeType eType, __in UINT32 index, __in UINT32 previousIndex, __out CollectionChangedEventArgs *outParam)
{
    if (outParam == nullptr)
    {
        return E_POINTER;
    }

    WindowsDuplicateString(objectId, &(outParam->objectId));
    outParam->eType = eType;
    outParam->index = index;
    outParam->previousIndex = previousIndex;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInParam_BigStruct(__in IDelegateWithInParam_BigStruct* delegateStruct, __in HSTRING objectId, __in CollectionChangeType eType, __in UINT32 index, __in UINT32 previousIndex)
{
    CollectionChangedEventArgs args;

    WindowsDuplicateString(objectId, &(args.objectId));
    HSTRING toPassObjectId;
    WindowsDuplicateString(objectId, &toPassObjectId);
    args.eType = eType;
    args.index = index;
    args.previousIndex = previousIndex;

    HRESULT hr = delegateStruct->Invoke(args, toPassObjectId, eType, index, previousIndex);

    WindowsDeleteString(toPassObjectId);
    WindowsDeleteString(objectId);
    return hr;
}

HRESULT Animals::AnimalFactory::CallDelegateWithOutParam_BigStruct(
    __in IDelegateWithOutParam_BigStruct* delegateStruct, 
    __out HSTRING *objectId, 
    __out CollectionChangeType *eType, 
    __out UINT32 *index, 
    __out UINT32 *previousIndex, 
    __out HSTRING *objectIdFromStruct, 
    __out CollectionChangeType *eTypeFromStruct, 
    __out UINT32 *indexFromStruct, 
    __out UINT32 *previousIndexFromStruct)
{
    if (objectId == nullptr || eType == nullptr || index == nullptr || previousIndex == nullptr || objectIdFromStruct == nullptr || eTypeFromStruct == nullptr || indexFromStruct == nullptr || previousIndexFromStruct == nullptr)
    {
        return E_POINTER;
    }

    CollectionChangedEventArgs args;
    HRESULT hr = delegateStruct->Invoke(&args, objectId, eType, index, previousIndex);

    *objectIdFromStruct = args.objectId;
    *eTypeFromStruct = args.eType;
    *indexFromStruct = args.index;
    *previousIndexFromStruct = args.previousIndex;

    return hr;
}



HRESULT Animals::AnimalFactory::MarshalInAndOutPackedByte(__in PackedByte inParam, __out PackedByte *outParam)
{
    if (nullptr == outParam)
    {
        return E_POINTER;
    }

    *outParam = inParam;
    return S_OK;
}

HRESULT Animals::AnimalFactory::GetPackedByteArray(
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_full_opt(*length) PackedByte **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    int bytesToAllocate = 5 * sizeof(PackedByte);
    *value = (PackedByte *)CoTaskMemAlloc(bytesToAllocate);
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    PackedByte packedByte;
    // write the values into value array
    for(size_t i = 0; i < 5; i++)
    {
        packedByte.Field0 = (byte)i;
        (*value)[i] = packedByte;
    }
    *length = 5;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutPackedByte(__in PackedByte inParam, __out PackedByte *outParam, __in IDelegatePackedByte *delegateIn)
{
    if (outParam == nullptr || delegateIn == nullptr)
    {
        return E_POINTER;
    }

    return delegateIn->Invoke(inParam, outParam);
}


HRESULT Animals::AnimalFactory::MarshalInAndOutPackedBoolean(__in PackedBoolean4 inParam, __out PackedBoolean4 *outParam)
{
    if (nullptr == outParam)
    {
        return E_POINTER;
    }

    *outParam = inParam;
    return S_OK;
}

HRESULT Animals::AnimalFactory::GetPackedBooleanArray(
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_full_opt(*length) PackedBoolean4 **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    int bytesToAllocate = 5 * sizeof(PackedBoolean4);
    *value = (PackedBoolean4 *)CoTaskMemAlloc(bytesToAllocate);
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    PackedBoolean4 packedBoolean4;
    // write the values into value array
    for(size_t i = 0; i < 5; i++)
    {
        packedBoolean4.Field0 = false;
        packedBoolean4.Field1 = true;
        packedBoolean4.Field2 = true;
        packedBoolean4.Field3 = false;
        (*value)[i] = packedBoolean4;
    }
    *length = 5;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutPackedBoolean(__in PackedBoolean4 inParam, __out PackedBoolean4 *outParam, __in IDelegatePackedBoolean *delegateIn)
{
    if (outParam == nullptr || delegateIn == nullptr)
    {
        return E_POINTER;
    }

    return delegateIn->Invoke(inParam, outParam);
}

HRESULT Animals::AnimalFactory::MarshalInAndOutOddSizedStruct(__in OddSizedStruct inParam, __out OddSizedStruct *outParam)
{
    if (nullptr == outParam)
    {
        return E_POINTER;
    }

    *outParam = inParam;
    return S_OK;
}

HRESULT Animals::AnimalFactory::GetOddSizedStructArray(
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_full_opt(*length) OddSizedStruct **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    int bytesToAllocate = 5 * sizeof(OddSizedStruct);
    *value = (OddSizedStruct *)CoTaskMemAlloc(bytesToAllocate);
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    OddSizedStruct oddSizedStruct;
    // write the values into value array
    for(size_t i = 0; i < 5; i++)
    {
        oddSizedStruct.Field0 = (byte)i;
        oddSizedStruct.Field1 = (byte)(i+50);
        oddSizedStruct.Field2 = (byte)(i+200);
        (*value)[i] = oddSizedStruct;
    }
    *length = 5;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutOddSizedStruct(__in OddSizedStruct inParam, __out OddSizedStruct *outParam, __in IDelegateOddSizedStruct *delegateIn)
{
    if (outParam == nullptr || delegateIn == nullptr)
    {
        return E_POINTER;
    }

    return delegateIn->Invoke(inParam, outParam);
}

HRESULT Animals::AnimalFactory::MarshalInAndOutSmallComplexStruct(__in SmallComplexStruct inParam, __out SmallComplexStruct *outParam)
{
    if (nullptr == outParam)
    {
        return E_POINTER;
    }

    *outParam = inParam;
    return S_OK;
}

HRESULT Animals::AnimalFactory::GetSmallComplexStructArray(
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_full_opt(*length) SmallComplexStruct **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    int bytesToAllocate = 5 * sizeof(SmallComplexStruct);
    *value = (SmallComplexStruct *)CoTaskMemAlloc(bytesToAllocate);
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    SmallComplexStruct smallComplexStruct;
    // write the values into value array
    for(size_t i = 0; i < 5; i++)
    {
        smallComplexStruct.Field0 = (byte)i;
        smallComplexStruct.Field1.Field0 = (byte)(i+50);
        smallComplexStruct.Field2 = (byte)(i+200);
        (*value)[i] = smallComplexStruct;
    }
    *length = 5;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutSmallComplexStruct(__in SmallComplexStruct inParam, __out SmallComplexStruct *outParam, __in IDelegateSmallComplexStruct *delegateIn)
{
    if (outParam == nullptr || delegateIn == nullptr)
    {
        return E_POINTER;
    }

    return delegateIn->Invoke(inParam, outParam);
}

HRESULT Animals::AnimalFactory::MarshalInAndOutBigComplexStruct(__in BigComplexStruct inParam, __out BigComplexStruct *outParam)
{
    if (nullptr == outParam)
    {
        return E_POINTER;
    }

    *outParam = inParam;
    return S_OK;
}

HRESULT Animals::AnimalFactory::GetBigComplexStructArray(
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_full_opt(*length) BigComplexStruct **value)
{
    if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    int bytesToAllocate = 5 * sizeof(BigComplexStruct);
    *value = (BigComplexStruct *)CoTaskMemAlloc(bytesToAllocate);
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    BigComplexStruct bigComplexStruct;
    // write the values into value array
    for(size_t i = 0; i < 5; i++)
    {
        bigComplexStruct.Field0 = (byte)i;
        
        bigComplexStruct.Field1.Field0 = (byte)(i+50);
        
        bigComplexStruct.Field2 = (byte)(i+200);
        
        bigComplexStruct.Field3.Field0 = false;
        bigComplexStruct.Field3.Field1 = true;
        bigComplexStruct.Field3.Field2 = false;
        bigComplexStruct.Field3.Field3 = true;
        
        bigComplexStruct.Field4.Field0 = (byte)(i+180);
        bigComplexStruct.Field4.Field1.Field0 = (byte)(i+150);
        bigComplexStruct.Field4.Field2 = (byte)(i+190);

        bigComplexStruct.Field5.Field0 = (byte)(i+80);
        bigComplexStruct.Field5.Field1.Field0 = (byte)(i+50);
        bigComplexStruct.Field5.Field2 = (byte)(i+90);

        bigComplexStruct.Field6 = (byte)(i+7);

        bigComplexStruct.Field7 = (int)(i+2000);

        (*value)[i] = bigComplexStruct;
    }
    *length = 5;

    return S_OK;
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutBigComplexStruct(__in BigComplexStruct inParam, __out BigComplexStruct *outParam, __in IDelegateBigComplexStruct *delegateIn)
{
    if (outParam == nullptr || delegateIn == nullptr)
    {
        return E_POINTER;
    }

    return delegateIn->Invoke(inParam, outParam);
}

HRESULT Animals::AnimalFactory::CallDelegateWithInFloat(__in IDelegateWithInParam_Float *inDelegate, __in float inValue)
{
    if (inDelegate == nullptr)
    {
        return E_POINTER;
    }

    return inDelegate->Invoke(inValue);
}

HRESULT Animals::AnimalFactory::CallDelegateWithOutFloat(__in IDelegateWithOutParam_Float *inDelegate, __out float *outValue)
{
    if (inDelegate == nullptr || outValue == nullptr)
    {
        return E_POINTER;
    }

    return inDelegate->Invoke(outValue);
}

HRESULT Animals::AnimalFactory::CallDelegateWithInOutFloat(__in IDelegateWithInOut_Float *inDelegate, __in int inValue1, __out float *outValue1, __in float inValue2, __in int inValue3, __in int inValue4, __in float inValue5, __out float *outValue2)
{
    if (inDelegate == nullptr || outValue1 == nullptr || outValue2 == nullptr)
    {
        return E_POINTER;
    }

    return inDelegate->Invoke(inValue1, outValue1, inValue2, inValue3, inValue4, inValue5, outValue2);
}    

HRESULT Animals::AnimalFactory::GetStringIntegerMap(__out Windows::Foundation::Collections::IMap<HSTRING, int> **outValue)
{
    IfNullReturnError(outValue, E_POINTER);
    *outValue = NULL;

    Microsoft::WRL::ComPtr<HashMap<HSTRING, int>> spMap;
    HRESULT hr = HashMap<HSTRING, int>::Make(&spMap); 
    IfFailedReturn(hr);

    boolean fReplaced;
    HSTRING hString = nullptr;

    WindowsCreateString(L"Hundred", 7, &hString);
    spMap->Insert(hString, 7, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"by", 2, &hString);
    spMap->Insert(hString, 2, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"Hundred And Fifty", 17, &hString);
    spMap->Insert(hString, 17, &fReplaced);
    WindowsDeleteString(hString);

    spMap.CopyTo(outValue);

    return hr;
}    

HRESULT Animals::AnimalFactory::GetObservableStringIntegerMap(__out Windows::Foundation::Collections::IObservableMap<HSTRING, int> **outValue)
{
    IfNullReturnError(outValue, E_POINTER);
    *outValue = NULL;

    Microsoft::WRL::ComPtr<ObservableHashMap<HSTRING, int>> spMap;
    HRESULT hr = ObservableHashMap<HSTRING, int>::Make(&spMap); 
    IfFailedReturn(hr);

    boolean fReplaced;
    HSTRING hString = nullptr;

    WindowsCreateString(L"Hundred", 7, &hString);
    spMap->Insert(hString, 100, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"Twenty", 6, &hString);
    spMap->Insert(hString, 20, &fReplaced);
    WindowsDeleteString(hString);

    WindowsCreateString(L"Five", 4, &hString);
    spMap->Insert(hString, 5, &fReplaced);
    WindowsDeleteString(hString);

    spMap.CopyTo(outValue);

    return hr;
}    

HRESULT Animals::AnimalFactory::GetDoubleObservableMap(__out IDoubleIObservableMap **outValue)
{
    IfNullReturnError(outValue, E_POINTER);
    *outValue = NULL;

    ComPtr<CIDoubleObservableMap> spDoubleObservableMap = Make<CIDoubleObservableMap>();

    spDoubleObservableMap.CopyTo(outValue);

    return S_OK;;
}    

HRESULT Animals::AnimalFactory::GetStringHiddenTypeMap(__out Windows::Foundation::Collections::IMap<HSTRING, IHiddenInterface *> **outValue, __out boolean *wasMethodCalled)
{
    IfNullReturnError(wasMethodCalled, E_POINTER);
    IfNullReturnError(outValue, E_POINTER);

    *wasMethodCalled = true;
    *outValue = nullptr;

    return S_OK;
}

HRESULT Animals::AnimalFactory::PassUInt8Array( 
            __in UINT32 length,
            __RPC__in_ecount_full(length) t_UInt8 *value,
            __out Windows::Foundation::Collections::IVector<t_UInt8> ** passedValuesVector)
{
    if (length != 0 && value == nullptr)
    {
        return E_POINTER;
    }

    *passedValuesVector = nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<t_UInt8>> sp;

    hr = Vector<t_UInt8>::Make(&sp);
    for (UINT32 i = 0; SUCCEEDED(hr) && i < length; i++)
    {
        hr = sp->Append(value[i]);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(passedValuesVector);
    }

    return hr;


}

HRESULT Animals::AnimalFactory::FillUInt8Array( 
            __in UINT32 length,
            __RPC__out_ecount_full(length) t_UInt8 *value,
            __in Windows::Foundation::Collections::IVector<t_UInt8> * fillFromVector)
{
    if (length != 0 && value == nullptr)
    {
        return E_POINTER;
    }

    UINT32 vectorLength = 0;
    HRESULT hr = S_OK;
    
    if (fillFromVector)
    {
        hr = fillFromVector->get_Size(&vectorLength);
    }
    
    for (UINT32 i = 0; SUCCEEDED(hr) && i < vectorLength && i < length; i++)
    {
        hr = fillFromVector->GetAt(i, &value[i]);
    }

    return hr;
}

HRESULT Animals::AnimalFactory::GetStaticAnimalAsInspectable(__out IInspectable **staticInspectableAnimal)
{
    return this->QueryInterface(__uuidof(IStaticAnimal2), (void **)staticInspectableAnimal);
}

HRESULT Animals::AnimalFactory::GetStaticAnimalAsStaticInterface(__out IStaticAnimal2 **staticAnimal)
{
    return this->QueryInterface(__uuidof(IStaticAnimal2), (void **)staticAnimal);
}


HRESULT Animals::AnimalFactory::TestDefaultDino(__in IDino *inValue, __out boolean *isSame)
{
    if (isSame == nullptr)
    {
        return E_POINTER;
    }

    if (inValue == nullptr)
    {
        *isSame = true;
        return S_OK;
    }

    IDino *interfacePtr = nullptr;
    HRESULT hr = inValue->QueryInterface(__uuidof(IDino), (void **)&interfacePtr);
    if (FAILED(hr))
    {
        return hr;
    }

    *isSame = (inValue == interfacePtr);
    return S_OK;

}

HRESULT Animals::AnimalFactory::TestDefaultFish(__in IFish *inValue, __out boolean *isSame)
{
    if (isSame == nullptr)
    {
        return E_POINTER;
    }

    if (inValue == nullptr)
    {
        *isSame = true;
        return S_OK;
    }

    IFish *interfacePtr = nullptr;
    HRESULT hr = inValue->QueryInterface(__uuidof(IFish), (void **)&interfacePtr);
    if (FAILED(hr))
    {
        return hr;
    }

    *isSame = (inValue == interfacePtr);
    return S_OK;
}

HRESULT Animals::AnimalFactory::TestDefaultAnimal(__in IAnimal *inValue, __out boolean *isSame)
{
    if (isSame == nullptr)
    {
        return E_POINTER;
    }

    if (inValue == nullptr)
    {
        *isSame = true;
        return S_OK;
    }

    IAnimal *interfacePtr = nullptr;
    HRESULT hr = inValue->QueryInterface(__uuidof(IAnimal), (void **)&interfacePtr);
    if (FAILED(hr))
    {
        return hr;
    }

    *isSame = (inValue == interfacePtr);
    return S_OK;
}

HRESULT Animals::AnimalFactory::TestDefaultMultipleIVector(__in Windows::Foundation::Collections::IVector<int> *inValue, __out boolean *isSame)
{
    if (isSame == nullptr)
    {
        return E_POINTER;
    }

    if (inValue == nullptr)
    {
        *isSame = true;
        return S_OK;
    }

    Windows::Foundation::Collections::IVector<int> *interfacePtr = nullptr;
    HRESULT hr = inValue->QueryInterface(__uuidof(Windows::Foundation::Collections::IVector<int>), (void **)&interfacePtr);
    if (FAILED(hr))
    {
        return hr;
    }

    *isSame = (inValue == interfacePtr);
    return S_OK;

}


HRESULT
Animals::AnimalServer::GetArrayHSTRING(__out Windows::Foundation::Collections::IVector<HSTRING> **outVector, __in int from, __in int to)
{
    // Make vector to send back so that we can check if returned array is marshalled correctly
    Microsoft::WRL::ComPtr<Vector<HSTRING>> sp;
    HRESULT hr = Vector<HSTRING>::Make(&sp);
    if (FAILED(hr))
    {
        return hr;
    }
    
    for (int i = from; SUCCEEDED(hr) && i < (int)m_arrayLengthHSTRING && i < to; i++)
    {
        hr = sp->Append(m_arrayHSTRING[i]);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outVector);
    }

    return hr;
}

HRESULT Animals::AnimalServer::PassArrayHSTRINGCore(__in UINT32 length, __RPC__in_ecount_part(length, lengthValue) HSTRING *value, __in UINT32 lengthValue)
{
    if (NULL == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue > length)
    {
        return E_INVALIDARG;
    }

    if (NULL != m_arrayHSTRING)
    {
        for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
        {
            WindowsDeleteString(m_arrayHSTRING[iIndex]);
        }

        // Clear the array
        CoTaskMemFree(m_arrayHSTRING);
        m_arrayHSTRING = nullptr;
        m_arrayLengthHSTRING = 0;
        m_arraySizeHSTRING = 0;
    }

    m_arraySizeHSTRING = length;
    if (m_arraySizeHSTRING > 0)
    {
        m_arrayHSTRING = (HSTRING*)CoTaskMemAlloc(m_arraySizeHSTRING * sizeof(HSTRING));
        if (m_arrayHSTRING == NULL)
        {
            return E_OUTOFMEMORY;
        }

        m_arrayLengthHSTRING = lengthValue;
        // read in the values
        for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
        {
            HSTRING item;
            WindowsDuplicateString(value[i], &item);
            m_arrayHSTRING[i] = item;
        }
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::PassArrayHSTRING(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) HSTRING *value,
    __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = PassArrayHSTRINGCore(length, value, length);
    IfFailedReturn(hr);

    return GetArrayHSTRING(outVector, 0, length);
}

HRESULT Animals::AnimalServer::PassArrayWithInLengthHSTRING(
    __in UINT32 length, 
    __RPC__in_ecount_part(length, lengthValue) HSTRING *value, 
    __in UINT32 lengthValue)
{
    return  PassArrayHSTRINGCore(length, value, lengthValue);
}
            
HRESULT Animals::AnimalServer::PassArrayWithOutLengthHSTRING( 
    __in UINT32 length,
    __RPC__in_ecount_part(length, *lengthValue) HSTRING *value,
    __RPC__out UINT32 *lengthValue)
{
    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }
    
    HRESULT hr =  PassArrayHSTRINGCore(length, value, length);
    IfFailedReturn(hr);

    *lengthValue = m_arrayLengthHSTRING;
    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::FillArrayHSTRING(
    __in UINT32 length, 
    __RPC__out_ecount_full(length) HSTRING *value,
    __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector)
{
    if (NULL == value && length != 0)
    {
        return E_POINTER;
    }

    if (outVector == NULL)
    {
        return E_POINTER;
    }

    // write the values into value array
    for(size_t i = 0; i < length && i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        value[i] = item;
    }

    for(size_t i = m_arrayLengthHSTRING; i < length; i++)
    {
        value[i] = NULL;
    }

    return GetArrayHSTRING(outVector, 0, length);
}

HRESULT Animals::AnimalServer::FillArrayWithInLengthHSTRING( 
    __in UINT32 length,
    __RPC__out_ecount_part(length, lengthValue) HSTRING *value,
    __in UINT32 lengthValue)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue > length)
    {
        return E_INVALIDARG;
    }

    // write the values into value array
    size_t i = 0;
    for(; i < lengthValue && i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        value[i] = item;
    }

    for(; i < lengthValue; i++)
    {
        value[i] = nullptr;
    }

    return S_OK;
}
            
HRESULT Animals::AnimalServer::FillArrayWithOutLengthHSTRING( 
    __in UINT32 length,
    __RPC__out_ecount_part(length, *lengthValue) HSTRING *value,
    __RPC__out UINT32 *lengthValue)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }

    *lengthValue = 0;

    // write the values into value array
    for(size_t i = 0; i < length && i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        value[i] = item;
        (*lengthValue)++;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::ReceiveArrayHSTRING(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) HSTRING **value,
    __out Windows::Foundation::Collections::IVector<HSTRING> ** outVector)
{
    if (NULL == length || NULL == value || outVector == NULL)
    {
        return E_POINTER;
    }

    *value = (HSTRING *)CoTaskMemAlloc(m_arraySizeHSTRING * sizeof(HSTRING));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        (*value)[i] = item;
    }

    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        (*value)[i] = nullptr;
    }

    *length = m_arraySizeHSTRING;

    return GetArrayHSTRING(outVector, 0, *length);
}

HRESULT Animals::AnimalServer::ReceiveArrayWithInLengthHSTRING( 
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_part_opt(*length, lengthValue) HSTRING **value,
    __in UINT32 lengthValue)
{
    if (nullptr == length || nullptr == value)
    {
        return E_POINTER;
    }

    UINT32 finalSize = (lengthValue > m_arraySizeHSTRING) ? lengthValue : m_arraySizeHSTRING;

    *value = (HSTRING *)CoTaskMemAlloc(finalSize * sizeof(HSTRING));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    size_t i = 0;
    for(; i < lengthValue && i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        (*value)[i] = item;
    }

    for (; i < lengthValue; i++)
    {
        (*value)[i] = nullptr;
    }

    *length = finalSize;
    return S_OK;
}
            
HRESULT Animals::AnimalServer::ReceiveArrayWithOutLengthHSTRING( 
    __RPC__out UINT32 *length,
    __RPC__deref_out_ecount_part_opt(*length, *lengthValue) HSTRING **value,
    __RPC__out UINT32 *lengthValue)
{
   if (NULL == length || NULL == value)
    {
        return E_POINTER;
    }

    if (lengthValue == nullptr)
    {
        return E_POINTER;
    }

    *value = (HSTRING *)CoTaskMemAlloc(m_arraySizeHSTRING * sizeof(HSTRING));
    if (*value == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        (*value)[i] = item;
    }
    *lengthValue = m_arrayLengthHSTRING;
    *length = m_arraySizeHSTRING;

    return S_OK;
}

HRESULT
Animals::AnimalServer::CallDelegatePassArrayHSTRING(__in IDelegateWithInParam_ArrayHSTRING* delegatePassArray)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = m_arrayLengthHSTRING;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        pArray[i] = item;
    }
    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        pArray[i] = nullptr;
    }

    HRESULT hr = delegatePassArray->Invoke(this, arraySize, pArray);

    for (int iIndex = 0; iIndex < (int)arrayLength; iIndex++)
    {
        WindowsDeleteString(pArray[iIndex]);
    }
    CoTaskMemFree(pArray);

    return hr;
}

HRESULT
Animals::AnimalServer::CallDelegateFillArrayHSTRING(__in IDelegateWithInOutParam_ArrayHSTRING* delegateFillArray)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateFillArray->Invoke(this, arraySize, pArray);

    if (SUCCEEDED(hr))
    {
        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arraySize;
        m_arraySizeHSTRING = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT
Animals::AnimalServer::CallDelegateReceiveArrayHSTRING(__in IDelegateWithOutParam_ArrayHSTRING* delegateReceiveArray)
{
    UINT32 arraySize = 0;
    HSTRING *pArray = NULL;

    HRESULT hr = delegateReceiveArray->Invoke(this, &arraySize, &pArray);
    if (SUCCEEDED(hr))
    {
        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arraySize;
        m_arraySizeHSTRING = arraySize;
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithInLengthHSTRING(__in IDelegatePassArrayWithInLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = m_arrayLengthHSTRING;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        pArray[i] = item;
    }
   
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, arrayLength);
    for (int iIndex = 0; iIndex < (int)arrayLength; iIndex++)
    {
        WindowsDeleteString(pArray[iIndex]);
    }
    CoTaskMemFree(pArray);

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLengthHSTRING(__in IDelegatePassArrayWithOutLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        pArray[i] = item;
    }
    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        pArray[i] = nullptr;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    for (int iIndex = 0; iIndex < (int)arrayLength; iIndex++)
    {
        WindowsDeleteString(pArray[iIndex]);
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLengthWithRetValLengthHSTRING(__in IDelegatePassArrayWithOutLengthWithRetValLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        pArray[i] = item;
    }
    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        pArray[i] = nullptr;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    for (int iIndex = 0; iIndex < (int)arrayLength; iIndex++)
    {
        WindowsDeleteString(pArray[iIndex]);
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING(__in IDelegatePassArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    for(size_t i = 0; i < m_arrayLengthHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        pArray[i] = item;
    }
    for (size_t i = m_arrayLengthHSTRING; i < m_arraySizeHSTRING; i++)
    {
        pArray[i] = nullptr;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength, randomRetVal);
    if (arrayLength > arraySize)
    {
        hr = E_UNEXPECTED;
    }
    for (int iIndex = 0; iIndex < (int)arrayLength; iIndex++)
    {
        WindowsDeleteString(pArray[iIndex]);
    }
    CoTaskMemFree(pArray);
    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithInLengthHSTRING(__in IDelegateFillArrayWithInLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = m_arrayLengthHSTRING;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = delegateIn->Invoke(arraySize, pArray, arrayLength);

    if (SUCCEEDED(hr))
    {
        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;
}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLengthHSTRING(__in IDelegateFillArrayWithOutLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLengthWithRetValLengthHSTRING(__in IDelegateFillArrayWithOutLengthWithRetValLengthHSTRING *delegateIn)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING(__in IDelegateFillArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal)
{
    UINT32 arraySize = m_arraySizeHSTRING;
    UINT32 arrayLength = 0;
    HSTRING *pArray = (HSTRING *)CoTaskMemAlloc(arraySize * sizeof(HSTRING));
    if (pArray == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = delegateIn->Invoke(arraySize, pArray, &arrayLength, randomRetVal);

    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            hr = E_UNEXPECTED;
            CoTaskMemFree(pArray);
            return hr;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }
    else
    {
        CoTaskMemFree(pArray);
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithInLengthHSTRING(__in IDelegateReceiveArrayWithInLengthHSTRING *delegateIn)
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = m_arrayLengthHSTRING;
    HSTRING *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }

        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLengthHSTRING(__in IDelegateReceiveArrayWithOutLengthHSTRING *delegateIn) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    HSTRING *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }


        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING(__in IDelegateReceiveArrayWithOutLengthWithRetValLengthHSTRING *delegateIn) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    HSTRING *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }


        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }

    return hr;

}

HRESULT Animals::AnimalServer::CallDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING(__in IDelegateReceiveArrayWithOutLengthWithRetValRandomParamHSTRING *delegateIn, __out int *randomRetVal) 
{
    UINT32 arraySize = 0;
    UINT32 arrayLength = 0;
    HSTRING *pArray = NULL;

    HRESULT hr = delegateIn->Invoke(&arraySize, &pArray, &arrayLength, randomRetVal);
    if (SUCCEEDED(hr))
    {
        if (arrayLength > arraySize)
        {
            return E_UNEXPECTED;
        }

        if (m_arrayHSTRING)
        {
            for (int iIndex = 0; iIndex < (int)m_arrayLengthHSTRING; iIndex++)
            {
                WindowsDeleteString(m_arrayHSTRING[iIndex]);
            }
            CoTaskMemFree(m_arrayHSTRING);
        }


        m_arrayHSTRING = pArray;
        m_arrayLengthHSTRING = arrayLength;
        m_arraySizeHSTRING = arraySize;
    }

    return hr;

}


MarshalMethodImp(Bool);
MarshalMethodImp(UInt8);
MarshalMethodImp(Int32);
MarshalMethodImp(UInt32);
MarshalMethodImp(Int64);
MarshalMethodImp(UInt64);
MarshalMethodImp(Single);
MarshalMethodImp(Double);
MarshalMethodImp(Char16);
IFACEMETHODIMP
Animals::AnimalServer::MarshalDimensions(Dimensions _in, __out Dimensions* _out)
{
    *_out = _in;
    return S_OK;
}
IFACEMETHODIMP
Animals::AnimalServer::MarshalOuterStruct(OuterStruct _in, __out OuterStruct* _out)
{
    *_out = _in;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::MarshalStudyInfo(StudyInfo _in, __out StudyInfo* _out)
{
    *_out = _in;
    WindowsDuplicateString(_in.StudyName,&_out->StudyName);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::VerifyMarshalGUID(HSTRING expected, GUID _in, __out GUID* _out)
{
    HRESULT hr = S_OK;
    LPOLESTR actual;

    UINT32 strLength;
    PCWSTR expectedStr = WindowsGetStringRawBuffer(expected, &strLength);

    hr = StringFromCLSID(_in, &actual);
    if(SUCCEEDED(hr))
    {
        if (wcscmp((wchar_t*)actual, (wchar_t*)expectedStr) != 0)
        {
            hr = E_FAIL;
        }
        CoTaskMemFree(actual);
    }

    *_out = _in;
    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::GetNULLHSTRING(__out HSTRING* _out)
{
    *_out = NULL;
    return S_OK;
}
//MIDLRT bug: Win8 112810
//MarshalMethodImp(Dimensions);
//MarshalMethodImp(OuterStruct);

IFACEMETHODIMP Animals::AnimalServer::CallDelegateWithOutParam_HSTRING( 
    Animals::IDelegateWithOutParam_HSTRING * onDelegateWithOutHSTRING, HSTRING *outParam)
{
    return onDelegateWithOutHSTRING->Invoke(this, outParam);
}


IFACEMETHODIMP Animals::AnimalServer::CallDelegateWithOutParam_int( 
    Animals::IDelegateWithOutParam_int * onDelegateWithOutint, int *outParam)
{
    return onDelegateWithOutint->Invoke(this, outParam);
}

IFACEMETHODIMP
Animals::AnimalServer::CallDelegateWithOutParam_Interface(
    Animals::IDelegateWithOutParam_Interface * onDelegateWithOutInterface,
    IAnimal **outParam) 
{ 
    return onDelegateWithOutInterface->Invoke(this, outParam);
} 

IFACEMETHODIMP
Animals::AnimalServer::CallDelegateWithOutParam_Struct(
    Animals::IDelegateWithOutParam_Struct * onDelegateWithOutStruct,
    Dimensions *outParam) 
{ 
    return onDelegateWithOutStruct->Invoke(this, outParam);
} 

IFACEMETHODIMP
Animals::AnimalServer::CallDelegateWithOutParam_InOutMixed(
    Animals::IDelegateWithOutParam_InOutMixed* onDelegateWithInOutMixed,
    Dimensions *outParam,
    int weight) 
{ 
    return onDelegateWithInOutMixed->Invoke(this, outParam, weight);
} 

IFACEMETHODIMP
Animals::AnimalServer::CallDelegateWithMultipleOutParams(
    Animals::IDelegateWithOutParam_MultipleOutParams* onDelegateWithMultipleOutParams,
    Names * names, 
    int *newWeight, 
    int weight, 
    IAnimal **outAnimal)
{
    return onDelegateWithMultipleOutParams->Invoke(this, names, newWeight, weight, outAnimal);
} 

IFACEMETHODIMP
Animals::AnimalServer::MarshalNullAsDelegate(
    Animals::IDelegateWithOutParam_HSTRING* inDelegate, 
    HSTRING *outMessage)
{
    if (inDelegate == NULL)
    {
        WindowsCreateString(L"Success", 7, outMessage);
        return S_OK;
    }
    else
    {
        WindowsCreateString(L"Fail", 5, outMessage);
        return S_OK;
    }
}

IFACEMETHODIMP
Animals::AnimalServer::MethodDelegateAsOutParam(
    Animals::IDelegateWithOutParam_HSTRING* inDelegate, 
    Animals::IDelegateWithOutParam_HSTRING** outDelegate)
{
    if (outDelegate == NULL)
    {
        return E_POINTER;
    }

    if (inDelegate != NULL)
    {
        // Invoke the in Delegate
        HSTRING hString;
        HRESULT hr = inDelegate->Invoke(this, &hString);
        if (SUCCEEDED(hr))
        {
            WindowsDeleteString(hString);
        }

        // Set the out delegate 
        *outDelegate = inDelegate;
        (*outDelegate)->AddRef();
    }
    else
    {
        *outDelegate = NULL;
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::GetNativeDelegateAsOutParam(
    Animals::IDelegateWithOutParam_HSTRING **outDelegate) 
{
    if (outDelegate == NULL)
    {
        return E_POINTER;
    }

    ComPtr<IDelegateWithOutParam_HSTRING> spListener = Make<AnimalDelegateWithOutParam_HSTRING>();
    spListener.CopyTo(outDelegate);

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateIterable(
    __in Windows::Foundation::Collections::IIterable<int> *inIterable,
    __out Windows::Foundation::Collections::IIterable<int> **outIterable)
{
    if (outIterable == NULL)
    {
        return E_POINTER;
    }

    if (inIterable == NULL)
    {
        *outIterable = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IIterable<int>> spDuplicate = Make<AnimalIIterable_int>(inIterable, this);
    spDuplicate.CopyTo(outIterable);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateStringIterable(
    __in Windows::Foundation::Collections::IIterable<HSTRING> *inIterable,
    __out Windows::Foundation::Collections::IIterable<HSTRING> **outIterable)
{
    if (outIterable == NULL)
    {
        return E_POINTER;
    }

    if (inIterable == NULL)
    {
        *outIterable = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IIterable<HSTRING>> spDuplicate = Make<AnimalIIterable_HSTRING>(inIterable, this);
    spDuplicate.CopyTo(outIterable);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateIterator(
    __in Windows::Foundation::Collections::IIterator<int> *inIterator,
    __out Windows::Foundation::Collections::IIterator<int> **outIterator)
{
    if (outIterator == NULL)
    {
        return E_POINTER;
    }

    if (inIterator == NULL)
    {
        *outIterator = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IIterator<int>> spDuplicate = Make<AnimalIIterator_int>(inIterator);
    spDuplicate.CopyTo(outIterator);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateStringIterator(
    __in Windows::Foundation::Collections::IIterator<HSTRING> *inIterator,
    __out Windows::Foundation::Collections::IIterator<HSTRING> **outIterator)
{
    if (outIterator == NULL)
    {
        return E_POINTER;
    }

    if (inIterator == NULL)
    {
        *outIterator = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IIterator<HSTRING>> spDuplicate = Make<AnimalIIterator_HSTRING>(inIterator);
    spDuplicate.CopyTo(outIterator);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateVectorView(
    __in Windows::Foundation::Collections::IVectorView<int> *inVectorView,
    __out Windows::Foundation::Collections::IVectorView<int> **outVectorView)
{
    if (outVectorView == NULL)
    {
        return E_POINTER;
    }

    if (inVectorView == NULL)
    {
        *outVectorView = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IVectorView<int>> spDuplicate = Make<AnimalIVectorView_int>(inVectorView, this);
    spDuplicate.CopyTo(outVectorView);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateStringVectorView(
    __in Windows::Foundation::Collections::IVectorView<HSTRING> *inVectorView,
    __out Windows::Foundation::Collections::IVectorView<HSTRING> **outVectorView)
{
    if (outVectorView == NULL)
    {
        return E_POINTER;
    }

    if (inVectorView == NULL)
    {
        *outVectorView = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IVectorView<HSTRING>> spDuplicate = Make<AnimalIVectorView_HSTRING>(inVectorView, this);
    spDuplicate.CopyTo(outVectorView);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateVector(
    __in Windows::Foundation::Collections::IVector<int> *inVector,
    __out Windows::Foundation::Collections::IVector<int> **outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    if (inVector == NULL)
    {
        *outVector = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IVector<int>> spDuplicate = Make<AnimalIVector_int>(inVector, this);
    spDuplicate.CopyTo(outVector);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::GetReadOnlyVector(
    __in Windows::Foundation::Collections::IVector<int> *inVector,
    __out Windows::Foundation::Collections::IVector<int> **outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    if (inVector == NULL)
    {
        *outVector = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IVector<int>> spDuplicate = Make<AnimalReadOnlyVector_int>(inVector, this);
    spDuplicate.CopyTo(outVector);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::DuplicateStringVector(
    __in Windows::Foundation::Collections::IVector<HSTRING> *inVector,
    __out Windows::Foundation::Collections::IVector<HSTRING> **outVector)
{
    if (outVector == NULL)
    {
        return E_POINTER;
    }

    if (inVector == NULL)
    {
        *outVector = NULL;
        return S_OK;
    }

    ComPtr<Windows::Foundation::Collections::IVector<HSTRING>> spDuplicate = Make<AnimalIVector_HSTRING>(inVector, this);
    spDuplicate.CopyTo(outVector);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameIterable(
    __in Windows::Foundation::Collections::IIterable<int> *inIterable,
    __out Windows::Foundation::Collections::IIterable<int> **outIterable)
{
    SendBackSameInterface(inIterable, outIterable);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameStringIterable(
    __in Windows::Foundation::Collections::IIterable<HSTRING> *inIterable,
    __out Windows::Foundation::Collections::IIterable<HSTRING> **outIterable)
{
    SendBackSameInterface(inIterable, outIterable);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameIterator(
    __in Windows::Foundation::Collections::IIterator<int> *inIterator,
    __out Windows::Foundation::Collections::IIterator<int> **outIterator)
{
    SendBackSameInterface(inIterator, outIterator);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameStringIterator(
    __in Windows::Foundation::Collections::IIterator<HSTRING> *inIterator,
    __out Windows::Foundation::Collections::IIterator<HSTRING> **outIterator)
{
    SendBackSameInterface(inIterator, outIterator);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameVectorView(
    __in Windows::Foundation::Collections::IVectorView<int> *inVectorView,
    __out Windows::Foundation::Collections::IVectorView<int> **outVectorView)
{
    SendBackSameInterface(inVectorView, outVectorView);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameStringVectorView(
    __in Windows::Foundation::Collections::IVectorView<HSTRING> *inVectorView,
    __out Windows::Foundation::Collections::IVectorView<HSTRING> **outVectorView)
{
    SendBackSameInterface(inVectorView, outVectorView);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameVector(
    __in Windows::Foundation::Collections::IVector<int> *inVector,
    __out Windows::Foundation::Collections::IVector<int> **outVector)
{
    SendBackSameInterface(inVector, outVector);
}

IFACEMETHODIMP
Animals::AnimalServer::SendBackSameStringVector(
    __in Windows::Foundation::Collections::IVector<HSTRING> *inVector,
    __out Windows::Foundation::Collections::IVector<HSTRING> **outVector)
{
    SendBackSameInterface(inVector, outVector);
}

IFACEMETHODIMP
Animals::AnimalServer::GetObservableVector(__out Windows::Foundation::Collections::IObservableVector<int> **outObservableVector)
{
    if (NULL == outObservableVector)
    {
        return E_POINTER;
    }

    *outObservableVector = NULL;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<ObservableVector<int>> sp;

    hr = ObservableVector<int>::Make(&sp);
    for (int i = 1; SUCCEEDED(hr) && i < 10; i++)
    {
        hr = sp->Append(i);
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(outObservableVector);
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalServer::GetObservableStringVector(__out Windows::Foundation::Collections::IObservableVector<HSTRING> **outObservableVector)
{
    if (NULL == outObservableVector)
    {
        return E_POINTER;
    }

    *outObservableVector = NULL;

    Microsoft::WRL::ComPtr<ObservableVector<HSTRING>> sp;

    IfFailedReturn(ObservableVector<HSTRING>::Make(&sp));

    auto spAppend = [&](PCNZWCH str) -> HRESULT{
        HSTRING sValue;
        IfFailedReturn(WindowsCreateString(str, (UINT32)wcslen(str), &sValue));
        HRESULT hr = sp->Append(sValue);
        IfFailedReturn(WindowsDeleteString(sValue));
        return hr;
    };

    IfFailedReturn(spAppend(L"Blue"));
    IfFailedReturn(spAppend(L"Red"));
    IfFailedReturn(spAppend(L"Yellow"));
    IfFailedReturn(spAppend(L"Green"));
    IfFailedReturn(spAppend(L"Pink"));
    IfFailedReturn(spAppend(L"Black"));
    IfFailedReturn(spAppend(L"White"));
    IfFailedReturn(spAppend(L"Tan"));
    IfFailedReturn(spAppend(L"Magenta"));
    IfFailedReturn(spAppend(L"Orange"));

    IfFailedReturn(sp.CopyTo(outObservableVector));

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterable_int::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IIterable<int>);
    *iidCount = 1;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterable_int::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IIterable`1<Int32>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterable_int::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pIterable->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIIterable_int::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterator<int> *pFirst = NULL;
    HRESULT hr = m_pIterable->First(&pFirst);
    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIIterable_HSTRING::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IIterable<HSTRING>);
    *iidCount = 1;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterable_HSTRING::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IIterable`1<String>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterable_HSTRING::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pIterable->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIIterable_HSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterator<HSTRING> *pFirst = NULL;
    HRESULT hr = m_pIterable->First(&pFirst);
    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateStringIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IIterator<int>);
    *iidCount = 1;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IIterator`1<Int32>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pIterator->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::get_Current(__out int * current)
{
    return m_pIterator->get_Current(current);
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::get_HasCurrent(__RPC__out boolean *hasCurrent)
{
    return m_pIterator->get_HasCurrent(hasCurrent);
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::MoveNext(__RPC__out boolean *hasCurrent)
{
    return m_pIterator->MoveNext(hasCurrent);
}

IFACEMETHODIMP
Animals::AnimalIIterator_int::GetMany(__in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pIterator->GetMany(capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IIterator<HSTRING>);
    *iidCount = 1;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IIterator`1<String>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pIterator->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::get_Current(__out HSTRING * current)
{
    return m_pIterator->get_Current(current);
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::get_HasCurrent(__RPC__out boolean *hasCurrent)
{
    return m_pIterator->get_HasCurrent(hasCurrent);
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::MoveNext(__RPC__out boolean *hasCurrent)
{
    return m_pIterator->MoveNext(hasCurrent);
}

IFACEMETHODIMP
Animals::AnimalIIterator_HSTRING::GetMany(__in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pIterator->GetMany(capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IVectorView<int>);
    (*iids)[1] = __uuidof(Windows::Foundation::Collections::IIterable<int>);
    *iidCount = 2;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IVectorView`1<Int32>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pVectorView->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVectorView->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    if (FAILED(hr))
    {
        return hr;
    }

    Windows::Foundation::Collections::IIterator<int> *pFirst = NULL;
    hr = pIIterable->First(&pFirst);
    pIIterable->Release();

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::GetAt(__in unsigned index, __out int *item)
{
    return m_pVectorView->GetAt(index, item);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::get_Size(__out unsigned *size)
{
    return m_pVectorView->get_Size(size);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVectorView->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_int::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVectorView->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IVectorView<HSTRING>);
    (*iids)[1] = __uuidof(Windows::Foundation::Collections::IIterable<HSTRING>);
    *iidCount = 2;
    return S_OK;

}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IVectorView`1<String>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pVectorView->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVectorView->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    if (FAILED(hr))
    {
        return hr;
    }

    Windows::Foundation::Collections::IIterator<HSTRING> *pFirst = NULL;
    hr = pIIterable->First(&pFirst);
    pIIterable->Release();

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateStringIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::GetAt(__in unsigned index, __out HSTRING *item)
{
    return m_pVectorView->GetAt(index, item);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::get_Size(__out unsigned *size)
{
    return m_pVectorView->get_Size(size);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::IndexOf(__in_opt HSTRING value, __out unsigned *index, __out boolean *found)
{
    return m_pVectorView->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::AnimalIVectorView_HSTRING::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pVectorView->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IVector<int>);
    (*iids)[1] = __uuidof(Windows::Foundation::Collections::IIterable<int>);
    *iidCount = 2;
    return S_OK;

}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IVector`1<Int32>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pVector->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    if (FAILED(hr))
    {
        return hr;
    }

    Windows::Foundation::Collections::IIterator<int> *pFirst = NULL;
    hr = pIIterable->First(&pFirst);
    pIIterable->Release();

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    Windows::Foundation::Collections::IVectorView<int> *pReturnValue = NULL;
    HRESULT hr = m_pVector->GetView(&pReturnValue);

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateVectorView(pReturnValue, returnValue);
        pReturnValue->Release();
    }
    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVector_int::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::AnimalIVector_int::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::AnimalIVector_int::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIVector_int::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) int *value)
{
    return m_pVector->ReplaceAll(count, value);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IVector<HSTRING>);
    (*iids)[1] = __uuidof(Windows::Foundation::Collections::IIterable<HSTRING>);
    *iidCount = 2;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IVector`1<String>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pVector->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::First(__out Windows::Foundation::Collections::IIterator<HSTRING> **first)
{
    Windows::Foundation::Collections::IIterable<HSTRING> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<HSTRING>), (void **)&pIIterable);
    if (FAILED(hr))
    {
        return hr;
    }

    Windows::Foundation::Collections::IIterator<HSTRING> *pFirst = NULL;
    hr = pIIterable->First(&pFirst);
    pIIterable->Release();

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateStringIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetAt(__in unsigned index, __out HSTRING *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::IndexOf(__in_opt HSTRING value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetView(__deref_out_opt IVectorView<HSTRING> **returnValue)
{
    Windows::Foundation::Collections::IVectorView<HSTRING> *pReturnValue = NULL;
    HRESULT hr = m_pVector->GetView(&pReturnValue);

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateStringVectorView(pReturnValue, returnValue);
        pReturnValue->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::SetAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::InsertAt(__in unsigned index, __in_opt HSTRING value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::Append(__in_opt HSTRING value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::Clear()
{
    return m_pVector->Clear();
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) HSTRING *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalIVector_HSTRING::ReplaceAll(unsigned int count, __RPC__in_ecount_full(count) HSTRING *value)
{
    return m_pVector->ReplaceAll(count, value);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
{
    IfNullReturnError(iids, E_POINTER);
    IfNullReturnError(iidCount, E_POINTER);
    *iids = (IID *)CoTaskMemAlloc(sizeof(IID) * 2);
    IfNullReturnError(*iids, E_OUTOFMEMORY);

    (*iids)[0] = __uuidof(Windows::Foundation::Collections::IVector<int>);
    (*iids)[1] = __uuidof(Windows::Foundation::Collections::IIterable<int>);
    *iidCount = 2;
    return S_OK;

}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
{
    const wchar_t *name = L"Windows.Foundation.Collections.IVector`1<Int32>";
    IfNullReturnError(className, E_POINTER);
    WindowsCreateString(name, (UINT32)wcslen(name), className);
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
{
    return m_pVector->GetTrustLevel(trustLevel);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    if (FAILED(hr))
    {
        return hr;
    }

    Windows::Foundation::Collections::IIterator<int> *pFirst = NULL;
    hr = pIIterable->First(&pFirst);
    pIIterable->Release();

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateIterator(pFirst, first);
        pFirst->Release();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    Windows::Foundation::Collections::IVectorView<int> *pReturnValue = NULL;
    HRESULT hr = m_pVector->GetView(&pReturnValue);

    if (SUCCEEDED(hr))
    {
        hr = m_pAnimal->DuplicateVectorView(pReturnValue, returnValue);
        pReturnValue->Release();
    }
    return hr;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::SetAt(__in unsigned , __in_opt int )
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::InsertAt(__in unsigned , __in_opt int )
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::RemoveAt(__in unsigned )
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::Append(__in_opt int )
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::RemoveAtEnd()
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::Clear()
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::GetMany(__in unsigned int startIndex, __in unsigned int capacity, __RPC__out_ecount_part(capacity, *actual) int *items, __RPC__out unsigned int *actual)
{
    return m_pVector->GetMany(startIndex, capacity, items, actual);
}

IFACEMETHODIMP
Animals::AnimalReadOnlyVector_int::ReplaceAll(unsigned int , __RPC__in_ecount_full(count) int *)
{
    return E_ILLEGAL_STATE_CHANGE;
}

IFACEMETHODIMP
Animals::AnimalFactory::AnimalBornWithWeight(
    __in IAnimal* mother, 
    __in int weight, 
    __deref_out IAnimal** ppAnimal)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->put_Mother(mother);
        spAnimal->put_Weight(weight);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::AnimalBornWithStats(
    __in IAnimal* mother, 
    __in int weight, 
    __in int legs1, 
    __in int legs2, 
    __in int legs3, 
    __deref_out IAnimal** ppAnimal)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;
    int numberOfLegs = legs1 + legs2 + legs3;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->put_Mother(mother);
        spAnimal->put_Weight(weight);
        spAnimal->SetNumLegs(numberOfLegs);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;
    *ppInspectable = nullptr;

    ComPtr<AnimalServer> spObj = Make<AnimalServer>();

    hr = spObj.As(&spIAnimal);
    if (SUCCEEDED(hr))
    {
        *ppInspectable = spIAnimal.Detach();
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::CreateWithLegs(
    __in int numberOfLegs,
    __deref_out IAnimal **ppAnimal)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->SetNumLegs(numberOfLegs);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::CreateWithMoreLegs(
    __in int legs1, 
    __in int legs2, 
    __in int legs3, 
    __deref_out IAnimal** ppAnimal)
{
    HRESULT hr = S_OK;

    if(*ppAnimal != nullptr)
    {
        return E_INVALIDARG;
    }

    ComPtr<IAnimal> spIAnimal;
    int numberOfLegs = legs1 + legs2 + legs3;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->SetNumLegs(numberOfLegs);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::CreateWithEvenMoreLegs(
    __in int legs1, 
    __in int legs2, 
    __in int legs3, 
    __in int legs4, 
    __in int legs5, 
    __in int legs6, 
    __deref_out IAnimal** ppAnimal)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;
    int numberOfLegs = legs1 + legs2 + legs3 + legs4 + legs5 + legs6;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->SetNumLegs(numberOfLegs);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::CreateWithMostLegs(
    __in int legs1, 
    __in int legs2, 
    __in int legs3, 
    __in int legs4, 
    __in int legs5, 
    __in int legs6, 
    __in int legs7, 
    __deref_out IAnimal** ppAnimal)
{
    HRESULT hr = S_OK;
    ComPtr<IAnimal> spIAnimal;
    int numberOfLegs = legs1 + legs2 + legs3 + legs4 + legs5 + legs6 + legs7;

    if (ppAnimal != nullptr)
    {
        ComPtr<AnimalServer> spAnimal = Make<AnimalServer>();
        spAnimal->SetNumLegs(numberOfLegs);
        hr = spAnimal.As(&spIAnimal);
        if (SUCCEEDED(hr))
        {
            *ppAnimal = spIAnimal.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// IStaticAnimal
IFACEMETHODIMP
Animals::AnimalFactory::GetAnswer(
    __out int* answer)
{
    *answer = 42;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::get_IsLovable(
    __out boolean* answer)
{
    *answer = m_isLovable;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::put_IsLovable(
    __in boolean answer)
{
    m_isLovable = answer;
    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::TakeANap(__in int numberOfMinutes, __out boolean* isDreaming)
{
    if (numberOfMinutes <= 0)
    {
        return E_INVALIDARG;
    }
    else
    {
        if (numberOfMinutes > 10)
        {
            *isDreaming = true;
        }
        else
        {
            *isDreaming = false;
        }
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::DinoMarshalAs(__deref_out IDino** dino)
{
    HRESULT hr = S_OK;
    ComPtr<IDino> spIDino;

    if (dino != nullptr)
    {
        ComPtr<DinoServer> spDino = Make<DinoServer>();
        hr = spDino.As(&spIDino);
        if (SUCCEEDED(hr))
        {
            *dino = spIDino.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::DinoDefault(__deref_out IDino** dino)
{
    HRESULT hr = S_OK;
    ComPtr<IDino> spIDino;

    if (dino != nullptr)
    {
        ComPtr<DinoServer> spDino = Make<DinoServer>();
        hr = spDino.As(&spIDino);
        if (SUCCEEDED(hr))
        {
            *dino = spIDino.Detach();
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


IFACEMETHODIMP
Animals::AnimalFactory::DinoDefaultVector(__deref_out Windows::Foundation::Collections::IVector<Dino*>** dino)
{
    if (NULL == dino)
    {
        return E_POINTER;
    }

    *dino = NULL;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<Vector<Dino*>> sp;

    hr = Vector<Dino*>::Make(&sp);
    
    for (int i = 0; SUCCEEDED(hr) && i < 2; i++)
    {
        ComPtr<DinoServer> spDino = Make<DinoServer>();
        sp->Append(spDino.Get());
    }

    if (SUCCEEDED(hr))
    {
        sp.CopyTo(dino);
    }

    return hr;
}

// IStaticAnimal2
IFACEMETHODIMP
Animals::AnimalFactory::GetCLSID(__out GUID* clsid)
{
    HRESULT hr = S_OK;

    if ((clsid->Data1 != NULL) || (clsid->Data2 != NULL) || (clsid->Data3 != NULL))
    {
        return E_INVALIDARG;
    }
    for (int i = 0; i < 8; i++)
    {
        if (clsid->Data4[i] != NULL)
        {
            return E_INVALIDARG;
        }
    }

    LPCOLESTR string = L"{EB561C4D-2526-4A9E-94D3-4743A5EB658B}";

    hr = CLSIDFromString(string, (LPCLSID)clsid);

    return hr;
}

IFACEMETHODIMP
Animals::AnimalFactory::MultiplyNumbers(__in int value1, __in int value2, __out int* result)
{
    if (NULL == result)
    {
        return E_POINTER;
    }

    *result = value1*value2;

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::GetRefCount(__in IInspectable *inValue, __out t_UInt64 *refCount)
{
    if (inValue == NULL)
    {
        *refCount = 0;
        return S_OK;
    }

    inValue->AddRef();
    // Assume that one addref was done to give us the input
    *refCount = inValue->Release() - 1;

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::get_MyFishRefCount(__out t_UInt64 *refCount)
{
    if (m_Fish == NULL)
    {
        *refCount = 0;
        return S_OK;
    }

    m_Fish->AddRef();
    *refCount = m_Fish->Release();

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::get_MyToasterRefCount(__out t_UInt64 *refCount)
{
    if (m_Toaster == NULL)
    {
        *refCount = 0;
        return S_OK;
    }

    m_Toaster->AddRef();
    *refCount = m_Toaster->Release();

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::put_MyStaticArrayProp(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) int *value)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (m_array)
    {
        CoTaskMemFree(m_array);
        m_array = nullptr;
        m_arraySize = 0;
    }
    
    if (length > 0)
    {
        m_array = (int*)CoTaskMemAlloc(length * sizeof(int));
        if (m_array == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        // read in the values
        m_arraySize = length;
        for(size_t i = 0; i < m_arraySize; i++)
        {
            m_array[i] = *value;
            value += 1;
        }
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::get_MyStaticArrayProp(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) int **value)
{
    if (nullptr == length || nullptr == value)
    {
        return E_POINTER;
    }

    *value = (int *)CoTaskMemAlloc(m_arraySize * sizeof(int));
    if (*value == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arraySize; i++)
    {
        (*value)[i] = m_array[i];
    }
    *length = m_arraySize;

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::put_MyStaticArrayPropHSTRING(
    __in UINT32 length,  
    __RPC__in_ecount_full(length) HSTRING *value)
{
    if (nullptr == value && length != 0)
    {
        return E_POINTER;
    }

    if (NULL != m_arrayHSTRING)
    {
        for (int iIndex = 0; iIndex < (int)m_arraySizeHSTRING; iIndex++)
        {
            WindowsDeleteString(m_arrayHSTRING[iIndex]);
        }

        // Clear the array
        CoTaskMemFree(m_arrayHSTRING);
        m_arrayHSTRING = nullptr;
        m_arraySizeHSTRING = 0;
    }

    if (length > 0)
    {
        m_arrayHSTRING = (HSTRING*)CoTaskMemAlloc(length * sizeof(HSTRING));
        if (m_arrayHSTRING == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        // read in the values
        m_arraySizeHSTRING = length;
        for(size_t i = 0; i < m_arraySizeHSTRING; i++)
        {
            HSTRING item;
            WindowsDuplicateString(value[i], &item);
            m_arrayHSTRING[i] = item;
        }
    }

    return S_OK;
}

IFACEMETHODIMP
Animals::AnimalFactory::get_MyStaticArrayPropHSTRING(
    __RPC__out UINT32 *length, 
    __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
{
    if (nullptr == length || nullptr == value)
    {
        return E_POINTER;
    }

    *value = (HSTRING *)CoTaskMemAlloc(m_arraySizeHSTRING * sizeof(HSTRING));
    if (*value == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // write the values into value array
    for(size_t i = 0; i < m_arraySizeHSTRING; i++)
    {
        HSTRING item;
        WindowsDuplicateString(m_arrayHSTRING[i], &item);
        (*value)[i] = item;
    }

    *length = m_arraySizeHSTRING;
    return S_OK;
}

// IActivateInstance
IFACEMETHODIMP
Animals::PomapoodleFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    // Block ability to provide an instance.
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

// IStaticPuppy
IFACEMETHODIMP
Animals::PomapoodleFactory::EatCookies(__in int numberOfCookies, __out int* cookiesEaten)
{
    if (numberOfCookies <= 0)
    {
        return E_INVALIDARG;
    }

    *cookiesEaten = numberOfCookies-1;

    _evtCookiesEaten.InvokeAll(nullptr, *cookiesEaten);

    return S_OK;
}

IFACEMETHODIMP
Animals::PomapoodleFactory::add_CookiesEatenEvent(
    __in Animals::ICookiesEatenHandler *clickHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtCookiesEaten.Add(clickHandler, pCookie);
}
                
IFACEMETHODIMP
Animals::PomapoodleFactory::remove_CookiesEatenEvent(
    __in EventRegistrationToken iCookie)
{
    return _evtCookiesEaten.Remove(iCookie);
}

IFACEMETHODIMP
Animals::AnimalDelegateWithOutParam_HSTRING::Invoke(
    Animals::IAnimal* sender, 
    __out HSTRING *outParam)
{
    if (sender == NULL)
    {
        return E_INVALIDARG;
    }

    Animals::Names names;
    HRESULT hr = sender->GetNames(&names);
    IfFailedReturn(hr);

    *outParam = names.Common;

    WindowsDeleteString(names.Scientific);
    WindowsDeleteString(names.AlsoKnownAs);

    return hr;
}
