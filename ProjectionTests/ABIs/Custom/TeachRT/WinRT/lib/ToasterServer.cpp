//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "kitchen_common.h"
#include "ToasterServer.h"
#include "ToastServer.h"

// Define this for convenience, but don't put this in a header, otherwise 
// it defeats the purpose of using a namespace.
using namespace Fabrikam::Kitchen;
using namespace Windows::Foundation::Diagnostics;

#define Assert NT_ASSERT

class BackgroundPreheatRequest
{
    LPSTREAM m_marshalledOnComplete;
    IToaster* m_toaster;
    HANDLE m_doneEvent;
    IToastCompleteHandler* m_onCompleteSmuggledPointer;
    HRESULT *m_hrDelegate;

public:

    static HRESULT BeginExecute(IToaster* toaster, HANDLE doneEvent, IToastCompleteHandler* onComplete, HRESULT *hrDelegate, bool smugglePointer)
    {
        Assert(toaster != NULL);

        HRESULT hr = S_OK;

        // Marshall the delegate interface prior to passing it to another thread.
        LPSTREAM marshalledOnComplete = NULL;
        if (onComplete && !smugglePointer)
        {
            hr = CoMarshalInterThreadInterfaceInStream(__uuidof(IToastCompleteHandler), (IUnknown*)onComplete, &marshalledOnComplete);
        }
        else if (smugglePointer && onComplete)
        {
            onComplete->AddRef();
        }
        Assert(SUCCEEDED(hr));
        
        // Create a ToastRequest object
        auto request = new BackgroundPreheatRequest(toaster, doneEvent, marshalledOnComplete, hrDelegate, (smugglePointer) ? onComplete : nullptr);
        
        // Start a worker thread. The thread will call the callback once complete.
        if(::CreateThread(NULL, 0, BackgroundPreheatRequest::WorkerThreadProc, request, 0, NULL) == NULL)
        {
            delete request;
            return E_FAIL;
        }

        return hr;
    }
    
private:
    BackgroundPreheatRequest(IToaster* toaster, HANDLE doneEvent, LPSTREAM marshalledOnComplete, HRESULT *hrDelegate, IToastCompleteHandler* onCompleteSmuggledPointer) 
    {
        m_toaster = toaster;
        m_marshalledOnComplete = marshalledOnComplete;
        m_doneEvent = doneEvent;
        m_onCompleteSmuggledPointer = onCompleteSmuggledPointer;
        m_hrDelegate = hrDelegate;
    }

    // Name: Execute
    // Info: Called from a worker thread to create a toast
    void Execute()
    {
        HRESULT hr = S_OK;
        
        // Emulate preheat operation
        ::Sleep(200);
        
        // Unmarshall the delegate interface pointer
        IToastCompleteHandler* onComplete = NULL;
        if (m_marshalledOnComplete)
        {
            hr = CoGetInterfaceAndReleaseStream(m_marshalledOnComplete, __uuidof(IToastCompleteHandler), (void**)(&onComplete));
        }
        Assert(SUCCEEDED(hr));

        if (m_onCompleteSmuggledPointer)
        {
            Assert(onComplete == nullptr);
            onComplete = m_onCompleteSmuggledPointer;
        }
        
        // Call the delegate
        if (onComplete)
        {
            *m_hrDelegate = onComplete->Invoke(m_toaster, nullptr);
            onComplete->Release();
        }

        m_toaster->InvokePreheatCompleteBackgroundEvents(m_toaster);

        ::SetEvent(m_doneEvent);
    }

    // Name: MakeToastWorker
    // Info: The thread proc
    // Args: param - a BackgroundPreheatRequest object pointer.
    static DWORD __stdcall WorkerThreadProc(LPVOID param) 
    {
        HRESULT hr = S_OK;
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        Assert(SUCCEEDED(hr));
        
        auto request = (BackgroundPreheatRequest*)param;
        Assert(request != NULL);
        
        // Execute the request
        request->Execute();
        
        // Free the BackgroundPreheatRequest object
        delete request;

        return 0;
    }
};

Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::ApplianceElectricityConsumptionReporter(HSTRING applianceName)
{
    WindowsDuplicateString(applianceName, &_applianceName);
}
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::~ApplianceElectricityConsumptionReporter()
{
    if(_applianceName)
    {
        WindowsDeleteString(_applianceName);
    }
}

IFACEMETHODIMP
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::get_ApplianceName(__out HSTRING *name) 
{
    if (name == NULL)
    {
        return E_POINTER;
    }

    WindowsDuplicateString(_applianceName, name);
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::add_ApplianceSwitchOnEvent( 
                __in Fabrikam::Kitchen::IApplianceSwitchOnHandler * switchOnHandler,
                __out EventRegistrationToken *pCookie)
{
    if (switchOnHandler == NULL)
    {
        return E_POINTER;
    }

    return _evtApplianceSwitchOnEvent.Add(switchOnHandler, pCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::remove_ApplianceSwitchOnEvent( 
                __in EventRegistrationToken iCookie) 
{
    return _evtApplianceSwitchOnEvent.Remove(iCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::add_ApplianceSwitchOffEvent( 
                __in Fabrikam::Kitchen::IApplianceSwitchOffHandler * switchOffHandler,
                __out EventRegistrationToken *pCookie)
{
    if (switchOffHandler == NULL)
    {
        return E_POINTER;
    }

    return _evtApplianceSwitchOffEvent.Add(switchOffHandler, pCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::remove_ApplianceSwitchOffEvent( 
                __in EventRegistrationToken iCookie) 
{
    return _evtApplianceSwitchOffEvent.Remove(iCookie);
}

void Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::InvokeSwitchOn(HSTRING msg)
{
    _evtApplianceSwitchOnEvent.InvokeAll(this, msg);
}

void Fabrikam::Kitchen::ApplianceElectricityConsumptionReporter::InvokeSwitchOff(HSTRING msg, UINT32 unitCount)
{
    _evtApplianceSwitchOffEvent.InvokeAll(this, msg, unitCount);
}

Fabrikam::Kitchen::ToasterServer::ToasterServer()
{
    HSTRING name = NULL;
    WindowsCreateString(L"Toaster", 7, &name);

    auto spElectricityReporter = Microsoft::WRL::Make<ApplianceElectricityConsumptionReporter>(name);
    WindowsDeleteString(name);

    _electricityReporter = spElectricityReporter.Detach();

    _indirectToaster = nullptr;
    _rootedHandler = nullptr;
}

Fabrikam::Kitchen::ToasterServer::~ToasterServer()
{
    if(_electricityReporter)
    {
        _electricityReporter->Release();
    }

    if (_indirectToaster)
    {
        _indirectToaster->Release();
    }

    if (_rootedHandler)
    {
        _rootedHandler->Release();
    }
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::get_Size(__out Dimensions *pdims)
{
    *pdims = _dims;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::put_Size(Dimensions dims)
{
    _dims = dims;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::get_ElectricityReporter(
    __out IApplianceElectricityConsumptionReporter **electricityReporter)
{
    if (electricityReporter == NULL)
    {
        return E_POINTER;
    }

    if (_electricityReporter)
    {
        *electricityReporter = _electricityReporter;
        _electricityReporter->AddRef();
        return S_OK;
    }
    
    return E_OUTOFMEMORY;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::MakeToast(
    __in HSTRING hstrMessage, 
    __deref_out IToast **ppToast)
{
    HRESULT hr;

    *ppToast = NULL;

    _electricityReporter->InvokeSwitchOn(hstrMessage);

    // Starting to make toast.
    _evtToastStart.InvokeAll(this);

    // As in classic COM, servers can provide instances of other servers. The
    // additional servers may be provided just by interface, or metadata can specify
    // that the returned types are runtime classes.
    auto spToast = Microsoft::WRL::Make<ToastServer>();

    if (spToast)
    {
        // Two-phase init is the recommended way to initialize newly allocated objects
        // within the implementation of servers that don't use exceptions, because
        // reporting HRESULTs from constructors is problematic.

        // This is an internal implementation detail. Two phase initialization MUST NOT 
        // be exposed at the ABI. Instead, use the one-phase construction pattern.
        // One phase construction will enable use of constructors to create types in
        // high-level language mappings.
        hr = spToast->PrivateInitialize(hstrMessage);
    }
    else
    {
        hr = E_OUTOFMEMORY;

        PCWSTR pszMessage;
        USHORT cchMessageExcludingNull;

        // Do not report errors from failure to set up reporting, load msgs, etc. If
        // the message can't be loaded, return the original HRESULT, and report nothing.
        // Use RtlLoadString to load string resources without taking a dependency on user32.dll
        if (STATUS_SUCCESS ==  RtlLoadString((HMODULE) &__ImageBase, 
                                             IDS_TOASTER_ALLOCATION_FAILURE,
                                             nullptr,  // lang - null means use default
                                             0,        // flags
                                             &pszMessage, 
                                             &cchMessageExcludingNull,
                                             nullptr,  // lang
                                             nullptr)) // lang length
        {
            OriginateError(hr, cchMessageExcludingNull, pszMessage);
        }
    }

    if (SUCCEEDED(hr))
    {
        // ignore failures during notification.
        *ppToast = spToast.Detach();
        _evtToastComplete.InvokeAll(this, *ppToast);
        _electricityReporter->InvokeSwitchOff(hstrMessage, 5);
    }

    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_IndirectToastCompleteEvent(
    __in IToastCompleteHandler * clickHandler,
    __out EventRegistrationToken * eventCookie)
{
    if (_indirectToaster == nullptr)
    {
        return E_UNEXPECTED;
    }

    return _indirectToaster->add_ToastCompleteEvent(clickHandler, eventCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_IndirectToastCompleteEvent(__in EventRegistrationToken eventCookie)
{
    if (_indirectToaster == nullptr)
    {
        return E_UNEXPECTED;
    }

    return _indirectToaster->remove_ToastCompleteEvent(eventCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::get_IndirectToaster(__out IToaster **toaster)
{
    if (toaster == nullptr)
    {
        return E_POINTER;
    }

    if (_indirectToaster != nullptr)
    {
        _indirectToaster->AddRef();
    }
    *toaster = _indirectToaster;
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::put_IndirectToaster(__in IToaster *toaster)
{
    if (_indirectToaster != nullptr)
    {
        _indirectToaster->Release();
    }

    _indirectToaster = toaster;
    if (_indirectToaster != nullptr)
    {
        _indirectToaster->AddRef();
    }

    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::IndirectMakeToast(
    __in HSTRING hstrMessage, 
    __deref_out IToast **ppToast)
{
    if (_indirectToaster == nullptr)
    {
        return E_UNEXPECTED;
    }

    return _indirectToaster->MakeToast(hstrMessage, ppToast);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_RootedToastCompleteEvent(
    __in IToastCompleteHandler * clickHandler,
    __out EventRegistrationToken * eventCookie)
{
    if (_indirectToaster == nullptr)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = add_ToastCompleteEvent(clickHandler, eventCookie);
    if (SUCCEEDED(hr))
    {
        _indirectToaster->put_RootedHandler(clickHandler);
    }

    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_RootedToastCompleteEvent(__in EventRegistrationToken eventCookie)
{
    return remove_ToastCompleteEvent(eventCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::get_RootedHandler(__out IToastCompleteHandler ** clickHandler)
{
    if (clickHandler == nullptr)
    {
        return E_POINTER;
    }

    if (_rootedHandler)
    {
        _rootedHandler->AddRef();
    }
    *clickHandler = _rootedHandler;

    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::put_RootedHandler(__in IToastCompleteHandler * clickHandler)
{
    if (_rootedHandler)
    {
        _rootedHandler->Release();
    }

    _rootedHandler = clickHandler;
    if (_rootedHandler)
    {
        _rootedHandler->AddRef();
    }

    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::InvokeRootedHandler(__in IToaster *sender, __in IToast *toast)
{
    if (_rootedHandler == nullptr)
    {
        return E_POINTER;
    }

    return _rootedHandler->Invoke(sender, toast);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::PreheatInBackground(
    __in Fabrikam::Kitchen::IToastCompleteHandler* onPreheatComplete)
{
    _evtToasterPreheatStart.InvokeAll();
    HANDLE doneEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    HRESULT hr = S_OK;
    BackgroundPreheatRequest::BeginExecute(this, doneEvent, onPreheatComplete, &hr, false);
    //AtlWaitWithMessageLoop(doneEvent); // this does not work on wwahost, it blocks the COM RPC
    DWORD dwIndex;
    CoWaitForMultipleHandles(COWAIT_DISPATCH_CALLS | COWAIT_DISPATCH_WINDOW_MESSAGES, INFINITE, 1, &doneEvent, &dwIndex);
    return hr;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_PreheatCompleteBackground( 
    __in Fabrikam::Kitchen::IToastCompleteHandler *inHandler,
    __out EventRegistrationToken *pCookie)
{
    _evtToasterPreheatBackground.Add(inHandler, pCookie);
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_PreheatCompleteBackground( 
    __in EventRegistrationToken iCookie)
{
    _evtToasterPreheatBackground.Remove(iCookie);
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::InvokePreheatCompleteBackgroundEvents(__in Fabrikam::Kitchen::IToaster *sender)
{
    _evtToasterPreheatBackground.InvokeAll(sender, nullptr);
    return S_OK;
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::PreheatInBackgroundWithSmuggledDelegate(
    __in Fabrikam::Kitchen::IToastCompleteHandler* onPreheatComplete)
{
    _evtToasterPreheatStart.InvokeAll();
    HANDLE doneEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    HRESULT hr = S_OK;
    BackgroundPreheatRequest::BeginExecute(this, doneEvent, onPreheatComplete, &hr, true);
    //AtlWaitWithMessageLoop(doneEvent); // this does not work on wwahost, it blocks the COM RPC
    DWORD dwIndex;
    CoWaitForMultipleHandles(COWAIT_DISPATCH_CALLS | COWAIT_DISPATCH_WINDOW_MESSAGES, INFINITE, 1, &doneEvent, &dwIndex);
    return hr;
}
IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_ToastCompleteEvent(
    __in Fabrikam::Kitchen::IToastCompleteHandler *clickHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtToastComplete.Add(clickHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_ToastCompleteEvent(
    __in EventRegistrationToken iCookie)
{
    return _evtToastComplete.Remove(iCookie);
}


IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_ToastStartEvent(
    __in Fabrikam::Kitchen::IToastStartHandler *clickHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtToastStart.Add(clickHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_ToastStartEvent(
    __in EventRegistrationToken iCookie)
{
    return _evtToastStart.Remove(iCookie);
}

IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::add_PreheatStart(
    __in Fabrikam::Kitchen::IToasterPreheatStartHandler *inHandler,
    __out EventRegistrationToken *pCookie)
{
    return _evtToasterPreheatStart.Add(inHandler, pCookie);
}
                
IFACEMETHODIMP
Fabrikam::Kitchen::ToasterServer::remove_PreheatStart(
    __in EventRegistrationToken iCookie)
{
    return _evtToasterPreheatStart.Remove(iCookie);
}
