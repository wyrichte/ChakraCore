//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

namespace Js
{
    inline JavascriptString* CharStringCache::GetStringForCharA(char c)
    {
        AssertMsg(JavascriptString::IsASCII7BitChar(c), "GetStringForCharA must be called with ASCII 7bit chars only");

        PropertyString * str = charStringCacheA[c];
        if (str == null)
        {
            PropertyRecord const * propertyRecord;
            wchar_t wc = c;
            JavascriptLibrary * javascriptLibrary = JavascriptLibrary::FromCharStringCache(this);
            javascriptLibrary->GetScriptContext()->GetOrAddPropertyRecord(&wc, 1, &propertyRecord);
            str = javascriptLibrary->CreatePropertyString(propertyRecord);
            charStringCacheA[c] = str;
        }

        return str;
    }


    inline JavascriptString* CharStringCache::GetStringForChar(wchar_t c)
    {
#ifdef PROFILE_STRINGS
        StringProfiler::RecordSingleCharStringRequest(JavascriptLibrary::FromCharStringCache(this)->GetScriptContext());
#endif
        if (JavascriptString::IsASCII7BitChar(c))
        {
            return GetStringForCharA(JavascriptString::ToASCII7BitChar(c));
        }

        return GetStringForCharW(c);
    }
};