//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "StdAfx.h"
#include "SIMDFloat32x4Operation.h"

#ifdef SIMD_JS_ENABLED

namespace Js
{
    
    // Q: Are we allowed to call this as a constructor ? 
    Var SIMDFloat32x4Lib::EntryFloat32x4(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        // Assert(!(callInfo.Flags & CallFlags_New)); // comment out for language service -ls -stress run

        if (args.Info.Count == 2)
        {
            if (JavascriptSIMDFloat32x4::Is(args[1]))
            {
                return args[1];
            }
            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"float32x4");
        }
        
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();

        // Convert all args to float32 
        float fSIMDX = JavascriptConversion::ToFloat(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);
        float fSIMDY = JavascriptConversion::ToFloat(args.Info.Count >= 3 ? args[2] : undefinedVar, scriptContext);
        float fSIMDZ = JavascriptConversion::ToFloat(args.Info.Count >= 4 ? args[3] : undefinedVar, scriptContext);
        float fSIMDW = JavascriptConversion::ToFloat(args.Info.Count >= 5 ? args[4] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDFloat32x4Operation::OpFloat32x4(fSIMDX, fSIMDY, fSIMDZ, fSIMDW);
        
        return JavascriptSIMDFloat32x4::New(&lanes, scriptContext);
        
    }
    
    Var SIMDFloat32x4Lib::EntryZero(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        SIMDValue lanes = SIMDFloat32x4Operation::OpZero();

        return JavascriptSIMDFloat32x4::New(&lanes, scriptContext);
    }

    Var SIMDFloat32x4Lib::EntrySplat(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));
        
        Var undefinedVar = scriptContext->GetLibrary()->GetUndefined();
        float value = JavascriptConversion::ToFloat(args.Info.Count >= 2 ? args[1] : undefinedVar, scriptContext);

        SIMDValue lanes = SIMDFloat32x4Operation::OpSplat(value);

        return JavascriptSIMDFloat32x4::New(&lanes, scriptContext);
    }

    Var SIMDFloat32x4Lib::EntryFromFloat64x2(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat32x4::FromFloat64x2(instance, scriptContext);
        }
       JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat64x2");
    }

    Var SIMDFloat32x4Lib::EntryFromFloat64x2Bits(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat32x4::FromFloat64x2Bits(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromFloat64x2Bits");
    }

    Var SIMDFloat32x4Lib::EntryFromInt32x4(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat32x4::FromInt32x4(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromInt32x4");
    }

    Var SIMDFloat32x4Lib::EntryFromInt32x4Bits(RecyclableObject* function, CallInfo callInfo, ...)
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

            return JavascriptSIMDFloat32x4::FromInt32x4Bits(instance, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"fromInt32x4Bits");
    }

    // withX/Y/Z/W
    Var SIMDFloat32x4Lib::EntryWithX(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // first arg has to be of type float32x4, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return instance->CopyAndSetLane(SIMD_X, JavascriptConversion::ToFloat(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"withX");
    }

    Var SIMDFloat32x4Lib::EntryWithY(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // first arg has to be of type float32x4, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return instance->CopyAndSetLane(SIMD_Y, JavascriptConversion::ToFloat(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"withY");
    }

    Var SIMDFloat32x4Lib::EntryWithZ(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // first arg has to be of type float32x4, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return instance->CopyAndSetLane(SIMD_Z, JavascriptConversion::ToFloat(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"withZ");
    }

    Var SIMDFloat32x4Lib::EntryWithW(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // first arg has to be of type float32x4, so cannot be missing. 
        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            // if value arg is missing, then it is undefined.
            Var value = args.Info.Count >= 3 ? args[2] : scriptContext->GetLibrary()->GetUndefined();

            JavascriptSIMDFloat32x4 *instance = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(instance);

            return instance->CopyAndSetLane(SIMD_W, JavascriptConversion::ToFloat(value, scriptContext), scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"withW");
    }

    // UnaryOps
    Var SIMDFloat32x4Lib::EntryAbs(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpAbs(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"abs");
    }

    Var SIMDFloat32x4Lib::EntryNeg(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpNeg(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"neg");
    }

    Var SIMDFloat32x4Lib::EntryNot(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpNot(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"not");
    }

    Var SIMDFloat32x4Lib::EntryReciprocal(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpReciprocal(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"reciprocal");
    }

    Var SIMDFloat32x4Lib::EntryReciprocalSqrt(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpReciprocalSqrt(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"reciprocalSqrt");
    }

    Var SIMDFloat32x4Lib::EntrySqrt(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count >= 2 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result = SIMDFloat32x4Operation::OpSqrt(a->GetValue());

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }
        else
            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"sqrt");
    }
    
    Var SIMDFloat32x4Lib::EntryAdd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpAdd(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"add");
    }

    Var SIMDFloat32x4Lib::EntrySub(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3)
        {
            // strict type on both operands
            if (JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
            {
                JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
                JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
                Assert(a && b);

                SIMDValue result, aValue, bValue;

                aValue = a->GetValue();
                bValue = b->GetValue();

                result = SIMDFloat32x4Operation::OpSub(aValue, bValue);

                return JavascriptSIMDFloat32x4::New(&result, scriptContext);
            }
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"sub");
    }

    Var SIMDFloat32x4Lib::EntryMul(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpMul(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"mul");
    }

    Var SIMDFloat32x4Lib::EntryDiv(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpDiv(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"div");
    }

    Var SIMDFloat32x4Lib::EntryAnd(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpAnd(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"and");
    }

    Var SIMDFloat32x4Lib::EntryOr(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpOr(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"or");
    }

    Var SIMDFloat32x4Lib::EntryXor(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpXor(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"xor");
    }

    Var SIMDFloat32x4Lib::EntryMin(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpMin(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"min");
    }

    Var SIMDFloat32x4Lib::EntryMax(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpMax(aValue, bValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"max");
    }

    Var SIMDFloat32x4Lib::EntryScale(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue result, aValue;

            aValue = a->GetValue();
            float scaleValue = JavascriptConversion::ToFloat(args[2], scriptContext);

            result = SIMDFloat32x4Operation::OpScale(aValue, scaleValue);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"scale");
    }

    Var SIMDFloat32x4Lib::EntryLessThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpLessThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"lessThan");
    }

    Var SIMDFloat32x4Lib::EntryLessThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpLessThanOrEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"lessThanOrEqual");
    }

    Var SIMDFloat32x4Lib::EntryEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"equal");
    }

    Var SIMDFloat32x4Lib::EntryNotEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpNotEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"NotEqual");
    }

    Var SIMDFloat32x4Lib::EntryGreaterThan(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpGreaterThan(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"greaterThan");
    }

    Var SIMDFloat32x4Lib::EntryGreaterThanOrEqual(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            SIMDValue result, aValue, bValue;

            aValue = a->GetValue();
            bValue = b->GetValue();

            result = SIMDFloat32x4Operation::OpGreaterThanOrEqual(aValue, bValue);

            return JavascriptSIMDInt32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"greaterThanOrEqual");
    }

    Var SIMDFloat32x4Lib::EntryShuffle(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        if (args.Info.Count >= 3 && JavascriptSIMDFloat32x4::Is(args[1]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            Assert(a);

            SIMDValue source, result;
            Var maskVar = args[2];
            int32 mask = JavascriptConversion::ToInt32(maskVar, scriptContext);
            // only the lowest byte matters
            mask &= 0xff;

            source = a->GetValue();
            result = SIMDFloat32x4Operation::OpShuffle(source, mask);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"shuffle");
    }

    Var SIMDFloat32x4Lib::EntryShuffleMix(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // If any of the args are missing, then it is Undefined type which causes TypeError exception.
        // strict type on both operands
        if (args.Info.Count >= 4 && JavascriptSIMDFloat32x4::Is(args[1]) && JavascriptSIMDFloat32x4::Is(args[2]))
        {
            JavascriptSIMDFloat32x4 *a = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *b = JavascriptSIMDFloat32x4::FromVar(args[2]);
            Assert(a && b);

            Var maskVar = args[3];
            int32 mask = JavascriptConversion::ToInt32(maskVar, scriptContext);
             
            // only the lowest byte matters
            mask &= 0xff;

            SIMDValue source1, source2, result;

            source1 = a->GetValue();
            source2 = b->GetValue();

            result = SIMDFloat32x4Operation::OpShuffleMix(source1, source2, mask);

            return JavascriptSIMDFloat32x4::New(&result, scriptContext);
            
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"shuffleMix");
    }

    Var SIMDFloat32x4Lib::EntryClamp(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // we expect at least 3 explicit args with Float32x4 type
        if (args.Info.Count >= 4 && 
            JavascriptSIMDFloat32x4::Is(args[1]) && 
            JavascriptSIMDFloat32x4::Is(args[2]) && 
            JavascriptSIMDFloat32x4::Is(args[3]))
        {
            JavascriptSIMDFloat32x4 *t = JavascriptSIMDFloat32x4::FromVar(args[1]);
            JavascriptSIMDFloat32x4 *lower = JavascriptSIMDFloat32x4::FromVar(args[2]);
            JavascriptSIMDFloat32x4 *upper = JavascriptSIMDFloat32x4::FromVar(args[3]);
            Assert(t && lower && upper);

            SIMDValue tValue, lowerValue, upperValue, resultValue;
            tValue     = t->GetValue();
            lowerValue = lower->GetValue();
            upperValue = upper->GetValue();
            
            resultValue = SIMDFloat32x4Operation::OpClamp(tValue, lowerValue, upperValue);

            return JavascriptSIMDFloat32x4::New(&resultValue, scriptContext);
            
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"clamp");
    }
    
    Var SIMDFloat32x4Lib::EntrySelect(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");
        Assert(!(callInfo.Flags & CallFlags_New));

        // we expect at least 3 explicit args (Int32x4, Float32x4, Float32x4)
        if (args.Info.Count >= 4 &&
            JavascriptSIMDInt32x4::Is(args[1]) && 
            JavascriptSIMDFloat32x4::Is(args[2]) &&
            JavascriptSIMDFloat32x4::Is(args[3]))
        {
            JavascriptSIMDInt32x4 *mask = JavascriptSIMDInt32x4::FromVar(args[1]);
            Assert(mask);
            JavascriptSIMDFloat32x4 *fmask  = JavascriptSIMDFloat32x4::FromInt32x4Bits(mask, scriptContext);
            JavascriptSIMDFloat32x4 *tvalue = JavascriptSIMDFloat32x4::FromVar(args[2]);
            JavascriptSIMDFloat32x4 *fvalue = JavascriptSIMDFloat32x4::FromVar(args[3]);
            Assert(fmask && tvalue && fvalue);

            SIMDValue maskValue, trueValue, falseValue, resultValue;
            maskValue  = fmask->GetValue();
            trueValue  = tvalue->GetValue();
            falseValue = fvalue->GetValue();

            resultValue = SIMDFloat32x4Operation::OpSelect(maskValue, trueValue, falseValue);

            return JavascriptSIMDFloat32x4::New(&resultValue, scriptContext);
        }

        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"select");
    }
}

#endif