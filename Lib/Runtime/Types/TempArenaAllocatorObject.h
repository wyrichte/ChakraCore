// Copyright (C) Microsoft. All rights reserved. 

#pragma once

namespace Js
{
    template <bool isGuestArena>
    class TempArenaAllocatorWrapper sealed : public FinalizableObject
    {
    private:
        ArenaAllocator allocator;
        ArenaData ** externalGuestArenaRef;
        Recycler * recycler;

        TempArenaAllocatorWrapper(__in LPCWSTR name, PageAllocator * pageAllocator, void (*outOfMemoryFunc)());

    public:
        

        static TempArenaAllocatorWrapper* Create(ThreadContext * threadContext);

        virtual void Finalize(bool isShutdown) override
        {
        }

        virtual void Dispose(bool isShutdown) override;
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isnt TrackableObject"); }

        ArenaAllocator *GetAllocator()
        {
            return &allocator;
        }

    };

    typedef TempArenaAllocatorWrapper<true> TempGuestArenaAllocatorObject;
    typedef TempArenaAllocatorWrapper<false> TempArenaAllocatorObject;
 }
