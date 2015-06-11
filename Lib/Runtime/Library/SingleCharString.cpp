//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    DEFINE_RECYCLER_TRACKER_PERF_COUNTER(SingleCharString);

    SingleCharString::SingleCharString(wchar_t ch, StaticType * type) : JavascriptString(type, 1, m_buff)
    {
        m_buff[0] = ch;
        m_buff[1] = L'\0';

#ifdef PROFILE_STRINGS
        StringProfiler::RecordNewString( this->GetScriptContext(), this->m_buff, 1 );
#endif
    }

    /*static*/ SingleCharString* SingleCharString::New(wchar_t ch, ScriptContext* scriptContext)
    {
        Assert(scriptContext != null);

        return RecyclerNew(scriptContext->GetRecycler(),SingleCharString,ch,
            scriptContext->GetLibrary()->GetStringTypeStatic());
    }

    /*static*/ SingleCharString* SingleCharString::New(wchar_t ch, ScriptContext* scriptContext, ArenaAllocator* arena)
    {
        Assert(scriptContext != null);
        Assert(arena != null);

        return Anew(arena, SingleCharString, ch,
            scriptContext->GetLibrary()->GetStringTypeStatic());
    }
} 
