//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptStringIterator::JavascriptStringIterator(DynamicType* type, JavascriptString* string):
        DynamicObject(type),
        m_string(string),
        m_nextIndex(0)
    {
        Assert(type->GetTypeId() == TypeIds_StringIterator);
    }

    Var JavascriptStringIterator::EntryNext(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptLibrary* library = scriptContext->GetLibrary();

        Assert(!(callInfo.Flags & CallFlags_New));

        Var thisObj = args[0];
        
        if (!JavascriptStringIterator::Is(thisObj))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedStringIterator, L"String Iterator.prototype.next");
        }
        
        JavascriptStringIterator* iterator = JavascriptStringIterator::FromVar(thisObj);
        JavascriptString* string = iterator->m_string;

        if (string == nullptr)
        {
            return library->CreateIteratorResultObjectUndefinedTrue();
        }

        charcount_t length = string->GetLength();
        charcount_t index = iterator->m_nextIndex;

        if (index >= length)
        {
            // Nulling out the m_string field is important so that the iterator
            // does not keep the string alive after iteration is completed.
            iterator->m_string = nullptr;
            return library->CreateIteratorResultObjectUndefinedTrue();
        }

        wchar_t chFirst = string->GetItem(index);
        Var result;

        if (index + 1 == string->GetLength() ||
            !NumberUtilities::IsSurrogateLowerPart(chFirst) ||
            !NumberUtilities::IsSurrogateUpperPart(string->GetItem(index + 1)))
        {
            result = scriptContext->GetLibrary()->GetCharStringCache().GetStringForChar(chFirst);
            iterator->m_nextIndex += 1;
        }
        else
        {
            result = JavascriptString::SubstringCore(string, index, 2, scriptContext);
            iterator->m_nextIndex += 2;
        }

        return library->CreateIteratorResultObjectValueFalse(result);
    }

    Var JavascriptStringIterator::EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);

        Assert(!(callInfo.Flags & CallFlags_New));

        // Simply return 'this'
        return args[0];
    }
} //namespace Js