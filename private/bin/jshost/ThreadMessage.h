/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

// This is a helper class to allow us to communicate from the main thread to the host
// thread and vice versa based on message pumps and post messages.

// This helper automates the process of gathering the member function pointer
// and args into a struct posting the message, calling the function, then
// deleting the stuct.

typedef enum
{
    M_IGNORED,
    M_HANDLED,
    M_QUIT
} MessageReply;

template<typename classType>
class Message
{
    classType* _this;
    DWORD _threadId;
    static const int WM_MSG_PIPE = WM_USER+1;

public:
    class FunctionCallResult;


private:
    class FunctionObject
    {
        HRESULT _hr;
        LONG _refCount;
        HANDLE _event;

    protected:
        virtual HRESULT InternalMakeCall() = 0;

    public:
        FunctionObject()
            : _hr(S_OK),
              _refCount(1),
              _event(NULL)
        {
        }

        ULONG STDMETHODCALLTYPE AddRef()
        {
            return (ULONG)InterlockedIncrement(&_refCount);
        }

        ULONG STDMETHODCALLTYPE Release()
        {
            long currentCount = InterlockedDecrement(&_refCount);
            if (currentCount == 0)
            {
                delete this;
            }
            return currentCount;
        }

        virtual ~FunctionObject()
        {
            if (_event)
            {
                CloseHandle(_event);
            }
        }
        virtual MessageReply MsgReply()
        {
            return M_HANDLED;
        }
        void MakeCall()
        {
            _hr = InternalMakeCall();
            if (_event)
            {
                SetEvent(_event);
            }
        }

        HRESULT BlockOnResult(DWORD dwMilliseconds)
        {
            Assert(_event);
            WaitForSingleObject(_event, dwMilliseconds);

            // an extra wait (10ms) here to have a padding before getting started with execution.
            ::Sleep(10);

            return _hr;
        }

        HRESULT PostMessageObject(Message* message, FunctionCallResult* pResult)
        {
            if (pResult)
            {
                // Create an Auto-reset Event in an intial state of not signaled
                _event = CreateEvent(NULL, FALSE, FALSE, NULL);
                pResult->SetResultFunction(this);
            }

            PostThreadMessage(message->_threadId, WM_MSG_PIPE, (WPARAM)this, (LPARAM)message->_this);
            return S_OK;
        }

        HANDLE GetTheWaitEvent()
        {
            return _event;
        }
    };

public:
    class FunctionCallResult
    {
        FunctionObject * _pFuncObj;

    public:
        void SetResultFunction(FunctionObject* pFuncObj)
        {
            _pFuncObj = pFuncObj;
            if (_pFuncObj)
            {
                _pFuncObj->AddRef();
            }
        }

        FunctionCallResult()
            :_pFuncObj(NULL)
        {
        }
        ~FunctionCallResult()
        {
            if (_pFuncObj)
            {
                _pFuncObj->Release();
            }
        }

        HANDLE GetTheWaitEvent()
        {
            return _pFuncObj->GetTheWaitEvent();
        }

        HRESULT BlockOnResult(DWORD dwMilliseconds = INFINITE)
        {
            return _pFuncObj->BlockOnResult(dwMilliseconds);
        }
    };

private:

    template<typename PtrMemberFunction>
    class ThreadQuitFunction: public FunctionObject
    {
        classType* _this;
        PtrMemberFunction _pFunc;

        virtual MessageReply MsgReply()
        {
            return M_QUIT;
        }

        virtual HRESULT InternalMakeCall()
        {
            return (*_this.*_pFunc)();
        }

    public:
        ThreadQuitFunction(classType* classInstance, PtrMemberFunction pFunc)
            :_this(classInstance),
            _pFunc(pFunc)
        {
        }

    };

    template<typename PtrMemberFunction>
    class ThreadCallableFunction: public FunctionObject
    {
        classType* _this;
        PtrMemberFunction _pFunc;

        virtual HRESULT InternalMakeCall()
        {
            return (*_this.*_pFunc)();
        }

    public:
        ThreadCallableFunction(classType* classInstance, PtrMemberFunction pFunc)
            :_this(classInstance),
            _pFunc(pFunc)
        {
        }

    };

    template<typename PtrMemberFunction, typename Arg1 >
    class ThreadCallableFunction1: public FunctionObject
    {
        classType* _this;
        PtrMemberFunction _pFunc;
        Arg1 _arg1;

        virtual HRESULT InternalMakeCall()
        {
            return (*_this.*_pFunc)(_arg1);
        }

    public:
        ThreadCallableFunction1(classType* classInstance, PtrMemberFunction pFunc, Arg1& arg1)
            :_this(classInstance),
            _pFunc(pFunc),
            _arg1(arg1)
        {
        }

    };

    template<typename PtrMemberFunction, typename Arg1, typename Arg2 >
    class ThreadCallableFunction2: public FunctionObject
    {
        classType* _this;
        PtrMemberFunction _pFunc;
        Arg1 _arg1;
        Arg2 _arg2;

        virtual HRESULT InternalMakeCall()
        {
            return (*_this.*_pFunc)(_arg1, _arg2);
        }

    public:
        ThreadCallableFunction2(classType* classInstance, PtrMemberFunction pFunc, Arg1& arg1, Arg2& arg2)
            :_this(classInstance),
            _pFunc(pFunc),
            _arg1(arg1),
            _arg2(arg2)
        {
        }

    };

public:
    Message(classType* classInstance, DWORD threadId)
        :_this(classInstance),
         _threadId(threadId)
    {
    }

    static MessageReply TryDispatch(MSG& msg)
    {
        if (msg.message == WM_MSG_PIPE && msg.lParam != 0) 
        {

            FunctionObject* pFunc = (FunctionObject*)(msg.wParam);
            pFunc->MakeCall();
            MessageReply reply = pFunc->MsgReply();
            pFunc->Release();
            return reply;
        }
        return M_IGNORED;
    }

    void SetThreadId(DWORD newThreadId)
    {
        _threadId = newThreadId;
    }

    void WaitForThread()
    {
        HANDLE hThread = OpenThread(SYNCHRONIZE,false,_threadId);
        if (hThread != NULL)
        {
            WaitForSingleObject(hThread,INFINITE);
            CloseHandle(hThread);
        }
    }

    template<typename PtrMemberFunction>
    HRESULT Quit(PtrMemberFunction pFunc,FunctionCallResult* pResult)
    {
        if (_threadId != GetCurrentThreadId())
        {
            FunctionObject* pMsg = new ThreadQuitFunction<PtrMemberFunction>(_this,pFunc);
            return pMsg->PostMessageObject(this, pResult);
        }
        return S_FALSE;
    }

    // Ansynchronous Calls

    template<typename PtrMemberFunction>
    HRESULT AsyncCall(PtrMemberFunction pFunc, FunctionCallResult* pResult)
    {
        if (_threadId != GetCurrentThreadId())
        {
            FunctionObject* pMsg = new ThreadCallableFunction<PtrMemberFunction>(_this,pFunc);
            return pMsg->PostMessageObject(this, pResult);
        }
        return S_FALSE;
    }

    template<typename PtrMemberFunction, typename Arg1>
    HRESULT AsyncCall(PtrMemberFunction pFunc, Arg1& arg1, FunctionCallResult* pResult)
    {
        if (_threadId != GetCurrentThreadId())
        {
            FunctionObject* pMsg = new ThreadCallableFunction1<PtrMemberFunction,Arg1>(_this, pFunc, arg1);
            return pMsg->PostMessageObject(this, pResult);
        }
        return S_FALSE;
    }

    template<typename PtrMemberFunction, typename Arg1, typename Arg2>
    HRESULT AsyncCall(PtrMemberFunction pFunc, Arg1& arg1, Arg2& arg2, FunctionCallResult* pResult)
    {
        if (_threadId != GetCurrentThreadId())
        {
            FunctionObject* pMsg = new ThreadCallableFunction2<PtrMemberFunction,Arg1,Arg2>(_this, pFunc, arg1, arg2);
            return pMsg->PostMessageObject(this, pResult);
        }
        return S_FALSE;
    }

    // Add AsyncCall with formals up to ArgN to handle N arguments

    // We could add Sync calls ... but the FunctionCallResult allows the AsyncCall to be
    // used to make a Sync Call by blocking on the Result.
};
