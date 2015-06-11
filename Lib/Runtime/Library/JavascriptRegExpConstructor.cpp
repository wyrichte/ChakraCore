//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    JavascriptRegExpConstructor::JavascriptRegExpConstructor(DynamicType * type) :
        RuntimeFunction(type, &JavascriptRegExp::EntryInfo::NewInstance),
        reset(false),
        lastPattern(null),
        lastMatch() // undefined
    {
        DebugOnly(VerifyEntryPoint());
        ScriptContext* scriptContext = this->GetScriptContext();        
        JavascriptString* emptyString = scriptContext->GetLibrary()->GetEmptyString();
        this->lastInput = emptyString;
        this->index = JavascriptNumber::ToVar(-1, scriptContext);
        this->lastIndex = JavascriptNumber::ToVar(-1, scriptContext);
        this->lastParen = emptyString;
        this->leftContext = emptyString;
        this->rightContext = emptyString;
        for (int i = 0; i < NumCtorCaptures; i++)
        {
            this->captures[i] = emptyString;
        }
    }
    

    
    BOOL JavascriptRegExpConstructor::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext * requestContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        *enumerator = RecyclerNew(GetScriptContext()->GetRecycler(), JavascriptRegExpObjectEnumerator, this, requestContext, enumNonEnumerable, enumSymbols);
        return true;
    }




    void JavascriptRegExpConstructor::SetLastMatch(UnifiedRegex::RegexPattern* lastPattern, JavascriptString* lastInput, UnifiedRegex::GroupInfo lastMatch)
    {
        AssertMsg(!lastMatch.IsUndefined(), "SetLastMatch should only be called if there's a successful match");
        AssertMsg(lastPattern != null, "lastPattern should not be null");
        AssertMsg(lastInput != null, "lastInput should not be null");
        AssertMsg(JavascriptOperators::GetTypeId(lastInput) != TypeIds_Null, "lastInput should not be JavaScript null");

        this->lastPattern = lastPattern;
        this->lastInput = lastInput;
        this->lastMatch = lastMatch;
        this->reset = true;
    }

    void JavascriptRegExpConstructor::EnsureValues()
    {
        if (reset)
        {
            Assert(!lastMatch.IsUndefined());
            ScriptContext* scriptContext = this->GetScriptContext();
            UnifiedRegex::RegexPattern* pattern = lastPattern;
            JavascriptString* emptyString = scriptContext->GetLibrary()->GetEmptyString();
            const CharCount lastInputLen = lastInput->GetLength();
            // IE8 quirk: match of length 0 is regarded as length 1
            CharCount lastIndexVal = lastMatch.EndOffset();
            this->index = JavascriptNumber::ToVar(lastMatch.offset, scriptContext);
            this->lastIndex = JavascriptNumber::ToVar(lastIndexVal, scriptContext);
            this->leftContext = lastMatch.offset > 0 ? SubString::New(lastInput, 0, lastMatch.offset) : emptyString;
            this->rightContext = lastIndexVal > 0 && lastIndexVal < lastInputLen ? SubString::New(lastInput, lastIndexVal, lastInputLen - lastIndexVal) : emptyString;

            Var nonMatchValue = RegexHelper::NonMatchValue(scriptContext, true);
            captures[0] = RegexHelper::GetString(scriptContext, lastInput, nonMatchValue, lastMatch);
            int numGroups = pattern->NumGroups();
            if (numGroups > 1)
            {
                // The RegExp constructor's lastMatch holds the last *successfull* match on any regular expression.
                // That regular expression may since have been used for *unseccessfull* matches, in which case
                // its groups will have been reset. Updating the RegExp constructor with the group binding after
                // every match is prohibitively slow. Instead, run the match again using the known last input string.
                if (!pattern->WasLastMatchSuccessful())
                {
                    RegexHelper::SimpleMatch(scriptContext, pattern, lastInput->GetString(), lastInputLen, lastMatch.offset);
                }
                Assert(pattern->WasLastMatchSuccessful());
                for (int groupId = 1; groupId < min(numGroups, NumCtorCaptures); groupId++)
                    captures[groupId] = RegexHelper::GetGroup(scriptContext, pattern, lastInput, nonMatchValue, groupId);

                this->lastParen = numGroups <= NumCtorCaptures ? captures[numGroups - 1] :
                    RegexHelper::GetGroup(scriptContext, pattern, lastInput, nonMatchValue, numGroups - 1);
            }
            else
            {
                this->lastParen = emptyString;
            }
            for (int groupId = numGroups; groupId < NumCtorCaptures; groupId++)
                captures[groupId] = emptyString;
            reset = false;
        }
    }


    JavascriptRegExpConstructor* JavascriptRegExpConstructor::MakeCopyOnWriteObject(ScriptContext* scriptContext)
    {
        VERIFY_COPY_ON_WRITE_ENABLED_RET();

        typedef CopyOnWriteObject< JavascriptRegExpConstructor, JavascriptRegExpConstructorProperties > JavascriptRegExpConstructorCopy;
        JavascriptRegExpConstructorCopy *result = RecyclerNew(scriptContext->GetRecycler(), JavascriptRegExpConstructorCopy,
            scriptContext->GetLibrary()->CreateFunctionType(this->GetEntryPoint()), this, scriptContext);

        Assert(this->functionInfo == &JavascriptRegExp::EntryInfo::NewInstance);
        Assert(result->functionInfo == &JavascriptRegExp::EntryInfo::NewInstance);

        if (lastPattern)
            result->lastPattern = scriptContext->CopyPattern(lastPattern);
        if (lastInput)
            result->lastInput = (Js::JavascriptString *)scriptContext->CopyOnWrite(lastInput);
        result->lastMatch = lastMatch;
        result->reset = !lastMatch.IsUndefined();

        return result;
    }


} // namespace Js
