//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//
// ATTENTION - THIS FILE CONTAINS THIRD PARTY OPEN SOURCE CODE: ASMJS COMPONENT OF ODINMONKEY
// IT IS CLEARED ONLY FOR LIMITED USE BY CHAKRA TEAM FOR THE CHAKRA JAVASCRIPT RUNTIME COMPONENT. 
// DO NOT USE OR SHARE THIS CODE WITHOUT APPROVAL PURSUANT TO THE MICROSOFT OPEN 
// SOURCE SOFTWARE APPROVAL POLICY.
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{

    bool ParserWrapper::ParseVarOrConstStatement( AsmJSParser &parser, ParseNode **var )
    {
        Assert( parser );
        *var = null;
        ParseNode *body = parser->sxFnc.pnodeBody;
        if( body )
        {
            ParseNode* lhs = GetBinaryLeft( body );
            ParseNode* rhs = GetBinaryRight( body );
            if( rhs && rhs->nop == knopList )
            {
                AssertMsg( lhs->nop == knopStr, "this should be use asm" );
                *var = rhs;
                return true;
            }
        }
        return false;
    }

    bool ParserWrapper::IsDefinition( ParseNode *arg )
    {
        //TODO, eliminate duplicates
        return true;
    }



    ParseNode* ParserWrapper::NextInList( ParseNode *node )
    {
        Assert( node->nop == knopList );
        return node->sxBin.pnode2;
    }

    ParseNode* ParserWrapper::NextVar( ParseNode *node )
    {
        return node->sxVar.pnodeNext;
    }

    ParseNode* ParserWrapper::FunctionArgsList( ParseNode *node, unsigned &numformals )
    {
        Assert( node->nop == knopFncDecl );
        PnFnc func = node->sxFnc;
        ParseNode* first = func.pnodeArgs;
        for( ParseNode* pnode = first; pnode; pnode = pnode->sxVar.pnodeNext, numformals++ );
        return first;
    }

    PropertyName ParserWrapper::VariableName( ParseNode *node )
    {
        return node->name();
    }

    PropertyName ParserWrapper::FunctionName( ParseNode *node )
    {
        if( node->nop == knopFncDecl )
        {
            PnFnc function = node->sxFnc;
            if( function.pnodeNames && function.pnodeNames->nop == knopVarDecl )
            {
                return function.pnodeNames->sxVar.pid;
            }
        }
        return null;
    }

    ParseNode * ParserWrapper::GetVarDeclList( ParseNode * pnode )
    {
        ParseNode* varNode = pnode;
        while (varNode->nop == knopList)
        {
            ParseNode * var = GetBinaryLeft(varNode);
            if (var->nop == knopVarDecl)
            {
                return var;
            }
            else if (var->nop == knopVarDeclList || var->nop == knopList)
            {
                var = GetBinaryLeft(var);
                if (var->nop == knopVarDecl)
                {
                    return var;
                }
            }
            varNode = GetBinaryRight(varNode);
        }
        return nullptr;
    }

    void ParserWrapper::ReachEndVarDeclList( ParseNode** outNode )
    {
        ParseNode* pnode = *outNode;
        // moving down to the last var declaration
        while( pnode->nop == knopList )
        {
            ParseNode* var = GetBinaryLeft( pnode );
            if (var->nop == knopVarDecl)
            {
                pnode = GetBinaryRight( pnode );
                continue;
            }
            else if (var->nop == knopVarDeclList || var->nop == knopList)
            {
                var = GetBinaryLeft( var );
                if (var->nop == knopVarDecl)
                {
                    pnode = GetBinaryRight( pnode );
                    continue;
                }
            }
            break;
        }
        *outNode = pnode;
    }

    AsmJsCompilationException::AsmJsCompilationException( const wchar_t* _msg, ... )
    {
        va_list arglist;
        va_start( arglist, _msg );
        vswprintf_s( msg_, _msg, arglist );
    }

    Var AsmJsChangeHeapBuffer(RecyclableObject * function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New));

        if (args.Info.Count < 1 || !ArrayBuffer::Is(args[1]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayBufferObject);
        }

        
        ArrayBuffer* newArrayBuffer = ArrayBuffer::FromVar(args[1]);
        if (newArrayBuffer->IsDetached() || newArrayBuffer->GetByteLength() & 0xffffff || newArrayBuffer->GetByteLength() <= 0xffffff || newArrayBuffer->GetByteLength() > 0x80000000)
        {
            return JavascriptBoolean::ToVar(FALSE, scriptContext);
        }
        FrameDisplay* frame = ((ScriptFunction*)function)->GetEnvironment();
        Var* moduleArrayBuffer = (Var*)frame->GetItem(0) + AsmJsModuleMemory::MemoryTableBeginOffset;
        *moduleArrayBuffer = newArrayBuffer;
        return JavascriptBoolean::ToVar(TRUE, scriptContext);
    }

#if _M_X64

#ifdef SIMD_JS_ENABLED
    
    // returns an array containing the size of each argument
    uint *GetArgsSizesArray(ScriptFunction* func)
    {
        AsmJsFunctionInfo* info = func->GetFunctionBody()->GetAsmJsFunctionInfo();
        return info->GetArgsSizesArray();
    }
    
#endif

    int GetStackSizeForAsmJsUnboxing(ScriptFunction* func)
    {
#ifdef SIMD_JS_ENABLED
        AsmJsFunctionInfo* info = func->GetFunctionBody()->GetAsmJsFunctionInfo();
        int argSize = MachPtr;
        for (uint i = 0; i < info->GetArgCount(); i++)
        {
            if (info->GetArgType(i).isSIMD())
            {
                argSize += sizeof(AsmJsSIMDValue);
            }
            else
            {
                argSize += MachPtr;
            }
        }
        argSize = ::Math::Align<int32>(argSize, 16);
        
#else
        int argSize = ::Math::Align<int32>((func->GetFunctionBody()->GetAsmJsFunctionInfo()->GetArgCount() + 1) * MachPtr, 16);
#endif

        if (argSize < 32)
        {
            argSize = 32; // convention is to always allocate spill space for rcx,rdx,r8,r9
        }
        PROBE_STACK_CALL(func->GetScriptContext(), func, argSize);
        return argSize;
    }

    void * UnboxAsmJsArguments(ScriptFunction* func, Var * origArgs, char * argDst, CallInfo callInfo)
    {
        void * address = func->GetEntryPointInfo()->address;
        Assert(address);
        AsmJsFunctionInfo* info = func->GetFunctionBody()->GetAsmJsFunctionInfo();
        ScriptContext* scriptContext = func->GetScriptContext();

        AsmJsModuleInfo::EnsureHeapAttached(func);

        uint actualArgCount = callInfo.Count - 1; // -1 for ScriptFunction
        argDst = argDst + MachPtr; // add one first so as to skip the ScriptFunction argument
        for (uint i = 0; i < info->GetArgCount(); i++)
        {
            
            if (info->GetArgType(i).isInt())
            {
                int32 intVal;
                if (i < actualArgCount)
                {
                    intVal = JavascriptMath::ToInt32(*origArgs, scriptContext);
                }
                else
                {
                    intVal = 0;
                }
                
                *(int64*)(argDst) = 0;
                *(int32*)argDst = intVal;
                
                argDst = argDst + MachPtr;
            }
            else if (info->GetArgType(i).isFloat())
            {
                float floatVal;
                if (i < actualArgCount)
                {
                    floatVal = (float)(JavascriptConversion::ToNumber(*origArgs, scriptContext));
                }
                else
                {
                    floatVal = (float)(JavascriptNumber::NaN);
                }
                *(int64*)(argDst) = 0;
                *(float*)argDst = floatVal;
                argDst = argDst + MachPtr;;
            }
            else if (info->GetArgType(i).isDouble())
            {
                double doubleVal;
                if (i < actualArgCount)
                {
                    doubleVal = JavascriptConversion::ToNumber(*origArgs, scriptContext);
                }
                else
                {
                    doubleVal = JavascriptNumber::NaN;
                }
                *(int64*)(argDst) = 0;
                *(double*)argDst = doubleVal;
                argDst = argDst + MachPtr;
            }
#ifdef SIMD_JS_ENABLED
            else if (SIMD_JS_FLAG && info->GetArgType(i).isSIMD())
            {
                AsmJsVarType argType = info->GetArgType(i);
                AsmJsSIMDValue simdVal = { 0, 0, 0, 0 };
                // SIMD values are copied unaligned.
                // SIMD values cannot be implicitly coerced from/to other types. If the SIMD parameter is missing (i.e. Undefined), we throw type error since there is not equivalent SIMD value to coerce to.
                switch (argType.which())
                {
                case AsmJsType::Int32x4:
                    if (!JavascriptSIMDInt32x4::Is(*origArgs))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"Int32x4");
                    }
                    simdVal = ((JavascriptSIMDInt32x4*)(*origArgs))->GetValue();
                    break;
                case AsmJsType::Float32x4:
                    if (!JavascriptSIMDFloat32x4::Is(*origArgs))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"Float32x4");
                    }
                    simdVal = ((JavascriptSIMDFloat32x4*)(*origArgs))->GetValue();
                    break;
                case AsmJsType::Float64x2:
                    if (!JavascriptSIMDFloat64x2::Is(*origArgs))
                    {
                        JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"Float64x2");
                    }
                    simdVal = ((JavascriptSIMDFloat64x2*)(*origArgs))->GetValue();
                    break;
                default:
                    Assert(UNREACHED);
                }
                *(AsmJsSIMDValue*)argDst = simdVal;
                argDst = argDst + sizeof(AsmJsSIMDValue); 
            }
#endif
            ++origArgs;
        }
        // for convenience, lets take the opportunity to return the asm.js entrypoint address
        return address;
    }

#ifdef SIMD_JS_ENABLED
    Var BoxAsmJsReturnValue(ScriptFunction* func, int intRetVal, double doubleRetVal, float floatRetVal, __m128 simdRetVal)
#else
    Var BoxAsmJsReturnValue(ScriptFunction* func, int intRetVal, double doubleRetVal, float floatRetVal)
#endif
    {
        // ExternalEntryPoint doesn't know the return value, so it will send garbage for everything except actual return type
        Var returnValue = nullptr;
        // make call and convert primitive type back to Var
        AsmJsFunctionInfo* info = func->GetFunctionBody()->GetAsmJsFunctionInfo();
        switch (info->GetReturnType().which())
        {
        case AsmJsRetType::Void:
            returnValue = JavascriptOperators::OP_LdUndef(func->GetScriptContext());
            break;
        case AsmJsRetType::Signed:{
            returnValue = JavascriptNumber::ToVar(intRetVal, func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Double:{
            returnValue = JavascriptNumber::New(doubleRetVal, func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Float:{
            returnValue = JavascriptNumber::New(floatRetVal, func->GetScriptContext());
            break;
#ifdef SIMD_JS_ENABLED
        case AsmJsRetType::Float32x4:
        {
            X86SIMDValue simdVal;
            simdVal.m128_value = simdRetVal;
            returnValue = JavascriptSIMDFloat32x4::New(&X86SIMDValue::ToSIMDValue(simdVal), func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Int32x4:
        {
            X86SIMDValue simdVal;
            simdVal.m128_value = simdRetVal;
            returnValue = JavascriptSIMDInt32x4::New(&X86SIMDValue::ToSIMDValue(simdVal), func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Float64x2:
        {
            X86SIMDValue simdVal;
            simdVal.m128_value = simdRetVal;
            returnValue = JavascriptSIMDFloat64x2::New(&X86SIMDValue::ToSIMDValue(simdVal), func->GetScriptContext());
            break;
        }
#endif
        }
        default:
            Assume(UNREACHED);
        }

        return returnValue;
    }

#elif _M_IX86
    Var AsmJsExternalEntryPoint(RecyclableObject* entryObject, CallInfo callInfo, ...)
    {
        ScriptFunction* func = (ScriptFunction*)entryObject;
        FunctionBody* body = func->GetFunctionBody();
        AsmJsFunctionInfo* info = body->GetAsmJsFunctionInfo();
        ScriptContext* scriptContext = func->GetScriptContext();
        const int argOffset = 2 * sizeof(void*) + sizeof(Js::RecyclableObject*) + sizeof(Js::CallInfo) + sizeof(Var);
        const uint argInCount = callInfo.Count - 1;
        int argSize = info->GetArgByteSize();
        Var* args;
        char* dst;
        Var returnValue = 0;

        AsmJsModuleInfo::EnsureHeapAttached(func);

        argSize = ::Math::Align<int32>(argSize, 8);
        // Allocate stack space for args

        __asm
        {
            mov eax, ebp
            add eax, argOffset
            mov args, eax
            sub esp, argSize
            mov dst, esp
        };

        // Unbox Var to primitive type
        {
            int32 intVal; double doubleVal; float floatVal;
            for (uint i = 0; i < info->GetArgCount(); i++)
            {
                if (info->GetArgType(i).isInt())
                {
                    if (i < argInCount)
                    {
                        intVal = JavascriptMath::ToInt32(*args, scriptContext);
                    }
                    else
                    {
                        intVal = 0;
                    }
                    *(int32*)dst = intVal;
                    dst += sizeof(int32);
                }
                else if (info->GetArgType(i).isFloat())
                {
                    if (i < argInCount)
                    {
                        floatVal = (float)(JavascriptConversion::ToNumber(*args, scriptContext));
                    }
                    else
                    {
                        floatVal = (float)(JavascriptNumber::NaN);
                    }
                    *(float*)dst = floatVal;
                    dst += sizeof(float);
                }
                else if (info->GetArgType(i).isDouble())
                {
                    if (i < argInCount)
                    {
                        doubleVal = JavascriptConversion::ToNumber(*args, scriptContext);
                    }
                    else
                    {
                        doubleVal = JavascriptNumber::NaN;
                    }
                    *(double*)dst = doubleVal;
                    dst += sizeof(double);
                }
#ifdef SIMD_JS_ENABLED
                else if (SIMD_JS_FLAG && info->GetArgType(i).isSIMD())
                {
                    AsmJsVarType argType = info->GetArgType(i);
                    AsmJsSIMDValue simdVal;
                    // SIMD values are copied unaligned.
                    // SIMD values cannot be implicitly coerced from/to other types. If the SIMD parameter is missing (i.e. Undefined), we throw type error since there is not equivalent SIMD value to coerce to.
                    switch (argType.which())
                    {
                    case AsmJsType::Int32x4:
                        if (!JavascriptSIMDInt32x4::Is(*args))
                        {
                            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInt32x4TypeMismatch, L"Int32x4");
                        }
                        simdVal = ((JavascriptSIMDInt32x4*)(*args))->GetValue();
                        break;
                    case AsmJsType::Float32x4:
                        if (!JavascriptSIMDFloat32x4::Is(*args))
                        {
                            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat32x4TypeMismatch, L"Float32x4");
                        }
                        simdVal = ((JavascriptSIMDFloat32x4*)(*args))->GetValue();
                        break;
                    case AsmJsType::Float64x2:
                        if (!JavascriptSIMDFloat64x2::Is(*args))
                        {
                            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdFloat64x2TypeMismatch, L"Float64x2");
                        }
                        simdVal = ((JavascriptSIMDFloat64x2*)(*args))->GetValue();
                        break;
                    default:
                        Assert(UNREACHED);
                    }
                    *(AsmJsSIMDValue*)dst = simdVal;
                    dst += sizeof(AsmJsSIMDValue);
                }
#endif
                else
                {
                    AssertMsg(UNREACHED, "Invalid function arg type.");
                }
                ++args;
            }
        }

        const void * asmJSEntryPoint = func->GetEntryPointInfo()->address;
        // make call and convert primitive type back to Var
        switch (info->GetReturnType().which())
        {
        case AsmJsRetType::Void:
            __asm
            {
#ifdef _CONTROL_FLOW_GUARD
                mov  ecx, asmJSEntryPoint
                call[__guard_check_icall_fptr]
#endif
                push func
                call ecx
            }
            returnValue = JavascriptOperators::OP_LdUndef(func->GetScriptContext());
            break;
        case AsmJsRetType::Signed:{
            int32 ival = 0;
            __asm
            {
#ifdef _CONTROL_FLOW_GUARD
                mov  ecx, asmJSEntryPoint
                call[__guard_check_icall_fptr]
#endif
                push func
                call ecx
                mov ival, eax
            }
            returnValue = JavascriptNumber::ToVar(ival, func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Double:{
            double dval = 0;
            __asm
            {
#ifdef _CONTROL_FLOW_GUARD
                mov  ecx, asmJSEntryPoint
                call[__guard_check_icall_fptr]
#endif
                push func
                call ecx
                movsd dval, xmm0
            }
            returnValue = JavascriptNumber::New(dval, func->GetScriptContext());
            break;
        }
        case AsmJsRetType::Float:{
            float fval = 0;
            __asm
            {
#ifdef _CONTROL_FLOW_GUARD
                mov  ecx, asmJSEntryPoint
                call[__guard_check_icall_fptr]
#endif
                push func
                call ecx
                movss fval, xmm0
            }
            returnValue = JavascriptNumber::New((double)fval, func->GetScriptContext());
            break;
        }
#ifdef SIMD_JS_ENABLED
        case AsmJsRetType::Int32x4:
            AsmJsSIMDValue simdVal;
            simdVal.Zero();
            if (SIMD_JS_FLAG)
            {
                __asm
                {
                    push func
                        call asmJSEntryPoint
                        movups simdVal, xmm0
                }
                returnValue = JavascriptSIMDInt32x4::New(&simdVal, func->GetScriptContext());
                break;
            }
            Assert(UNREACHED);
        case AsmJsRetType::Float32x4:
            simdVal.Zero();
            if (SIMD_JS_FLAG)
            {
                __asm
                {
                    push func
                        call asmJSEntryPoint
                        movups simdVal, xmm0
                }
                returnValue = JavascriptSIMDFloat32x4::New(&simdVal, func->GetScriptContext());
                break;
            }
            Assert(UNREACHED);
        case AsmJsRetType::Float64x2:
            simdVal.Zero();
            if (SIMD_JS_FLAG)
            {
                __asm
                {
                    push func
                        call asmJSEntryPoint
                        movups simdVal, xmm0
                }
                returnValue = JavascriptSIMDFloat64x2::New(&simdVal, func->GetScriptContext());
                break;
            }
            Assert(UNREACHED);
#endif
        default:
            Assume(UNREACHED);
        }
        return returnValue;
    }
#endif

}
