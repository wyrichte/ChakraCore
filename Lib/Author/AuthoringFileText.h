//
//    Copyright (C) Microsoft.  All rights reserved.
//
#pragma once

namespace Authoring
{
    class AuthoringFileText : public RefCounted<DeletePolicy::OperatorDelete>
    {
        LPUTF8 _buffer;
        size_t _length;
        charcount_t _codePoints;
        ArenaAllocator _alloc;
        ThreadContext* _threadContext;

        charcount_t _cachedCharacterIndex;
        size_t _cachedByteIndex;

        typedef JsUtil::BaseDictionary<Js::ScriptContext*, Js::Utf8SourceInfo*, Recycler> ScriptContextSourceInfoDictionary;
        RecyclerRootPtr<ScriptContextSourceInfoDictionary> _crossScriptSourceInfos;

        AuthoringFileText(Js::ScriptContext* scriptContext, LPCWSTR buffer, charcount_t length); 

    public:
        static AuthoringFileText* New(Js::ScriptContext* scriptContext, LPCWSTR buffer, charcount_t length);

        virtual ~AuthoringFileText();

        bool Empty() { return !_buffer || _length == 0; }

        // Return the UTF8 sequence of the buffer.
        LPCUTF8 Buffer() { return _buffer; }

        // Return the UTF8 sequence starting at ich code-points into the buffer.
        LPCUTF8 Buffer(charcount_t ich);

        // The length, in bytes, of the UTF8 sequence of the file.
        size_t Length() { return _length; }

        // Get the variant of the source info for the given script context
        Js::Utf8SourceInfo* GetSourceInfoForScriptContext(Js::ScriptContext* scriptContext);

        // Remove the source info entry for the given script context
        void RemoveSourceInfoForScriptContext(Js::ScriptContext* scriptContext);

        // The number of code-points encoded into the UTF8 sequence.
        charcount_t CodePoints() { return _codePoints; }

        // Returns a number of new lines (\r\n) in the specified range
        int GetNewLineCountInRange(charcount_t from, charcount_t to);

    private:
        bool IsCesu8()
        {
            // Currently assume the buffer is always Cesu8. 
            // For buffers read directly from disk this is not going to be the case.
            return true;
        }

        utf8::DecodeOptions DecodeOptions() 
        {
            return IsCesu8() ? utf8::doAllowThreeByteSurrogates : utf8::doDefault;
        }
    };


}

