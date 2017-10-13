//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once 

interface ITracker;

// HostVariant wraps a host object in the form of a COM VARIANT. It is exposed to the script as a member of a HostDispatch.
// The reason for the separation between HostVariant and HostDispatch is that the HostDispatch, like
// other script objects, is associated with the ScriptContext/ScriptSite in which it was created.
// The HostVariant can be accessed from multiple contexts. See HostDispatch::Create - if the host object implements
// ITracker, the HostVariant that wraps it will be reused each time the object is passed to us by the host.
// But we need to expose it through a different HostDispatch each time (or, at least, one HostDispatch per script)
// to avoid allowing script objects from different ScriptContext's to access one another directly. That can lead,
// for instance, to objects being invoked after their ScriptSite's have been closed. (See Eze #1371, 1446.)

class HostVariant : public FinalizableObject
{
    friend class HostDispatch;    
    friend class ScriptEngine;
    friend class JavascriptThreadService;
    
private:
    HostVariant();
    HRESULT Initialize(VARIANT * variant);
public:

    // TODO: clean these out to private, and use New/Create instead. 
    HostVariant(IDispatch *pdisp, Js::ScriptContext* scriptContext);
    HostVariant(IDispatch *pdisp);
    HostVariant(IDispatchEx *pdispex);
    HostVariant(ITracker *tracker, Js::ScriptContext* scriptContext);
    HostVariant(HostVariant* oldHostVariant, Js::ScriptContext* scriptContext);

    // For variants passed to the engine that that doesn't support IDispatch*, we just need to hold the reference, and do Equality checks.     
    HostVariant(LPCOLESTR itemName);

    virtual void Mark(Recycler * recycler) override
    {
        this->MarkTrackedObjects(recycler);
    }
    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;

    IDispatch * FinalizeInternal();
    static void FinalizeDispatch(IDispatch * dispatch);
    BOOL IsTracked() const { return isTracked; }

    IDispatch* GetDispatchNoRef() 
    {
        if (isUnknown || this->varDispatch.vt != VT_DISPATCH)
        {
            return nullptr;
        }
        return this->varDispatch.pdispVal;
    }

    IDispatch* GetDispatch() 
    {
        if (isUnknown || this->varDispatch.vt != VT_DISPATCH)
        {
            return nullptr;
        }
        if (this->varDispatch.pdispVal)
        {
            this->varDispatch.pdispVal->AddRef();
        }
        return this->varDispatch.pdispVal; 
    }

protected:

    ~HostVariant()
    {
        if (this->isUnknown)
        {
            VariantClear(&(this->varDispatch));
        }
        else
        {
            if (this->varDispatch.vt == VT_DISPATCH && this->varDispatch.pdispVal)
            {
                this->varDispatch.pdispVal->Release();
                this->varDispatch.pdispVal = nullptr;
            }
        }
    }

private:

    bool supportIDispatchEx : 1;
    bool isUnknown: 1;
    bool isTracked: 1;
    VARIANT varDispatch;

    BOOL IsIDispatch() { return !(isUnknown || supportIDispatchEx); }
    void SetupTracker(ITracker* inTracker);
    void MarkTrackedObjects(Recycler * recycler);
    IDispatch* GetIDispatchAddress() { return  this->varDispatch.pdispVal; }



#ifdef TRACK_DISPATCH
    static CriticalSection s_cs;
    
    void LogAlloc();
    void LogFree();

    static const int StackTraceDepth = 20;
    static const int StackToSkip = 1;
    static int activeHostVariantCount;
    StackBackTrace * stackBackTrace;

    struct LinkList
    {
        void * object;
        struct LinkList* next;
    };
    static LinkList* allocatedLinkList ;
    void DumpLeak();
#endif
};

