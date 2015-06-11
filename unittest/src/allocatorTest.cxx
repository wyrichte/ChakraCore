/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

UTEST_GROUP(arenaAllocator)
{
    PageAllocator * pageAllocator;
    ArenaAllocator* _pArena;

    void Setup()
    {
        pageAllocator = new PageAllocator(NULL, Js::Configuration::Global.flags);
        _pArena = new ArenaAllocator(L"unittest", pageAllocator, NULL);
        UT_ASSERT(_pArena != NULL);
    }

    void Cleanup()
    {
        delete _pArena;
        delete pageAllocator;
    }

    UTEST_CASE(alloc)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);
    };

    UTEST_CASE(allocZero)
    {
        char* pBuff = NULL;
        pBuff = _pArena->AllocZero(100);

        UT_ASSERT(pBuff != NULL);

        for (int i = 0; i < 100; i++)
        {
            UT_ASSERT(pBuff[i] == 0);
        }
    };

    UTEST_CASE(reallocLarger)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);

        pBuff[0] = 'A';
        pBuff[99] = 'Z';

        char* pBuffRe = NULL;
        pBuffRe = _pArena->Realloc(pBuff, 100, 200);

        UT_ASSERT(pBuffRe != NULL);
        UT_ASSERT(pBuffRe[0] == 'A');
        UT_ASSERT(pBuffRe[99] == 'Z');
    };

    UTEST_CASE(reallocSmaller)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);

        pBuff[0] = 'A';
        pBuff[99] = 'Z';

        char* pBuffRe = NULL;
        pBuffRe = _pArena->Realloc(pBuff, 100, 50);

        // making it smaller shouldn't reallocate
        UT_ASSERT(pBuffRe == pBuff);
        UT_ASSERT(pBuffRe[0] == 'A');
    };

    UTEST_CASE(reallocSizeZero)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);

        char* pBuffRe = NULL;
        pBuffRe = _pArena->Realloc(pBuff, 100, 0);

        // arena doesn't return null on zero allocation
        // same as making the buffer smaller, it won't reallocate
        UT_ASSERT(pBuffRe == pBuff);
    };

    UTEST_CASE(free)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);

        _pArena->Free(pBuff, 100);

        // Nothing to assert here, just make sure we don't AV.
    };

    UTEST_CASE(reset)
    {
        char* pBuff = NULL;
        pBuff = _pArena->Alloc(100);

        UT_ASSERT(pBuff != NULL);

        _pArena->Reset();

        // Nothing to assert here, just make sure we don't AV.
    };
};

