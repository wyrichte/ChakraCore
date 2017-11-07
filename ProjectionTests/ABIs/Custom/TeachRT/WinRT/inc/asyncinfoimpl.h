// Copyright (c) Microsoft Corporation.  All Rights Reserved

#pragma once

#include <wrl\implements.h>
#include <wrl\event.h>
#include <new.h>
#include <winerror.h>
#include <ntassert.h>
#include <intrin.h>

// placeholder for missing definition coming from fbl_rex_com_core
#ifndef E_ILLEGAL_METHOD_CALL
#define E_ILLEGAL_METHOD_CALL            _HRESULT_TYPEDEF_(0x8000000EL)
#endif

namespace Microsoft {
namespace WRL {
namespace Details {
// maps internal definitions for AsyncStatus and defines states that are not client visible
enum AsyncStatusInternal 
{
    // client visible states (must match AsyncStatus exactly)
    _Started = Started,
    _Completed = Completed,
    _Cancelled = Canceled,
    _Error = Error,

    // non-client visible internal states
    _Created,
    _Closed,
    _Undefined
};

template < typename T >
struct DerefHelper;

template < typename T >
struct DerefHelper<T*>
{
	typedef T DerefType;
};
}

// CAsyncInfo - base class that implements the WinRT Async state machine
// this base class is designed to be used with minATL to implement an async worker object
// derivation usage: class CMyWorkObject : public Windows::WRL::RuntimeClass<CAsyncInfo>

template < typename TComplete, typename TProgress = Details::Nil >
class AsyncBase : public AsyncBase< TComplete, Details::Nil >
{
    typedef typename Details::ArgTraitsHelper< TProgress >::Traits ProgressTraits;
    friend class AsyncBase< TComplete, Details::Nil >;

public:

    // since this is designed to be used inside of an RuntimeClass<> template, we can 
    // only have a default ctor
    AsyncBase() : 
        progressDelegate_(nullptr) 
    {
    }

    // Delegate Helpers
    STDMETHOD(PutOnProgress)(TProgress* progressHandler)
    {
        HRESULT hr = S_OK;
        hr = CheckValidStateForDelegateCall();
        if (SUCCEEDED(hr))
        {
            progressDelegate_ = progressHandler;
        }
        return hr;
    }

    STDMETHOD(GetOnProgress)(TProgress** progressHandler)
    {
        progressDelegate_.CopyTo(progressHandler);
        return CheckValidStateForDelegateCall();
    }

    void FireProgress(const typename ProgressTraits::Arg2Type arg)
    {
        ComPtr<Windows::Foundation::IAsyncInfo> asyncInfo = this;
        ComPtr<Details::DerefHelper<ProgressTraits::Arg1Type>::DerefType> operationInterface;
        if (progressDelegate_ && SUCCEEDED(asyncInfo.As(&operationInterface)))
        {
            progressDelegate_->Invoke(operationInterface.Get(), arg);
        }
    }

    void FireCompletion(void) override
    {
        AsyncBase< TComplete, Details::Nil >::FireCompletion();
        if (progressDelegate_)
        {
            progressDelegate_ = nullptr;
        }
    }

private:
    Microsoft::WRL::ComPtr<TProgress> progressDelegate_;
};

template < typename TComplete >
class AsyncBase< TComplete, Details::Nil > : public Microsoft::WRL::Implements< Windows::Foundation::IAsyncInfo >
{
    typedef typename Details::ArgTraitsHelper< TComplete >::Traits CompleteTraits;

public:

    // since this is designed to be used inside of an activatableclass<> template, we can 
    // only have a default ctor
    AsyncBase() : 
        currentStatus_(Details::AsyncStatusInternal::_Created), 
        id_(1), 
        errorCode_(S_OK),
        completeDelegate_(nullptr)
    {     
    }

    STDMETHOD(put_Id)(const unsigned int id)
    {
        if (id == 0)
        {
            return E_INVALIDARG;
        }
        id_ = id;
        return CheckValidStateForAsyncInfoCall();
    }

    // IAsyncInfo Methods
    STDMETHOD(get_Id)(unsigned int *id) override
    {
        *id = id_;
        return CheckValidStateForAsyncInfoCall();
    }

    STDMETHOD(get_Status)(Windows::Foundation::AsyncStatus *status) override
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);
        *status = static_cast<Windows::Foundation::AsyncStatus>(current);
        return CheckValidStateForAsyncInfoCall();
    }

    STDMETHOD(get_ErrorCode)(HRESULT* errorCode) override
    {
        *errorCode = errorCode_;
        return CheckValidStateForAsyncInfoCall();
    }

    STDMETHOD(Start)(void)
    {
        HRESULT hr = S_OK;
        if (TransitionToState(Details::_Started))
        {
            hr = OnStart();
        }
        else
        {
            hr = E_ILLEGAL_STATE_CHANGE;
        }
        return hr;
    }

    STDMETHOD(Cancel)(void)
    {
        if (TransitionToState(Details::_Cancelled))
        {
            OnCancel();
        }
        return S_OK;
    }

    STDMETHOD(Close)(void) override
    {
        HRESULT hr = S_OK;
        if (TransitionToState(Details::_Closed)) 
        {
            OnClose();
        }
        else
        {
            hr = E_ILLEGAL_STATE_CHANGE;
        }
        return hr;    
    }

    // Delegate helpers
    STDMETHOD(PutOnComplete)(TComplete* completeHandler)
    {
        HRESULT hr = S_OK;
        hr = CheckValidStateForDelegateCall();
        if (SUCCEEDED(hr))
        {
            completeDelegate_ = completeHandler;
        }
        return hr;
    }

    STDMETHOD(GetOnComplete)(TComplete** completeHandler)
    {
        completeDelegate_.CopyTo(completeHandler);
        return CheckValidStateForDelegateCall();
    }

    virtual void FireCompletion()
    {
        ComPtr<Windows::Foundation::IAsyncInfo> asyncInfo = this;
        ComPtr<Details::DerefHelper<CompleteTraits::Arg1Type>::DerefType> operationInterface;
        TryTransitionToCompleted();
        if (completeDelegate_ && SUCCEEDED(asyncInfo.As(&operationInterface)))
        {
            Details::AsyncStatusInternal current = Details::_Undefined;
            CurrentStatus(&current);
            completeDelegate_->Invoke(operationInterface.Get(), static_cast<AsyncStatus>(current));
            completeDelegate_ = nullptr;
        }
    }

protected:

    inline void CurrentStatus(Details::AsyncStatusInternal *status) 
    {
        InterlockedCompareExchange(reinterpret_cast<LONG*>(status), currentStatus_, static_cast<LONG>(*status));
        NT_ASSERT(*status != Details::_Undefined);
    }

    inline void ErrorCode(HRESULT *error) 
    {
        InterlockedCompareExchange(reinterpret_cast<LONG*>(error), errorCode_, static_cast<LONG>(*error));
    }

    //
    // TransitionToStateInternal - this is the form used by the async work object
    //                             this form does an unconditional transition to one of 
    //                             the 3 terminal states
    //                             it returns the new (terminal) state
    bool TryTransitionToCompleted(void)
    {
        return TransitionToState(Details::AsyncStatusInternal::_Completed);
    }

    bool TryTransitionToError(const HRESULT error)
    {
        // this ICE call prevents an existing error code from being overwritten, i.e. it will only
        // write the error if we are currently holding the default S_OK value
        _InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&errorCode_), error, S_OK);
        return TransitionToState(Details::AsyncStatusInternal::_Error);
        // if TransitionToState() returns true, then we did a valid state transition
        // queue firing of completed event (cannot be done from this call frame)
        // otherwise we are already in a terminal state: error, cancelled, completed, or closed
        // and we ignore the transition request to the Error state
    }

    // This method checks to see if the delegate properties can be 
    // modified in the current state and generates the appropriate 
    // errro hr in the case of violation.
    inline HRESULT CheckValidStateForDelegateCall()
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);
        if (current != Details::_Created)
        {
            return E_ILLEGAL_METHOD_CALL;
        }
        return S_OK;
    }

    // This method checks to see if results can be collected in the 
    // current state and generates the appropriate error hr in 
    // the case of a violation.
    inline HRESULT CheckValidStateForResultsCall()
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);
        if (current != Details::_Started && 
            current != Details::_Completed)
        {
            return E_ILLEGAL_METHOD_CALL;
        }
        return S_OK;
    }
	
    // This method can be called by derived classes periodically to determine 
    // whether the asynchronous operation should continue processing or should 
    // be halted.
    inline bool ContinueAsyncOperation()
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);
        return (current == Details::_Started);
    }

    // These two methods are used to allow the async worker implementation do work on
    // state transitions. No real "work" should be done in these methods. In other words
    // they should not block for a long time on UI timescales.
    virtual HRESULT OnStart(void) = 0;
    virtual void OnClose(void) = 0;
    virtual void OnCancel(void) = 0;

private:

    // This method is used to check if calls to the AsyncInfo properties
    // (id, status, errorcode) are legal in the current state. It also 
    // generates the appropriate error hr to return in the case of an 
    // illegal call.
    inline HRESULT CheckValidStateForAsyncInfoCall()
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);
        if (current == Details::_Closed)
        {
            return E_ILLEGAL_METHOD_CALL;
        }
        return S_OK;
    }

    inline bool TransitionToState(const Details::AsyncStatusInternal newState)
    {
        Details::AsyncStatusInternal current = Details::_Undefined;
        CurrentStatus(&current);

        // This enforces the valid state transitions of the asynchronous worker object 
        // state machine.
        switch(newState)
        {
        case Details::_Started:
            if (current != Details::_Created) 
            {
                return false;
            }
            break;
        case Details::_Completed:
            if (current != Details::_Started) 
            {
                return false;
            }
            break;
        case Details::_Cancelled:
            if (current != Details::_Created && 
               current != Details::_Started) 
            {
                return false;
            }
            break;
        case Details::_Error:
            if (current != Details::_Started) 
            {
                return false;
            }
            break;
        case Details::_Closed:
            if (!IsTerminalState(current)) 
            {
                return false;
            }
            break;
        default:
            return false;
            break;
        }
        // attempt the transition to the new state
        // Note: if currentStatus_ == current, then there was no intervening write 
        // by the async work object and the swap succeeded. 
        Details::AsyncStatusInternal retState = static_cast<Details::AsyncStatusInternal>(
                _InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(&currentStatus_), 
                                            newState, 
                                            static_cast<LONG>(current)));

        // ICE returns the former state, if the returned state and the 
        // state we captured at the beginning of this method are the same, 
        // the swap succeeded.
        return (retState == current);
    }

    inline bool IsTerminalState()
    {
        AsyncStatusInternal current = _Undefined;
        CurrentStatus(&current);
        return IsTerminalState(current);
    }

    inline bool IsTerminalState(Details::AsyncStatusInternal status)
    {
        return (status == Details::_Error || 
                status == Details::_Cancelled || 
                status == Details::_Completed || 
                status == Details::_Closed);
    }

private:
    Microsoft::WRL::ComPtr<TComplete> completeDelegate_;
    Details::AsyncStatusInternal volatile currentStatus_;
    HRESULT volatile errorCode_;
    unsigned int id_;
};

} // namespace Microsoft 
} // namespace WRL
