//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    extern const __declspec(selectany) uint32 TypedArrayViewMask[] =
    {
        (uint32)~0 //TYPE_INT8 
        , (uint32)~0 //TYPE_UINT8
        , (uint32)~1 //TYPE_INT16
        , (uint32)~1 //TYPE_UINT16
        , (uint32)~3 //TYPE_INT32
        , (uint32)~3 //TYPE_UINT32
        , (uint32)~3 //TYPE_FLOAT32
        , (uint32)~7 //TYPE_FLOAT64
    };

    typedef void(InterpreterStackFrame::*ArrFunc)(uint32, RegSlot);

    __declspec(selectany) const ArrFunc InterpreterStackFrame::StArrFunc[8] =
    {
        &InterpreterStackFrame::OP_StArr<int8>,
        &InterpreterStackFrame::OP_StArr<uint8>,
        &InterpreterStackFrame::OP_StArr<int16>,
        &InterpreterStackFrame::OP_StArr<uint16>,
        &InterpreterStackFrame::OP_StArr<int32>,
        &InterpreterStackFrame::OP_StArr<uint32>,
        &InterpreterStackFrame::OP_StArr<float>,
        &InterpreterStackFrame::OP_StArr<double>,
    };

    __declspec(selectany) const ArrFunc InterpreterStackFrame::LdArrFunc[8] =
    {
        &InterpreterStackFrame::OP_LdArr<int8>,
        &InterpreterStackFrame::OP_LdArr<uint8>,
        &InterpreterStackFrame::OP_LdArr<int16>,
        &InterpreterStackFrame::OP_LdArr<uint16>,
        &InterpreterStackFrame::OP_LdArr<int32>,
        &InterpreterStackFrame::OP_LdArr<uint32>,
        &InterpreterStackFrame::OP_LdArr<float>,
        &InterpreterStackFrame::OP_LdArr<double>,
    };

    inline const Var Arguments::operator [](int idxArg) const
    {
        AssertMsg((idxArg < (int)Info.Count) && (idxArg >= 0), "Ensure a valid argument index");

        return Values[idxArg];
    }

    inline void InterpreterStackFrame::ValidateRegValue(Var value, bool allowStackVar, bool allowStackVarOnDisabledStackNestedFunc) const
    {
#if DBG
        if (value != null && !TaggedNumber::Is(value))
        {
            if (!allowStackVar || !this->m_functionBody->DoStackNestedFunc())
            {
                Assert(!ThreadContext::IsOnStack(value)
                    || (allowStackVar && allowStackVarOnDisabledStackNestedFunc && StackScriptFunction::IsBoxed(value)));
            }
            Assert(!CrossSite::NeedMarshalVar(value, GetScriptContext()));
        }
#endif
    }
    template <typename RegSlotType>
    inline Var InterpreterStackFrame::GetReg(RegSlotType localRegisterID) const
    {
        Var value = m_localSlots[localRegisterID];
        ValidateRegValue(value);
        return value;
    }

    template <typename RegSlotType>
    inline void InterpreterStackFrame::SetReg(RegSlotType localRegisterID, Var value)
    {
        Assert(localRegisterID == 0 || localRegisterID >= m_functionBody->GetConstantCount());
        ValidateRegValue(value);
        m_localSlots[localRegisterID] = value;
    }

    template <typename T>
    inline T InterpreterStackFrame::GetRegRaw( RegSlot localRegisterID ) const
    {
        return (T)m_localIntSlots[localRegisterID];
    }

    // specialized version for doubles
    template <>
    inline double InterpreterStackFrame::GetRegRaw(RegSlot localRegisterID) const
    {
        return (double)m_localDoubleSlots[localRegisterID];
    }
    
    template <>
    inline float InterpreterStackFrame::GetRegRaw(RegSlot localRegisterID) const
    {
        return (float)m_localFloatSlots[localRegisterID];
    }

    template <typename T> 
    inline void InterpreterStackFrame::SetRegRaw(RegSlot localRegisterID, T bValue)
    {
        m_localIntSlots[localRegisterID] = (int)bValue;
    }

    template <>
    inline void InterpreterStackFrame::SetRegRaw(RegSlot localRegisterID, float bValue)
    {
        m_localFloatSlots[localRegisterID] = (float)bValue;
    }

    template <>
    inline void InterpreterStackFrame::SetRegRaw(RegSlot localRegisterID, double bValue)
    {
        m_localDoubleSlots[localRegisterID] = bValue;
    }
    
    template <typename RegSlotType> 
    inline int InterpreterStackFrame::GetRegRawInt( RegSlotType localRegisterID ) const
    {
        return m_localIntSlots[localRegisterID];
    }
    template <typename RegSlotType>
    inline double InterpreterStackFrame::GetRegRawDouble( RegSlotType localRegisterID ) const
    {
        return m_localDoubleSlots[localRegisterID];
    }

    template <typename RegSlotType>
    inline float InterpreterStackFrame::GetRegRawFloat(RegSlotType localRegisterID) const
    {
        return m_localFloatSlots[localRegisterID];
    }

    template <typename RegSlotType> 
    inline void InterpreterStackFrame::SetRegRawInt( RegSlotType localRegisterID, int bValue )
    {
        m_localIntSlots[localRegisterID] = bValue;
    }

    template <typename RegSlotType>
    inline void InterpreterStackFrame::SetRegRawDouble( RegSlotType localRegisterID, double bValue )
    {
        m_localDoubleSlots[localRegisterID] = bValue;
    }

    template <typename RegSlotType>
    inline void InterpreterStackFrame::SetRegRawFloat(RegSlotType localRegisterID, float bValue)
    {
        m_localFloatSlots[localRegisterID] = bValue;
    }

    template <typename RegSlotType>
    inline Var InterpreterStackFrame::GetRegAllowStackVar(RegSlotType localRegisterID) const
    {
        Var value = m_localSlots[localRegisterID];
        ValidateRegValue(value, true);
        return value;
    }

    template <typename RegSlotType>
    inline void InterpreterStackFrame::SetRegAllowStackVar(RegSlotType localRegisterID, Var value)
    {
        Assert(localRegisterID == 0 || localRegisterID >= m_functionBody->GetConstantCount());
        ValidateRegValue(value, true);
        m_localSlots[localRegisterID] = value;
    }

    template <typename RegSlotType>
    inline Var InterpreterStackFrame::GetRegAllowStackVarEnableOnly(RegSlotType localRegisterID) const
    {
        Var value = m_localSlots[localRegisterID];
        ValidateRegValue(value, true, false);
        return value;
    }

    template <typename RegSlotType>
    inline void InterpreterStackFrame::SetRegAllowStackVarEnableOnly(RegSlotType localRegisterID, Var value)
    {
        Assert(localRegisterID == 0 || localRegisterID >= m_functionBody->GetConstantCount());
        ValidateRegValue(value, true, false);
        m_localSlots[localRegisterID] = value;
    }
#ifdef SIMD_JS_ENABLED

    template <>
    inline AsmJsSIMDValue InterpreterStackFrame::GetRegRaw(RegSlot localRegisterID) const
    {
        return (AsmJsSIMDValue)m_localSimdSlots[localRegisterID];
    }

    template<>
    inline void InterpreterStackFrame::SetRegRaw(RegSlot localRegisterID, AsmJsSIMDValue bValue)
    {
        m_localSimdSlots[localRegisterID] = bValue;
    }

    template <typename RegSlotType> 
    inline AsmJsSIMDValue InterpreterStackFrame::GetRegRawSimd(RegSlotType localRegisterID) const
    {
        return m_localSimdSlots[localRegisterID];
    }

    template <typename RegSlotType> 
    inline void InterpreterStackFrame::SetRegRawSimd(RegSlotType localRegisterID, AsmJsSIMDValue bValue)
    {
        m_localSimdSlots[localRegisterID] = bValue;
    }
#endif
    inline Var InterpreterStackFrame::GetNonVarReg(RegSlot localRegisterID) const
    {
        return m_localSlots[localRegisterID];
    }

    inline void InterpreterStackFrame::SetNonVarReg(RegSlot localRegisterID, Var aValue)
    {
        m_localSlots[localRegisterID] = aValue;
    }

    inline Var InterpreterStackFrame::GetRootObject() const
    {
        Var rootObject = GetReg(Js::FunctionBody::RootObjectRegSlot);
        Assert(rootObject == this->GetFunctionBody()->LoadRootObject());
        return rootObject;
    }
    inline Var InterpreterStackFrame::OP_ArgIn0()
    {
        return m_inParams[0];
    }

    template <class T>
    inline void InterpreterStackFrame::OP_ProfiledArgOut_A(const unaligned T * playout)
    {
        FunctionBody* functionBody = this->m_functionBody;
        DynamicProfileInfo * dynamicProfileInfo = functionBody->GetDynamicProfileInfo();

        Assert(playout->Reg > FunctionBody::FirstRegSlot && playout->Reg < functionBody->GetConstantCount());
        Var value = GetReg(playout->Reg);
        if (value != nullptr && TaggedInt::Is(value))
        {
            dynamicProfileInfo->RecordConstParameterAtCallSite(playout->profileId, playout->Arg);
        }
        SetOut(playout->Arg, GetReg(playout->Reg));
    }
    template <class T>
    inline void InterpreterStackFrame::OP_ArgOut_A(const unaligned T * playout)
    {
        SetOut( playout->Arg, GetReg(playout->Reg));
    }
#if DBG
    template <class T>
    inline void InterpreterStackFrame::OP_ArgOut_ANonVar(const unaligned T * playout)
    {
        SetOut( playout->Arg, GetNonVarReg(playout->Reg));
    }
#endif
    inline BOOL InterpreterStackFrame::OP_BrFalse_A(Var aValue, ScriptContext* scriptContext)
    {
        return !JavascriptConversion::ToBoolean(aValue, scriptContext);
    }

    inline BOOL InterpreterStackFrame::OP_BrTrue_A(Var aValue, ScriptContext* scriptContext)
    {
        return JavascriptConversion::ToBoolean(aValue, scriptContext);
    }

    inline BOOL InterpreterStackFrame::OP_BrNotNull_A(Var aValue)
    {
        return aValue != NULL;
    }

    inline BOOL InterpreterStackFrame::OP_BrOnHasProperty(Var argInstance, uint propertyIdIndex, ScriptContext* scriptContext)
    {
        return JavascriptOperators::OP_HasProperty(argInstance,
            this->m_functionBody->GetReferencedPropertyId(propertyIdIndex), scriptContext);
    }

    inline BOOL InterpreterStackFrame::OP_BrOnNoProperty(Var argInstance, uint propertyIdIndex, ScriptContext* scriptContext)
    {
        return !JavascriptOperators::OP_HasProperty(argInstance,
            this->m_functionBody->GetReferencedPropertyId(propertyIdIndex), scriptContext);
    }

    template<class T>
    void InterpreterStackFrame::OP_LdLen(const unaligned T * const playout)
    {
        Assert(playout);

        ThreadContext* threadContext = this->GetScriptContext()->GetThreadContext();
        ImplicitCallFlags savedImplicitCallFlags = threadContext->GetImplicitCallFlags();
        threadContext->ClearImplicitCallFlags();

        const auto instance = GetReg(playout->R1);
        Var length = JavascriptOperators::OP_GetLength(instance, GetScriptContext());

        threadContext->CheckAndResetImplicitCallAccessorFlag();
        threadContext->AddImplicitCallFlags(savedImplicitCallFlags);

        SetReg(playout->R0, length);
    }

    template<class T>
    void InterpreterStackFrame::OP_ProfiledLdLen(const unaligned OpLayoutDynamicProfile<T> *const playout)
    {
        Assert(playout);

        const auto functionBody = m_functionBody;
        const auto profileData = functionBody->GetDynamicProfileInfo();

        const auto instance = GetReg(playout->R1);
        LdElemInfo ldElemInfo;
        ldElemInfo.arrayType = ValueType::Uninitialized.Merge(instance);

        ThreadContext* threadContext = this->GetScriptContext()->GetThreadContext();
        ImplicitCallFlags savedImplicitCallFlags = threadContext->GetImplicitCallFlags();
        threadContext->ClearImplicitCallFlags();

        Var length = JavascriptOperators::OP_GetLength(instance, GetScriptContext());

        threadContext->CheckAndResetImplicitCallAccessorFlag();
        threadContext->AddImplicitCallFlags(savedImplicitCallFlags);

        ldElemInfo.elemType = ldElemInfo.elemType.Merge(length);
        profileData->RecordElementLoad(functionBody, playout->profileId, ldElemInfo);

        SetReg(playout->R0, length);
    }

    template <class T>
    inline void InterpreterStackFrame::OP_LdFunctionExpression(const unaligned T * playout)
    {
        // Make sure we get the boxed function object if is there, (or the function itself)
        SetRegAllowStackVar(playout->R0, StackScriptFunction::GetCurrentFunctionObject(this->function->GetRealFunctionObject()));
    }

    template <class T>
    inline void InterpreterStackFrame::OP_StFunctionExpression(const unaligned T * playout)
    {
        Var instance = GetReg(playout->Instance);

        JavascriptOperators::OP_StFunctionExpression(instance,
            this->m_functionBody->GetReferencedPropertyId(playout->PropertyIdIndex), GetReg(playout->Value));
    }

    inline Var InterpreterStackFrame::OP_Ld_A(Var aValue)
    {
        return aValue;
    }

    inline Var InterpreterStackFrame::OP_LdEnv()
    {
        return this->function->GetEnvironment();
    }

    template <typename T2>
    inline void InterpreterStackFrame::OP_StArr(  uint32 index, RegSlot value  )
    {
        JavascriptArrayBuffer* arr = *(JavascriptArrayBuffer**)GetNonVarReg( AsmJsFunctionMemory::ArrayBufferRegister );
        if( index < ( arr->GetByteLength() ) )
        {
            BYTE* buffer = arr->GetBuffer();
            *(T2*)(buffer+index) = (T2)GetRegRaw<T2>( value );
        }
    }

    template<> inline double InterpreterStackFrame::GetArrayViewOverflowVal()
    {
        return *(double*)&NumberConstants::k_Nan;
    }

    template<> inline float InterpreterStackFrame::GetArrayViewOverflowVal()
    {
        return (float)*(double*)&NumberConstants::k_Nan;
    }

    template<typename T> T InterpreterStackFrame::GetArrayViewOverflowVal()
    {
        return 0;
    }

    template <class T> 
    inline void InterpreterStackFrame::OP_LdArrFunc( const unaligned T* playout )
    {
        Var* arr = (Var*)GetNonVarReg( playout->Instance );
        const uint32 index = (uint32)GetRegRawInt( playout->SlotIndex );
        m_localSlots[playout->Value] = arr[index];
    }

    template <typename T2>
    inline void InterpreterStackFrame::OP_LdArr( uint32 index, RegSlot value )
    {
        JavascriptArrayBuffer* arr = *(JavascriptArrayBuffer**)GetNonVarReg( AsmJsFunctionMemory::ArrayBufferRegister );
        BYTE* buffer = arr->GetBuffer();
        T2 val = index < ( arr->GetByteLength() ) ? *(T2*)(buffer+index) : GetArrayViewOverflowVal<T2>();
        SetRegRaw<T2>( value, val );
    }

    template <class T, typename T2>
    inline void InterpreterStackFrame::OP_StSlotPrimitive( const unaligned T* playout )
    {
        T2* buffer = (T2*)GetNonVarReg( playout->Instance );
        buffer[playout->SlotIndex] = GetRegRaw<T2>( playout->Value );
    }

    template <class T, typename T2> 
    inline void InterpreterStackFrame::OP_LdSlotPrimitive( const unaligned T* playout )
    {
        T2* buffer = (T2*)GetNonVarReg( playout->Instance );
        SetRegRaw<T2>( playout->Value, buffer[playout->SlotIndex] );
    }
    
    template <class T>
    inline void InterpreterStackFrame::OP_LdArrGeneric( const unaligned T* playout )
    {
        Assert( playout->ViewType < 8 );
        const uint32 index = (uint32)GetRegRawInt( playout->SlotIndex ) & TypedArrayViewMask[playout->ViewType];
        (this->*LdArrFunc[playout->ViewType])( index, playout->Value );
    }
    template <class T> 
    inline void InterpreterStackFrame::OP_LdArrConstIndex( const unaligned T* playout )
    {
        const uint32 index = playout->SlotIndex;
        Assert( playout->ViewType < 8 );
        (this->*LdArrFunc[playout->ViewType])( index, playout->Value );
    }
    template <class T> 
    inline void InterpreterStackFrame::OP_StArrGeneric( const unaligned T* playout )
    {
        Assert( playout->ViewType < 8 );
        const uint32 index = (uint32)GetRegRawInt( playout->SlotIndex ) & TypedArrayViewMask[playout->ViewType];
        (this->*StArrFunc[playout->ViewType])( index, playout->Value );
    }
    template <class T> 
    inline void InterpreterStackFrame::OP_StArrConstIndex( const unaligned T* playout )
    {
        const uint32 index = playout->SlotIndex;
        Assert( playout->ViewType < 8 );
        (this->*StArrFunc[playout->ViewType])( index, playout->Value );
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_LdSlot(Var instance, const unaligned T* playout)
    {
        return ((Var*)(instance))[playout->SlotIndex];
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_ProfiledLdSlot(Var instance, const unaligned T* playout)
    {
        Var value = ((Var*)(instance))[playout->SlotIndex];
        ProfilingHelpers::ProfileLdSlot(value, GetFunctionBody(), playout->profileId);
        return value;
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_LdSlotChkUndecl(Var instance, const unaligned T* playout)
    {
        Var value = OP_LdSlot(instance, playout);
        OP_ChkUndecl(value);
        return value;
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_ProfiledLdSlotChkUndecl(Var instance, const unaligned T* playout)
    {
        Var value = OP_LdSlotChkUndecl(instance, playout);
        ProfilingHelpers::ProfileLdSlot(value, GetFunctionBody(), playout->profileId);
        return value;
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_LdObjSlot(Var instance, const unaligned T* playout)
    {
        Var *slotArray = *(Var**)((char*)instance + DynamicObject::GetOffsetOfAuxSlots());
        return slotArray[playout->SlotIndex];
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_ProfiledLdObjSlot(Var instance, const unaligned T* playout)
    {
        Var *slotArray = *(Var**)((char*)instance + DynamicObject::GetOffsetOfAuxSlots());
        Var value = slotArray[playout->SlotIndex];
        ProfilingHelpers::ProfileLdSlot(value, GetFunctionBody(), playout->profileId);
        return value;
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_LdObjSlotChkUndecl(Var instance, const unaligned T* playout)
    {
        Var value = OP_LdObjSlot(instance, playout);
        OP_ChkUndecl(value);
        return value;
    }

    template <class T>
    inline Var InterpreterStackFrame::OP_ProfiledLdObjSlotChkUndecl(Var instance, const unaligned T* playout)
    {
        Var value = OP_LdObjSlotChkUndecl(instance, playout);
        ProfilingHelpers::ProfileLdSlot(value, GetFunctionBody(), playout->profileId);
        return value;
    }

    inline void InterpreterStackFrame::OP_StSlot(Var instance, int32 slotIndex, Var value)
    {
        // We emit OpCode::StSlot in the bytecode only for scope slot arrays, which are not recyclable objects.
        ((Var*)(instance))[slotIndex] = value;
    }

    inline void InterpreterStackFrame::OP_StSlotChkUndecl(Var instance, int32 slotIndex, Var value)
    {
        // We emit OpCode::StSlot in the bytecode only for scope slot arrays, which are not recyclable objects.
        OP_ChkUndecl(((Var*)(instance))[slotIndex]);
        ((Var*)(instance))[slotIndex] = value;
    }

    inline void InterpreterStackFrame::OP_StObjSlot(Var instance, int32 slotIndex, Var value)
    {
        // It would be nice to assert that it's ok to store directly to slot, but we don't have the propertyId.
        Var *slotArray = *(Var**)((char*)instance + DynamicObject::GetOffsetOfAuxSlots());
        slotArray[slotIndex] = value;
    }

    inline void InterpreterStackFrame::OP_StObjSlotChkUndecl(Var instance, int32 slotIndex, Var value)
    {
        // It would be nice to assert that it's ok to store directly to slot, but we don't have the propertyId.
        Var *slotArray = *(Var**)((char*)instance + DynamicObject::GetOffsetOfAuxSlots());
        OP_ChkUndecl(slotArray[slotIndex]);
        slotArray[slotIndex] = value;
    }

    inline Var InterpreterStackFrame::OP_LdStackArgPtr(void)
    {
        // Return the address of the first param after "this".
        return m_inParams + 1;
    }

    // Called for the debug purpose, to create the arguments object explicitly even though script has not declared it.
    inline Var InterpreterStackFrame::CreateHeapArguments(ScriptContext* scriptContext)
    {
        return JavascriptOperators::LoadHeapArguments(this->function->GetRealFunctionObject(), this->m_inSlotsCount - 1, &this->m_inParams[1], scriptContext->GetLibrary()->GetNull(), (PropertyId*)scriptContext->GetLibrary()->GetNull(), scriptContext, false);
    }

    inline Var InterpreterStackFrame::OP_LdHeapArguments(Var frameObj, Var argsArray, ScriptContext* scriptContext)
    {
        Var args = JavascriptOperators::LoadHeapArguments(this->function->GetRealFunctionObject(), this->m_inSlotsCount - 1, &this->m_inParams[1], frameObj, (PropertyId*)argsArray, scriptContext, false);
        this->m_arguments = args;
        return args;
    }

    inline Var InterpreterStackFrame::OP_LdLetHeapArguments(Var frameObj, Var argsArray, ScriptContext* scriptContext)
    {
        Var args = JavascriptOperators::LoadHeapArguments(this->function->GetRealFunctionObject(), this->m_inSlotsCount - 1, &this->m_inParams[1], frameObj, (PropertyId*)argsArray, scriptContext, true);
        this->m_arguments = args;
        return args;
    }

    inline Var InterpreterStackFrame::OP_LdHeapArgsCached(Var frameObj, ScriptContext* scriptContext)
    {
        uint32 formalsCount = this->m_functionBody->GetInParamsCount() - 1;
        Var args = JavascriptOperators::LoadHeapArgsCached(this->function->GetRealFunctionObject(), this->m_inSlotsCount - 1, formalsCount, &this->m_inParams[1], frameObj, scriptContext, false);
        this->m_arguments = args;
        return args;
    }

    inline Var InterpreterStackFrame::OP_LdLetHeapArgsCached(Var frameObj, ScriptContext* scriptContext)
    {
        uint32 formalsCount = this->m_functionBody->GetInParamsCount() - 1;
        Var args = JavascriptOperators::LoadHeapArgsCached(this->function->GetRealFunctionObject(), this->m_inSlotsCount - 1, formalsCount, &this->m_inParams[1], frameObj, scriptContext, true);
        this->m_arguments = args;
        return args;
    }

    inline Var InterpreterStackFrame::OP_LdArgumentsFromFrame()
    {
        return this->m_arguments;
    }

    inline void* InterpreterStackFrame::OP_LdArgCnt()
    {
      return (void*)m_inSlotsCount;
    }

    inline Var InterpreterStackFrame::OP_ResumeYield(Var yieldDataVar, RegSlot yieldStarIterator)
    {
        ResumeYieldData* yieldData = static_cast<ResumeYieldData*>(yieldDataVar);
        RecyclableObject* iterator = yieldStarIterator != Constants::NoRegister ? RecyclableObject::FromVar(GetNonVarReg(yieldStarIterator)) : nullptr;

        return JavascriptOperators::OP_ResumeYield(yieldData, iterator);
    }

    inline void* InterpreterStackFrame::operator new(size_t byteSize, void* previousAllocation)
    {
        //
        // Placement 'new' is used by InterpreterStackFrame to initialize the C++ object on the RcInterpreter's
        // program stack:
        // - Unlike most other allocations, the previously allocated memory will __not__ be
        //   zero-initialized, as we do not want the overhead of zero-initializing the frame when
        //   calling functions.
        //
        // NOTE: If we wanted to add C# semantics of all locals are automatically zero-initialized,
        // need to determine the most efficient mechanism for this.
        //

        return previousAllocation;
    }

    inline void __cdecl InterpreterStackFrame::operator delete(void * allocationToFree, void * previousAllocation)
    {
        AssertMsg(allocationToFree == previousAllocation, "Memory locations should match");
        AssertMsg(false, "This function should never actually be called");
    }
} // namespace Js
