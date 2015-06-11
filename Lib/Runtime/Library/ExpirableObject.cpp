#include "stdafx.h"

ExpirableObject::ExpirableObject(ThreadContext* threadContext):
    isUsed(false),
    registrationHandle(null)
{
    if (threadContext)
    {
        threadContext->RegisterExpirableObject(this);
    }
}

void ExpirableObject::Finalize(bool isShutdown)
{
    if (!isShutdown && this->registrationHandle != null)
    {
        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();

        threadContext->UnregisterExpirableObject(this);
    }
}

void ExpirableObject::Dispose(bool isShutdown)
{
    if (!isShutdown && this->registrationHandle == null)
    {
        ThreadContext* threadContext = ThreadContext::GetContextForCurrentThread();
        threadContext->DisposeExpirableObject(this);
    }
}

void ExpirableObject::EnterExpirableCollectMode()
{
    this->isUsed = false;
}

bool ExpirableObject::IsObjectUsed()
{
    return this->isUsed;
}

void ExpirableObject::SetIsObjectUsed()
{
    this->isUsed = true;
}
