
#include "stdafx.h"
#include "ScriptDirectUnitTests.h"


Var MyFPCWTest(Var function, CallInfo callInfo, Var _this, ...)
{
#if _M_IX86
    unsigned int i = 0;
    _control87(i, _MCW_EM);
#else
    unsigned int i = 0;
    _controlfp_s(0, i, _MCW_EM);
#endif
    return NULL;
}


HRESULT ExternalFuncFpcwTest(IActiveScriptDirect* activeScriptDirect)
{
    HRESULT hr;
    PropertyId propertyId;
    Var jsFunction;
    Var varResult;

    Print("Test basic RunExternalFuncFpcwTest functionality");

    // Add it to the GlobalObject.
    hr = activeScriptDirect->GetOrAddPropertyId(L"MyFPCWTest", &propertyId);
    IfFailedReturn(hr);

    hr = activeScriptDirect->BuildDirectFunction(NULL, MyFPCWTest, propertyId, &jsFunction);
    IfFailedReturn(hr);

    Var args[1];
    args[0] = jsFunction;
    CallInfo callInfo = { 1, CallFlags_None };
    hr = activeScriptDirect->Execute(jsFunction, callInfo, args, /*serviceProvider*/ NULL, &varResult);
    IfFailedReturn(hr);

    return NOERROR;
}

void RunExternalFuncFpcwTest(MyScriptDirectTests* mytest)
{
    HRESULT hr;
    try
    {
        hr = ExternalFuncFpcwTest(mytest->GetScriptDirectNoRef());
        if (FAILED(hr))
        {
            Print("Test RunExternalFuncFpcwTest failed", false);
        }
    }
    catch (std::string message)
    {
        Print(message, false);
    }
    catch (exception ex)
    {
        Print(ex.what(), false);
    }
}
