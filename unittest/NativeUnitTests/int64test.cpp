
#include "stdafx.h"
#include "ScriptDirectUnitTests.h"


struct Int64TestVar {
    LPWSTR varName;
    unsigned __int64 value;
    BOOL isSigned; 
};

struct Int64TestCase {
    unsigned __int64 value;
    BOOL srcIsSigned; 
    BOOL dstIsSigned;
    LPWSTR testCode;
    BOOL sameResult;
};

Int64TestCase int64TestCases[] = 
{
    {0, FALSE, FALSE, NULL, TRUE},
    {65536, FALSE, FALSE, NULL, TRUE },
    {0xffffffff, FALSE, FALSE, NULL, TRUE },
    {0x3fffffff, FALSE, FALSE, NULL, TRUE },
    {0xfffffffff, FALSE, FALSE, NULL, TRUE },
    {0xffffffffffffff, FALSE, FALSE, NULL, TRUE },
    {0xffffffffffffffff, FALSE, FALSE, NULL, TRUE },
    {0x7fffffffffffffff, FALSE, FALSE, NULL, TRUE },
    {0x8000000000000000, FALSE, FALSE, NULL, TRUE },
    {0xfffffffffff3333, FALSE, FALSE, NULL, TRUE },

    {0, FALSE, TRUE, NULL, TRUE},
    {65536, FALSE, TRUE, NULL, TRUE },
    {0xffffffff, FALSE, TRUE, NULL, TRUE },
    {0x3fffffff, FALSE, TRUE, NULL, TRUE },
    {0xfffffffff, FALSE, TRUE, NULL, TRUE },
    {0xffffffffffffff, FALSE, TRUE, NULL, TRUE },
    {0xffffffffffffffff, FALSE, TRUE, NULL, TRUE },
    {0x7fffffffffffffff, FALSE, TRUE, NULL, TRUE },
    {0x8000000000000000, FALSE, TRUE, NULL, TRUE },
    {0xfffffffffff3333, FALSE, TRUE, NULL, TRUE },

    {0, TRUE, TRUE, NULL, TRUE},
    {65536, TRUE, TRUE, NULL, TRUE },
    {0xffffffff, TRUE, TRUE, NULL, TRUE },
    {0x3fffffff, TRUE, TRUE, NULL, TRUE },
    {0xfffffffff, TRUE, TRUE, NULL, TRUE },
    {0xffffffffffffff, TRUE, TRUE, NULL, TRUE },
    {0xffffffffffffffff, TRUE, TRUE, NULL, TRUE },
    {0x7fffffffffffffff, TRUE, TRUE, NULL, TRUE },
    {0x8000000000000000, TRUE, TRUE, NULL, TRUE },
    {0xfffffffffff3333, TRUE, TRUE, NULL, TRUE },

    {0, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE},
    {65536, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x3fffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xfffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x7fffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x8000000000000000, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), TRUE },
    {0xfffffffffff3333, FALSE, FALSE, _u("varInt64 = varInt64 +1;"), FALSE },

    {0, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE},
    {65536, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x3fffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xfffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0xffffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x7fffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },
    {0x8000000000000000, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), TRUE },
    {0xfffffffffff3333, TRUE, TRUE, _u("varInt64 = varInt64 +1;"), FALSE },

    {0, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE},
    {65536, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0x3fffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xfffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), FALSE },
    {0xffffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), FALSE },
    {0x7fffffffffffffff, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), FALSE },
    {0x8000000000000000, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xfffffffffff3333, FALSE, FALSE, _u("varInt64 = varInt64 +1 -1;"), FALSE },

    {0, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE},
    {65536, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0x3fffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xfffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), FALSE },
    {0xffffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0x7fffffffffffffff, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), FALSE },
    {0x8000000000000000, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), TRUE },
    {0xfffffffffff3333, TRUE, TRUE, _u("varInt64 = varInt64 +1 -1;"), FALSE },

};


Int64TestVar testVars[] = 
{ 
    { _u("unsignedIntVar0"), 0, FALSE },
    { _u("unsignedIntVar1"), 65536, FALSE, }, 
    { _u("unsignedIntVar2"), 0xffffffff, FALSE, }, 
    { _u("unsignedIntVar3"), 0x3fffffff, FALSE, }, 
    { _u("unsignedIntVar4"), 0xfffffffff, FALSE, }, 
    { _u("unsignedIntVar5"), 0xffffffffffffff, FALSE, }, 
    { _u("unsignedIntVar6"), 0xfffffffffffffff, FALSE, }, 
    { _u("unsignedIntVar7"), 0xffffffffffffffff, FALSE, }, 
    { _u("unsignedIntVar8"), 0x7fffffffffffffff, FALSE, }, 
    { _u("unsignedIntVar9"), 0x8000000000000000, FALSE, }, 
    { _u("unsignedIntVara"), 0xfffffffffff3333, FALSE, }, 
    { _u("signedIntVar0"), 0, TRUE },
    { _u("signedIntVar1"), 65536, TRUE, }, 
    { _u("signedIntVar2"), 0xffffffff, TRUE, }, 
    { _u("signedIntVar3"), 0x3fffffff, TRUE, }, 
    { _u("signedIntVar4"), 0xfffffffff, TRUE, }, 
    { _u("signedIntVar5"), 0xffffffffffffff, TRUE, }, 
    { _u("signedIntVar6"), 0xfffffffffffffff, TRUE, }, 
    { _u("signedIntVar7"), 0xfffffffffff3333, TRUE, }, 
    { _u("signedIntVar8"), 0x7fffffffffffffff, TRUE, }, 
    { _u("signedIntVar9"), 0x8000000000000000, TRUE, }, 
    { _u("signedIntVara"), 0xfffffffffff3333, TRUE, }, 
};

LPWSTR testCases[] = 
{
    _u("function testproto() {WScript.Echo('first');Number.prototype.foo=function(){WScript.Echo(this.bar);}; unsignedIntVara.foo();}; testproto();"),

    _u("function test0() {if (unsignedIntVar0 == 0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test0();"),
    _u("function test1() {if (unsignedIntVar1 == 65536) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test1();"),
    _u("function test2() {if (unsignedIntVar2 == 0xffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test2();"),
    _u("function test3() {if (unsignedIntVar3 == 0x3fffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test3();"),
    _u("function test4() {if (unsignedIntVar4 == 0xfffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test4();"),
    _u("function test5() {if (unsignedIntVar5 == 0xffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test5();"),
    _u("function test6() {if (unsignedIntVar6 == 0xfffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test6();"),
    _u("function test7() {if (unsignedIntVar7 == 0xffffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test7();"),
    _u("function test8() {if (unsignedIntVar8 == 0x7fffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test8();"),
    _u("function test9() {if (unsignedIntVar9 == 0x8000000000000000) WScript.Echo('succeeded'); else WScript.Echo('failed')}; test9();"),
    _u("function testa() {if (unsignedIntVara == 0xfffffffffff3333) WScript.Echo('succeeded'); else WScript.Echo('failed')} testa();"),

    _u("function testsigned0() {if (signedIntVar0 == 0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned0();"),
    _u("function testsigned1() {if (signedIntVar1 == 65536) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned1();"),
    _u("function testsigned2() {if (signedIntVar2 == 0xffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned2();"),
    _u("function testsigned3() {if (signedIntVar3 == 0x3fffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned3();"),
    _u("function testsigned4() {if (signedIntVar4 == 0xfffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned4();"),
    _u("function testsigned5() {if (signedIntVar5 == 0xffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned5();"),
    _u("function testsigned6() {if (signedIntVar6 == 0xfffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned6();"),
    _u("function testsigned7() {if (signedIntVar7 == 0xffffffffffffffff) WScript.Echo('failed'); else WScript.Echo('succeeded')}; testsigned7();"), // sign difference
    _u("function testsigned8() {if (signedIntVar8 == 0x7fffffffffffffff) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigned8();"),
    _u("function testsigned9() {if (signedIntVar9 == 0x8000000000000000) WScript.Echo('failed'); else WScript.Echo('succeeded')}; testsigned9();"),// sign difference
    _u("function testsigneda() {if (signedIntVara == 0xfffffffffff3333) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsigneda();"),

    _u("function testequal0() {if (unsignedIntVar0 == signedIntVar0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal0();"),
    _u("function testequal1() {if (unsignedIntVar1 == signedIntVar1) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal1();"),
    _u("function testequal2() {if (unsignedIntVar2 == signedIntVar2) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal2();"),
    _u("function testequal3() {if (unsignedIntVar3 == signedIntVar3) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal3();"),
    _u("function testequal4() {if (unsignedIntVar4 == signedIntVar4) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal4();"),
    _u("function testequal5() {if (unsignedIntVar5 == signedIntVar5) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal5();"),
    _u("function testequal6() {if (unsignedIntVar6 == signedIntVar6) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal6();"),
    _u("function testequal7() {if (unsignedIntVar7 == signedIntVar7) WScript.Echo('failed'); else WScript.Echo('succeeded')}; testequal7();"),
    _u("function testequal8() {if (unsignedIntVar8 == signedIntVar8) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal8();"),
    _u("function testequal9() {if (unsignedIntVar9 == signedIntVar9) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequal9();"),
    _u("function testequala() {if (unsignedIntVara == signedIntVara) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testequala();"),

    _u("function teststrictequal0() {if (unsignedIntVar0 === signedIntVar0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal0();"),
    _u("function teststrictequal1() {if (unsignedIntVar1 === signedIntVar1) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal1();"),
    _u("function teststrictequal2() {if (unsignedIntVar2 === signedIntVar2) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal2();"),
    _u("function teststrictequal3() {if (unsignedIntVar3 === signedIntVar3) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal3();"),
    _u("function teststrictequal4() {if (unsignedIntVar4 === signedIntVar4) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal4();"),
    _u("function teststrictequal5() {if (unsignedIntVar5 === signedIntVar5) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal5();"),
    _u("function teststrictequal6() {if (unsignedIntVar6 === signedIntVar6) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal6();"),
    _u("function teststrictequal7() {if (unsignedIntVar7 === signedIntVar7) WScript.Echo('failed'); else WScript.Echo('succeeded')}; teststrictequal7();"),
    _u("function teststrictequal8() {if (unsignedIntVar8 === signedIntVar8) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal8();"),
    _u("function teststrictequal9() {if (unsignedIntVar9 === signedIntVar9) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequal9();"),
    _u("function teststrictequala() {if (unsignedIntVara === signedIntVara) WScript.Echo('succeeded'); else WScript.Echo('failed')}; teststrictequala();"),

    _u("function testgt0() {if (unsignedIntVar0 > signedIntVar0)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt0();"),
    _u("function testgt1() {if (unsignedIntVar1 > signedIntVar1)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt1();"),
    _u("function testgt2() {if (unsignedIntVar2 > signedIntVar2)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt2();"),
    _u("function testgt3() {if (unsignedIntVar3 > signedIntVar3)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt3();"),
    _u("function testgt4() {if (unsignedIntVar4 > signedIntVar4)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt4();"),
    _u("function testgt5() {if (unsignedIntVar5 > signedIntVar5)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt5();"),
    _u("function testgt6() {if (unsignedIntVar6 > signedIntVar6)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt6();"),
    _u("function testgt7() {if (unsignedIntVar7 > signedIntVar7)  WScript.Echo('succeeded'); else WScript.Echo('failed')}; testgt7();"),
    _u("function testgt8() {if (unsignedIntVar8 > signedIntVar8)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgt8();"),
    _u("function testgt9() {if (unsignedIntVar9 > signedIntVar9)  WScript.Echo('succeeded'); else WScript.Echo('failed')}; testgt9();"),
    _u("function testgta() {if (unsignedIntVara > signedIntVara)  WScript.Echo('failed'); else WScript.Echo('succeeded')}; testgta();"),

    _u("function testarithmeticequal0() {if (unsignedIntVar0 + 1 -1 == unsignedIntVar0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal0();"),
    _u("function testarithmeticequal1() {if (unsignedIntVar1 + 1 -1 == unsignedIntVar1) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal1();"),
    _u("function testarithmeticequal2() {if (unsignedIntVar2 + 1 -1 == unsignedIntVar2) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal2();"),
    _u("function testarithmeticequal3() {if (unsignedIntVar3 + 1 -1 == unsignedIntVar3) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal3();"),
    _u("function testarithmeticequal4() {if (unsignedIntVar4 + 1 -1 == unsignedIntVar4) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal4();"),
    _u("function testarithmeticequal5() {if (unsignedIntVar5 + 1 -1 == unsignedIntVar5) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal5();"),
    _u("function testarithmeticequal6() {if (unsignedIntVar6 + 1 -1 == unsignedIntVar6) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal6();"),
    _u("function testarithmeticequal7() {if (unsignedIntVar7 + 1 -1 == unsignedIntVar7) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal7();"),
    _u("function testarithmeticequal8() {if (unsignedIntVar8 + 1 -1 == unsignedIntVar8) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal8();"),
    _u("function testarithmeticequal9() {if (unsignedIntVar9 + 1 -1 == unsignedIntVar9)  WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequal9();"),
    _u("function testarithmeticequala() {if (unsignedIntVara + 1 -1 == unsignedIntVara) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testarithmeticequala();"),

    _u("function testsignedarithmeticequal0() {if (signedIntVar0 + 1 -1 == signedIntVar0) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal0();"),
    _u("function testsignedarithmeticequal1() {if (signedIntVar1 + 1 -1 == signedIntVar1) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal1();"),
    _u("function testsignedarithmeticequal2() {if (signedIntVar2 + 1 -1 == signedIntVar2) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal2();"),
    _u("function testsignedarithmeticequal3() {if (signedIntVar3 + 1 -1 == signedIntVar3) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal3();"),
    _u("function testsignedarithmeticequal4() {if (signedIntVar4 + 1 -1 == signedIntVar4) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal4();"),
    _u("function testsignedarithmeticequal5() {if (signedIntVar5 + 1 -1 == signedIntVar5) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal5();"),
    _u("function testsignedarithmeticequal6() {if (signedIntVar6 + 1 -1 == signedIntVar6) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal6();"),
    _u("function testsignedarithmeticequal7() {if (signedIntVar7 + 1 -1 == signedIntVar7) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal7();"),
    _u("function testsignedarithmeticequal8() {if (signedIntVar8 + 1 -1 == signedIntVar8) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal8();"),
    _u("function testsignedarithmeticequal9() {if (signedIntVar9 + 1 -1 == signedIntVar9)  WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequal9();"),
    _u("function testsignedarithmeticequala() {if (signedIntVara + 1 -1 == signedIntVara) WScript.Echo('succeeded'); else WScript.Echo('failed')}; testsignedarithmeticequala();"),
};

LPWSTR builtinTestCases[] = {
	_u("(function() { var operations = Object.getOwnPropertyNames(Number.prototype); for (j in operations) WScript.Echo(operations[j]); for (i = 0; i < 10; i++)	{ for (j in operations) {WScript.Echo(Number.prototype[operations[j]].call(this['signedIntVar' + i])) }}})();"),
	_u("(function() { var operations = Object.getOwnPropertyNames(Number.prototype); for (j in operations) WScript.Echo(operations[j]); for (i = 0; i < 10; i++)	{ for (j in operations) {WScript.Echo(Number.prototype[operations[j]].call(this['signedIntVar' + i])) }}})();")
};

HRESULT SetupTestInt64(IActiveScriptDirect* activeScriptDirect)
{
    Var testVar;
    HRESULT hr;
    Var globalObject;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    IfFailedReturn(hr);
    ITypeOperations* defaultScriptOperations;
    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    if (FAILED(hr))
    {
        return hr;
    }

    PropertyId propertyId;
    BOOL wasPropertySet;

    for (size_t i = 0 ; i < RTL_NUMBER_OF(testVars); i++)
    {
        if (testVars[i].isSigned)
        {
            hr = activeScriptDirect->Int64ToVar(testVars[i].value, &testVar);
        }
        else
        {
            hr = activeScriptDirect->UInt64ToVar(testVars[i].value, &testVar);
        }
        IfFailedReturn(hr);
        hr = activeScriptDirect->GetOrAddPropertyId(testVars[i].varName, &propertyId);
        IfFailedReturn(hr);

        hr = defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, testVar, &wasPropertySet);
        IfFailedReturn(hr);
    }

    return hr;
}

HRESULT TestRoundTrip(IActiveScriptDirect* activeScriptDirect, Int64TestCase* testCase)
{
    Var testVar;
    HRESULT hr;
    Var globalObject, topFunc;
    hr = activeScriptDirect->GetGlobalObject(&globalObject);
    IfFailedReturn(hr);
    PropertyId propertyId;
    BOOL wasPropertySet;
    Var resultVar;
    CallInfo callInfo = {0, CallFlags_None};
    ITypeOperations* defaultScriptOperations;
    hr = activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = activeScriptDirect->GetOrAddPropertyId(_u("varInt64"), &propertyId);
    IfFailedReturn(hr);

    if (testCase->srcIsSigned)
    {
        IfFailedReturn(activeScriptDirect->Int64ToVar(testCase->value, &testVar));
    }
    else
    {
        IfFailedReturn(activeScriptDirect->UInt64ToVar(testCase->value, &testVar));
    }
    defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, propertyId, testVar, &wasPropertySet);
    BOOL isEqual;
    if (testCase->testCode)
    {
        IfFailedReturn(activeScriptDirect->Parse(testCase->testCode, &topFunc));
        IfFailedReturn(activeScriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &resultVar));
    }
    IfFailedReturn(defaultScriptOperations->GetOwnProperty(activeScriptDirect, globalObject, propertyId, &resultVar, &wasPropertySet));
    if (testCase->dstIsSigned)
    {
        __int64 retInt64;
        activeScriptDirect->VarToInt64(resultVar, &retInt64);
        if (testCase->srcIsSigned)
        {
            isEqual = (static_cast<__int64>(testCase->value) == retInt64);
        }
        else
        {
            isEqual = (static_cast<unsigned __int64>(retInt64) == testCase->value);
        }
        if (! (isEqual ^ testCase->sameResult))
        {
            return NOERROR;
        }
        printf("test failure: left: %16x right %16x \n", testCase->value, retInt64);
        return E_FAIL;
    }
    else
    {
        unsigned __int64 retInt64;
        activeScriptDirect->VarToUInt64(resultVar, &retInt64);
        if (testCase->srcIsSigned)
        {
            isEqual = (static_cast<__int64>(testCase->value) == static_cast<__int64>(retInt64));
        }
        else
        {
            isEqual = (testCase->value == retInt64);
        }
        isEqual = (retInt64 == testCase->value);
        if (! (isEqual ^ testCase->sameResult))
        {
            return NOERROR;
        }
        printf("test failure: left: %l right %l \n", testCase->value, retInt64);
        return E_FAIL;
    }
}

HRESULT TestToVARIANT(IActiveScriptDirect* activeScriptDirect)
{
    HRESULT hr = NOERROR;
    Var int64Var;
    hr = activeScriptDirect->Int64ToVar(0xffffffffff, &int64Var);
    IfFailedReturn(hr);
    VARIANT variant;
    hr = activeScriptDirect->ChangeTypeFromVar(int64Var, VT_R8, &variant);
    IfFailedReturn(hr);
    printf("int64 convert back to r8 is 0x%llx\n", (__int64)variant.dblVal);

    hr = activeScriptDirect->UInt64ToVar(0x80000000000, &int64Var);
    IfFailedReturn(hr);
    hr = activeScriptDirect->ChangeTypeFromVar(int64Var, VT_R8, &variant);
    IfFailedReturn(hr);
    unsigned __int64 result = (unsigned __int64)(variant.dblVal);
    printf("uint64 convert back to r8 is 0x%llx\n", result);

    return NOERROR;
}

HRESULT TestBuiltins(IActiveScriptDirect* activeScriptDirect)
{
	HRESULT hr;
    Var topFunc;
    Var varResult;
    CallInfo callInfo = {0, CallFlags_None};
	for (int i = 0; i < RTL_NUMBER_OF(builtinTestCases); i++)
	{
		printf("buildin test case %d ", i);
		hr = activeScriptDirect->Parse(builtinTestCases[i], &topFunc);
		IfFailedReturn(hr);
		hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &varResult);
		IfFailedReturn(hr);
	}
	return NOERROR;
}

HRESULT TestInt64(IActiveScriptDirect* activeScriptDirect)
{
    printf("test int64\n");
    HRESULT hr;
    hr = SetupTestInt64(activeScriptDirect);
    IfFailedReturn(hr);
    Var topFunc;
    Var varResult;
    CallInfo callInfo = {0, CallFlags_None};
 
    for (int i = 0; i < RTL_NUMBER_OF(testCases); i++)
    {
        printf("test case %d ", i);
        hr = activeScriptDirect->Parse(testCases[i], &topFunc);
        IfFailedReturn(hr);
        hr = activeScriptDirect->Execute(topFunc, callInfo, NULL, /*serviceProvider*/ NULL, &varResult);
        IfFailedReturn(hr);
    }

    printf("test round trip\n");
    for (int i = 0; i < RTL_NUMBER_OF(int64TestCases); i++)
    {
        printf("test case %d \n", i);
        IfFailedReturn(TestRoundTrip(activeScriptDirect, &int64TestCases[i]));
    }

    printf("test converting to VARIANT\n");
    IfFailedReturn(TestToVARIANT(activeScriptDirect));

    IfFailedReturn(TestBuiltins(activeScriptDirect));
    printf("int64 test succeeded\n");

    return NOERROR;
}


//Bug OS 109046
HRESULT UInt64EqualityTest(IActiveScriptDirect* activeScriptDirect, __int32 intValue, unsigned __int64 uint64Value)
{
	HRESULT hr;
	Var uint64maxVar(nullptr);
	Var minusOneInt32Var(nullptr);
	Var globalObject(nullptr);
	Var topFunc(nullptr);

	IfFailedReturn(activeScriptDirect->UInt64ToVar(uint64Value, &uint64maxVar));
	IfFailedReturn(activeScriptDirect->IntToVar(intValue, &minusOneInt32Var));
	IfFailedReturn(activeScriptDirect->GetGlobalObject(&globalObject));
	IfFailedReturn(activeScriptDirect->Parse(_u("function t(){ if (minusOneInt32 == uint64max || uint64max == minusOneInt32) { WScript.Echo('Values are equal, comparing \"' + minusOneInt32 + '\" and \"'+ uint64max+'\"!'); } else { WScript.Echo('Values are not equal, comparing \"' + minusOneInt32 + '\" and \"'+ uint64max+'\"!'); } }; t();"), &topFunc));

	ITypeOperations* defaultScriptOperations(nullptr);
	Var result(nullptr);
	PropertyId uint64MaxPropID;
	PropertyId minusOneInt32PropID;
	BOOL success = false;
	IfFailedReturn(activeScriptDirect->GetDefaultTypeOperations(&defaultScriptOperations));
	IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(_u("uint64max"), &uint64MaxPropID));
	IfFailedReturn(activeScriptDirect->GetOrAddPropertyId(_u("minusOneInt32"), &minusOneInt32PropID));
	IfFailedReturn(defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, uint64MaxPropID, uint64maxVar, &success));
	IfFailedReturn(defaultScriptOperations->SetProperty(activeScriptDirect, globalObject, minusOneInt32PropID, minusOneInt32Var, &success));
	
	if (!success)
	{
		return E_FAIL;
	}

	CallInfo callInfo = { 0, CallFlags_None };
	IfFailedReturn(activeScriptDirect->Execute(topFunc, callInfo, /*parameters*/ nullptr, /*serviceProvider*/ nullptr, &result));
	
	return NOERROR;
}

void RunInt64Tests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    HRESULT hr;
    try
    {
		IActiveScriptDirect *scriptDirect = mytest->GetScriptDirectNoRef();

		hr = UInt64EqualityTest(scriptDirect, -1, 0xFFFFFFFFFFFFFFFF);
		if (FAILED(hr))
		{
			printf("UInt64Equality test failed with %x\n!", hr);
		}

		hr = UInt64EqualityTest(scriptDirect, 0x7FFFFFFF, 0xFFFFFFFF7FFFFFFF);
		if (FAILED(hr))
		{
			printf("UInt64Equality test failed with %x\n!", hr);
		}

		hr = UInt64EqualityTest(scriptDirect, 0x7FFFFFFF, 0x000000007FFFFFFF);
		if (FAILED(hr))
		{
			printf("UInt64Equality test failed with %x\n!", hr);
		}

		hr = TestInt64(scriptDirect);
        if (FAILED(hr))
        {
            printf("int64 test failed with %x\n", hr);
        }
    }
    catch(std::string message)
    {
        Print(message, false);
    }
    catch(exception ex)
    {
        Print(ex.what(), false);
    }
}
