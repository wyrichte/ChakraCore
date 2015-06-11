//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

// Description: Implementation for the ABI Projector delegates

#pragma once

#if _M_AMD64 || defined(_M_ARM32_OR_ARM64)
Declare_Extern_ClassMethodImpl(Delegate, Invoke)
#endif 

// Maximum parameters ABI delegates support
#define MAX_DELEGATE_PARAMS 128L

// The JavaScript callback also needs 'this' parameter
#define MAX_CALLBACK_PARAMS MAX_DELEGATE_PARAMS + 1L

namespace Projection
{
    // -----------------------------------------------------------------------------------
    // A template class for a limited size, on-stack list of Js::Var objects   
    // -----------------------------------------------------------------------------------
    class JsVarList
    {
        Js::Var *m_values;
        int m_size;
        int maxSize;

    private:
        void CreateNewValues(int maxSize)
        {
            Clear();

            this->maxSize = maxSize;
            m_size = 0;
            m_values = HeapNewArray(Js::Var, maxSize);
        }

    public:
        JsVarList(int maxSize) : m_values(nullptr)
        {
            CreateNewValues(maxSize);
        }

        ~JsVarList()
        {
            Clear();
        }

        HRESULT Add(Js::Var value)
        {
            Assert(m_size < maxSize);
            if (m_size < maxSize) 
            {
                m_values[m_size++] = value;
                return S_OK;
            }

            return E_FAIL;
        }

        void Clear() 
        { 
            if (m_values != nullptr)
            {
                HeapDeleteArray(maxSize, m_values);
            }

            m_size = 0; 
            maxSize = 0;
        }

        int Count() 
        {	
            Assert(m_size <= maxSize); 
            return m_size; 
        }

        void ReInit(int newSize) 
        { 
            CreateNewValues(newSize);
        }

        Js::Var* Values() 
        {	
            return m_values; 
        }
    };

    // -----------------------------------------------------------------------------------
    // Delegate object which is provided to ABI methods. 
    // Calls JavaScript callback function when invoked by ABI.
    // -----------------------------------------------------------------------------------
    class Delegate sealed : public CUnknownImpl
    {
        friend class ProjectionMarshaler;

        RecyclerWeakReference<Js::JavascriptFunction> *callback;
        RtABIMETHODSIGNATURE signature;
        RtEVENT eventInfo;
        EventProjectionHandler *eventProjectionHandler;
        IUnknown* prioritizedDelegate;

        size_t paramsCount;
        size_t sizeOfCallStack;

    public:
        static HRESULT Create(
            __in ProjectionContext *projectionContext,
            __in LPCWSTR delegateTypeName,
            __in RtABIMETHODSIGNATURE signature,
            __in Js::JavascriptFunction* callback,
            __in RtEVENT eventInfo,
            __in bool isInAsyncInterface,
            __out Delegate** newDelegateObject);

        void _declspec(noreturn) ThrowFatalDisconnectedDelegateError(Js::ScriptContext *scriptContext);

        //
        // IDelegate members 
        //
        CUnknownMethodImpl_ArgT_Def(Delegate, Invoke);

        void MarkScriptContextDependentResources(Recycler *recycler, bool inPartialCollect) override;
#ifdef WINRTFINDPREMATURECOLLECTION
        void VerifyNotDisconnected() override;
#endif
        bool SupportsWeakDelegate();
        bool IsRooted();
        void StrongMark(Recycler *recycler);
        void WeakMark(Recycler *recycler);
        void RemoveStrongRef();
        void SetEventProjectionHandler(EventProjectionHandler *eventProjectionHandler) { this->eventProjectionHandler = eventProjectionHandler; }

        virtual USHORT GetWinrtTypeFlags() override;
        virtual IUnknown* GetUnknown() override;
        UINT GetHeapObjectRelationshipInfoSize() override;
        void FillHeapObjectRelationshipInfo(ProfilerHeapObjectOptionalInfo *optionalInfo) override;

        Js::JavascriptFunction *GetCallback() 
        {
            return (callback) ? callback->Get() : nullptr;
        }

    private:
        Delegate(ProjectionContext *projectionContext);
        virtual ~Delegate();

        HRESULT Initialize(
            __in LPCWSTR delegateTypeName,
            __in RtABIMETHODSIGNATURE signature,
            __in RecyclerWeakReference<Js::JavascriptFunction> *callback,
            __in RtEVENT eventInfo);

        Var GetEvObjectFromJsParams(
            __in JsVarList &jsCallbackParams, 
            __in Var evVar);

        // wwahost uses a SQM stream to log data; a single data point in the stream is provided to Chakra to
        // log the disconnected delegate problem. This is the offset within wwahost stream. 
        // wwahost will reject any other data point.
        static const unsigned long DISCONNECTED_DELEGATES = 29;
    };

    Var DelegateForwarderThunk(Var method, Js::CallInfo callInfo, ...);
}
