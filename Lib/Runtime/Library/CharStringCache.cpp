//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include <StdAfx.h>

namespace Js
{
    CharStringCache::CharStringCache() : charStringCache(null) { memset(charStringCacheA, 0, sizeof charStringCacheA); }

    JavascriptString* CharStringCache::GetStringForCharW(wchar_t c)
    {        
        Assert(!JavascriptString::IsASCII7BitChar(c));
        JavascriptString* str;
        ScriptContext * scriptContext = JavascriptLibrary::FromCharStringCache(this)->GetScriptContext();
        if (!scriptContext->IsClosed())
        {
            if (charStringCache == null)
            {
                Recycler * recycler = scriptContext->GetRecycler();
                charStringCache = RecyclerNew(recycler, CharStringCacheMap, recycler, 17);
            }
            if (!charStringCache->TryGetValue(c, &str))
            {
                str = SingleCharString::New(c, scriptContext);
                charStringCache->Add(c, str);
            }
        }
        else
        {
            str = SingleCharString::New(c, scriptContext);
        }
        return str;
    }

    JavascriptString* CharStringCache::GetStringForCharSP(codepoint_t c)
    {
        Assert(c >= 0x10000);
        wchar_t buffer[2];
        Js::NumberUtilities::CodePointAsSurrogatePair(c, buffer, buffer + 1);
        JavascriptString* str = JavascriptString::NewCopyBuffer(buffer, 2, JavascriptLibrary::FromCharStringCache(this)->GetScriptContext());
        // TODO: perhaps do some sort of cache for supplementary characters
        return str;
    }    
};