
//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once 

/*
    This class is used to connect HostDispatch and HostVariant. HostVariant is designed
    to be put in DOM object's tracker alias. HostDispatch is scriptcontext specified 
    recyclable object, and we need to have a refcounted object to conect these two. 
    When a HostDispatach is marshalled to a different scriptcontext, the two HostDispatch
    is connected to the same RefCountedHostVariant, with the refcount being the number
    of scriptsites connected to the same HostVariant. In ScriptSite::Close, we release 
    the refcount, and disconnect the HostVariant totally if there is no more active refcount.
*/
class RefCountedHostVariant : public FinalizableObject
{
    friend class HostVariant;
public:
    RefCountedHostVariant(HostVariant* hostVariant);
    HostVariant* GetHostVariant() const { return hostVariant; }
    ULONG Release();
    ULONG AddRef() {refCount++; return refCount; }
    virtual void Finalize(bool isShutdown) override;
    virtual void Dispose(bool isShutdown) override;
    virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

private:
    // number of HostDispatch connected to the HostVariant.
    ULONG refCount;
    HostVariant* hostVariant;
};
