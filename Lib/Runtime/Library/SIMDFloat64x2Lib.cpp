//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "StdAfx.h"
#include "SIMDFloat64x2Operation.h"

#ifdef SIMD_JS_ENABLED

namespace Js
{

    // Q: Are we allowed to call this as a constructor ? 
    Var SIMDFloat64x2Lib::EntryFloat64x2(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        //Assert(!(callInfo.Flags & CallFlags_New));    // comment out to satisfy languager service -ls -stress run

        if (args.Info.Count == 2)
        {
            if (JavascriptSIMDFloat64x2::Is(args[1]))
            {
                return args[1];
            }
            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"float64x2");
        }

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();

        double dSIMDX = JavascriptConversion::ToNumber(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);
        double dSIMDY = JavascriptConversion::ToNumber(args.Info.Count >= 3 ? args[2] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDFloat64x2Operation::OpFloat64x2(dSIMDX, dSIMDY);

        return JavascriptSIMDFloat64x2::New(&lanes, scriptContext);
    }

    Var SIMDFloat64x2Lib::EntryZero(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        SIMDValue lanes = SIMDFloat64x2Operation::OpZero();

        return JavascriptSIMDFloat64x2::New(&lanes, scriptContext);
    }

    Var SIMDFloat64x2Lib::EntrySplat(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        double value = JavascriptConversion::ToNumber(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDFloat64x2Operation::OpSplat(value);

        return JavascriptSIMDFloat64x2::New(&lanes, scriptContext);
    }

    Var SIMDFloat64x2Lib::EntryFromFloat32x4(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat64x2::FromFloat32x4(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat32x4");
    }

    Var SIMDFloat64x2Lib::EntryFromFloat32x4Bits(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat64x2::FromFloat32x4Bits(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat32x4Bits");
    }

    Var SIMDFloat64x2Lib::EntryFromInt32x4(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDFloat64x2::FromInt32x4(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromInt32x4");
    }

    Var SIMDFloat64x2Lib::EntryFromInt32x4Bits(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDInt32x4::Is(args[1]))
        {
            JavascriptSIMDInt32x4 *instance = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(instance);

            return JavascriptSIMDFloat64x2::FromInt32x4Bits(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromInt32x4Bits");
    }


    // withX/Y
    Var SIMDFloat64x2Lib::EntryWithX(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withX(t, value)
        
        // first arg has to be of type float64x2, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *instance = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(instance);

            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();
            return instance->CopyAndSetLane(SIMD_X, JavascriptConversion::ToNumber(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"withX");
    }

    Var SIMDFloat64x2Lib::EntryWithY(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // withY(t, value)

        // first arg has to be of type float64x2, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *instance = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(instance);

            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();
            return instance->CopyAndSetLane(SIMD_Y, JavascriptConversion::ToNumber(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"withY");
    }

    Var SIMDFloat64x2Lib::EntryNot(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpNot(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"not");
    }

    Var SIMDFloat64x2Lib::EntryAbs(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpAbs(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"abs");
    }

    Var SIMDFloat64x2Lib::EntryNeg(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpNeg(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"neg");
    }

    Var SIMDFloat64x2Lib::EntrySqrt(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpSqrt(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"sqrt");
    }

    Var SIMDFloat64x2Lib::EntryReciprocal(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpReciprocal(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"reciprocal");
    }

    Var SIMDFloat64x2Lib::EntryReciprocalSqrt(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, value;
            value = a->GetValue();
            result = SIMDFloat64x2Operation::OpReciprocalSqrt(value);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"reciprocalSqrt");
    }

    Var SIMDFloat64x2Lib::EntryAdd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpAdd(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"add");
    }

    Var SIMDFloat64x2Lib::EntrySub(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpSub(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"sub");
    }

    Var SIMDFloat64x2Lib::EntryMul(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpMul(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"mul");
    }

    Var SIMDFloat64x2Lib::EntryDiv(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpDiv(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"div");
    }

    Var SIMDFloat64x2Lib::EntryAnd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpAnd(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"and");
    }

    Var SIMDFloat64x2Lib::EntryOr(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpOr(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"or");
    }

    Var SIMDFloat64x2Lib::EntryXor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpXor(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"xor");
    }

    Var SIMDFloat64x2Lib::EntryMin(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpMin(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"min");
    }

    Var SIMDFloat64x2Lib::EntryMax(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat64x2Operation::OpMax(aValue, bValue);
            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"max");
    }

    Var SIMDFloat64x2Lib::EntryScale(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            Assert(a);

            SIMDValue result, aValue;

            aValue = a->GetValue();
            double scaleValue = JavascriptConversion::ToNumber(args[2], scriptContext);
            result = SIMDFloat64x2Operation::OpScale(aValue, scaleValue);

            return JavascriptSIMDFloat64x2::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"scale");
    }

    Var SIMDFloat64x2Lib::EntryLessThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpLessThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"lessThan");
    }
    
    Var SIMDFloat64x2Lib::EntryLessThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpLessThanOrEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"lessThanOrEqual");
    }

    Var SIMDFloat64x2Lib::EntryEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"equal");
    }

    Var SIMDFloat64x2Lib::EntryNotEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpNotEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"notEqual");
    }

    Var SIMDFloat64x2Lib::EntryGreaterThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpGreaterThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"greaterThan");
    }

    Var SIMDFloat64x2Lib::EntryGreaterThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();
            result = SIMDFloat64x2Operation::OpGreaterThanOrEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"greaterThanOrEqual");
    }

    Var SIMDFloat64x2Lib::EntryShuffle(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat64x2::Is(args[1]))
        {
            return InnerShuffle(args[1], args[1], args[2], scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"shuffle");
    }
    
    Var SIMDFloat64x2Lib::EntryShuffleMix(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 4 && JavascriptSIMDFloat64x2::Is(args[1]) && JavascriptSIMDFloat64x2::Is(args[2]))
        {
            return InnerShuffle(args[1], args[2], args[3], scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"shuffleMix");
    }

    

    Var SIMDFloat64x2Lib::EntryClamp(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // we expect at least 3 explicit args with Float64x2 type
        if (args.Info.Count >= 4 && 
            JavascriptSIMDFloat64x2::Is(args[1]) && 
            JavascriptSIMDFloat64x2::Is(args[2]) && 
            JavascriptSIMDFloat64x2::Is(args[3]))
        {
            JavascriptSIMDFloat64x2 *t     = JavascriptSIMDFloat64x2::FromVar(args[1]);
            JavascriptSIMDFloat64x2 *lower = JavascriptSIMDFloat64x2::FromVar(args[2]);
            JavascriptSIMDFloat64x2 *upper = JavascriptSIMDFloat64x2::FromVar(args[3]);
            Assert(t && lower && upper);

            SIMDValue tValue, lowerValue, upperValue, resultValue;

            tValue = t->GetValue();
            lowerValue = lower->GetValue();
            upperValue = upper->GetValue();
            
            resultValue = SIMDFloat64x2Operation::OpClamp(tValue, lowerValue, upperValue);

            return JavascriptSIMDFloat64x2::New(&resultValue, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"clamp");
    }

    Var SIMDFloat64x2Lib::EntrySelect(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // we expect at least 3 explicit args (Int32x4, Float32x4, Float32x4)
        if (args.Info.Count >= 4 &&
            JavascriptSIMDInt32x4::Is(args[1]) &&
            JavascriptSIMDFloat64x2::Is(args[2]) &&
            JavascriptSIMDFloat64x2::Is(args[3]))
        {
            JavascriptSIMDInt32x4 *mask = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(mask);

            JavascriptSIMDFloat64x2 *fmask = JavascriptSIMDFloat64x2::FromInt32x4Bits(mask, scriptContext);
            JavascriptSIMDFloat64x2 *tvalue = JavascriptSIMDFloat64x2::FromVar(args[2]);
            JavascriptSIMDFloat64x2 *fvalue = JavascriptSIMDFloat64x2::FromVar(args[3]);
            Assert(fmask && tvalue && fvalue);

            SIMDValue maskValue, trueValue, falseValue, resultValue;

            maskValue = fmask->GetValue();
            trueValue = tvalue->GetValue();
            falseValue = fvalue->GetValue();

            resultValue = SIMDFloat64x2Operation::OpSelect(maskValue, trueValue, falseValue);

            return JavascriptSIMDFloat64x2::New(&resultValue, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"select");
    }

    Var SIMDFloat64x2Lib::InnerShuffle(Var src1, Var src2, Var maskVar, ScriptContext* scriptContext)
    {
        JavascriptSIMDFloat64x2 *a = JavascriptSIMDFloat64x2::FromVar(src1);
        JavascriptSIMDFloat64x2 *b = JavascriptSIMDFloat64x2::FromVar(src2);
        Assert(a && b);

        int32 mask = JavascriptConversion::ToInt32(maskVar, scriptContext);
        // only the lowest 2 bits matters, the rest has to be zeroed
        mask &= 0x3;

        SIMDValue source1, source2, result;

        source1 = a->GetValue();
        source2 = b->GetValue();

        result = SIMDFloat64x2Operation::OpShuffleMix(source1, source2, mask);

        return JavascriptSIMDFloat64x2::New(&result, scriptContext);
    }
}

#endif