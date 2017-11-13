#include "stdafx.h"
#include "AnimalServer.h"
#include "FishServer.h"
#include "CollectionsServer.h"
#include "PropertyValueTests.h"

#include <winrt\windowscollectionsp.h>
using namespace Windows::Foundation::Collections::Internal;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;

#define GetUnsupportedTypeFromPV()                          \
        IfNullReturnError(value, E_POINTER);                \
        return TYPE_E_TYPEMISMATCH;

#define GetUnsupportedTypeArrayFromPV()                     \
        IfNullReturnError(length, E_POINTER);               \
        IfNullReturnError(value, E_POINTER);                \
        return TYPE_E_TYPEMISMATCH;

namespace Animals
{
    struct EqualityPredicate_Dimensions
    {
        HRESULT operator()(Dimensions strLhs, Dimensions strRhs, __out bool* fEquals) const
        {
            if (strLhs.Length == strRhs.Length && strLhs.Width == strRhs.Width)
            {
                *fEquals = true;
            }
            else
            {
                *fEquals = false;
            }
            return S_OK;
        }
    };

    struct Hash_Dimensions
    {
        HRESULT operator()(Dimensions strValue, __out UINT32* uHash) const
        {
            *uHash = XWinRT::Fnv1a(reinterpret_cast<const BYTE*>(&strValue), sizeof(Dimensions));
            return S_OK;
        }
    };

    struct LifetimeTraits_Dimensions
    {
        //  Initialize an uninitialized buffer to an 'empty' value.
        //  Used to no-fail return or set an 'empty' value.
        static void Construct(__out Dimensions* uninitializedElement)
        {
            uninitializedElement->Length = 0;
            uninitializedElement->Width = 0;
        };

        //  Initialized an element from an already initialized one, 
        //  'copying' it.
        static HRESULT Construct(__out Dimensions* uninitializedElement, Dimensions source)
        {
            uninitializedElement->Length = source.Length;
            uninitializedElement->Width = source.Width;
            return S_OK;
        }

        //  Release all resources owned by the specified
        //  element, leaving it in an uninitialized state
        static void Destroy(__inout Dimensions* initializedElement)
        {
            initializedElement->Length = 0;
            initializedElement->Width = 0;
        };
    };

    struct EqualityPredicate_Date
    {
        HRESULT operator()(Windows::Foundation::DateTime strLhs, Windows::Foundation::DateTime strRhs, __out bool* fEquals) const
        {
            if (strLhs.UniversalTime == strRhs.UniversalTime)
            {
                *fEquals = true;
            }
            else
            {
                *fEquals = false;
            }
            return S_OK;
        }
    };

    struct LifetimeTraits_Date
    {
        //  Initialize an uninitialized buffer to an 'empty' value.
        //  Used to no-fail return or set an 'empty' value.
        static void Construct(__out Windows::Foundation::DateTime* uninitializedElement)
        {
            uninitializedElement->UniversalTime = 0;
        };

        //  Initialized an element from an already initialized one, 
        //  'copying' it.
        static HRESULT Construct(__out Windows::Foundation::DateTime* uninitializedElement, Windows::Foundation::DateTime source)
        {
            uninitializedElement->UniversalTime = source.UniversalTime;
            return S_OK;
        }

        //  Release all resources owned by the specified
        //  element, leaving it in an uninitialized state
        static void Destroy(__inout Windows::Foundation::DateTime* initializedElement)
        {
            initializedElement->UniversalTime = 0;
        };
    };

    struct EqualityPredicate_TimeSpan
    {
        HRESULT operator()(Windows::Foundation::TimeSpan strLhs, Windows::Foundation::TimeSpan strRhs, __out bool* fEquals) const
        {
            if (strLhs.Duration == strRhs.Duration)
            {
                *fEquals = true;
            }
            else
            {
                *fEquals = false;
            }
            return S_OK;
        }
    };

    struct LifetimeTraits_TimeSpan
    {
        //  Initialize an uninitialized buffer to an 'empty' value.
        //  Used to no-fail return or set an 'empty' value.
        static void Construct(__out Windows::Foundation::TimeSpan* uninitializedElement)
        {
            uninitializedElement->Duration = 0;
        };

        //  Initialized an element from an already initialized one, 
        //  'copying' it.
        static HRESULT Construct(__out Windows::Foundation::TimeSpan* uninitializedElement, Windows::Foundation::TimeSpan source)
        {
            uninitializedElement->Duration = source.Duration;
            return S_OK;
        }

        //  Release all resources owned by the specified
        //  element, leaving it in an uninitialized state
        static void Destroy(__inout Windows::Foundation::TimeSpan* initializedElement)
        {
            initializedElement->Duration = 0;
        };
    };

    struct EqualityPredicate_EventRegistration
    {
        HRESULT operator()(EventRegistrationToken strLhs, EventRegistrationToken strRhs, __out bool* fEquals) const
        {
            if (strLhs.value == strRhs.value)
            {
                *fEquals = true;
            }
            else
            {
                *fEquals = false;
            }
            return S_OK;
        }
    };

    struct LifetimeTraits_EventRegistration
    {
        //  Initialize an uninitialized buffer to an 'empty' value.
        //  Used to no-fail return or set an 'empty' value.
        static void Construct(__out EventRegistrationToken* uninitializedElement)
        {
            uninitializedElement->value = 0;
        };

        //  Initialized an element from an already initialized one, 
        //  'copying' it.
        static HRESULT Construct(__out EventRegistrationToken* uninitializedElement, EventRegistrationToken source)
        {
            uninitializedElement->value = source.value;
            return S_OK;
        }

        //  Release all resources owned by the specified
        //  element, leaving it in an uninitialized state
        static void Destroy(__inout EventRegistrationToken* initializedElement)
        {
            initializedElement->value = 0;
        };
    };

    CPropertyValueTests::CPropertyValueTests() : m_empty(nullptr), m_propertyValue(nullptr), m_dimensions(nullptr)
    {
        Windows::Internal::StringReference strFactory(L"Windows.Foundation.PropertyValue");
        Windows::Foundation::GetActivationFactory(strFactory.Get(), &spPropertyValueFactory);
    }


    CPropertyValueTests::~CPropertyValueTests()
    {
        if (m_empty)
        {
            m_empty->Release();
            m_empty = nullptr;
        }

        if (m_propertyValue)
        {
            m_propertyValue->Release();
            m_propertyValue = nullptr;
        }

        if (m_dimensions)
        {
            m_dimensions->Release();
            m_dimensions = nullptr;
        }
    }

    // {3463F772-274F-449D-8B25-822742C2B3FF}
    static const GUID guidArrayGuid1 = 
    { 0x3463f772, 0x274f, 0x449d, { 0x8b, 0x25, 0x82, 0x27, 0x42, 0xc2, 0xb3, 0xff } };

    // {3B3B41BC-96E3-43FE-8EC1-7E3DDE4F776C}
    static const GUID guidArrayGuid2 = 
    { 0x3b3b41bc, 0x96e3, 0x43fe, { 0x8e, 0xc1, 0x7e, 0x3d, 0xde, 0x4f, 0x77, 0x6c } };

    // {C1A5F085-740C-4991-9342-60B1E471BEB9}
    static const GUID guidArrayGuid3 = 
    { 0xc1a5f085, 0x740c, 0x4991, { 0x93, 0x42, 0x60, 0xb1, 0xe4, 0x71, 0xbe, 0xb9 } };

    IFACEMETHODIMP CPropertyValueTests::ReceiveGuidArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        GUID *values= (GUID *)CoTaskMemAlloc(3 * sizeof(GUID));
        *length = 3;
        *outValue = values;

        values[0] = guidArrayGuid1;
        values[1] = guidArrayGuid2;
        values[2] = guidArrayGuid3;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveAnimalArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IAnimal ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IAnimal **values= (IAnimal **)CoTaskMemAlloc(3 * sizeof(IAnimal *));
        *length = 3;
        *outValue = values;

        HSTRING hString;
        IAnimal *pAnimal;
        ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal1", 7, &hString);
        spAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        values[0] = pAnimal;

        values[1] = nullptr;

        spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal3", 7, &hString);
        spAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        values[2] = pAnimal;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveFishArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IFish ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IFish **values= (IFish **)CoTaskMemAlloc(2 * sizeof(IFish *));
        *length = 2;
        *outValue = values;

        HSTRING hString;
        IFish *pFish;
        ComPtr<IFish> spFish = Make<FishServer>();
        spFish.CopyTo(&pFish);
        WindowsCreateString(L"Nemo", 4, &hString);
        spFish->put_Name(hString);
        WindowsDeleteString(hString);
        values[0] = pFish;

        spFish = Make<FishServer>();
        spFish.CopyTo(&pFish);
        WindowsCreateString(L"Dori", 4, &hString);
        spFish->put_Name(hString);
        WindowsDeleteString(hString);
        values[1] = pFish;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IVector<int> ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IVector<int> **values= (IVector<int> **)CoTaskMemAlloc(2 * sizeof(IVector<int> *));
        *length = 2;
        *outValue = values;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<int>> sp;
        hr = Vector<int>::Make(&sp);
        for (int i = 1; SUCCEEDED(hr) && i <= 5; i++)
        {
            hr = sp->Append(i);
        }
        if (SUCCEEDED(hr))
        {
            sp.CopyTo(&values[0]);
        }

        hr = Vector<int>::Make(&sp);
        for (int i = 1; SUCCEEDED(hr) && i <= 4; i++)
        {
            hr = sp->Append(i * 100);
        }
        if (SUCCEEDED(hr))
        {
            sp.CopyTo(&values[1]);
        }

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveDateArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Windows::Foundation::DateTime *values= (Windows::Foundation::DateTime *)CoTaskMemAlloc(2 * sizeof(Windows::Foundation::DateTime));
        *length = 2;
        *outValue = values;

        Windows::Foundation::DateTime dateTime;

        dateTime.UniversalTime = 1265068800000 * 10000 + 5;   // 2-1-1641
        values[0] = dateTime;

        dateTime.UniversalTime = 0;   // 12-31-1600
        values[1] = dateTime;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveTimeSpanArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Windows::Foundation::TimeSpan *values= (Windows::Foundation::TimeSpan *)CoTaskMemAlloc(3 * sizeof(Windows::Foundation::TimeSpan));
        *length = 3;
        *outValue = values;

        Windows::Foundation::TimeSpan timeSpan;

        timeSpan.Duration = 1265068800000 * 10000 + 5;   // 2-1-1641 - 12-31-1600
        values[0] = timeSpan;

        timeSpan.Duration = 0;   // 12-31-1600 - 12-31-1600
        values[1] = timeSpan;

        timeSpan.Duration = 60i64 * 60i64 * 1000i64 * 10000i64;     // 1 hour
        values[2] = timeSpan;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceivePointArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Windows::Foundation::Point *values= (Windows::Foundation::Point *)CoTaskMemAlloc(3 * sizeof(Windows::Foundation::Point));
        *length = 3;
        *outValue = values;

        Windows::Foundation::Point point;

        point.X = 10;
        point.Y = 40;
        values[0] = point;

        point.X = 30;
        point.Y = 50;
        values[1] = point;

        point.X = 100;
        point.Y = 50;
        values[2] = point;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveSizeArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Windows::Foundation::Size *values= (Windows::Foundation::Size *)CoTaskMemAlloc(3 * sizeof(Windows::Foundation::Size));
        *length = 3;
        *outValue = values;

        Windows::Foundation::Size size;

        size.Height = 10;
        size.Width = 40;
        values[0] = size;

        size.Height = 30;
        size.Width = 50;
        values[1] = size;

        size.Height = 100;
        size.Width = 50;
        values[2] = size;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveRectArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Windows::Foundation::Rect *values= (Windows::Foundation::Rect *)CoTaskMemAlloc(3 * sizeof(Windows::Foundation::Rect));
        *length = 3;
        *outValue = values;

        Windows::Foundation::Rect rect;

        rect.X = 10;
        rect.Y = 40;
        rect.Height = 10;
        rect.Width = 40;
        values[0] = rect;

        rect.X = 30;
        rect.Y = 50;
        rect.Height = 30;
        rect.Width = 50;
        values[1] = rect;

        rect.X = 100;
        rect.Y = 50;
        rect.Height = 100;
        rect.Width = 50;
        values[2] = rect;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveBooleanArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        boolean *values= (boolean *)CoTaskMemAlloc(4 * sizeof(boolean));
        *length = 4;
        *outValue = values;

        values[0] = true;
        values[1] = false;
        values[2] = true;
        values[3] = true;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveStringArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        HSTRING *values= (HSTRING *)CoTaskMemAlloc(5 * sizeof(HSTRING));
        *length = 5;
        *outValue = values;

        HSTRING hString;
        WindowsCreateString(L"Javascript", 10, &hString);
        values[0] = hString;

        WindowsCreateString(L"is", 2, &hString);
        values[1] = hString;

        WindowsCreateString(L"present", 7, &hString);
        values[2] = hString;

        WindowsCreateString(L"and", 3, &hString);
        values[3] = hString;

        WindowsCreateString(L"future", 6, &hString);
        values[4] = hString;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveInspectableArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IInspectable **values= (IInspectable **)CoTaskMemAlloc(3 * sizeof(IInspectable *));
        *length = 3;
        *outValue = values;

        HSTRING hString;
        IAnimal *pAnimal;
        ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
        spAnimal.CopyTo(&pAnimal);
        WindowsCreateString(L"Animal1", 7, &hString);
        spAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        values[0] = pAnimal;

        values[1] = nullptr;

        IFish *pFish;
        ComPtr<IFish> spFish = Make<FishServer>();
        spFish.CopyTo(&pFish);
        WindowsCreateString(L"Nemo", 4, &hString);
        spFish->put_Name(hString);
        WindowsDeleteString(hString);
        values[2] = pFish;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveChar16Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        wchar_t *values= (wchar_t *)CoTaskMemAlloc(10 * sizeof(wchar_t));
        *length = 10;
        *outValue = values;

        values[0] = L'P';
        values[1] = L'r';
        values[2] = L'o';
        values[3] = L'j';
        values[4] = L'e';
        values[5] = L'c';
        values[6] = L't';
        values[7] = L'i';
        values[8] = L'o';
        values[9] = L'n';
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveUInt8Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        byte *values= (byte *)CoTaskMemAlloc(4 * sizeof(byte));
        *length = 4;
        *outValue = values;

        values[0] = 0x00;
        values[1] = 0x02;
        values[2] = 0x20;
        values[3] = 0x22;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveInt16Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        short *values= (short *)CoTaskMemAlloc(4 * sizeof(short));
        *length = 4;
        *outValue = values;

        values[0] = 10;
        values[1] = -20;
        values[2] = 30;
        values[3] = -40;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveUInt16Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        unsigned short *values= (unsigned short *)CoTaskMemAlloc(2 * sizeof(unsigned short));
        *length = 2;
        *outValue = values;

        values[0] = 10;
        values[1] = 20;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveInt32Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        INT32 *values= (INT32 *)CoTaskMemAlloc(4 * sizeof(INT32));
        *length = 4;
        *outValue = values;

        values[0] = 1000;
        values[1] = -2000;
        values[2] = 3000;
        values[3] = -4000;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveUInt32Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        UINT32 *values= (UINT32 *)CoTaskMemAlloc(2 * sizeof(UINT32));
        *length = 2;
        *outValue = values;

        values[0] = 1000;
        values[1] = 2000;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveInt64Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        INT64 *values= (INT64 *)CoTaskMemAlloc(4 * sizeof(INT64));
        *length = 4;
        *outValue = values;

        values[0] = 100000;
        values[1] = -200000;
        values[2] = 300000;
        values[3] = -400000;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveUInt64Array(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        UINT64 *values= (UINT64 *)CoTaskMemAlloc(2 * sizeof(UINT64));
        *length = 2;
        *outValue = values;

        values[0] = 100000;
        values[1] = 200000;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveFloatArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        float *values= (float *)CoTaskMemAlloc(4 * sizeof(float));
        *length = 4;
        *outValue = values;

        values[0] = (float)78.3;
        values[1] = (float)67.9;
        values[2] = (float)99.4;
        values[3] = (float)-32.2;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveDoubleArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        double *values= (double *)CoTaskMemAlloc(2 * sizeof(double));
        *length = 2;
        *outValue = values;

        values[0] = 13.4;
        values[1] = 56.8;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveStructArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Dimensions *values= (Dimensions *)CoTaskMemAlloc(2 * sizeof(Dimensions));
        *length = 2;
        *outValue = values;

        Dimensions dimensions = { 40, 40 };
        values[0] = dimensions;

        dimensions.Length = 100;
        values[1] = dimensions;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveEnumArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        Phylum *values= (Phylum *)CoTaskMemAlloc(4 * sizeof(Phylum));
        *length = 4;
        *outValue = values;

        values[0] = Phylum_Entoprocta;
        values[1] = Phylum_Mollusca;
        values[2] = Phylum_Arthropoda;
        values[3] = Phylum_Orthonectida;

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveWinrtDelegateArray(__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IDelegateWithOutParam_HSTRING **values= (IDelegateWithOutParam_HSTRING **)CoTaskMemAlloc(2 * sizeof(IDelegateWithOutParam_HSTRING *));
        *length = 2;
        *outValue = values;

        ComPtr<IDelegateWithOutParam_HSTRING> spListener = Make<AnimalDelegateWithOutParam_HSTRING>();
        spListener.CopyTo(&values[0]);

        spListener = Make<AnimalDelegateWithOutParam_HSTRING>();
        spListener.CopyTo(&values[1]);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveJSDelegateArray(__in IDelegateWithOutParam_HSTRING *delegate1, __in IDelegateWithOutParam_HSTRING *delegate2, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);

        IDelegateWithOutParam_HSTRING **values= (IDelegateWithOutParam_HSTRING **)CoTaskMemAlloc(2 * sizeof(IDelegateWithOutParam_HSTRING *));
        *length = 2;
        *outValue = values;

        values[0] = delegate1;
        values[0]->AddRef();

        values[1] = delegate2;
        values[1]->AddRef();

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfVector(__out IVector<IVector<int> *> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        Microsoft::WRL::ComPtr<Vector<IVector<int> *>> spVector;
        HRESULT hr = Vector<IVector<int> *>::Make(&spVector);
        IfFailedReturn(hr);

        //IVector<int> *myVector;
        Microsoft::WRL::ComPtr<Vector<int>> sp, sp1;
        hr = Vector<int>::Make(&sp);
        IfFailedReturn(hr);

        for (int i = 1; SUCCEEDED(hr) && i <= 5; i++)
        {
            hr = sp->Append(i);
            IfFailedReturn(hr);
        }

        //sp.CopyTo(&myVector);
        spVector->Append(sp.Get());

        hr = Vector<int>::Make(&sp1);
        IfFailedReturn(hr);

        for (int i = 1; SUCCEEDED(hr) && i <= 4; i++)
        {
            hr = sp1->Append(i * 100);
            IfFailedReturn(hr);
        }

        //sp1.CopyTo(&myVector);
        spVector->Append(sp1.Get());

        spVector.CopyTo(outValue);
        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfStruct(__out IVector<Dimensions> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        Microsoft::WRL::ComPtr<Vector<Dimensions, EqualityPredicate_Dimensions, LifetimeTraits_Dimensions>> spVector;
        HRESULT hr = Vector<Dimensions, EqualityPredicate_Dimensions, LifetimeTraits_Dimensions>::Make(&spVector);
        IfFailedReturn(hr);

        Dimensions dimension = { 100, 100 };
        spVector->Append(dimension);
        dimension.Length = 150;
        spVector->Append(dimension);

        spVector.CopyTo(outValue);
        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveMapOfStructAndVector(__out IMap<Dimensions, IVector<HSTRING> *> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        Microsoft::WRL::ComPtr<HashMap<Dimensions, IVector<HSTRING> *, Hash_Dimensions, EqualityPredicate_Dimensions, LifetimeTraits_Dimensions>> spMap;
        HRESULT hr = HashMap<Dimensions, IVector<HSTRING> *, Hash_Dimensions, EqualityPredicate_Dimensions, LifetimeTraits_Dimensions>::Make(&spMap);
        IfFailedReturn(hr);

        Dimensions dimension = { 100, 100 };

        // Create the vector 
        HSTRING hString;
        IVector<HSTRING> *myVector;
        Microsoft::WRL::ComPtr<Vector<HSTRING>> sp;
        hr = Vector<HSTRING>::Make(&sp);
        IfFailedReturn(hr);
        WindowsCreateString(L"Hundred", 7, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"by", 2, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"Hundred", 7, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        sp.CopyTo(&myVector);
        boolean fReplaced;
        spMap->Insert(dimension, myVector, &fReplaced);
        myVector->Release();

        dimension.Length = 150;

        // Create the vector 
        hr = Vector<HSTRING>::Make(&sp);
        IfFailedReturn(hr);
        WindowsCreateString(L"Hundred And Fifty", 17, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"by", 2, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"Hundred", 7, &hString);
        sp->Append(hString);
        WindowsDeleteString(hString);
        sp.CopyTo(&myVector);
        spMap->Insert(dimension, myVector, &fReplaced);
        myVector->Release();

        spMap.CopyTo(outValue);
        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfRCObservableVector(__out IVector<RCIObservable *> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<RCIObservable*>> sp;

        hr = Vector<RCIObservable*>::Make(&sp);
        IfFailedReturn(hr);

        for (int i = 0; SUCCEEDED(hr) && i < 2; i++)
        {
            ComPtr<RCIObservableServer> spVector = Make<RCIObservableServer>();
            sp->Append(spVector.Get());
        }

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }

    //IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfHRESULT(__out IVector<HRESULT> **outValue)
    //{
    //    IfNullReturnError(outValue, E_POINTER);
    //    *outValue = NULL;

    //    HRESULT hr = S_OK;
    //    Microsoft::WRL::ComPtr<Vector<HRESULT>> sp;

    //    hr = Vector<HRESULT>::Make(&sp);
    //    IfFailedReturn(hr);

    //    sp->Append(S_OK);
    //    sp->Append(E_FAIL);
    //    sp->Append(E_INVALIDARG);
    //    sp->Append(E_POINTER);
    //    sp->Append(E_OUTOFMEMORY);
    //    sp->Append(S_FALSE);

    //    if (SUCCEEDED(hr))
    //    {
    //        sp.CopyTo(outValue);
    //    }

    //    return hr;
    //}

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfGuid(__out IVector<GUID> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<GUID>> sp;

        hr = Vector<GUID>::Make(&sp);
        IfFailedReturn(hr);

        sp->Append(guidArrayGuid1);
        sp->Append(guidArrayGuid2);
        sp->Append(guidArrayGuid3);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfDate(__out IVector<Windows::Foundation::DateTime> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<Windows::Foundation::DateTime, EqualityPredicate_Date, LifetimeTraits_Date>> sp;

        hr = Vector<Windows::Foundation::DateTime, EqualityPredicate_Date, LifetimeTraits_Date>::Make(&sp);
        IfFailedReturn(hr);

        Windows::Foundation::DateTime dateTime;

        dateTime.UniversalTime = 1265068800000 * 10000 + 5;   // 2-1-1641
        sp->Append(dateTime);

        dateTime.UniversalTime = 0;   // 12-31-1600
        sp->Append(dateTime);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfTimeSpan(__out IVector<Windows::Foundation::TimeSpan> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<Windows::Foundation::TimeSpan, EqualityPredicate_TimeSpan, LifetimeTraits_TimeSpan>> sp;

        hr = Vector<Windows::Foundation::TimeSpan, EqualityPredicate_TimeSpan, LifetimeTraits_TimeSpan>::Make(&sp);
        IfFailedReturn(hr);

        Windows::Foundation::TimeSpan timeSpan;

        timeSpan.Duration = 1265068800000 * 10000 + 5;   // 2-1-1641 - 12-31-1600
        sp->Append(timeSpan);

        timeSpan.Duration = 0;   // 12-31-1600 - 12-31-1600
        sp->Append(timeSpan);

        timeSpan.Duration = 60i64 * 60i64 * 1000i64 * 10000i64;     // 1 hour
        sp->Append(timeSpan);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }


    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfEnum(__out IVector<Phylum> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<Phylum>> sp;

        hr = Vector<Phylum>::Make(&sp);
        IfFailedReturn(hr);

        sp->Append(Phylum_Entoprocta);
        sp->Append(Phylum_Mollusca);
        sp->Append(Phylum_Arthropoda);
        sp->Append(Phylum_Orthonectida);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfDelegate(__out IVector<IDelegateWithOutParam_HSTRING *> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<IDelegateWithOutParam_HSTRING *>> sp;

        hr = Vector<IDelegateWithOutParam_HSTRING *>::Make(&sp);
        IfFailedReturn(hr);

        Animals::IDelegateWithOutParam_HSTRING *outDelegate;
        ComPtr<IDelegateWithOutParam_HSTRING> spListener = Make<AnimalDelegateWithOutParam_HSTRING>();
        spListener.CopyTo(&outDelegate);
        sp->Append(outDelegate);
        outDelegate->Release();

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }

    class CustomAsyncInfo :
        public Microsoft::WRL::RuntimeClass<
            Windows::Foundation::IAsyncInfo,
            ICustomAsync
        >
    {
        InspectableClass(L"Animals.ICustomAsync", FullTrust);
        ICustomAsyncCompleted * completed;
        ICustomAsyncProgress * progress;
        AsyncStatus status;
    public:
        void Initialize()
        {
            completed = nullptr;
            progress = nullptr;
            status = Windows::Foundation::AsyncStatus::Started;
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Completed(ICustomAsyncCompleted ** ppcompleted) 
        { 
            *ppcompleted = completed;
            if (completed)
            {
                completed->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE put_Completed(ICustomAsyncCompleted * pcompleted) 
        { 
            if (completed)
            {
                completed->Release();
            }

            completed = pcompleted;
            if(completed)
            {
                completed->AddRef();
            }
            return S_OK; 
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Progress(ICustomAsyncProgress ** ppprogress) 
        { 
            *ppprogress = progress;
            if (progress)
            {
                progress->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE put_Progress(ICustomAsyncProgress * pprogress) 
        { 
            if (progress)
            {
                progress->Release();
            }

            progress = pprogress;
            if (progress)
            {
                progress->AddRef();
            }
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE GetResults(int * result)
        { 
            *result = 192; 
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE MoveToCompleted(void) 
        { 
            if(status == Windows::Foundation::AsyncStatus::Started)
            {
                status = Windows::Foundation::AsyncStatus::Completed;
                if(completed)
                {
                    completed->Invoke(this, status);
                }
                return S_OK;
            }
            return E_FAIL; 
        }

        virtual HRESULT STDMETHODCALLTYPE MoveToError(void) 
        { 
            if(status == Windows::Foundation::AsyncStatus::Started)
            {
                status = Windows::Foundation::AsyncStatus::Error;
                if(completed)
                {
                    completed->Invoke(this, status);
                }
                return S_OK;
            }
            return E_FAIL; 
        }

        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Id( 
            /* [retval][out] */ __RPC__out unsigned __int32 *id) 
        { 
            *id = 0; 
            return S_OK; 
        }
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Status( 
            /* [retval][out] */ __RPC__out AsyncStatus *pstatus) 
        { 
            *pstatus = status; 
            return S_OK; 
        }
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_ErrorCode( 
            /* [retval][out] */ __RPC__out HRESULT *errorCode) 
        { 
            if (status == Windows::Foundation::AsyncStatus::Error)
            {
                *errorCode = E_FAIL; 
            }
            return S_OK; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Start( void) 
        { 
            return S_OK; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) 
        { 
            if(status == Windows::Foundation::AsyncStatus::Started)
            {
                status = Windows::Foundation::AsyncStatus::Canceled;
                completed->Invoke(this, status);
                return S_OK;
            }
            return E_FAIL; 
        }
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) 
        { 
            return S_OK; 
        }
    };


    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfAsyncInfo(__out IVector<Windows::Foundation::IAsyncInfo *> **outValue)
    {
        auto instance = Microsoft::WRL::Make<CustomAsyncInfo>();
        instance->Initialize();
        auto asyncOp =  instance.Get();

        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<Windows::Foundation::IAsyncInfo *>> sp;

        hr = Vector<Windows::Foundation::IAsyncInfo *>::Make(&sp);
        IfFailedReturn(hr);

        sp->Append(asyncOp);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }
        return hr;
    }


    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfEventRegistration(__out IVector<EventRegistrationToken> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        HRESULT hr = S_OK;
        Microsoft::WRL::ComPtr<Vector<EventRegistrationToken, EqualityPredicate_EventRegistration, LifetimeTraits_EventRegistration>> sp;

        hr = Vector<EventRegistrationToken, EqualityPredicate_EventRegistration, LifetimeTraits_EventRegistration>::Make(&sp);
        IfFailedReturn(hr);

        EventRegistrationToken eventRegistration = { 300 };
        sp->Append(eventRegistration);

        if (SUCCEEDED(hr))
        {
            sp.CopyTo(outValue);
        }

        return hr;
    }


    IFACEMETHODIMP CPropertyValueTests::ReceiveMapOfStringAndInspectable(__out Windows::Foundation::Collections::IMap<HSTRING, IInspectable *> **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        *outValue = NULL;

        Microsoft::WRL::ComPtr<HashMap<HSTRING, IInspectable *>> spMap;
        HRESULT hr = HashMap<HSTRING, IInspectable *>::Make(&spMap);
        IfFailedReturn(hr);

        // Add Animal
        boolean fReplaced;
        HSTRING hString;
        IInspectable *inspectable;

        ComPtr<IAnimal> spAnimal = Make<AnimalServer>();
        WindowsCreateString(L"Animal1", 7, &hString);
        spAnimal->SetGreeting(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"Animals.Animal", 14, &hString);
        spAnimal.CopyTo(&inspectable);
        spMap->Insert(hString, inspectable, &fReplaced);
        inspectable->Release();
        WindowsDeleteString(hString);

        // Fish
        ComPtr<IFish> spFish = Make<FishServer>();
        WindowsCreateString(L"Nemo", 4, &hString);
        spFish->put_Name(hString);
        WindowsDeleteString(hString);
        WindowsCreateString(L"Animals.Fish", 12, &hString);
        spFish.CopyTo(&inspectable);
        spMap->Insert(hString, inspectable, &fReplaced);
        inspectable->Release();
        WindowsDeleteString(hString);

        // IVector<int>
        Microsoft::WRL::ComPtr<Vector<int>> spVector;
        hr = Vector<int>::Make(&spVector);
        for (int i = 1; SUCCEEDED(hr) && i <= 5; i++)
        {
            spVector->Append(i);
        }
        WindowsCreateString(L"Windows.Foundation.Collections.IVector<Int>", 43, &hString);
        spVector.CopyTo(&inspectable);
        spMap->Insert(hString, inspectable, &fReplaced);
        inspectable->Release();
        WindowsDeleteString(hString);

        // IMap<>
        IMap<Dimensions, IVector<HSTRING> *> *pMap;
        ReceiveMapOfStructAndVector(&pMap);
        WindowsCreateString(L"Windows.Foundation.Collections.IMap<Animals.Dimensions,Windows.Foundation.Collections.IVector<String>>", 102, &hString);
        spMap->Insert(hString, (IInspectable *)pMap, &fReplaced);
        WindowsDeleteString(hString);
        pMap->Release();

        spMap.CopyTo(outValue);

        return hr;
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfVector_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfVector((IVector<IVector<int> *> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfStruct_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfStruct((IVector<Dimensions> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveMapOfStructAndVector_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveMapOfStructAndVector((IMap<Dimensions, IVector<HSTRING> *> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfRCObservableVector_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfRCObservableVector((IVector<RCIObservable*> **)outValue);
    }

    //IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfHRESULT_InspectableOut(__out IInspectable **outValue)
    //{
    //    return ReceiveVectorOfHRESULT((IVector<HRESULT> **)outValue);
    //}

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfGuid_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfGuid((IVector<GUID> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfDate_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfDate((IVector<Windows::Foundation::DateTime> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfTimeSpan_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfTimeSpan((IVector<Windows::Foundation::TimeSpan> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfEnum_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfEnum((IVector<Phylum> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfDelegate_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfDelegate((IVector<IDelegateWithOutParam_HSTRING *> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfAsyncInfo_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfAsyncInfo((IVector<Windows::Foundation::IAsyncInfo *> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveVectorOfEventRegistration_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveVectorOfEventRegistration((IVector<EventRegistrationToken> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::ReceiveMapOfStringAndInspectable_InspectableOut(__out IInspectable **outValue)
    {
        return ReceiveMapOfStringAndInspectable((IMap<HSTRING, IInspectable *> **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::IsSameDelegate(__in IDelegateWithOutParam_HSTRING *inValue1, __in IDelegateWithOutParam_HSTRING *inValue2, __out boolean *isSame)
    {
        IfNullReturnError(isSame, E_POINTER);
        *isSame = inValue1 == (inValue2);
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestNull_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = (inValue == nullptr) ? true : false;
        *outValue = nullptr;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out boolean *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Boolean)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetBoolean(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestString_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out HSTRING *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_String)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetString(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestNumber_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out double *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Double)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetDouble(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out  Windows::Foundation::DateTime *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_DateTime)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetDateTime(outValue);
    }


    IFACEMETHODIMP CPropertyValueTests::TestInspectable_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);

        if (SUCCEEDED(hr) || hr != E_NOINTERFACE)
        {
            return hr;
        }

        *isValidType = true;
        *outValue = inValue;
        inValue->AddRef();
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_InspectableArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetInspectableArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuidArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_GuidArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetGuidArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDateArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_DateTimeArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetDateTimeArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensionsArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);
        
        if (propertyType != Windows::Foundation::PropertyType_OtherTypeArray)
        {
            return hr;
        }

        ComPtr<IReferenceArray<Dimensions>> referenceArrayT;
        hr = inValue->QueryInterface(__uuidof(IReferenceArray<Dimensions>), (LPVOID *)&referenceArrayT);
        IfFailedReturn(hr);

        *isValidType = true;
        return referenceArrayT->get_Value(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpanArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_TimeSpanArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetTimeSpanArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPointArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_PointArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetPointArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSizeArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_SizeArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetSizeArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRectArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_RectArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetRectArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnumArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);
        
        if (propertyType != Windows::Foundation::PropertyType_OtherTypeArray)
        {
            return hr;
        }

        ComPtr<IReferenceArray<Phylum>> referenceArrayT;
        hr = inValue->QueryInterface(__uuidof(IReferenceArray<Phylum>), (LPVOID *)&referenceArrayT);
        IfFailedReturn(hr);

        *isValidType = true;
        return referenceArrayT->get_Value(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBooleanArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_BooleanArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetBooleanArray(length, outValue);
    }


    IFACEMETHODIMP CPropertyValueTests::TestStringArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_StringArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetStringArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Char16Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetChar16Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt8Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetUInt8Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int16Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetInt16Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt16Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetUInt16Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int32Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetInt32Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt32Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetUInt32Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int64Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetInt64Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64Array_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt64Array)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetUInt64Array(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloatArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_SingleArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetSingleArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDoubleArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_DoubleArray)
        {
            return hr;
        }

        *isValidType = true;
        return propertyValue->GetDoubleArray(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDelegateArray_InspectableIn(__in IInspectable *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(length, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);
        
        if (propertyType != Windows::Foundation::PropertyType_OtherTypeArray)
        {
            return hr;
        }

        ComPtr<IReferenceArray<IDelegateWithOutParam_HSTRING *>> referenceArrayT;
        hr = inValue->QueryInterface(__uuidof(IReferenceArray<IDelegateWithOutParam_HSTRING *>), (LPVOID *)&referenceArrayT);
        IfFailedReturn(hr);

        *isValidType = true;
        return referenceArrayT->get_Value(length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestNull_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);

        *outValue = nullptr;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_InspectableOut(__in boolean inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateBoolean(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestString_InspectableOut(__in HSTRING inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateString(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16_InspectableOut(__in wchar_t inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateChar16(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8_InspectableOut(__in byte inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt8(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16_InspectableOut(__in short inValue, __out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        return spPropertyValueFactory->CreateInt16(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16_InspectableOut(__in unsigned short inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt16(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32_InspectableOut(__in INT32 inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInt32(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32_InspectableOut(__in UINT32 inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt32(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64_InspectableOut(__in INT64 inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInt64(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64_InspectableOut(__in UINT64 inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt64(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloat_InspectableOut(__in float inValue, __out IInspectable **outValue) 
    {
        return spPropertyValueFactory->CreateSingle(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDouble_InspectableOut(__in double inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateDouble(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuid_InspectableOut(__in GUID inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateGuid(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_InspectableOut(__in Windows::Foundation::DateTime inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateDateTime(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensions_InspectableOut(__in Windows::Foundation::IReference<Dimensions> *inValue, __out IInspectable **outValue)
    {
        *outValue = inValue;
        if (inValue)
        {
            inValue->AddRef();
        }

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpan_InspectableOut(__in Windows::Foundation::TimeSpan inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateTimeSpan(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPoint_InspectableOut(__in Windows::Foundation::Point inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreatePoint(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSize_InspectableOut(__in Windows::Foundation::Size inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateSize(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRect_InspectableOut(__in Windows::Foundation::Rect inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateRect(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnum_InspectableOut(__in Windows::Foundation::IReference<Phylum> *inValue, __out IInspectable **outValue)
    {
        *outValue = inValue;
        if (inValue)
        {
            inValue->AddRef();
        }

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV1_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue1> sp = Make<CRCPropertyValue1>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV2_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue2> sp = Make<CRCPropertyValue2>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV3_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue3> sp = Make<CRCPropertyValue3>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV4_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue4> sp = Make<CRCPropertyValue4>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV5_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue5> sp = Make<CRCPropertyValue5>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV6_InspectableOut(__out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        ComPtr<CRCPropertyValue6> sp = Make<CRCPropertyValue6>();
        sp.CopyTo(outValue);

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuidArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) GUID *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateGuidArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDateArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::DateTime *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateDateTimeArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpanArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::TimeSpan *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateTimeSpanArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPointArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Point *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreatePointArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSizeArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Size *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateSizeArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRectArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Rect *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateRectArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBooleanArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) boolean *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateBooleanArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestStringArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) HSTRING *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateStringArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) wchar_t *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateChar16Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) byte *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt8Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) short *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInt16Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) unsigned short *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt16Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) INT32 *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInt32Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) UINT32 *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt32Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) INT64 *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInt64Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64Array_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) UINT64 *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateUInt64Array(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloatArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) float *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateSingleArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDoubleArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) double *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateDoubleArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) IInspectable **inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectableArray(length, inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestAnimalArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) IAnimal **inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectableArray(length, (IInspectable **)inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFishArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) IFish **inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectableArray(length, (IInspectable **)inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestVectorArray_InspectableOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Collections::IVector<int> **inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectableArray(length, (IInspectable **)inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxInspectable_InspectableOut(__in IInspectable *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectable(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInspectable_InspectableOut(__in IInspectable *inValue, __out IInspectable **outValue)
    {
        SendBackSameInterface(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxedNull_InspectableOut(__out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateEmpty(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestIterable_InspectableOut(__in Windows::Foundation::Collections::IIterable<int> *inValue, __out IInspectable **outValue)
    {
        SendBackSameInterface(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestIterator_InspectableOut(__in Windows::Foundation::Collections::IIterator<int> *inValue, __out IInspectable **outValue)
    {
        SendBackSameInterface(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestIVector_InspectableOut(__in Windows::Foundation::Collections::IVector<int> *inValue, __out IInspectable **outValue)
    {
        SendBackSameInterface(inValue, outValue);
    }
    
    IFACEMETHODIMP CPropertyValueTests::TestIVectorView_InspectableOut(__in Windows::Foundation::Collections::IVectorView<int> *inValue, __out IInspectable **outValue)
    {
        SendBackSameInterface(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxIterable_InspectableOut(__in Windows::Foundation::Collections::IIterable<int> *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectable(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxIterator_InspectableOut(__in Windows::Foundation::Collections::IIterator<int> *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectable(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxIVector_InspectableOut(__in Windows::Foundation::Collections::IVector<int> *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectable(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxIVectorView_InspectableOut(__in Windows::Foundation::Collections::IVectorView<int> *inValue, __out IInspectable **outValue)
    {
        return spPropertyValueFactory->CreateInspectable(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::GetRuntimeClassWithEmptyString(__out IInspectable **inspectable)
    {
        IfNullReturnError(inspectable, E_POINTER);
        if (m_empty == nullptr)
        {
            ComPtr<IInspectable> spDuplicate = Make<CEmptyGRCNString>();
            spDuplicate.CopyTo(&m_empty);

            if (m_empty == nullptr)
            {
                return E_OUTOFMEMORY;
            }
        }

        *inspectable = m_empty;
        (*inspectable)->AddRef();

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::VerifyRuntimeClassWithEmptyString(__in IInspectable *inspectable, __out boolean *isSame)
    {
        IfNullReturnError(isSame, E_POINTER);

        *isSame = (inspectable == m_empty);
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::GetRuntimeClassWithFailingGRCN(__out IInspectable **inspectable)
    {
        IfNullReturnError(inspectable, E_POINTER);

        ComPtr<IInspectable> spDuplicate = Make<CFailingGRCNString>();
        spDuplicate.CopyTo(inspectable);
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::GetRuntimeClassWithEmptyStringAsInterface(__out IEmptyGRCN **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);

        ComPtr<IEmptyGRCN> spDuplicate = Make<CEmptyGRCN>();
        spDuplicate.CopyTo(outValue);
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestFailingRuntimeClassNameWithAnotherInterface(__in IInspectable *inValue, __out IInspectable **outFailingValue, __out IInspectable **outValue)
    {
        IfNullReturnError(outFailingValue, E_POINTER);
        IfNullReturnError(outValue, E_POINTER);

        ComPtr<IInspectable> spDuplicate = Make<CFailingGRCNString>();
        spDuplicate.CopyTo(outFailingValue);

        SendBackSameInterface(inValue, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestNull_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestNull_InspectableIn(inValue, isValidType, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out boolean *outValue)
    {
        return TestBoolean_InspectableIn(inValue, isValidType, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestString_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out HSTRING *outValue)
    {
        return TestString_InspectableIn(inValue, isValidType, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestNumber_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out double *outValue)
    {
        return TestNumber_InspectableIn(inValue, isValidType, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out  Windows::Foundation::DateTime *outValue)
    {
        return TestDate_InspectableIn(inValue, isValidType, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInspectable_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out IInspectable **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);

        *isValidType = false;
        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            return hr;
        }

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Inspectable)
        {
            return hr;
        }

        *isValidType = true;
        return TYPE_E_TYPEMISMATCH;
    }

    IFACEMETHODIMP CPropertyValueTests::TestArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***outValue)
    {
        return TestArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuidArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **outValue)
    {
        return TestGuidArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDateArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **outValue)
    {
        return TestDateArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensionsArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Dimensions **outValue)
    {
        return TestDimensionsArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpanArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **outValue)
    {
        return TestTimeSpanArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPointArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **outValue)
    {
        return TestPointArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSizeArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **outValue)
    {
        return TestSizeArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRectArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **outValue)
    {
        return TestRectArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnumArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Phylum **outValue)
    {
        if ((nullptr == inValue) || (nullptr == isValidType) || (nullptr == length) || (nullptr == outValue))
        {
            return E_POINTER;
        }

        return TestEnumArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBooleanArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **outValue)
    {
        return TestBooleanArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestStringArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **outValue)
    {
        return TestStringArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) wchar_t **outValue)
    {
        return TestChar16Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) byte **outValue)
    {
        return TestUInt8Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) short **outValue)
    {
        return TestInt16Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) unsigned short **outValue)
    {
        return TestUInt16Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **outValue)
    {
        return TestInt32Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **outValue)
    {
        return TestUInt32Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **outValue)
    {
        return TestInt64Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64Array_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **outValue)
    {
        return TestUInt64Array_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloatArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) float **outValue)
    {
        return TestFloatArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDoubleArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) double **outValue)
    {
        return TestDoubleArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDelegateArray_IPropertyValueIn(__in Windows::Foundation::IPropertyValue *inValue, __out boolean *isValidType, __out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IDelegateWithOutParam_HSTRING ***outValue)
    {
        return TestDelegateArray_InspectableIn(inValue, isValidType, length, outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestNull_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        IfNullReturnError(outValue, E_POINTER);

        *outValue = nullptr;
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_IPropertyValueOut(__in boolean inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestBoolean_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestString_IPropertyValueOut(__in HSTRING inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestString_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16_IPropertyValueOut(__in wchar_t inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestChar16_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8_IPropertyValueOut(__in byte inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt8_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16_IPropertyValueOut(__in short inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt16_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16_IPropertyValueOut(__in unsigned short inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt16_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32_IPropertyValueOut(__in INT32 inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt32_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32_IPropertyValueOut(__in UINT32 inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt32_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64_IPropertyValueOut(__in INT64 inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt64_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64_IPropertyValueOut(__in UINT64 inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt64_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloat_IPropertyValueOut(__in float inValue, __out Windows::Foundation::IPropertyValue **outValue) 
    {
        return TestFloat_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDouble_IPropertyValueOut(__in double inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestDouble_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuid_IPropertyValueOut(__in GUID inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestGuid_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_IPropertyValueOut(__in Windows::Foundation::DateTime inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestDate_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensions_IPropertyValueOut(__in Windows::Foundation::IReference<Dimensions> *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        *outValue = nullptr;
        if (inValue)
        {
            return inValue->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **)outValue);
        }

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpan_IPropertyValueOut(__in Windows::Foundation::TimeSpan inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestTimeSpan_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPoint_IPropertyValueOut(__in Windows::Foundation::Point inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestPoint_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSize_IPropertyValueOut(__in Windows::Foundation::Size inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestSize_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRect_IPropertyValueOut(__in Windows::Foundation::Rect inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRect_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnum_IPropertyValueOut(__in Windows::Foundation::IReference<Phylum> *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        *outValue = nullptr;
        if (inValue)
        {
            return inValue->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **)outValue);
        }

        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV1_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV1_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV2_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV2_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV3_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV3_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV4_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV4_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV5_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV5_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV6_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV6_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuidArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) GUID *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestGuidArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDateArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::DateTime *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestDateArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpanArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::TimeSpan *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestTimeSpanArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPointArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Point *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestPointArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSizeArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Size *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestSizeArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRectArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Rect *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRectArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBooleanArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) boolean *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestBooleanArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestStringArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) HSTRING *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestStringArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) wchar_t *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestChar16Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) byte *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt8Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) short *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt16Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) unsigned short *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt16Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) INT32 *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt32Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) UINT32 *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt32Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) INT64 *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestInt64Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64Array_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) UINT64 *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestUInt64Array_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloatArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) float *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestFloatArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDoubleArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) double *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestDoubleArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) IInspectable **inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestAnimalArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) IAnimal **inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestAnimalArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFishArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) IFish **inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestFishArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestVectorArray_IPropertyValueOut(__in UINT32 length, __RPC__in_ecount_full(length) Windows::Foundation::Collections::IVector<int> **inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestVectorArray_InspectableOut(length, inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxInspectable_IPropertyValueOut(__in IInspectable *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        ComPtr<Windows::Foundation::IPropertyValue> propertyValue;
        HRESULT hr = inValue->QueryInterface(Windows::Foundation::IID_IPropertyValue, &propertyValue);
        if (SUCCEEDED(hr) || hr != E_NOINTERFACE)
        {
            return E_INVALIDARG;
        }

        return TestBoxInspectable_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInspectable_IPropertyValueOut(__in IInspectable *inValue, __out Windows::Foundation::IPropertyValue **outValue)
    {
        return inValue->QueryInterface(Windows::Foundation::IID_IPropertyValue, (void **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoxedNull_IPropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestBoxedNull_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV1_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV2_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV3_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV4_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV5_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV6_PropertyValueIn(__in Windows::Foundation::IPropertyValue *)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV1_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV1_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV2_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV2_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV3_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV3_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV4_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV4_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV5_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV5_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRCPV6_PropertyValueOut(__out Windows::Foundation::IPropertyValue **outValue)
    {
        return TestRCPV6_InspectableOut((IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_ReferenceIn(__in Windows::Foundation::IReference<bool> *inValue, __out boolean *isNull, __out boolean *isValidType, __out boolean *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<bool>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<bool>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Boolean)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16_ReferenceIn(__in Windows::Foundation::IReference<wchar_t> *inValue, __out boolean *isNull, __out boolean *isValidType, __out wchar_t *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<wchar_t>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<wchar_t>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Char16)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8_ReferenceIn(__in Windows::Foundation::IReference<byte> *inValue, __out boolean *isNull, __out boolean *isValidType, __out byte *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<byte>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<byte>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt8)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16_ReferenceIn(__in Windows::Foundation::IReference<short> *inValue, __out boolean *isNull, __out boolean *isValidType, __out short *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<short>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<short>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int16)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16_ReferenceIn(__in Windows::Foundation::IReference<unsigned short> *inValue, __out boolean *isNull, __out boolean *isValidType, __out unsigned short *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<unsigned short>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<unsigned short>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt16)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32_ReferenceIn(__in Windows::Foundation::IReference<INT32> *inValue, __out boolean *isNull, __out boolean *isValidType, __out INT32 *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<INT32>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<INT32>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int32)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32_ReferenceIn(__in Windows::Foundation::IReference<UINT32> *inValue, __out boolean *isNull, __out boolean *isValidType, __out UINT32 *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<UINT32>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<UINT32>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt32)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64_ReferenceIn(__in Windows::Foundation::IReference<INT64> *inValue, __out boolean *isNull, __out boolean *isValidType, __out INT64 *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<INT64>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<INT64>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Int64)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64_ReferenceIn(__in Windows::Foundation::IReference<UINT64> *inValue, __out boolean *isNull, __out boolean *isValidType, __out UINT64 *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<UINT64>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<UINT64>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_UInt64)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloat_ReferenceIn(__in Windows::Foundation::IReference<float> *inValue, __out boolean *isNull, __out boolean *isValidType, __out float *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<float>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<float>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Single)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDouble_ReferenceIn(__in Windows::Foundation::IReference<double> *inValue, __out boolean *isNull, __out boolean *isValidType, __out double *outValue)    
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<double>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<double>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Double)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuid_ReferenceIn(__in Windows::Foundation::IReference<GUID> *inValue, __out boolean *isNull, __out boolean *isValidType, __out GUID *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<GUID>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<GUID>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Guid)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_ReferenceIn(__in Windows::Foundation::IReference<Windows::Foundation::DateTime> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::DateTime *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Windows::Foundation::DateTime>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Windows::Foundation::DateTime>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_DateTime)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensions_ReferenceIn(__in Windows::Foundation::IReference<Dimensions> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Dimensions *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Dimensions>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Dimensions>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != Windows::Foundation::PropertyType_OtherType)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpan_ReferenceIn(__in Windows::Foundation::IReference<Windows::Foundation::TimeSpan> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::TimeSpan *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Windows::Foundation::TimeSpan>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Windows::Foundation::TimeSpan>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_TimeSpan)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }
        
    IFACEMETHODIMP CPropertyValueTests::TestPoint_ReferenceIn(__in Windows::Foundation::IReference<Windows::Foundation::Point> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Point *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Windows::Foundation::Point>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Windows::Foundation::Point>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Point)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSize_ReferenceIn(__in Windows::Foundation::IReference<Windows::Foundation::Size> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Size *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Windows::Foundation::Size>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Windows::Foundation::Size>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Size)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRect_ReferenceIn(__in Windows::Foundation::IReference<Windows::Foundation::Rect> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Windows::Foundation::Rect *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Windows::Foundation::Rect>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Windows::Foundation::Rect>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != PropertyType_Rect)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnum_ReferenceIn(__in Windows::Foundation::IReference<Phylum> *inValue, __out boolean *isNull, __out boolean *isValidType, __out Phylum *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        IfNullReturnError(isValidType, E_POINTER);
        IfNullReturnError(isNull, E_POINTER);

        HRESULT hr = S_OK;
        if (inValue == nullptr)
        {
            *isNull = true;
            *isValidType = true;
            return hr;
        }

        *isValidType = false;
        *isNull = false;

        ComPtr<IReference<Phylum>> referenceT;
        hr = inValue->QueryInterface(__uuidof(IReference<Phylum>), (LPVOID *)&referenceT);
        IfFailedReturn(hr);

        ComPtr<IPropertyValue> propertyValue;
        hr = inValue->QueryInterface(IID_IPropertyValue, (LPVOID *)&propertyValue);
        IfFailedReturn(hr);

        PropertyType propertyType;
        hr = propertyValue->get_Type(&propertyType);
        IfFailedReturn(hr);

        if (propertyType != Windows::Foundation::PropertyType_OtherType)
        {
            return hr;
        }

        *isValidType = true;
        return referenceT->get_Value(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestBoolean_ReferenceOut(__out Windows::Foundation::IReference<bool> **outValue, __in boolean inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestBoolean_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestChar16_ReferenceOut(__out Windows::Foundation::IReference<wchar_t> **outValue, __in wchar_t inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestChar16_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt8_ReferenceOut(__out Windows::Foundation::IReference<byte> **outValue, __in byte inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestUInt8_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt16_ReferenceOut(__out Windows::Foundation::IReference<short> **outValue, __in short inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestInt16_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt16_ReferenceOut(__out Windows::Foundation::IReference<unsigned short> **outValue, __in unsigned short inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestUInt16_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt32_ReferenceOut(__out Windows::Foundation::IReference<INT32> **outValue, __in INT32 inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestInt32_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt32_ReferenceOut(__out Windows::Foundation::IReference<UINT32> **outValue, __in UINT32 inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestUInt32_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestInt64_ReferenceOut(__out Windows::Foundation::IReference<INT64> **outValue, __in INT64 inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestInt64_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestUInt64_ReferenceOut(__out Windows::Foundation::IReference<UINT64> **outValue, __in UINT64 inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestUInt64_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestFloat_ReferenceOut(__out Windows::Foundation::IReference<float> **outValue, __in float inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestFloat_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }
        
    IFACEMETHODIMP CPropertyValueTests::TestDouble_ReferenceOut(__out Windows::Foundation::IReference<double> **outValue, __in double inValue)    
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestDouble_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestGuid_ReferenceOut(__out Windows::Foundation::IReference<GUID> **outValue, __in GUID inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestGuid_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDate_ReferenceOut(__out Windows::Foundation::IReference<Windows::Foundation::DateTime> **outValue, __in Windows::Foundation::DateTime inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestDate_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestDimensions_ReferenceOut(__out Windows::Foundation::IReference<Dimensions> **outValue, __in Windows::Foundation::IReference<Dimensions> *inValue)
    {
        return TestDimensions_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestTimeSpan_ReferenceOut(__out Windows::Foundation::IReference<Windows::Foundation::TimeSpan> **outValue, __in Windows::Foundation::TimeSpan inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestTimeSpan_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestPoint_ReferenceOut(__out Windows::Foundation::IReference<Windows::Foundation::Point> **outValue, __in Windows::Foundation::Point inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestPoint_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestSize_ReferenceOut(__out Windows::Foundation::IReference<Windows::Foundation::Size> **outValue, __in Windows::Foundation::Size inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestSize_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestRect_ReferenceOut(__out Windows::Foundation::IReference<Windows::Foundation::Rect> **outValue, __in Windows::Foundation::Rect inValue)
    {
        ComPtr<IPropertyValue> propertyValue;
        HRESULT hr = TestRect_InspectableOut(inValue, (IInspectable **)&propertyValue);
        IfFailedReturn(hr);

        return propertyValue.Get()->QueryInterface(outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::TestEnum_ReferenceOut(__out Windows::Foundation::IReference<Phylum> **outValue, __in Windows::Foundation::IReference<Phylum> *inValue)
    {
        return TestEnum_InspectableOut(inValue, (IInspectable **)outValue);
    }

    IFACEMETHODIMP CPropertyValueTests::get_MyPropertyValue(__out Windows::Foundation::IPropertyValue **value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = m_propertyValue;
        if (m_propertyValue)
        {
            m_propertyValue->AddRef();
        }
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::put_MyPropertyValue(__in Windows::Foundation::IPropertyValue *value)
    {
        if (m_propertyValue)
        {
            m_propertyValue->Release();
        }
        m_propertyValue = value;
        if (m_propertyValue)
        {
            m_propertyValue->AddRef();
        }
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::get_MyDimensionsReference(__out Windows::Foundation::IReference<Dimensions> **value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = m_dimensions;
        if (m_dimensions)
        {
            m_dimensions->AddRef();
        }
        return S_OK;
    }

    IFACEMETHODIMP CPropertyValueTests::put_MyDimensionsReference(__in Windows::Foundation::IReference<Dimensions> *value)
    {
        if (m_dimensions)
        {
            m_dimensions->Release();
        }
        m_dimensions = value;
        if (m_dimensions)
        {
            m_dimensions->AddRef();
        }
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue1::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_OtherType;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue1::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetChar16(__RPC__out WCHAR *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }


    IFACEMETHODIMP CRCPropertyValue1::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue1::get_Value(__RPC__out Dimensions *value)
    {
        IfNullReturnError(value, E_POINTER);
        (*value).Length = 100;
        (*value).Width = 20;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue2::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_OtherType;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue2::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetChar16(__RPC__out WCHAR *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue2::get_Value(__RPC__out Dimensions *value)
    {
        IfNullReturnError(value, E_POINTER);
        (*value).Length = 100;
        (*value).Width = 20;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue3::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_OtherType;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue3::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetChar16(__RPC__out WCHAR *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue3::get_Value(__RPC__out Dimensions *value)
    {
        IfNullReturnError(value, E_POINTER);
        (*value).Length = 100;
        (*value).Width = 20;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue4::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_Char16;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue4::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetChar16(__RPC__out WCHAR *value)
    {
        return get_Value(value);
    }

    IFACEMETHODIMP CRCPropertyValue4::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue4::get_Value(__RPC__out WCHAR *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = L'D';
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue5::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_Char16;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue5::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetChar16(__RPC__out WCHAR *value)
    {
        return get_Value(value);
    }

    IFACEMETHODIMP CRCPropertyValue5::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue5::get_Value(__RPC__out WCHAR *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = L'E';
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue6::get_Type(__RPC__out enum Windows::Foundation::PropertyType *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = Windows::Foundation::PropertyType_Char16;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue6::get_IsNumericScalar(__RPC__out boolean *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = false;
        return S_OK;
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt8(__RPC__out BYTE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt16(__RPC__out INT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt16(__RPC__out UINT16 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt32(__RPC__out INT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt32(__RPC__out UINT32 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt64(__RPC__out INT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt64(__RPC__out UINT64 *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetSingle(__RPC__out FLOAT *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetDouble(__RPC__out DOUBLE *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetChar16(__RPC__out WCHAR *value)
    {
        return get_Value(value);
    }

    IFACEMETHODIMP CRCPropertyValue6::GetBoolean(__RPC__out boolean *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetString(__RPC__deref_out_opt HSTRING *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetGuid(__RPC__out GUID *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetDateTime(__RPC__out Windows::Foundation::DateTime *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetTimeSpan(__RPC__out Windows::Foundation::TimeSpan *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetPoint(__RPC__out Windows::Foundation::Point *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetSize(__RPC__out Windows::Foundation::Size *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetRect(__RPC__out Windows::Foundation::Rect *value)
    {
        GetUnsupportedTypeFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt8Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) BYTE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT16 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt32Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT32 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) INT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetUInt64Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) UINT64 **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetSingleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) FLOAT **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetDoubleArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) DOUBLE **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetChar16Array(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) WCHAR **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetBooleanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) boolean **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetStringArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) HSTRING **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetInspectableArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) IInspectable ***value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetGuidArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) GUID **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetDateTimeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::DateTime **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetTimeSpanArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::TimeSpan **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetPointArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Point **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetSizeArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Size **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::GetRectArray(__RPC__out UINT32 *length, __RPC__deref_out_ecount_full_opt(*length) Windows::Foundation::Rect **value)
    {
        GetUnsupportedTypeArrayFromPV();
    }

    IFACEMETHODIMP CRCPropertyValue6::get_Value(__RPC__out WCHAR *value)
    {
        IfNullReturnError(value, E_POINTER);
        *value = L'F';
        return S_OK;
    }

    IFACEMETHODIMP CFailingGRCNString::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        IfNullReturnError(iids, E_POINTER);
        IfNullReturnError(iidCount, E_POINTER);
        *iidCount = 0;
        *iids = nullptr;
        return S_OK;
    }

    IFACEMETHODIMP CFailingGRCNString::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
    {
        IfNullReturnError(className, E_POINTER);
        return E_FAIL;
    }

    IFACEMETHODIMP CFailingGRCNString::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
    {
        IfNullReturnError(trustLevel, E_POINTER);
        *trustLevel = BaseTrust;
        return S_OK;
    }

    IFACEMETHODIMP CEmptyGRCN::GetMyClassName(__out HSTRING *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        return WindowsCreateString(L"CEmptyGRCN", (UINT32)wcslen(L"CEmptyGRCN"), outValue);
    }

    IFACEMETHODIMP CEmptyGRCNInterface::GetMyClassName(__out HSTRING *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        return WindowsCreateString(L"CEmptyGRCNInterface", (UINT32)wcslen(L"CEmptyGRCNInterface"), outValue);
    }

    IFACEMETHODIMP CEmptyFailingGRCNString::GetIids(__RPC__out ULONG *iidCount, __RPC__deref_out_ecount_full_opt(*iidCount) IID **iids)
    {
        IfNullReturnError(iids, E_POINTER);
        IfNullReturnError(iidCount, E_POINTER);
        *iidCount = 1;
        *iids = (IID *)CoTaskMemAlloc(sizeof(IID));
        (*iids)[0] = Animals::IID_IEmptyGRCN;
        return S_OK;
    }

    IFACEMETHODIMP CEmptyFailingGRCNString::GetRuntimeClassName(__RPC__deref_out_opt HSTRING *className)
    {
        IfNullReturnError(className, E_POINTER);
        return E_FAIL;
    }

    IFACEMETHODIMP CEmptyFailingGRCNString::GetTrustLevel(__RPC__out TrustLevel *trustLevel)
    {
        IfNullReturnError(trustLevel, E_POINTER);
        *trustLevel = BaseTrust;
        return S_OK;
    }

    IFACEMETHODIMP CEmptyFailingGRCNString::GetMyClassName(__out HSTRING *outValue)
    {
        IfNullReturnError(outValue, E_POINTER);
        return WindowsCreateString(L"CEmptyFailingGRCNString", (UINT32)wcslen(L"CEmptyFailingGRCNString"), outValue);
    }
}