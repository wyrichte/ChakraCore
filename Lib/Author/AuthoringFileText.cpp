//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

using namespace Authoring;

TYPE_STATS(AuthoringFileText, L"AuthoringFileText")

void ScriptContextFreed(void *data, Js::ScriptContext *scriptContext)
{
    Assert(data);
    ((AuthoringFileText *)data)->RemoveSourceInfoForScriptContext(scriptContext);
}

AuthoringFileText::AuthoringFileText(Js::ScriptContext* scriptContext, LPCWSTR buffer, charcount_t length) 
    : _alloc(L"ls:SourceCode", scriptContext->GetThreadContext()->GetPageAllocator(), Js::Throw::OutOfMemory), 
    _threadContext(scriptContext->GetThreadContext()), _cachedByteIndex(0), _cachedCharacterIndex(0)
{
    Js::Utf8SourceInfo* sourceInfo = null;
    _buffer = UTF8String::FromWStr(&_alloc, scriptContext, buffer, length, _length, &sourceInfo);
    _codePoints = length;

    // Keep a list for source infos for different script contexts
    Recycler * recycler = scriptContext->GetRecycler();
    ScriptContextSourceInfoDictionary * crossScriptSourceInfos = RecyclerNew(scriptContext->GetRecycler(), ScriptContextSourceInfoDictionary, recycler);
    
    // Add first before we pin, otherwise, we may leak if we OOM as the dtor won't be ran if there is exception in the ctor
    crossScriptSourceInfos->Add(scriptContext, sourceInfo);
    _crossScriptSourceInfos.Root(crossScriptSourceInfos, recycler);

    // Observe when script contexts are released to record when we can remove the script context from the source info cache.
    RefCountedScriptContext::ObserveFree(ScriptContextFreed, this);
}

AuthoringFileText::~AuthoringFileText() 
{
    Assert(_threadContext);

    _crossScriptSourceInfos.Unroot(_threadContext->GetRecycler());

    RefCountedScriptContext::UnobserveFree(ScriptContextFreed, this);
}

Js::Utf8SourceInfo* AuthoringFileText::GetSourceInfoForScriptContext(Js::ScriptContext* scriptContext)
{
    Js::Utf8SourceInfo* matchingSourceInfo = null;

    if (!this->_crossScriptSourceInfos->TryGetValue(scriptContext, &matchingSourceInfo))
    {
        Js::Utf8SourceInfo* existingSourceInfo = null;

        this->_crossScriptSourceInfos->MapUntil([&existingSourceInfo] (Js::ScriptContext*, Js::Utf8SourceInfo* value)
        {
            if (value)
            {
                existingSourceInfo = value;
                return true;
            }
            return false;
        });

        // If there is at least one source info in the dictionary, clone it.
        if (existingSourceInfo)
        {
            matchingSourceInfo = scriptContext->CloneSourceCrossContext(existingSourceInfo);
        }
        // Otherwise, create a new one.
        else
        {
            matchingSourceInfo = Js::Utf8SourceInfo::NewWithNoCopy(scriptContext, _buffer, _codePoints, _length, scriptContext->GetModuleSrcInfo(kmodGlobal));
        }

        this->_crossScriptSourceInfos->Add(scriptContext, matchingSourceInfo);
    }

    return matchingSourceInfo;
}

void AuthoringFileText::RemoveSourceInfoForScriptContext(Js::ScriptContext* scriptContext)
{
    this->_crossScriptSourceInfos->Remove(scriptContext);
}

AuthoringFileText* AuthoringFileText::New(Js::ScriptContext* scriptContext, LPCWSTR buffer, charcount_t length)
{
    Assert(scriptContext);
    Assert(buffer);
    return new AuthoringFileText(scriptContext, buffer, length);
}

LPCUTF8 AuthoringFileText::Buffer(charcount_t ich)
{
    // If _length == _codePoints there are no non-ASCII characters in the buffer so the code-points and UTF8CHAR are mapped 1:1.
    if (_length == _codePoints) return &_buffer[ich];

    // If the ich is less than the cached ich, reset from the beginning.
    if (ich < _cachedCharacterIndex)
    {
        _cachedCharacterIndex = 0;
        _cachedByteIndex = 0;
    }

    // If the character location left after the cached location doesn't contain UTF8 character, we can directly index
    if (_length - _cachedByteIndex == _codePoints - _cachedCharacterIndex)
    {
        Assert(_cachedByteIndex + (ich - _cachedCharacterIndex) == utf8::CharacterIndexToByteIndex(_buffer, _length, ich, DecodeOptions()));

        return &_buffer[_cachedByteIndex + (ich - _cachedCharacterIndex)];
    }

    // Otherwise scan from the last known position to ich.
    auto byteIndex = utf8::CharacterIndexToByteIndex(&_buffer[_cachedByteIndex], _length - _cachedByteIndex, ich - _cachedCharacterIndex, DecodeOptions());

    // Save the calculated position for the next call.
    _cachedByteIndex = _cachedByteIndex + byteIndex;
    _cachedCharacterIndex = ich;

    Assert(_cachedByteIndex == utf8::CharacterIndexToByteIndex(_buffer, _length, ich, DecodeOptions()));

    // Return the UTF8 location of ich calculated above.
    return &_buffer[_cachedByteIndex];
}

int AuthoringFileText::GetNewLineCountInRange(charcount_t from, charcount_t to)
{
    Assert(from <= to);
    auto options = DecodeOptions();
    auto end = _buffer + _length;
    int  newLineCount = 0;
    LPCUTF8 p = Buffer(from);
    for(auto i = from; i < to; i++)
    {
        switch (utf8::Decode(p, end, options)) 
        {
        case '\r':
            if (p < end && *p == '\n')
            {
                // This is a \r\n pair, advance past the \n.
                p++;
                i++;
            }
            // Intentional fall-through
        case '\n':
        case 0x2028:
        case 0x2029:
            newLineCount++;
            break;
        }
    }
    return newLineCount;
}

