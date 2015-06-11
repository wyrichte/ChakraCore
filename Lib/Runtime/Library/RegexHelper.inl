//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    // ----------------------------------------------------------------------
    // Primitives
    // ----------------------------------------------------------------------    
    __inline void RegexHelper::PrimBeginMatch(RegexMatchState& state, ScriptContext* scriptContext, UnifiedRegex::RegexPattern* pattern, const wchar_t* input, CharCount inputLength, bool alwaysNeedAlloc)
    {
        state.input = input;
        if (pattern->rep.unified.matcher == 0)
            pattern->rep.unified.matcher = UnifiedRegex::Matcher::New(scriptContext, pattern);
        if (alwaysNeedAlloc)
            state.tempAllocatorObj = scriptContext->GetTemporaryAllocator(L"RegexUnifiedExecTemp");
        else
            state.tempAllocatorObj = 0;        
    }
    
    __inline UnifiedRegex::GroupInfo 
    RegexHelper::PrimMatch(RegexMatchState& state, ScriptContext* scriptContext, UnifiedRegex::RegexPattern* pattern, CharCount inputLength, CharCount offset)
    {       
        Assert(pattern->rep.unified.program != 0);
        Assert(pattern->rep.unified.matcher != 0);
#if ENABLE_REGEX_CONFIG_OPTIONS
        UnifiedRegex::RegexStats* stats = 0;
        if (REGEX_CONFIG_FLAG(RegexProfile))
        {
            stats = scriptContext->GetRegexStatsDatabase()->GetRegexStats(pattern);
            scriptContext->GetRegexStatsDatabase()->BeginProfile();
        }
        UnifiedRegex::DebugWriter* w = 0;
        if (REGEX_CONFIG_FLAG(RegexTracing) && CONFIG_FLAG(Verbose))
            w = scriptContext->GetRegexDebugWriter();
#endif

        pattern->rep.unified.matcher->Match
            ( state.input
            , inputLength
            , offset
            , scriptContext
#if ENABLE_REGEX_CONFIG_OPTIONS
            , stats
            , w
#endif
            );

#if ENABLE_REGEX_CONFIG_OPTIONS
        if (REGEX_CONFIG_FLAG(RegexProfile))
            scriptContext->GetRegexStatsDatabase()->EndProfile(stats, UnifiedRegex::RegexStats::Execute);
#endif
        return pattern->GetGroup(0);
    }
    
    __inline void RegexHelper::PrimEndMatch(RegexMatchState& state, ScriptContext* scriptContext, UnifiedRegex::RegexPattern* pattern)
    {
        if (state.tempAllocatorObj != 0)
            scriptContext->ReleaseTemporaryAllocator(state.tempAllocatorObj);        
    }

    __inline Var RegexHelper::NonMatchValue(ScriptContext* scriptContext, bool isGlobalCtor)
    {
        // SPEC DEVIATION: The $n properties of the RegExp ctor use empty strings rather than undefined to represent
        //                 the non-match value, even in ES5 mode.
        if (isGlobalCtor)
            return scriptContext->GetLibrary()->GetEmptyString();
        else
            return scriptContext->GetLibrary()->GetUndefined();
    }

    __inline Var RegexHelper::GetString(ScriptContext* scriptContext, JavascriptString* input, Var nonMatchValue, UnifiedRegex::GroupInfo group)
    {
        if (group.IsUndefined())
            return nonMatchValue;
        switch (group.length)
        {
        case 0:
            return scriptContext->GetLibrary()->GetEmptyString();
        case 1:
            {
                const wchar_t* inputStr = input->GetString();
                return scriptContext->GetLibrary()->GetCharStringCache().GetStringForChar(inputStr[group.offset]);
            }
        case 2:
            {
                const wchar_t* inputStr = input->GetString();
                PropertyString* propString=scriptContext->GetPropertyString2(inputStr[group.offset],inputStr[group.offset+1]);
                if (propString != 0)
                    return propString;
                // fall-through for default
            }
        default:
            return SubString::New(input, group.offset, group.length);
        }
    }

    __inline Var RegexHelper::GetGroup(ScriptContext* scriptContext, UnifiedRegex::RegexPattern* pattern, JavascriptString* input, Var nonMatchValue, int groupId)
    {
        return GetString(scriptContext, input, nonMatchValue, pattern->GetGroup(groupId));
    }

    // ======================================================================
    // Match results propogate into three places:
    //  - The match result array. Generally the array has string entries for the overall match substring,
    //    followed by final bindings for each group, plus the fields:
    //     - 'input': string used in match
    //     - 'index': index of first character of match in input
    //     - 'lastIndex' (IE extension): one plus index of last character of match in input
    //    However, for String.match with a global match, the result is an array of all match results
    //    (ignoring any group bindings). But in IE8 mode we also bind the above fields to that array,
    //    using the results of the last sucessfull primitive match.
    //  - The regular expression object has writable field:
    //     - 'lastIndex': one plus index of last character of last match in last input
    //     - 'lastInput
    //  - (IE extension) The RegExp constructor object has fields:
    //     - '$n': last match substrings, using "" for undefined in all modes
    //     - etc (see JavascriptRegExpConstructorType.cpp)
    //
    // There are also three influences on what gets propogated where and when:
    //  - IE8 vs ES5 mode
    //  - Whether the regular expression is global
    //  - Whether the primitive operations runs the regular expression until failure (eg String.match) or
    //    just once (eg RegExp.exec), or use the underlying matching machinery implicitly (eg String.split).
    //
    // Here are the rules:
    //  - RegExp is updated for the last *successfull* primitive match, except for String.replace.
    //    In particular, for String.match with a global regex, the final failing match *does not* reset RegExp.
    //  - Except for String.search in EC5 mode (which does not update 'lastIndex'), the regular expressions
    //    lastIndex is updated as follows:
    //     - ES5 mode, if a primitive match fails then the regular expression 'lastIndex' is set to 0. In particular,
    //       the final failing primitive match for String.match with a global regex forces 'lastIndex' to be reset.
    //       However, if a primitive match succeeds then the regular expression 'lastIndex' is updated only for
    //       a global regex.
    //     - In IE8 mode, the regular expression's 'lastIndex' is always set to 0 for failure or the last index
    //       for success. However:
    //        - The last failing match in a String.match with a global regex does NOT reset 'lastIndex'.
    //        - If the regular expression matched empty, the last index is set assuming the pattern actually matched
    //          one input character. This applies even if the pattern matched empty one beyond the end of the string
    //          in a String.match with a global regex (!). For our own sanity, we isolate this particular case
    //          within JavascriptRegExp when setting the lastIndexVar value.
    //  - In all modes, 'lastIndex' determines the starting search index only for global regular expressions.
    //
    // ======================================================================

    __inline void RegexHelper::PropagateLastMatch
        ( ScriptContext* scriptContext
        , bool isGlobal
        , bool isSticky
        , JavascriptRegExp* regularExpression
        , JavascriptString* lastInput
        , UnifiedRegex::GroupInfo lastSuccessfullMatch
        , UnifiedRegex::GroupInfo lastActualMatch
        , bool updateRegex
        , bool updateCtor)
    {
        if (updateRegex)
        {
            PropagateLastMatchToRegex(scriptContext, isGlobal, isSticky, regularExpression, lastSuccessfullMatch, lastActualMatch);
        }
        if (updateCtor)
        {
            PropagateLastMatchToCtor(scriptContext, regularExpression, lastInput, lastSuccessfullMatch);
        }
    }

    __inline void RegexHelper::PropagateLastMatchToRegex
        ( ScriptContext* scriptContext
        , bool isGlobal
        , bool isSticky
        , JavascriptRegExp* regularExpression
        , UnifiedRegex::GroupInfo lastSuccessfullMatch
        , UnifiedRegex::GroupInfo lastActualMatch )
    {
        if (lastActualMatch.IsUndefined())
        {
            regularExpression->SetLastIndex(0);
        }
        else if (isGlobal || isSticky)
        {
            CharCount lastIndex = lastActualMatch.EndOffset();
            Assert(lastIndex <= MaxCharCount);
            regularExpression->SetLastIndex((int32)lastIndex);
        }
    }

    __inline void RegexHelper::PropagateLastMatchToCtor
        ( ScriptContext* scriptContext
        , JavascriptRegExp* regularExpression
        , JavascriptString* lastInput
        , UnifiedRegex::GroupInfo lastSuccessfullMatch )
    {
        Assert(lastInput);

        if (!lastSuccessfullMatch.IsUndefined())
        {
            // Notes: 
            // - SPEC DEVIATION: The RegExp ctor holds some details of the last successfull match on any regular expression.
            // - For updating regex ctor's stats we are using entry function's context, rather than regex context, 
            //   the rational is: use same context of RegExp.prototype, on which the function was called. 
            //   So, if you call the function with remoteContext.regexInstance.exec.call(localRegexInstance, "match string"),
            //   we will update stats in the context related to the exec function, i.e. remoteContext.
            //   This is consistent with chrome.
            scriptContext->GetLibrary()->GetRegExpConstructor()->SetLastMatch(regularExpression->GetPattern(), lastInput, lastSuccessfullMatch);
        }
    }

    __inline bool RegexHelper::GetInitialOffset(bool isGlobal, bool isSticky, JavascriptRegExp* regularExpression, CharCount inputLength, CharCount& offset)
    {
        if (isGlobal || isSticky)
        {
            offset = regularExpression->GetLastIndex();
            if (offset <= MaxCharCount)
                return true;
            else
            {
                regularExpression->SetLastIndex(0);
                return false;
            }
        }
        else
        {
            offset = 0;
            return true;
        }
    }
    
    __inline JavascriptArray* RegexHelper::CreateMatchResult(void *const stackAllocationPointer, ScriptContext* scriptContext, bool isGlobal, int numGroups, JavascriptString* input)
    {
        if (isGlobal)
        {            
            // Use an ordinary array, with default initial capacity
            return scriptContext->GetLibrary()->CreateArrayOnStack(stackAllocationPointer);
        }
        else
            return JavascriptRegularExpressionResult::Create(stackAllocationPointer, numGroups, input, scriptContext);
    }
    
    __inline void RegexHelper::FinalizeMatchResult(ScriptContext* scriptContext, bool isGlobal, JavascriptArray* arr, UnifiedRegex::GroupInfo match)
    {
        if (!isGlobal)
            JavascriptRegularExpressionResult::SetMatch(arr, match);
        // else: arr is an ordinary array
    }
    
    __inline JavascriptArray* RegexHelper::CreateExecResult(void *const stackAllocationPointer, ScriptContext* scriptContext, int numGroups, JavascriptString* input, UnifiedRegex::GroupInfo match)
    {
        JavascriptArray* res = JavascriptRegularExpressionResult::Create(stackAllocationPointer, numGroups, input, scriptContext);
        JavascriptRegularExpressionResult::SetMatch(res, match);
        return res;
    }

    template<bool mustMatchEntireInput>
    BOOL RegexHelper::RegexTest_NonScript(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, const wchar_t *const input, const CharCount inputLength)
    {
        // This version of the function should only be used when testing the regex against a non-javascript string. That is,
        // this call was not initiated by script code. Hence, the RegExp constructor is not updated with the last match. If
        // 'mustMatchEntireInput' is true, this function also ignores the global/sticky flag and the lastIndex property, since it tests
        // for a match on the entire input string; in that case, the lastIndex property is not modified.
        
        UnifiedRegex::RegexPattern* pattern = regularExpression->GetPattern();        
        UnifiedRegex::GroupInfo match; // initially undefined

#if ENABLE_REGEX_CONFIG_OPTIONS
        Trace(scriptContext, UnifiedRegex::RegexStats::Test, regularExpression, input, inputLength);
#endif
        const bool isGlobal = pattern->IsGlobal();
        const bool isSticky = pattern->IsSticky();
        CharCount offset;
        if(mustMatchEntireInput)
            offset = 0; // needs to match the entire input, so ignore 'lastIndex' and always start from the beginning
        else if (!GetInitialOffset(isGlobal, isSticky, regularExpression, inputLength, offset))
            return false;

        if (mustMatchEntireInput || offset <= inputLength)
        {
            match = RegexHelper::SimpleMatch(scriptContext, pattern, input, inputLength, offset);
        }
        // else: match remains undefined

        if(!mustMatchEntireInput) // don't update 'lastIndex' when mustMatchEntireInput is true since the global flag is ignored
        {
            PropagateLastMatchToRegex(scriptContext, isGlobal, isSticky, regularExpression, match, match);
        }

        return mustMatchEntireInput ? match.offset == 0 && match.length == inputLength : !match.IsUndefined();
    }

    // Asserts if the value needs to be marshaled to taget context, except for IE9-compact when it marshals the value there.
    // Returns the resulting value.
    // This is supposed to be called for result/return value of the RegexXXX functions.
    // static
    template<typename T> 
    __inline T RegexHelper::CheckCrossContextAndMarshalResult(T value, ScriptContext* targetContext)
    {
        Assert(targetContext);       
        Assert(!CrossSite::NeedMarshalVar(value, targetContext));
        return value;
    }

    __inline Var RegexHelper::RegexMatchResultUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        return RegexHelper::RegexMatch(scriptContext, regularExpression, input, false);
    }

    __inline Var RegexHelper::RegexMatchResultUsedAndMayBeTemp(void *const stackAllocationPointer, ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        return RegexHelper::RegexMatch(scriptContext, regularExpression, input, false, stackAllocationPointer);
    }

    __inline Var RegexHelper::RegexMatchResultNotUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        if(!PHASE_OFF1(Js::RegexResultNotUsedPhase))
        {
            return RegexHelper::RegexMatch(scriptContext, regularExpression, input, true);
        }
        else
        {
            return RegexHelper::RegexMatch(scriptContext, regularExpression, input, false);
        }
    }

    __inline Var RegexHelper::RegexMatch(ScriptContext* entryFunctionContext, JavascriptRegExp *regularExpression, JavascriptString *input, bool noResult, void *const stackAllocationPointer)
    {        
        Var result = RegexHelper::RegexMatchImpl<true>(entryFunctionContext, regularExpression, input, noResult, stackAllocationPointer);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }
    
    __inline Var RegexHelper::RegexMatchNoHistory(ScriptContext* entryFunctionContext, JavascriptRegExp *regularExpression, JavascriptString *input, bool noResult)
    {        
        Var result = RegexHelper::RegexMatchImpl<false>(entryFunctionContext, regularExpression, input, noResult);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }

    __inline Var RegexHelper::RegexExecResultUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        return RegexHelper::RegexExec(scriptContext, regularExpression, input, false);
    }

    __inline Var RegexHelper::RegexExecResultUsedAndMayBeTemp(void *const stackAllocationPointer, ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        return RegexHelper::RegexExec(scriptContext, regularExpression, input, false, stackAllocationPointer);
    }

    __inline Var RegexHelper::RegexExecResultNotUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {
        if(!PHASE_OFF1(Js::RegexResultNotUsedPhase))
        {
            return RegexHelper::RegexExec(scriptContext, regularExpression, input, true);
        }
        else
        {
            return RegexHelper::RegexExec(scriptContext, regularExpression, input, false);
        }
    }

    __inline Var RegexHelper::RegexExec(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, bool noResult, void *const stackAllocationPointer)
    {        
        Var result = RegexHelper::RegexExecImpl(entryFunctionContext, regularExpression, input, noResult, stackAllocationPointer);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }

    __inline Var RegexHelper::RegexReplaceResultUsed(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, JavascriptString* replace)
    {
        return RegexHelper::RegexReplace(entryFunctionContext, regularExpression, input, replace, null, false);
    }

    __inline Var RegexHelper::RegexReplaceResultNotUsed(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, JavascriptString* replace)
    {
        if(!PHASE_OFF1(Js::RegexResultNotUsedPhase))
        {
            return RegexHelper::RegexReplace(entryFunctionContext, regularExpression, input, replace, null, true);
        }
        else
        {
            return RegexHelper::RegexReplace(entryFunctionContext, regularExpression, input, replace, null, false);
        }

    }

    __inline Var RegexHelper::RegexReplace(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, JavascriptString* replace, JavascriptString* options, bool noResult)
    {        
        Var result = RegexHelper::RegexReplaceImpl(entryFunctionContext, regularExpression, input, replace, options, noResult);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }

    __inline Var RegexHelper::RegexReplaceFunction(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, JavascriptFunction* replacefn, JavascriptString* options)
    {        
        Var result = RegexHelper::RegexReplaceImpl(entryFunctionContext, regularExpression, input, replacefn, options);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }

    __inline Var RegexHelper::RegexSearch(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input)
    {        
        Var result = RegexHelper::RegexSearchImpl(entryFunctionContext, regularExpression, input);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }

    __inline Var RegexHelper::RegexSplitResultUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input, CharCount limit)
    {
        return RegexHelper::RegexSplit(scriptContext, regularExpression, input, limit, false);
    }

    __inline Var RegexHelper::RegexSplitResultUsedAndMayBeTemp(void *const stackAllocationPointer, ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input, CharCount limit)
    {
        return RegexHelper::RegexSplit(scriptContext, regularExpression, input, limit, false, stackAllocationPointer);
    }

    __inline Var RegexHelper::RegexSplitResultNotUsed(ScriptContext* scriptContext, JavascriptRegExp* regularExpression, JavascriptString* input, CharCount limit)
    {
        if(!PHASE_OFF1(Js::RegexResultNotUsedPhase))
        {
            return RegexHelper::RegexSplit(scriptContext, regularExpression, input, limit, true);
        }
        else
        {
            return RegexHelper::RegexSplit(scriptContext, regularExpression, input, limit, false);
        }
    }

    __inline Var RegexHelper::RegexSplit(ScriptContext* entryFunctionContext, JavascriptRegExp* regularExpression, JavascriptString* input, CharCount limit, bool noResult, void *const stackAllocationPointer)
    {        
        Var result = RegexHelper::RegexSplitImpl(entryFunctionContext, regularExpression, input, limit, noResult, stackAllocationPointer);
        return RegexHelper::CheckCrossContextAndMarshalResult(result, entryFunctionContext);
    }
}
