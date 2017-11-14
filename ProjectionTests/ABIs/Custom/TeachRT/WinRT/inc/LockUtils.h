// Copyright (c) Microsoft Corporation.  All Rights Reserved.
#pragma once

// simple wrapper over a critical section, best used with CAutoLock for RAII
class CCritSec 
{
public:
    CCritSec() throw()
    {
        InitializeCriticalSection(&_CritSec);
    };

    ~CCritSec() throw()
    {
        DeleteCriticalSection(&_CritSec);
    };

    void Lock() throw() 
    {
        EnterCriticalSection(&_CritSec);
    };

    void Unlock() throw()
    {
        LeaveCriticalSection(&_CritSec);
    };

    // make copy constructor and assignment operator inaccessible
    CCritSec(const CCritSec &refCritSec);
    CCritSec &operator=(const CCritSec &refCritSec);

    CRITICAL_SECTION _CritSec;
};

// locks a critical section, and unlocks it automatically when the lock goes out of scope
class CAutoLock 
{
public:
    CAutoLock(CCritSec *plock) throw()
    {
        _pLock = plock;
        _pLock->Lock();
    };

    ~CAutoLock() throw() 
    {
        _pLock->Unlock();
    };

    // make copy constructor and assignment operator inaccessible
    CAutoLock(const CAutoLock &refAutoLock);
    CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
    CCritSec * _pLock;
};

//
//  SRW Lock class implementation.
// 
class CSRWLock
{
    // see:  minkernel/ntdll/srwlock.c & MSDN
public:
    CSRWLock() throw()
    {
        InitializeSRWLock(&_Lock);
    }

    // acquires the read lock
    void LockShared() throw()
    {
        AcquireSRWLockShared(&_Lock);
    }

    // acquires the write lock - note: SRWLock's are not re-entrant
    void LockExclusive() throw()
    {
        AcquireSRWLockExclusive(&_Lock);
    }

    // 
    void ReleaseShared() throw()
    {
        ReleaseSRWLockShared(&_Lock);
    }

    void ReleaseExclusive() throw() 
    {
        ReleaseSRWLockExclusive(&_Lock);
    }

    virtual ~CSRWLock() throw()
    {
        // there is no default release for an SRWLock
    }

private:
    SRWLOCK _Lock;
};

// RAII over an SRWLock (write)
class CSRWExclusiveAutoLock
{
public:
    CSRWExclusiveAutoLock(CSRWLock *srwLock)
    {
        _SRWLock = srwLock;
        srwLock->LockExclusive();
    }

    virtual ~CSRWExclusiveAutoLock()
    {
        _SRWLock->ReleaseExclusive();
    }
protected:
    CSRWLock *_SRWLock;
};

// RAII over an SRWLock (read)
class CSRWSharedAutoLock
{
public:
    CSRWSharedAutoLock(CSRWLock *srwLock)
    {
        _SRWLock = srwLock;
        srwLock->LockShared();
    }

    virtual ~CSRWSharedAutoLock()
    {
        _SRWLock->ReleaseShared();
    }
protected:
    CSRWLock *_SRWLock;
};


