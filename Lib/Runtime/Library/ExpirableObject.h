// Copyright (C) Microsoft. All rights reserved. 

class ThreadContext;

class ExpirableObject: public FinalizableObject
{
public:    
    friend class ThreadContext;
    ExpirableObject(ThreadContext* threadContext);

    virtual void Finalize(bool isShutdown);

    virtual void Dispose(bool isShutdown) override;

    virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

    // Called when an expirable object gets expired
    virtual void Expire() = 0;
    virtual void EnterExpirableCollectMode();

    bool IsObjectUsed();
    void SetIsObjectUsed();
    bool SupportsExpiration()
    {
        return (registrationHandle != null);
    }

private:
    void* registrationHandle;
    bool isUsed;
};
