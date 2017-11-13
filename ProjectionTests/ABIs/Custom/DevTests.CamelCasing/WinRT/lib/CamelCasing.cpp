#include "stdafx.h"
#include "StringVariations.h"
#include "SimpleNameCollisions.h"
#include "CrossMemberCollisions.h"

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::WRL;

template<class E, class T>
HRESULT InvokeAll(E& e, T t, const wchar_t *str){
    Windows::Internal::String string;
    IfFailedReturn(string.Initialize(str));
    e.InvokeAll(t, string.Get());
    return S_OK;
}

IFACEMETHODIMP
DevTests::CamelCasing::StringVariationsFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    HRESULT hr = S_OK;
    *ppInspectable = nullptr;
    ComPtr<ICasing> spICasing; 
        
    ComPtr<StringVariationsServer> spObj = Make<StringVariationsServer>();

    if (spObj != nullptr)
    {
        hr = spObj.As(&spICasing);
        if (SUCCEEDED(hr))
        {
            *ppInspectable = spICasing.Detach();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::OverloadStringVariationsFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    HRESULT hr = S_OK;
    *ppInspectable = nullptr;
    ComPtr<IOverloadCasing> spIOverloadCasing; 
        
    ComPtr<OverloadStringVariationsServer> spObj = Make<OverloadStringVariationsServer>();

    if (spObj != nullptr)
    {
        hr = spObj.As(&spIOverloadCasing);
        if (SUCCEEDED(hr))
        {
            *ppInspectable = spIOverloadCasing.Detach();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::StringVariationsFactory::StaticFireEvent(__in StaticStringVariationsEvent evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::StaticStringVariationsEvent_PascalStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtPascalStatic, this, L"PascalStaticEvent"));
            break;
        }
    case DevTests::CamelCasing::StaticStringVariationsEvent_UPPERCASESTATICEVENT:
        {
            IfFailedReturn(InvokeAll(_evtUPPERCASEStatic, this, L"UPPERCASESTATICEVENT"));
             break;
       }
    case DevTests::CamelCasing::StaticStringVariationsEvent_IInterfaceStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtIInterfaceStatic, this, L"IInterfaceStaticEvent"));
            break;
        }
    case DevTests::CamelCasing::StaticStringVariationsEvent_camelStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtCamelStatic, this, L"camelStaticEvent"));
            break;
        }
    case DevTests::CamelCasing::StaticStringVariationsEvent__PrivatePascalStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtPascalPrivateStatic, this, L"_PrivatePascalStaticEvent"));
             break;
       }
    case DevTests::CamelCasing::StaticStringVariationsEvent__PRIVATEUPPERCASESTATICEVENT:
        {
            IfFailedReturn(InvokeAll(_evtUPPERCASEPrivateStatic, this, L"_PRIVATEUPPERCASESTATICEVENT"));
            break;
        }
    case DevTests::CamelCasing::StaticStringVariationsEvent__IInterfacePrivateStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtIInterfacePrivateStatic, this, L"_IInterfacePrivateStaticEvent"));
            break;
        }
    case DevTests::CamelCasing::StaticStringVariationsEvent__privateCamelStaticEvent:
        {
            IfFailedReturn(InvokeAll(_evtCamelPrivateStatic, this, L"_privateCamelStaticEvent"));
             break;
       }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::StringVariationsServer::FireEvent(__in StringVariationsEvent evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::StringVariationsEvent_PascalEvent:
        {
            IfFailedReturn(InvokeAll(_evtPascal, this, L"PascalEvent"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent_UPPERCASEEVENT:
        {
            IfFailedReturn(InvokeAll(_evtUPPERCASE, this, L"UPPERCASEEVENT"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent_IInterfaceEvent:
        {
            IfFailedReturn(InvokeAll(_evtIInterface, this, L"IInterfaceEvent"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent_camelEvent:
        {
            IfFailedReturn(InvokeAll(_evtCamel, this, L"camelEvent"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent__PrivatePascalEvent:
        {
            IfFailedReturn(InvokeAll(_evtPascalPrivate, this, L"_PrivatePascalEvent"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent__PRIVATEUPPERCASEEVENT:
        {
            IfFailedReturn(InvokeAll(_evtUPPERCASEPrivate, this, L"_PRIVATEUPPERCASEEVENT"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent__IInterfacePrivateEvent:
        {
            IfFailedReturn(InvokeAll(_evtIInterfacePrivate, this, L"_IInterfacePrivateEvent"));
            break;
        }
    case DevTests::CamelCasing::StringVariationsEvent__privateCamelEvent:
        {
            IfFailedReturn(InvokeAll(_evtCamelPrivate, this, L"_privateCamelEvent"));
             break;
       }
    case DevTests::CamelCasing::StringVariationsEvent_F8Event:
        {
            IfFailedReturn(InvokeAll(_evtF8, this, L"F8Event"));
             break;
       }
    case DevTests::CamelCasing::StringVariationsEvent_ECDH521Event:
        {
            IfFailedReturn(InvokeAll(_evtECDH521, this, L"ECDH521Event"));
             break;
       }
    case DevTests::CamelCasing::StringVariationsEvent_UInt16Event:
        {
            IfFailedReturn(InvokeAll(_evtUInt16, this, L"UInt16Event"));
             break;
       }
    case DevTests::CamelCasing::StringVariationsEvent_NONCASED_EVENT:
        {
            IfFailedReturn(InvokeAll(_evtNONCASED, this, L"NONCASED_EVENT"));
             break;
       }
    case DevTests::CamelCasing::StringVariationsEvent_UITwoLetterAcronymEvent:
        {
            IfFailedReturn(InvokeAll(_evtTwoLetter, this, L"UITwoLetterAcronymEvent"));
             break;
       }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::SimpleNameCollisions::InternalConflictServer::FireEvent(__in ConflictingEvents evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_InternalPascalEvent:
        {
            IfFailedReturn(InvokeAll(_evtInternalPascal, this, L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.ConflictingEvent"));
            break;
        }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::SimpleNameCollisions::ExternalConflictDifferentCaseServer::FireEvent(__in ConflictingEvents evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_ExternalEvent:
        {
            IfFailedReturn(InvokeAll(_evtExternal, this, L"DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.ConflictingEvent"));
            break;
        }
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_ExternalCamelEvent:
        {
            IfFailedReturn(InvokeAll(_evtExternalCamel, this, L"DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.conflictingEvent"));
            break;
        }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::SimpleNameCollisions::StaticInternalConflictFactory::FireEvent(__in ConflictingEvents evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_InternalPascalEvent:
        {
            IfFailedReturn(InvokeAll(_evtInternalPascal, this, L"DevTests.CamelCasing.SimpleNameCollisions.IInternalEventConflict.ConflictingEvent"));
            break;
        }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::SimpleNameCollisions::StaticExternalConflictDifferentCaseFactory::FireEvent(__in ConflictingEvents evt)
{
    HRESULT hr = S_OK;

    switch (evt)
    {
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_ExternalEvent:
        {
            IfFailedReturn(InvokeAll(_evtExternal, this, L"DevTests.CamelCasing.SimpleNameCollisions.IExternalEventConflict.ConflictingEvent"));
            break;
        }
    case DevTests::CamelCasing::SimpleNameCollisions::ConflictingEvents_ExternalCamelEvent:
        {
            IfFailedReturn(InvokeAll(_evtExternalCamel, this, L"DevTests.CamelCasing.SimpleNameCollisions.IExternalCamelEventConflict.conflictingEvent"));
            break;
        }
    default:
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::BuiltInConflictsStaticFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    *ppInspectable = nullptr;
    return E_NOTIMPL;
}

int DevTests::CamelCasing::CrossMemberCollisions::BuiltInConflictsStaticFactory::m_hasOwnProperty = 0;
int DevTests::CamelCasing::CrossMemberCollisions::BuiltInConflictsStaticFactory::m_ToString = 0;

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CamelLengthConflictFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    HRESULT hr = S_OK;
    *ppInspectable = nullptr;
    ComPtr<IPascalLengthConflict> spIPascalLengthConflict; 
        
    ComPtr<CamelLengthConflictServer> spObj = Make<CamelLengthConflictServer>();

    if (spObj != nullptr)
    {
        hr = spObj.As(&spIPascalLengthConflict);
        if (SUCCEEDED(hr))
        {
            *ppInspectable = spIPascalLengthConflict.Detach();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::PascalLengthConflictFactory::ActivateInstance(__deref_out IInspectable **ppInspectable)
{
    HRESULT hr = S_OK;
    *ppInspectable = nullptr;
    ComPtr<ICamelLengthConflict> spICamelLengthConflict; 
        
    ComPtr<PascalLengthConflictServer> spObj = Make<PascalLengthConflictServer>();

    if (spObj != nullptr)
    {
        hr = spObj.As(&spICamelLengthConflict);
        if (SUCCEEDED(hr))
        {
            *ppInspectable = spICamelLengthConflict.Detach();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::CVectorInt()
{
    ComPtr<Vector<int>> sp;
    Vector<int>::Make(&sp);
    for (int i = 1; i < 10; i++)
    {
        sp->Append(i);
    }

    sp.CopyTo(&m_pVector);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::First(__out Windows::Foundation::Collections::IIterator<int> **first)
{
    Windows::Foundation::Collections::IIterable<int> *pIIterable = NULL;
    HRESULT hr = m_pVector->QueryInterface(__uuidof(Windows::Foundation::Collections::IIterable<int>), (void **)&pIIterable);
    IfFailedReturn(hr);

    hr = pIIterable->First(first);
    pIIterable->Release();

    return hr;
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::GetAt(__in unsigned index, __out int *item)
{
    return m_pVector->GetAt(index, item);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::get_Size(__out unsigned *size)
{
    return m_pVector->get_Size(size);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::IndexOf(__in_opt int value, __out unsigned *index, __out boolean *found)
{
    return m_pVector->IndexOf(value, index, found);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::GetView(__deref_out_opt IVectorView<int> **returnValue)
{
    return m_pVector->GetView(returnValue);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::SetAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->SetAt(index, value);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::InsertAt(__in unsigned index, __in_opt int value)
{
    return m_pVector->InsertAt(index, value);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::RemoveAt(__in unsigned index)
{
    return m_pVector->RemoveAt(index);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::Append(__in_opt int value)
{
    return m_pVector->Append(value);
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::RemoveAtEnd()
{
    return m_pVector->RemoveAtEnd();
}

IFACEMETHODIMP
DevTests::CamelCasing::CrossMemberCollisions::CVectorInt::Clear()
{
    return m_pVector->Clear();
}
