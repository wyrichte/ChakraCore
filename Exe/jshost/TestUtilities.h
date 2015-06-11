/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

typedef IInspectable * (*RuntimeClassFactoryFunction)();

class TestUtilities  /* TODO: Determine actual object */
{
private:
    static DelayLoadWinRt * m_WinRTLibrary;
    static Js::DelayLoadWinRtString * m_WinRTStringLibrary;
    static DevTests::SimpleTestNamespace::ISimpleInterface * GetAndUpdateSimpleClass(IActiveScriptDirect * scriptDirect, Var instance);
public:
    static IInspectable * ActivateRuntimeClassInstance(const WCHAR * runtimeClassName);
    static IInspectable * GetAnimalInstance();
    static IInspectable * GetRestrictedErrorAccessInstance();
    static IInspectable * GetRWineryInstance();
    static Var ActivateRuntimeClassAndConvertToVar(RuntimeClassFactoryFunction fnFactory, Var function, CallInfo callInfo, Var dynamo);

    static Var AnimalToVar(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var VectorIntToVar(Var  function, CallInfo callInfo, Var dynamo, ...);
    static Var GetRestrictedStringFromError(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var GetCapabilitySidFromError(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var GetMemoryFootprintOfRC(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var GetSystemStringFromHr(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var UpdateSimpleClassAndReturnAsVar(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var UpdateSimpleClassAndReturnAsVarByAlternateInterface(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var VarToDispExTest(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var ClearAllProjectionCaches(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var QueryPerformanceCounter(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var QueryPerformanceFrequency(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var DoNotSupportWeakDelegate(Var function, CallInfo callInfom, Var dynamo, ...);
    static Var SupportsWeakDelegate(Var function, CallInfo callInfom, Var dynamo, ...);
    static Var GetHostType(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var RestrictedErrorAccessInstanceToVar(Var function, CallInfo callInfo, Var dynamo, ...);
    static Var RWineryToVar(Var function, CallInfo callInfo, Var dynamo, ...);

    static void Initialize(IActiveScriptDirect* activeScriptDirect, DelayLoadWinRt * winRTLibrary, Js::DelayLoadWinRtString * winRTStringLibrary);
};
