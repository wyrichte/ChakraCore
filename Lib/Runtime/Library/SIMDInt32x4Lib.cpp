//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "StdAfx.h"
#include "SIMDInt32x4Operation.h"

#ifdef SIMD_JS_ENABLED

namespace Js
{
    
    // Q: Are we allowed to call this as a constructor ? 
    Var SIMDInt32x4Lib::EntryInt32x4(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        //Assert(!(callInfo.Flags & CallFlags_New));    //comment out due to -ls -stress run
        if (args.Info.Count == 2)
        {
            if (JavascriptSIMDInt32x4::Is(args[1]))
            {
                return args[1];
            }
            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"int32x4");
        }

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();

        int intSIMDX = JavascriptConversion::ToInt32(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);
        int intSIMDY = JavascriptConversion::ToInt32(args.Info.Count >= 3 ? args[2] : undefinedVar, scriptContext);
        int intSIMDZ = JavascriptConversion::ToInt32(args.Info.Count >= 4 ? args[3] : undefinedVar, scriptContext);
        int intSIMDW = JavascriptConversion::ToInt32(args.Info.Count >= 5 ? args[4] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDInt32x4Operation::OpInt32x4(intSIMDX, intSIMDY, intSIMDZ, intSIMDW); 
            
        return JavascriptSIMDInt32x4::New(&lanes, scriptContext);
    }

    Var SIMDInt32x4Lib::EntryZero(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        SIMDValue lanes = SIMDInt32x4Operation::OpZero();

        return JavascriptSIMDInt32x4::New(&lanes, scriptContext);
    }

    Var SIMDInt32x4Lib::EntrySplat(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        int value = JavascriptConversion::ToInt32(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDInt32x4Operation::OpSplat(value);

        return JavascriptSIMDInt32x4::New(&lanes, scriptContext);
    }

    Var SIMDInt32x4Lib::EntryBool(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // if incoming parameter is undefined, treat it as false (0), oterwise use typical Javascript boolean conversion.
        // after boolean conversion: if it's true, make it to be -1, if it's false, make it to be 0.
        int intSIMD_X = args.Info.Count >= 2 ? (JavascriptConversion::ToBoolean((args)[1], scriptContext) ? -1 : 0) : 0;
        int intSIMD_Y = args.Info.Count >= 3 ? (JavascriptConversion::ToBoolean((args)[2], scriptContext) ? -1 : 0) : 0;
        int intSIMD_Z = args.Info.Count >= 4 ? (JavascriptConversion::ToBoolean((args)[3], scriptContext) ? -1 : 0) : 0;
        int intSIMD_W = args.Info.Count >= 5 ? (JavascriptConversion::ToBoolean((args)[4], scriptContext) ? -1 : 0) : 0;

        SIMDValue value = SIMDInt32x4Operation::OpBool(intSIMD_X, intSIMD_Y, intSIMD_Z, intSIMD_W);

        return JavascriptSIMDInt32x4::New(&value, scriptContext);
    }

    Var SIMDInt32x4Lib::EntryFromFloat64x2(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *instance = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDInt32x4::FromFloat64x2(instance, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"fromFloat64x2");
    }

    Var SIMDInt32x4Lib::EntryFromFloat64x2Bits(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *instance = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDInt32x4::FromFloat64x2Bits(instance, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat64x2Bits");
    }

    Var SIMDInt32x4Lib::EntryFromFloat32x4(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDInt32x4::FromFloat32x4(instance, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"fromFloat32x4");
    }

    Var SIMDInt32x4Lib::EntryFromFloat32x4Bits(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDInt32x4::FromFloat32x4Bits(instance, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat32x4Bits");
    }

    // withX/Y/Z/W
    Var SIMDInt32x4Lib::EntryWithX(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withX(t, value)
        // if value arg is missing, then it is undefined.
        // t arg has to be int32x4, so cannot be missing.
        
        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLane(SIMD_X, JavascriptConversion::ToInt32(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withX");
    }

    Var SIMDInt32x4Lib::EntryWithY(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withY(t, value)
        // if value arg is missing, then it is undefined.
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLane(SIMD_Y, JavascriptConversion::ToInt32(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withY");
    }

    Var SIMDInt32x4Lib::EntryWithZ(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withZ(t, value)
        // if value arg is missing, then it is undefined.
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLane(SIMD_Z, JavascriptConversion::ToInt32(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withZ");
    }

    Var SIMDInt32x4Lib::EntryWithW(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withW(t, value)
        // if value arg is missing, then it is undefined.
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLane(SIMD_W, JavascriptConversion::ToInt32(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withW");
    }

    // withFlagX/Y/Z/W
    Var SIMDInt32x4Lib::EntryWithFlagX(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withFlagX(t, value)
        // if value arg is missing, then it is undefined, which is false by javascript semantics
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLaneFlag(SIMD_X, JavascriptConversion::ToBoolean(value, scriptContext), scriptContext); 
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withFlagX");
    }

    Var SIMDInt32x4Lib::EntryWithFlagY(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withFlagY(t, value)
        // if value arg is missing, then it is undefined, which is false by javascript semantics
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLaneFlag(SIMD_Y, JavascriptConversion::ToBoolean(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withFlagY");
    }

    Var SIMDInt32x4Lib::EntryWithFlagZ(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withFlagZ(t, value)
        // if value arg is missing, then it is undefined, which is false by javascript semantics
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLaneFlag(SIMD_Z, JavascriptConversion::ToBoolean(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withFlagZ");
    }

    Var SIMDInt32x4Lib::EntryWithFlagW(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withFlagW(t, value)
        // if value arg is missing, then it is undefined, which is false by javascript semantics
        // t arg has to be int32x4, so cannot be missing.

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            return instance->CopyAndSetLaneFlag(SIMD_W, JavascriptConversion::ToBoolean(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"withFlagW");
    }

    Var SIMDInt32x4Lib::EntryAbs(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue value, result;

            value = a->GetValue();
            result = SIMDInt32x4Operation::OpAbs(value);    

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"abs");
    }

    Var SIMDInt32x4Lib::EntryNeg(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue value, result;

            value = a->GetValue();
            result = SIMDInt32x4Operation::OpNeg(value);    

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"neg");
    }

    Var SIMDInt32x4Lib::EntryNot(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue value, result;

            value = a->GetValue();
            result = SIMDInt32x4Operation::OpNot(value);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"not");
    }

    Var SIMDInt32x4Lib::EntryAdd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpAdd(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"add");
    }

    Var SIMDInt32x4Lib::EntrySub(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpSub(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"sub");
    }

    Var SIMDInt32x4Lib::EntryMul(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpMul(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"mul");
    }

    Var SIMDInt32x4Lib::EntryAnd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpAnd(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"and");
    }

    Var SIMDInt32x4Lib::EntryOr(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;
            
            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpOr(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"or");
    }

    Var SIMDInt32x4Lib::EntryXor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpXor(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"xor");
    }

    Var SIMDInt32x4Lib::EntryMin(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpMin(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"min");
    }

    Var SIMDInt32x4Lib::EntryMax(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDInt32x4Operation::OpMax(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"max");
    }

    Var SIMDInt32x4Lib::EntryLessThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDInt32x4Operation::OpLessThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"lessThan");
    }

    Var SIMDInt32x4Lib::EntryEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDInt32x4Operation::OpEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"equal");
    }

    Var SIMDInt32x4Lib::EntryGreaterThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDInt32x4Operation::OpGreaterThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"greaterThan");
    }

    Var SIMDInt32x4Lib::EntryShuffle(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            Var maskVar = args[2];
            int32 mask = JavascriptConversion::ToInt32(maskVar, scriptContext);
            // only the lowest byte matters
            mask &= 0xff;

            SIMDValue source, result;

            source = a->GetValue();
            result = SIMDInt32x4Operation::OpShuffle(source, mask);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"shuffle");
    }

    Var SIMDInt32x4Lib::EntryShuffleMix(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 4 && JavascriptSIMDInt32x4::Is(args[1]) && JavascriptSIMDInt32x4::Is(args[2]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *b = JavascriptSIMDInt32x4::FromVar(args[2]);
            Assert(a && b);

            Var maskVar = args[3];
            int32 mask = JavascriptConversion::ToInt32(maskVar, scriptContext);

            // only the lowest byte matters
            mask &= 0xff;

            SIMDValue source1, source2, result;
            source1 = a->GetValue();
            source2 = b->GetValue();

            result = SIMDInt32x4Operation::OpShuffleMix(source1, source2, mask);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }
        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"shuffleMix");
    }

    Var SIMDInt32x4Lib::EntryShiftLeft(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result, aValue;

            aValue = a->GetValue();
            Var countVar = args[2]; // {int} bits Bit count
            int32 count = JavascriptConversion::ToInt32(countVar, scriptContext);

            result = SIMDInt32x4Operation::OpShiftLeft(aValue, count);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"shiftLeft");
    }

    Var SIMDInt32x4Lib::EntryShiftRightLogical(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result, aValue;

            aValue = a->GetValue();
            Var countVar = args[2]; // {int} bits Bit count
            int32 count = JavascriptConversion::ToInt32(countVar, scriptContext);

            result = SIMDInt32x4Operation::OpShiftRightLogical(aValue, count);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"shiftRightLogical");
    }

    Var SIMDInt32x4Lib::EntryShiftRightArithmetic(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 3 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *a = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result, aValue;

            aValue = a->GetValue();
            Var countVar = args[2]; // {int} bits Bit count
            int32 count = JavascriptConversion::ToInt32(countVar, scriptContext);

            result = SIMDInt32x4Operation::OpShiftRightArithmetic(aValue, count);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"shiftRightArithmetic");
    }

    Var SIMDInt32x4Lib::EntrySelect(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 4 && JavascriptSIMDInt32x4::Is(args[1]) && 
            JavascriptSIMDInt32x4::Is(args[2]) && JavascriptSIMDInt32x4::Is(args[3]))
        {
            JavascriptSIMDInt32x4 *m = JavascriptSIMDInt32x4::FromVar(args[1]);
            JavascriptSIMDInt32x4 *t = JavascriptSIMDInt32x4::FromVar(args[2]);
            JavascriptSIMDInt32x4 *f = JavascriptSIMDInt32x4::FromVar(args[3]);
            Assert(m && t && f);

            SIMDValue result, maskValue, trueValue, falseValue;

            maskValue   = m->GetValue();
            trueValue   = t->GetValue();
            falseValue  = f->GetValue();

            result = SIMDInt32x4Operation::OpSelect(maskValue, trueValue, falseValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"select");
    }
}

#endif