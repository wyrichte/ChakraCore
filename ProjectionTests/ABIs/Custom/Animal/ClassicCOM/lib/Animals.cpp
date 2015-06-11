#include "stdafx.h"
#include "Animals.h"
#include <wrl\dllexports.h>
#include <assert.h>
#include <stdio.h>
#define MarshalMethodImp(type) IFACEMETHODIMP Animals::AnimalServer::Marshal##type(t_##type _in, __out t_##type* _out) \
{ \
    *_out = _in; \
    return S_OK; \
}

#define IIMockImp(klass) \
    IFACEMETHODIMP klass::GetIids() \
    { \
        return E_NOTIMPL; \
    } \
    IFACEMETHODIMP klass::GetRuntimeClassName() \
    { \
        return E_NOTIMPL; \
    } \
    IFACEMETHODIMP klass::GetTrustLevel() \
    { \
        return E_NOTIMPL; \
    } \

using namespace Microsoft::WRL;

IIMockImp(Animals::AnimalFactory);

IFACEMETHODIMP Animals::AnimalFactory::Create(__deref_out IAnimal **ppObj)
{
    wprintf(L"In AnimalFactory::Create\n");

    HRESULT hr = S_OK;

    if(ppObj != nullptr)
    {
        ComPtr<AnimalServer> spObj = Make<AnimalServer>();
        *ppObj = spObj.Detach();
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

Animals::AnimalServer::AnimalServer() : m_Weight(50)
{
    InnerStruct s;
    s.a = 100;
    
    m_Dimensions.Length = 180;
    m_Dimensions.Width = 360;
    m_OuterStruct.Inner = s;
}

IIMockImp(Animals::AnimalServer);
IFACEMETHODIMP Animals::AnimalServer::GetDimensions(__out Dimensions *dimensions)
{
    *dimensions = m_Dimensions;
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::get_Weight(__out int* _weight)
{
    *_weight = m_Weight;
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::put_Weight(__in int _weight)
{
    if (_weight < 110)
    {
        m_Weight = _weight;
        return S_OK;
    }
    return E_FAIL;
}

IFACEMETHODIMP Animals::AnimalServer::GetNumLegs(__out int* numberOfLegs)
{
    *numberOfLegs=100; //centipede!
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::AddInts(int val1, int val2, __out int* result)
{
    *result = val1 + val2;
    return S_OK;
}

IFACEMETHODIMP Animals::AnimalServer::GetOuterStruct(__out OuterStruct* strct)
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

MarshalMethodImp(Bool);
MarshalMethodImp(UInt8);
MarshalMethodImp(Int32);
MarshalMethodImp(UInt32);
MarshalMethodImp(Int64);
MarshalMethodImp(UInt64);
MarshalMethodImp(Single);
MarshalMethodImp(Double);
MarshalMethodImp(Char16);
MarshalMethodImp(Dimensions);
MarshalMethodImp(OuterStruct);

Animals::FishServer::FishServer() : m_NumFins(5)
{}

IFACEMETHODIMP Animals::FishServer::GetNumFins(__out int* numberOfFins)
{
    *numberOfFins = m_NumFins;
    return S_OK;
}

IIMockImp(Animals::FishServer);

Animals::DinoServer::DinoServer() : m_canRoar(true)
{}

IIMockImp(Animals::DinoServer);

IFACEMETHODIMP Animals::DinoServer::CanRoar(__out boolean* result)
{
    *result = m_canRoar;
    return S_OK;
}

IFACEMETHODIMP Animals::DinoServer::Roar(int numtimes)
{
    for(int i = 0; i < numtimes; i++)
    {
        wprintf(L"Roar!\n");
    }
    return S_OK;
}

IFACEMETHODIMP Animals::DinoServer::IsExtinct(__out boolean* result)
{
    *result = true;
    return S_OK;
}

namespace Animals
{
    //TODO: Should AnimalServer be CoCreatable?
    CoCreatableClass(AnimalServer);
    CoCreatableClass(AnimalFactory);
    CoCreatableClass(FishServer);
    CoCreatableClass(DinoServer);
}
