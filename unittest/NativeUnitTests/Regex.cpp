// Copyright (C) Microsoft. All rights reserved. 
// Tests to verify regex-related APIs exposed to hosts

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

static const unsigned long Jserr_RegExpBadRange_ErrorCode = 5021;
static const HRESULT Jserr_RegExpBadRange_Hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_CONTROL, Jserr_RegExpBadRange_ErrorCode);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

void AssertRegexConstructorUnchanged(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify, const char *const m)
{
    static const wchar_t *const RegexConstructorProperties[] =
    {
        L"lastMatch",       L"''",
        L"lastParen",       L"''",
        L"leftContext",     L"''",
        L"rightContext",    L"''",
        L"index",           L"-1",
        L"input",           L"''",
        L"$_",              L"''",
        L"$1",              L"''",
        L"$2",              L"''",
        L"$3",              L"''",
        L"$4",              L"''",
        L"$5",              L"''",
        L"$6",              L"''",
        L"$7",              L"''",
        L"$8",              L"''",
        L"$9",              L"''",
        L"$&",              L"''",
        L"$+",              L"''",
        L"$`",              L"''",
        L"$'",              L"''"
    };
    static const int RegexConstructorNumProperties = sizeof(RegexConstructorProperties) / sizeof(RegexConstructorProperties[0]);

    std::wostringstream script;
    script << L"var failed = \"\"; var propertyName, propertyDefaultValue, propertyValue; do {";
    for(int i = 0; i < RegexConstructorNumProperties; i += 2)
        script
            << L"propertyName = \"" << RegexConstructorProperties[i] << L"\";"
            << L"propertyDefaultValue = " << RegexConstructorProperties[i + 1] << L";"
            << L"propertyValue = RegExp[propertyName];"
            << L"if(propertyValue !== propertyDefaultValue) { failed += propertyName + \"(\" + propertyValue + \") \"; }";
    script << L"} while(false);";

    mytest->ParseAndExecute(script.str().c_str());
    std::wstring failed = mytest->ToString(mytest->GetProperty(mytest->GetGlobalObject(), L"failed"));
    try
    {
        verify->Assert(failed.empty(), m);
    }
    catch(std::string message)
    {
        message += "    Changed RegExp constructor properties: ";
        for(std::wstring::const_iterator it = failed.begin(); it != failed.end(); ++it)
            message += (char)*it;
        throw message;
    }
}

template<unsigned int patternLengthPlusOne>
Var CreateRegex(
    const wchar_t (&pattern)[patternLengthPlusOne],
    const RegexFlags flags,
    MyScriptDirectTests *const mytest,
    Verifier<MyScriptDirectTests> *const verify,
    const char *const m)
{
    Var regex;
    const HRESULT hr = mytest->GetScriptDirectNoRef()->CreateRegex(pattern, patternLengthPlusOne - 1, flags, &regex);
    verify->FAIL_hr(hr, m);
    verify->AssertNotEqual<Var>(regex, 0, m);
    return regex;
}

template<unsigned int inputLengthPlusOne>
bool RegexTest(
    const Var regex,
    const wchar_t (&input)[inputLengthPlusOne],
    const bool mustMatchEntireInput,
    MyScriptDirectTests *const mytest,
    Verifier<MyScriptDirectTests> *const verify,
    const char *const m,
    const bool lastIndexMustBeZero = true)
{
    BOOL matched;
    const HRESULT hr =
        mytest->GetScriptDirectNoRef()->RegexTest(regex, input, inputLengthPlusOne - 1, mustMatchEntireInput, &matched);
    verify->FAIL_hr(hr, m);
    verify->Assert(matched == FALSE || matched == TRUE, m);
    AssertRegexConstructorUnchanged(mytest, verify, m);
    if(lastIndexMustBeZero)
        verify->AssertEqual(mytest->ToDouble(mytest->GetProperty(regex, L"lastIndex")), 0.0, m);
    return matched != FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests

void CreateRegex_FailPaths(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    const char *const m = "CreateRegex_FailPaths";
    Var regex;
    HRESULT hr;

    // Invalid parameters

    hr = mytest->GetScriptDirectNoRef()->CreateRegex(0, 0, RegexFlags_None, &regex);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    hr = mytest->GetScriptDirectNoRef()->CreateRegex(L"", INT_MAX, RegexFlags_None, &regex);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    hr = mytest->GetScriptDirectNoRef()->CreateRegex(L"", 0, RegexFlags_None, 0);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    // Invalid regex

    hr = mytest->GetScriptDirectNoRef()->CreateRegex(L"[b-a]", 5, RegexFlags_None, &regex);
    verify->AssertEqual(hr, Jserr_RegExpBadRange_Hr, m);
}

void CreateRegex_SimpleHappyPath(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    const char *const m = "CreateRegex_SimpleHappyPath";
    const Var regex = CreateRegex(L"", RegexFlags_None, mytest, verify, m);

    // For sanity, verify that these don't fail
    verify->FAIL_hr(mytest->PinObject(regex), m);
    verify->FAIL_hr(mytest->UnpinObject(regex), m);
}

void RegexTest_FailPaths(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    const char *const m = "RegexTest_FailPaths";
    BOOL matched;
    HRESULT hr;
    Var regex;

    hr = mytest->GetScriptDirectNoRef()->RegexTest(0, L"", 0, false, &matched);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    mytest->GetScriptDirectNoRef()->GetUndefined(&regex);
    hr = mytest->GetScriptDirectNoRef()->RegexTest(regex, L"", 0, false, &matched);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    regex = CreateRegex(L"", RegexFlags_None, mytest, verify, m);

    hr = mytest->GetScriptDirectNoRef()->RegexTest(regex, 0, 0, false, &matched);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    hr = mytest->GetScriptDirectNoRef()->RegexTest(regex, L"", INT_MAX, false, &matched);
    verify->AssertEqual(hr, E_INVALIDARG, m);

    hr = mytest->GetScriptDirectNoRef()->RegexTest(regex, L"", 0, false, 0);
    verify->AssertEqual(hr, E_INVALIDARG, m);
}

void RegexTest_MatchSubset(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    const char *const m = "RegexTest_MatchSubset";
    Var regex;
    bool matched;

    // No flags
    regex = CreateRegex(L"a+", RegexFlags_None, mytest, verify, m);
    matched = RegexTest(regex, L"baab", false, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"cbbc", false, mytest, verify, m);
    verify->Assert(!matched, m);

    // Ignore-case
    regex = CreateRegex(L"a+", RegexFlags_IgnoreCase, mytest, verify, m);
    matched = RegexTest(regex, L"baAb", false, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"cbBc", false, mytest, verify, m);
    verify->Assert(!matched, m);

    // Global
    regex = CreateRegex(L"a+", RegexFlags_Global, mytest, verify, m);
    matched = RegexTest(regex, L"baabaab", false, mytest, verify, m, false);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"cbbc", false, mytest, verify, m, false);
    verify->Assert(!matched, m);

    // Multiline
    regex = CreateRegex(L"^a+$", RegexFlags_Multiline, mytest, verify, m);
    matched = RegexTest(regex, L"baab\naa", false, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"cbbc\nbb", false, mytest, verify, m);
    verify->Assert(!matched, m);
}

void RegexTest_MatchEntireInput(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    const char *const m = "RegexTest_MatchEntireInput";
    Var regex;
    bool matched;

    // No flags
    regex = CreateRegex(L"a+", RegexFlags_None, mytest, verify, m);
    matched = RegexTest(regex, L"aa", true, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"baab", true, mytest, verify, m);
    verify->Assert(!matched, m);
    matched = RegexTest(regex, L"cbbc", true, mytest, verify, m);
    verify->Assert(!matched, m);

    // Ignore-case
    regex = CreateRegex(L"a+", RegexFlags_IgnoreCase, mytest, verify, m);
    matched = RegexTest(regex, L"aA", true, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"baAb", true, mytest, verify, m);
    verify->Assert(!matched, m);
    matched = RegexTest(regex, L"cbBc", true, mytest, verify, m);
    verify->Assert(!matched, m);

    // Global
    regex = CreateRegex(L"a+", RegexFlags_Global, mytest, verify, m);
    matched = RegexTest(regex, L"aa", true, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"aabaa", true, mytest, verify, m);
    verify->Assert(!matched, m);
    matched = RegexTest(regex, L"cbbc", true, mytest, verify, m);
    verify->Assert(!matched, m);

    // Multiline
    regex = CreateRegex(L"^a+$", RegexFlags_Multiline, mytest, verify, m);
    matched = RegexTest(regex, L"aa", true, mytest, verify, m);
    verify->Assert(matched, m);
    matched = RegexTest(regex, L"aa\naa", true, mytest, verify, m);
    verify->Assert(!matched, m);
    matched = RegexTest(regex, L"cbbc\nbb", true, mytest, verify, m);
    verify->Assert(!matched, m);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test drivers

static void (*const Tests[])(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify) =
{
    CreateRegex_FailPaths,
    CreateRegex_SimpleHappyPath,
    RegexTest_FailPaths,
    RegexTest_MatchSubset,
    RegexTest_MatchEntireInput
};

void RunRegexTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    for(int i = 0; i < sizeof(Tests) / sizeof(Tests[0]); ++i)
    {
        verify->ResetAssertionCounter();
        try
        {
            Tests[i](mytest, verify);
        }
        catch(std::string message)
        {
            Print(message, false);
            continue;
        }
        catch(exception ex)
        {
            Print(ex.what(), false);
            continue;
        }
        Summary(1);
    }
}
