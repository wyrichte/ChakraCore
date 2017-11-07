//  Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "OvenServer.h"
#include "CookieServer.h"
#include "PrivateInterfaces.h"
#include <wrl\async.h>
#include <winrt\windowscollectionsp.h>

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Collections::Internal;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;
using namespace Microsoft::WRL::Wrappers;

namespace Fabrikam {
namespace Kitchen {

#pragma region AsyncInfo Classes

class BakeOperationServer : 
    public Microsoft::WRL::RuntimeClass<
        Fabrikam::Kitchen::IBakeOperation,
        Microsoft::WRL::AsyncBase<IBakingCompletedHandler, IBakingProgressHandler>>
{
    InspectableClass(L"Fabrikam.Kitchen.IBakeOperation", FullTrust);

public:

    BakeOperationServer(int iAsyncId, int iNumCookies) :
        _iNumCookies(iNumCookies),
        _iElapsed(0),
        _hThread(NULL)
    {
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr))
        {
            hr = AsyncBase::put_Id(iAsyncId);
            if (FAILED(hr))
            {
                // This helps to propagate the error code back up to 
                // the caller of this constructor.
                AsyncBase::TryTransitionToError(hr);
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = Vector<ICookie*>::Make(&_spCookies);
            if (FAILED(hr))
            {
                // This helps to propagate the error code back up to 
                // the caller of this constructor.
                AsyncBase::TryTransitionToError(hr);
            }
        }
    }

    virtual ~BakeOperationServer()
    {
        // Make sure that on close is called even if the client
        // didn't explicitly call Close(). The OnClose method in 
        // this object is safe to call multiple times.
        OnClose();
        if (_hThread)
        {
            CloseHandle(_hThread);
        }
    }

    // IBakeOperation::put_Progress
    IFACEMETHODIMP put_Progress(IBakingProgressHandler *pProgressHandler) override
    {
        return AsyncBase::PutOnProgress(pProgressHandler);
    }

    // IBakeOperation::get_Progress
    IFACEMETHODIMP get_Progress(IBakingProgressHandler **ppProgressHandler) override
    {
        return AsyncBase::GetOnProgress(ppProgressHandler);
    }

    // IBakeOperation::put_Complete
    IFACEMETHODIMP put_Completed(IBakingCompletedHandler *pCompleteHandler) override
    {
        return AsyncBase::PutOnComplete(pCompleteHandler);
    }

    // IBakeOperation::get_Complete
    IFACEMETHODIMP get_Completed(IBakingCompletedHandler **ppCompleteHandler) override
    {
        return AsyncBase::GetOnComplete(ppCompleteHandler);
    }

    // IBakeOperation::GetResults
    IFACEMETHODIMP GetResults(IVectorView<ICookie*> **cookies) override
    {
        *cookies = NULL;       
        SRWLock::SyncLockExclusive srwExclusive = _srwVectorLock.LockExclusive();
        HRESULT hr = AsyncBase::CheckValidStateForResultsCall();
        if (SUCCEEDED(hr))
        {
            hr = _spCookies->GetView(cookies);
        }
        if (SUCCEEDED(hr))
        {
            // In order to not change the Vector once we have handed out 
            // a view to it we create a new Vector here that will store 
            // subsequent results. This allows the results to be deleted 
            // once the client is done with them. 
            hr = Vector<ICookie*>::Make(&_spCookies);
        }
        return hr;
    }

    HRESULT StartOperation()
    {
        return AsyncBase::Start();
    }

    // AsyncBase::OnStart
    HRESULT OnStart() override
    {
        HRESULT hr = S_OK;
        // Give a ref to the thread.
        AddRef();
        _hThread = CreateThread(NULL, 0, DoBakeAsyncStub, this, 0, &_dwThreadId);
        if (_hThread == NULL)
        {
            // If the thread didn't get created we need to release its ref.
            Release();
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        return hr;
    }

    // AsyncBase::OnClose
    void OnClose() override
    {
        // If we had any expensive resources that we were holding on
        // to we would release them here. 
    }

    // AsyncBase::OnCancel
    void OnCancel() override
    {
        // If we needed to perform any operations as a result of 
        // cancellation they would be here.
    }

private:

    // This static function is the thread entry point for BakeOperation 
    // objects.
    static DWORD WINAPI DoBakeAsyncStub(LPVOID lpParam)
    {
        HRESULT hr = S_OK;
        HMODULE hModule;
        Fabrikam::Kitchen::BakeOperationServer* pThis;

        // We must take a lock on the module to make sure that the DLL is not 
        // unloaded prior to exiting the threadproc. 
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
                          reinterpret_cast<LPCWSTR>(DoBakeAsyncStub), 
                          &hModule);
        pThis = reinterpret_cast<Fabrikam::Kitchen::BakeOperationServer*>(lpParam);
        if (pThis)
        {
            // Since this is a new thread make sure it is initialized. This runtime 
            // class is marked as an MTA only object so we should always initialize
            // this thread to be MTA. Alternatively this runtime class could be 
            // marked as Both and we could use the GIT to create the appropriate
            // interface pointer. 
            hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
            if (SUCCEEDED(hr))
            {
                pThis->DoBake();
                // Release the worker thread's ref on the object.
                pThis->Release();
                Windows::Foundation::Uninitialize();
            }
            else
            {
                pThis->TryTransitionToError(hr);
                // Release the worker thread's ref on the object.
                pThis->Release();
            }
        }
        FreeLibraryAndExitThread(hModule, 0);
    }

    // This is where the async operation is actually performed. This 
    // method is executed on its own thread
    void DoBake()
    {
        HRESULT hr;

        for (int i = 0; i < _iNumCookies; i++)
        {
            ComPtr<CookieServer> spCookieServer;
            ComPtr<ICookie> spCookie;

            // Each cookie takes 100ms to cook
            ::Sleep(100);
            _iElapsed += 100;
            
            // Add the baked cookie
            spCookieServer = Make<CookieServer>();
            hr = spCookieServer.As(&spCookie);
            if (SUCCEEDED(hr))
            {
                switch (i%4)
                {
                    case 0:
                        spCookieServer->put_Doneness(CookieDoneness_Raw);
                        break;
                    case 1:
                        spCookieServer->put_Doneness(CookieDoneness_Gooey);
                        break;
                    case 2:
                        spCookieServer->put_Doneness(CookieDoneness_Golden);
                        break;
                    case 3:
                        spCookieServer->put_Doneness(CookieDoneness_Burnt);
                        break;
                }
                
                // Scoping for lock lifetime
                {
                    SRWLock::SyncLockExclusive srwExclusive = _srwVectorLock.LockExclusive();
                    _spCookies->Append(spCookie.Detach());
                }
            }

            // Send a progress event
            AsyncBase::FireProgress(_iElapsed);

            if (!AsyncBase::ContinueAsyncOperation())
            {   
                break;
            }
        }
        // This attempts to transition to the completed state and fires the completed event. 
        // We may already be in another terminal state in which case we still need to fire
        // a completed event.
        AsyncBase::FireCompletion();
    }

private:

    DWORD                           _dwThreadId;
    HANDLE                          _hThread;
    int                             _iNumCookies;
    int                             _iElapsed;
    SRWLock                         _srwVectorLock;

    // In order to use the Vector store the PrivateInterface.idl file had to 
    // be defined. This specializes IVector<ICookie*> appropriately. Unfortunately
    // this introduces an interface and an idl file that we do not really need.
    // A solution to this issue will be forthcoming.
    ComPtr<Vector<ICookie*>>        _spCookies;
};

class TimerOperationServer :
    public Microsoft::WRL::RuntimeClass<
        Fabrikam::Kitchen::ITimerOperation, 
        Microsoft::WRL::AsyncBase<ITimerCompletedHandler>>
{
    InspectableClass(L"Fabrikam.Kitchen.ITimerOperation", FullTrust);

public:

    TimerOperationServer(int iAsyncId, int iDuration) :
        _iDuration(iDuration), 
        _hTimerQueue(NULL),
        _hTimer(NULL)
    {
        HRESULT hr = S_OK;

        hr = AsyncBase::put_Id(iAsyncId);
        if (FAILED(hr))
        {
            // This helps to propagate the error code back up to 
            // the caller of this constructor.
            AsyncBase::TryTransitionToError(hr);
        }
    }

    virtual ~TimerOperationServer()
    {
        // Make sure that on close is called even if the client
        // didn't explicitly call Close(). The OnClose method in 
        // this object is safe to call multiple times.
        OnClose();
    }

    // ITimerOperation::put_Completed
    IFACEMETHODIMP put_Completed(ITimerCompletedHandler *pCompleteHandler) override
    {
        return AsyncBase::PutOnComplete(pCompleteHandler);
    }

    // IBakeOperation::get_Complete
    IFACEMETHODIMP get_Completed(ITimerCompletedHandler **ppCompleteHandler) override
    {
        return AsyncBase::GetOnComplete(ppCompleteHandler);
    }

    // ITimerOperation::GetResults
    IFACEMETHODIMP GetResults(boolean *bTimerFired) override
    {
        *bTimerFired = _bTimerFired;
        return AsyncBase::CheckValidStateForResultsCall();;
    }

protected:

    // AsyncBase::OnStart
    HRESULT OnStart() override
    {
        HRESULT hr = S_OK;

        // Create the timer queue.
        _hTimerQueue = CreateTimerQueue();
        if (NULL == _hTimerQueue)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        if (SUCCEEDED(hr))
        {
            // In order to make sure that this object still exsists whent the timer
            // returns we take a ref here. 
            AddRef();
            if (!CreateTimerQueueTimer(&_hTimer, _hTimerQueue, 
                                       TimerRoutine, reinterpret_cast<PVOID>(this), _iDuration, 0, 0))
            {
                Release();
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        return hr;
    }

    // AsyncBase::OnClose
    void OnClose() override
    {
        if (_hTimerQueue)
        {
            DeleteTimerQueueEx(_hTimerQueue, NULL);
            _hTimerQueue = NULL;
        }
        // If we had any expensive resources that we were holding on
        // to we would release them here.
    }

    // AsyncBase::OnCancel
    void OnCancel() override
    {
        // If we needed to perform any operations as a result of 
        // cancellation they would be here.
    }

private:

    static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
    {
        HRESULT hr = S_OK;
        HMODULE hModule;

        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(TimerRoutine), &hModule);
        Fabrikam::Kitchen::TimerOperationServer* pThis = 
            reinterpret_cast<Fabrikam::Kitchen::TimerOperationServer*>(lpParam);
        if (pThis)
        {
            hr = Windows::Foundation::Initialize(RO_INIT_MULTITHREADED);
            if (SUCCEEDED(hr))
            {
                pThis->TimerFired(TimerOrWaitFired);
                // Release the timer's ref on the object.
                pThis->Release();
                Windows::Foundation::Uninitialize();
            }
            else
            {
                pThis->TryTransitionToError(hr);
                // Release the timer's ref on the object.
                pThis->Release();
            }
        }
        FreeLibrary(hModule);
    }

    void TimerFired(BOOLEAN bTimerFired)
    {
        _bTimerFired = bTimerFired;
        AsyncBase::FireCompletion();
    }

private:

    HANDLE                         _hTimer;
    HANDLE                         _hTimerQueue;
    ComPtr<ITimerCompletedHandler> _spCompleteHandler;
    int                            _iDuration;
    BOOLEAN                        _bTimerFired;
};

#pragma endregion

IFACEMETHODIMP
OvenServer::get_Size(__out Dimensions *pdims)
{
    *pdims = _dims;
    return S_OK;
}

IFACEMETHODIMP
OvenServer::put_Size(Dimensions dims)
{
    _dims = dims;
    return S_OK;
}

IFACEMETHODIMP
OvenServer::get_ElectricityReporter(
    __out IApplianceElectricityConsumptionReporter **electricityReporter)
{
    if (electricityReporter == NULL)
    {
        return E_POINTER;
    }

    *electricityReporter = NULL;
    return S_OK;
}

// This is an example of a simple single-result asynchronous method
// that does not return partial results or progress notifications.
IFACEMETHODIMP
OvenServer::TimerAsync(
    __in int iDuration,
    __deref_out ITimerOperation** asyncInfo)
{
    HRESULT hr = S_OK;
    *asyncInfo = nullptr;

    ComPtr<ITimerOperation> spAsyncInfo;
    ComPtr<TimerOperationServer>  spAsyncWorker = Make<TimerOperationServer>(_id, iDuration);

    // Ensure the allocation succeeded
    if (spAsyncWorker)
    {
        // the ctor may have failed, capture that error state
        Windows::Foundation::AsyncStatus status;
        // This should not fail here since we are just constructing
        // the object and should not be in the _Closed state. This
        // may fail any time after the interface has been given to the 
        // client.
        HRESULT hrStatus = spAsyncWorker->get_Status(&status);
        // Tolerate E_ASYNC_OPERATION_NOT_STARTED for now (operation hasn't been started at this point),
        // until Win8: 593998 is resolved
        NT_VERIFY(SUCCEEDED(hrStatus) || (hrStatus == E_ASYNC_OPERATION_NOT_STARTED));
        if (status != static_cast<Windows::Foundation::AsyncStatus>(Error)) // the ctor didn't fail
        {
            hr = spAsyncWorker.As(&spAsyncInfo);
            if (SUCCEEDED(hr))
            {
                hr = spAsyncInfo.CopyTo(asyncInfo);
            }
        }
        else
        {
            // Capture the ctor error code for the caller
            // Since we didn't detach the ComPtr the object
            // will be destroyed at the end of this function.
            NT_VERIFY(SUCCEEDED(spAsyncWorker->get_ErrorCode(&hr)));
        }
        // Increment _id to give each worker object a unique id.
        _id++;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

// This is an example of a more advanced multiple-result asynchronous 
// method that returns partial results and progress notifications.
IFACEMETHODIMP
OvenServer::BakeAsync(
    __in int iNumCookies,
    __deref_out IBakeOperation** asyncInfo)
{
    HRESULT hr = S_OK;
    *asyncInfo = nullptr;

    ComPtr<IBakeOperation> spAsyncInfo;
    ComPtr<BakeOperationServer>  spAsyncWorker = Make<BakeOperationServer>(_id, iNumCookies);

    // Ensure the allocation succeeded
    if (spAsyncWorker)
    {
        // the ctor may have failed, capture that error state
        Windows::Foundation::AsyncStatus status;
        // This should not fail here since we are just constructing
        // the object and should not be in the _Closed state. This
        // may fail any time after the interface has been given to the 
        // client.
        HRESULT hrStatus = spAsyncWorker->get_Status(&status);
        // Tolerate E_ASYNC_OPERATION_NOT_STARTED for now (operation hasn't been started at this point),
        // until Win8: 593998 is resolved
        NT_VERIFY(SUCCEEDED(hrStatus) || (hrStatus == E_ASYNC_OPERATION_NOT_STARTED));
        if (status != static_cast<Windows::Foundation::AsyncStatus>(Error)) // the ctor didn't fail
        {
            hr = spAsyncWorker.As(&spAsyncInfo);
        }
        else
        {
            // Capture the ctor error code for the caller
            // Since we didn't detach the ComPtr the object
            // will be destroyed at the end of this function.
            NT_VERIFY(SUCCEEDED(spAsyncWorker->get_ErrorCode(&hr)));
        }
        // Increment _id to give each worker object a unique id. Rollover is
        // very unlikely here but should not cause any problems should it occur.
        _id++;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = spAsyncWorker->StartOperation();
        if (SUCCEEDED(hr))
        {
            *asyncInfo = spAsyncInfo.Detach();
        }
    }

    return hr;
}

} // Namespace Kitchen
} // Namespace Fabrikam
