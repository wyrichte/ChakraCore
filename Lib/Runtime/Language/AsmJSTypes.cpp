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
#if DBG_DUMP
    template<> void PrintTmpRegisterAllocation<double>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"+D%d\n", loc);
    }
    template<> void PrintTmpRegisterDeAllocation<double>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"-D%d\n", loc);
    }
    template<> void PrintTmpRegisterAllocation<float>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"+F%d\n", loc);
    }
    template<> void PrintTmpRegisterDeAllocation<float>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"-F%d\n", loc);
    }
    template<> void PrintTmpRegisterAllocation<int>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"+I%d\n", loc);
    }
    template<> void PrintTmpRegisterDeAllocation<int>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"-I%d\n", loc);
    }

#ifdef SIMD_JS_ENABLED
    template<> void PrintTmpRegisterAllocation<AsmJsSIMDValue>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"+SIMD%d\n", loc);
    }

    template<> void PrintTmpRegisterDeAllocation<AsmJsSIMDValue>(RegSlot loc)
    {
        if (PHASE_ON1(AsmjsTmpRegisterAllocationPhase))
            Output::Print(L"-SIMD%d\n", loc);
    }
#endif
    template<typename T> void PrintTmpRegisterAllocation(RegSlot loc) {}
    template<typename T> void PrintTmpRegisterDeAllocation(RegSlot loc) {}
#endif

    const wchar_t * AsmJsType::toChars() const
    {
        switch (which_)
        {
        case Double:      return L"double";
        case MaybeDouble: return L"double?";
        case DoubleLit:   return L"doublelit";
        case Float:       return L"float";
        case Floatish:    return L"floatish";
        case FloatishDoubleLit: return L"FloatishDoubleLit";
        case MaybeFloat:  return L"float?";
        case Fixnum:      return L"fixnum";
        case Int:         return L"int";
        case Signed:      return L"signed";
        case Unsigned:    return L"unsigned";
        case Intish:      return L"intish";
        case Void:        return L"void";
#ifdef SIMD_JS_ENABLED
        case Int32x4:     return L"SIMD.Int32x4";
        case Float32x4:   return L"SIMD.Float32x4";
        case Float64x2:   return L"SIMD.Float64x2";
#endif

        }
        Assert(false);
        return L"none";
    }

#ifdef SIMD_JS_ENABLED
    bool AsmJsType::isSIMDType() const
    {
        return isSIMDInt32x4() || isSIMDFloat32x4() || isSIMDFloat64x2();
    }

    bool AsmJsType::isSIMDInt32x4() const
    {
        return which_ == Int32x4;
    }
    bool AsmJsType::isSIMDFloat32x4() const
    {
        return which_ == Float32x4;
    }
    bool AsmJsType::isSIMDFloat64x2() const
    {
        return which_ == Float64x2;
    }
#endif

    bool AsmJsType::isVarAsmJsType() const
    {
        return isInt() || isMaybeDouble() || isMaybeFloat();
    }

    bool AsmJsType::isExtern() const
    {
        return isDouble() || isSigned();
    }

    bool AsmJsType::isVoid() const
    {
        return which_ == Void;
    }

    bool AsmJsType::isFloatish() const
    {
        return isMaybeFloat() || which_ == Floatish;
    }

    bool AsmJsType::isFloatishDoubleLit() const
    {
        return isFloatish() || isDoubleLit();
    }

    bool AsmJsType::isMaybeFloat() const
    {
        return isFloat() || which_ == MaybeFloat;
    }

    bool AsmJsType::isFloat() const
    {
        return which_ == Float;
    }

    bool AsmJsType::isMaybeDouble() const
    {
        return isDouble() || which_ == MaybeDouble;
    }

    bool AsmJsType::isDouble() const
    {
        return isDoubleLit() || which_ == Double;
    }

    bool AsmJsType::isDoubleLit() const
    {
        return which_ == DoubleLit;
    }

    bool AsmJsType::isIntish() const
    {
        return isInt() || which_ == Intish;
    }

    bool AsmJsType::isInt() const
    {
        return isSigned() || isUnsigned() || which_ == Int;
    }

    bool AsmJsType::isUnsigned() const
    {
        return which_ == Unsigned || which_ == Fixnum;
    }

    bool AsmJsType::isSigned() const
    {
        return which_ == Signed || which_ == Fixnum;
    }

    bool AsmJsType::operator!=(AsmJsType rhs) const
    {
        return which_ != rhs.which_;
    }

    bool AsmJsType::operator==(AsmJsType rhs) const
    {
        return which_ == rhs.which_;
    }

    bool AsmJsType::isSubType(AsmJsType type) const
    {
        switch (type.which_)
        {
        case Js::AsmJsType::Double:
            return isDouble(); 
            break;

        case Js::AsmJsType::MaybeDouble:
            return isMaybeDouble();
            break;
        case Js::AsmJsType::DoubleLit:
            return isDoubleLit();
            break;
        case Js::AsmJsType::Float:
            return isFloat();
            break;
        case Js::AsmJsType::MaybeFloat:
            return isMaybeFloat();
            break;
        case Js::AsmJsType::Floatish:
            return isFloatish();
            break;
        case Js::AsmJsType::FloatishDoubleLit:
            return isFloatishDoubleLit();
            break;
        case Js::AsmJsType::Fixnum:
            return which_ == Fixnum;
            break;
        case Js::AsmJsType::Int:
            return isInt();
            break;
        case Js::AsmJsType::Signed:
            return isSigned();
            break;
        case Js::AsmJsType::Unsigned:
            return isUnsigned();
            break;
        case Js::AsmJsType::Intish:
            return isIntish();
            break;
        case Js::AsmJsType::Void:
            return isVoid();
            break;
#ifdef SIMD_JS_ENABLED
        case AsmJsType::Int32x4:
            return isSIMDInt32x4();
            break;
        case AsmJsType::Float32x4:
            return isSIMDFloat32x4();
            break;
        case AsmJsType::Float64x2:
            return isSIMDFloat64x2();
            break;
#endif
        default:
            break;
        }
        return false;
    }

    bool AsmJsType::isSuperType(AsmJsType type) const
    {
        return type.isSubType(which_);
    }

    Js::AsmJsRetType AsmJsType::toRetType() const
    {
        Which w = which_;
        // DoubleLit is for expressions only.
        if (w == DoubleLit)
        {
            w = Double;
        }
        return AsmJsRetType::Which(w);
    }

    /// RetType

    bool AsmJsRetType::operator!=(AsmJsRetType rhs) const
    {
        return which_ != rhs.which_;
    }

    bool AsmJsRetType::operator==(AsmJsRetType rhs) const
    {
        return which_ == rhs.which_;
    }

    Js::AsmJsType AsmJsRetType::toType() const
    {
        return AsmJsType::Which(which_);
    }

    Js::AsmJsVarType AsmJsRetType::toVarType() const
    {
        return AsmJsVarType::Which(which_);
    }

    Js::AsmJsRetType::Which AsmJsRetType::which() const
    {
        return which_;
    }

    AsmJsRetType::AsmJsRetType(AsmJSCoercion coercion)
    {
        switch (coercion)
        {
        case AsmJS_ToInt32: which_ = Signed; break;
        case AsmJS_ToNumber: which_ = Double; break;
        case AsmJS_FRound: which_ = Float; break;
#ifdef SIMD_JS_ENABLED
        case AsmJS_Int32x4: which_ = Int32x4; break;
        case AsmJS_Float32x4: which_ = Float32x4; break;
        case AsmJS_Float64x2: which_ = Float64x2; break;
#endif
        }
    }

    AsmJsRetType::AsmJsRetType(Which w) : which_(w)
    {

    }

    AsmJsRetType::AsmJsRetType() : which_(Which(-1))
    {

    }

    /// VarType

    bool AsmJsVarType::operator!=(AsmJsVarType rhs) const
    {
        return which_ != rhs.which_;
    }

    bool AsmJsVarType::operator==(AsmJsVarType rhs) const
    {
        return which_ == rhs.which_;
    }

    Js::AsmJsVarType AsmJsVarType::FromCheckedType(AsmJsType type)
    {
#ifdef SIMD_JS_ENABLED
        Assert( type.isInt() || type.isMaybeDouble() || type.isFloatish() || type.isSIMDType());
#else
        Assert(type.isInt() || type.isMaybeDouble() || type.isFloatish());
#endif
        if (type.isMaybeDouble())
            return Double;
        else if (type.isFloatish())
            return Float;
        else if (type.isInt())
            return Int;
#ifdef SIMD_JS_ENABLED
        else
        {
            // SIMD type
            return AsmJsVarType::Which(type.GetWhich());
        }
#endif

    }

    Js::AsmJSCoercion AsmJsVarType::toCoercion() const
    {
        switch (which_)
        {
        case Int:     return AsmJS_ToInt32;
        case Double:  return AsmJS_ToNumber;
        case Float:   return AsmJS_FRound;
#ifdef SIMD_JS_ENABLED
        case Int32x4:   return AsmJS_Int32x4;
        case Float32x4: return AsmJS_Float32x4;
        case Float64x2: return AsmJS_Float64x2;
#endif
        }
        Assert(false);
        return AsmJS_ToInt32;
    }

    Js::AsmJsType AsmJsVarType::toType() const
    {
        return AsmJsType::Which(which_);
    }

    Js::AsmJsVarType::Which AsmJsVarType::which() const
    {
        return which_;
    }

    AsmJsVarType::AsmJsVarType(AsmJSCoercion coercion)
    {
        switch (coercion)
        {
        case AsmJS_ToInt32: which_ = Int; break;
        case AsmJS_ToNumber: which_ = Double; break;
        case AsmJS_FRound: which_ = Float; break;
#ifdef SIMD_JS_ENABLED
        case AsmJS_Int32x4: which_ = Int32x4; break;
        case AsmJS_Float32x4: which_ = Float32x4; break;
        case AsmJS_Float64x2: which_ = Float64x2; break;
#endif
        }
    }

    AsmJsVarType::AsmJsVarType(Which w) : which_(w)
    {

    }

    AsmJsVarType::AsmJsVarType() : which_(Which(-1))
    {

    }

    template<>
    AsmJsMathConst* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == MathConstant);
        return (AsmJsMathConst*)this;
    }

    template<>
    AsmJsVar* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == Variable);
        return (AsmJsVar*)this;
    }

    template<>
    AsmJsVarBase* Js::AsmJsSymbol::Cast()
    {
        Assert( mType == Argument || mType == Variable || mType == ConstantImport);
        return ( AsmJsVarBase* )this;
    }

    template<>
    AsmJsFunctionDeclaration* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == ModuleFunction || mType == ImportFunction || mType == MathBuiltinFunction || mType == SIMDBuiltinFunction || mType == FuncPtrTable);
        return (AsmJsFunctionDeclaration*)this;
    }

    template<>
    AsmJsFunc* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == ModuleFunction);
        return (AsmJsFunc*)this;
    }

    template<>
    AsmJsImportFunction* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == ImportFunction);
        return (AsmJsImportFunction*)this;
    }

    template<>
    AsmJsMathFunction* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == MathBuiltinFunction);
        return (AsmJsMathFunction*)this;
    }

#ifdef SIMD_JS_ENABLED
    template<>
    AsmJsSIMDFunction* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == SIMDBuiltinFunction);
        return  (AsmJsSIMDFunction*) this;
    }
#endif
    template<>
    AsmJsArrayView* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == ArrayView);
        return (AsmJsArrayView*)this;
    }

    template<>
    AsmJsConstantImport* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == ConstantImport);
        return (AsmJsConstantImport*)this;
    }

    template<>
    AsmJsFunctionTable* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == FuncPtrTable);
        return (AsmJsFunctionTable*)this;
    }

    template<>
    AsmJsTypedArrayFunction* Js::AsmJsSymbol::Cast()
    {
        Assert(mType == TypedArrayBuiltinFunction);
        return (AsmJsTypedArrayFunction*)this;
    }

    Js::AsmJsType AsmJsMathConst::GetType() const
    {
        return AsmJsType::Double;
    }

    bool AsmJsMathConst::isMutable() const
    {
        return false;
    }

    bool AsmJsFunctionDeclaration::EnsureArgCount(uint32_t count)
    {
        if (mArgCount == Constants::UninitializedValue)
        {
            SetArgCount(count);
            return true;
        }
        else
        {
            return mArgCount == count;
        }
    }

    void AsmJsFunctionDeclaration::SetArgCount( uint32_t count )
    {
        Assert( mArgumentsType == nullptr );
        Assert(mArgCount == Constants::UninitializedValue);
        Assert(count != Constants::UninitializedValue);
        mArgCount = count;
        if( count > 0 )
        {
            mArgumentsType = AnewArrayZ( mAllocator, AsmJsType, count );
        }
    }

    bool AsmJsFunctionDeclaration::CheckAndSetReturnType(Js::AsmJsRetType val)
    {
        Assert((val != AsmJsRetType::Fixnum && val != AsmJsRetType::Unsigned && val != AsmJsRetType::Floatish) || GetSymbolType() == AsmJsSymbol::MathBuiltinFunction);
        if (mReturnTypeKnown)
        {
            Assert((mReturnType != AsmJsRetType::Fixnum && mReturnType != AsmJsRetType::Unsigned && mReturnType != AsmJsRetType::Floatish) || GetSymbolType() == AsmJsSymbol::MathBuiltinFunction);
            return mReturnType.toType().isSubType(val.toType());
        }
        mReturnType = val;
        mReturnTypeKnown = true;
        return true;
    }

    Js::AsmJsType AsmJsFunctionDeclaration::GetType() const
    {
        return mReturnType.toType();
    }

    bool AsmJsFunctionDeclaration::isMutable() const
    {
        return false;
    }
    bool AsmJsFunctionDeclaration::EnsureArgType(AsmJsVarBase* arg, int index)
    {
        if (mArgumentsType[index].GetWhich() == -1)
        {
            SetArgType(arg, index);
            return true;
        }
        else
        {
            return mArgumentsType[index] == arg->GetType();
        }
    }

    bool AsmJsFunctionDeclaration::SupportsArgCall( uint argCount, AsmJsType* args, AsmJsRetType& retType )
    {
        // we will assume the first reference to the function is correct, until proven wrong
        if (GetArgCount() == Constants::UninitializedValue)
        {
            SetArgCount(argCount);

            for (uint i = 0; i < argCount; i++)
            {
                if (args[i].isSubType(AsmJsType::Double))
                {
                    mArgumentsType[i] = AsmJsType::Double;
                }
                else if (args[i].isSubType(AsmJsType::Float))
                {
                    mArgumentsType[i] = AsmJsType::Float;
                }
                else if (args[i].isSubType(AsmJsType::Int))
                {
                    mArgumentsType[i] = AsmJsType::Int;
                }
#ifdef SIMD_JS_ENABLED
                else if (args[i].isSIMDType())
                {
                    mArgumentsType[i] = args[i];
                }
#endif
                else
                {
                    // call did not have valid argument type
                    return false;
                }
            }
            retType = mReturnType;
            return true;
        }
        else if( argCount == GetArgCount() )
        {
            for( uint i = 0; i < argCount; i++ )
            {
                if (!args[i].isSubType(mArgumentsType[i]))
                {
                    return false;
                }
            }
            retType = mReturnType;
            return true;
        }
        return false;
    }

    Js::ArgSlot AsmJsFunctionDeclaration::GetArgByteSize(int inArgCount) const
    {
#if _M_IX86
        if( GetSymbolType() == AsmJsSymbol::ImportFunction )
        {
            Assert(inArgCount != Constants::UninitializedValue);
            return static_cast<ArgSlot>( ( inArgCount*sizeof( Var ) ) );
        }
        Assert(GetArgCount() != Constants::UninitializedValue);
        ArgSlot bytesize = 0;
        for (uint i = 0; i < GetArgCount(); i++)
        {
            if( GetArgType(i).isMaybeDouble() )
            {
                bytesize += sizeof(double);
            }
            else if (GetArgType(i).isIntish())
            {
                bytesize += sizeof( int );
            }
            else if (GetArgType(i).isFloatish())
            {
                bytesize += sizeof(float);
            }
#ifdef SIMD_JS_ENABLED
            else if (GetArgType(i).isSIMDType())
            {
                bytesize += sizeof(AsmJsSIMDValue);
            }
#endif
#if DBG
            else
            {
                Assert(UNREACHED);
            }
#endif
        }
        return bytesize;


#elif _M_X64
        if (GetSymbolType() == AsmJsSymbol::ImportFunction)
        {
            Assert(inArgCount != Constants::UninitializedValue);
            return static_cast<ArgSlot>(inArgCount*MachPtr);
        }
        else
        {
            ArgSlot bytesize = 0;

#ifdef SIMD_JS_ENABLED
            for (uint i = 0; i < GetArgCount(); i++)
            {
                if (GetArgType(i).isSIMDType())
                {
                    bytesize += sizeof(AsmJsSIMDValue);
                }
                else
                {
                    bytesize += static_cast<ArgSlot>(MachPtr);
                }
            }
            return bytesize;
#else
            return static_cast<ArgSlot>(GetArgCount() * MachPtr);
#endif // SIMD_JS_ENABLED

        }
#else
        Assert(UNREACHED);
        return 0;
#endif

    }

    AsmJsMathFunction::AsmJsMathFunction( PropertyName name, ArenaAllocator* allocator, uint argCount, AsmJSMathBuiltinFunction builtIn, OpCodeAsmJs op, AsmJsRetType retType, ... ) :
        AsmJsFunctionDeclaration( name, AsmJsSymbol::MathBuiltinFunction, allocator )
        , mBuiltIn( builtIn )
        , mOverload( nullptr )
        , mOpCode(op)
    {
        bool ret = CheckAndSetReturnType(retType);
        Assert(ret);
        va_list arguments;

        SetArgCount( argCount );
        va_start( arguments, retType );
        for( uint iArg = 0; iArg < argCount; iArg++ )
        {
            SetArgType(va_arg(arguments, AsmJsType), iArg);
        }
        va_end(arguments);
    }

    void AsmJsMathFunction::SetOverload(AsmJsMathFunction* val)
    {
#if DBG
        AsmJsMathFunction* over = val->mOverload;
        while (over)
        {
            if (over == this)
            {
                Assert(false);
                break;
            }
            over = over->mOverload;
        }
#endif
        Assert(val->GetSymbolType() == GetSymbolType());
        if (this->mOverload)
        {
            this->mOverload->SetOverload(val);
        }
        else
        {
            mOverload = val;
        }
    }

    bool AsmJsMathFunction::CheckAndSetReturnType(Js::AsmJsRetType val)
    {
        return AsmJsFunctionDeclaration::CheckAndSetReturnType(val) || (mOverload && mOverload->CheckAndSetReturnType(val));
    }

    bool AsmJsMathFunction::SupportsArgCall( uint argCount, AsmJsType* args, AsmJsRetType& retType )
    {
        return AsmJsFunctionDeclaration::SupportsArgCall(argCount, args, retType) || (mOverload && mOverload->SupportsArgCall(argCount, args, retType));
    }

    bool AsmJsMathFunction::SupportsMathCall( uint argCount, AsmJsType* args, OpCodeAsmJs& op, AsmJsRetType& retType )
    {
        if (AsmJsFunctionDeclaration::SupportsArgCall(argCount, args, retType))
        {
            op = mOpCode;
            return true;
        }
        return mOverload && mOverload->SupportsMathCall(argCount, args, op, retType);
    }

    AsmJsFunc::AsmJsFunc(PropertyName name, ParseNode* pnodeFnc, ArenaAllocator* allocator) :
        AsmJsFunctionDeclaration(name, AsmJsSymbol::ModuleFunction, allocator)
        , mCompileTime(0)
        , mVarMap(allocator)
        , mBodyNode(nullptr)
        , mFncNode(pnodeFnc)
        , mIntRegisterSpace(allocator)
        , mFloatRegisterSpace(allocator)
        , mDoubleRegisterSpace( allocator )
        , mFuncInfo(pnodeFnc->sxFnc.funcInfo)
        , mFuncBody(nullptr)
#ifdef SIMD_JS_ENABLED
        , mSimdRegisterSpace(allocator)
        , mSimdVarsList(allocator)
#endif
        , mArgOutDepth(0)
        , mMaxArgOutDepth(0)
        , mDefined( false )
    {

    }

    /// AsmJsFunc
    AsmJsVarBase* AsmJsFunc::DefineVar( PropertyName name, bool isArg /*= false*/, bool isMutable /*= true*/ )
    {
        AsmJsVarBase* var = FindVar(name);
        if (var)
        {
            Output::Print(L"Variable redefinition: %s\n", name->Psz());
            return nullptr;
        }

        if (isArg)
        {
            // arg cannot be const
            Assert(isMutable);
            var = Anew( mAllocator, AsmJsArgument, name );
        }
        else
        {
            var = Anew(mAllocator, AsmJsVar, name, isMutable);
        }
        int addResult = mVarMap.AddNew(name->GetPropertyId(), var);
        if( addResult == -1 )
        {
            mAllocator->Free(var, isArg ? sizeof(AsmJsArgument) : sizeof(AsmJsVar));
            return nullptr;
        }
        return var;
    }


    AsmJsVarBase* AsmJsFunc::FindVar(const PropertyName name) const
    {
        return mVarMap.LookupWithKey(name->GetPropertyId(), nullptr);
    }

    void AsmJsFunc::ReleaseLocationGeneric(const EmitExpressionInfo* pnode)
    {
        if (pnode)
        {
            if (pnode->type.isIntish())
            {
                ReleaseLocation<int>(pnode);
            }
            else if (pnode->type.isMaybeDouble())
            {
                ReleaseLocation<double>(pnode);
            }
            else if (pnode->type.isFloatish())
            {
                ReleaseLocation<float>(pnode);
            }
            else if (pnode->type.isSIMDType())
            {
                ReleaseLocation<AsmJsSIMDValue>(pnode);
            }
        }
    }

    AsmJsSymbol* AsmJsFunc::LookupIdentifier(const PropertyName name, AsmJsLookupSource::Source* lookupSource /*= nullptr */) const
    {
        auto var = FindVar(name);
        if (var && lookupSource)
        {
            *lookupSource = AsmJsLookupSource::AsmJsFunction;
        }
        return var;
    }

    void AsmJsFunc::SetArgOutDepth( int outParamsCount )
    {
        mArgOutDepth = outParamsCount;
    }

    void AsmJsFunc::UpdateMaxArgOutDepth(int outParamsCount)
    {
        if (mMaxArgOutDepth < outParamsCount)
        {
            mMaxArgOutDepth = outParamsCount;
        }
    }

    bool AsmJsFunctionInfo::Init(AsmJsFunc* func)
    {
        const auto& intRegisterSpace = func->GetRegisterSpace<int>();
        const auto& doubleRegisterSpace = func->GetRegisterSpace<double>();
        const auto& floatRegisterSpace = func->GetRegisterSpace<float>();

#ifdef SIMD_JS_ENABLED
        const auto& simdRegisterSpace = func->GetRegisterSpace<AsmJsSIMDValue>();
#endif

        mIntConstCount = (intRegisterSpace.GetConstCount());
        mDoubleConstCount = (doubleRegisterSpace.GetConstCount());
        mFloatConstCount = (floatRegisterSpace.GetConstCount());

#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            mSimdConstCount = (simdRegisterSpace.GetConstCount());
        }
#endif

        mArgCount = func->GetArgCount();
        Recycler* recycler = func->GetFuncBody()->GetScriptContext()->GetRecycler();
        mbyteCodeTJMap = RecyclerNew(recycler, ByteCodeToTJMap,recycler);

        for( uint i = 0; i < GetArgCount(); i++ )
        {
            AsmJsType varType = func->GetArgType(i);
            SetArgType(AsmJsVarType::FromCheckedType(varType), i);
        }
        mIntVarCount = intRegisterSpace.GetVarCount();
        mDoubleVarCount = doubleRegisterSpace.GetVarCount();
        mFloatVarCount = floatRegisterSpace.GetVarCount();
#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            mSimdVarCount = simdRegisterSpace.GetVarCount();
        }
#endif
        mIntTmpCount = intRegisterSpace.GetTmpCount();
        mDoubleTmpCount = doubleRegisterSpace.GetTmpCount();
        mFloatTmpCount = floatRegisterSpace.GetTmpCount();
#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            mSimdTmpCount = simdRegisterSpace.GetTmpCount();
        }
#endif
        mReturnType = func->GetReturnType();

        mIntByteOffset = AsmJsFunctionMemory::RequiredVarConstants * sizeof(Var);

        const int totalIntCount = mIntConstCount + mIntVarCount + mIntTmpCount;
        const int count32bitsVarConst = (AsmJsFunctionMemory::RequiredVarConstants*sizeof(Var)) / sizeof(int);
        const int total32bitsBeforeFloat = count32bitsVarConst + totalIntCount;
        const int floatOffset32bitsFix = total32bitsBeforeFloat;
        // Offset of floats from (float*)m_localSlot 
        mFloatByteOffset = (floatOffset32bitsFix *sizeof(int));


        const int totalFloatCount = mFloatConstCount + mFloatVarCount + mFloatTmpCount;
        const int total32bitsBeforeDoubles = total32bitsBeforeFloat + totalFloatCount;
        // if its an odd number, add 1        
        const int doubleOffset32bitsFix = total32bitsBeforeDoubles + (total32bitsBeforeDoubles & 1);
        // Offset of doubles from (double*)m_localSlot 
        mDoubleByteOffset = (doubleOffset32bitsFix *sizeof(int));

#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG)
        {
            const int totalDoubleCount = mDoubleConstCount + mDoubleVarCount + mDoubleTmpCount;
            mSimdByteOffset = mDoubleByteOffset + totalDoubleCount * sizeof(double);
        }
#endif       
        if (PHASE_TRACE1(AsmjsInterpreterStackPhase))
        {
            Output::Print(L"ASMFunctionInfo Stack Data\n");
            Output::Print(L"==========================\n");
            Output::Print(L"RequiredVarConstants:%d\n", AsmJsFunctionMemory::RequiredVarConstants);
            Output::Print(L"IntOffset:%d  IntConstCount:%d  IntVarCount:%d  IntTmpCount:%d\n", mIntByteOffset, mIntConstCount, mIntVarCount, mIntTmpCount);
            Output::Print(L"FloatOffset:%d  FloatConstCount:%d  FloatVarCount:%d FloatTmpCount:%d\n", mFloatByteOffset, mFloatConstCount, mFloatVarCount, mFloatTmpCount);
            Output::Print(L"DoubleOffset:%d  DoubleConstCount:%d  DoubleVarCount:%d  DoubleTmpCount:%d\n", mDoubleByteOffset, mDoubleConstCount, mDoubleVarCount, mDoubleTmpCount);
#ifdef SIMD_JS_ENABLED
            if (SIMD_JS_FLAG)
            {
                Output::Print(L"SimdOffset:%d  SimdConstCount:%d  SimdVarCount:%d  SimdTmpCount:%d\n", mSimdByteOffset, mSimdConstCount, mSimdVarCount, mSimdTmpCount);
            }
            
#endif
            Output::Print(L"\n");
        }


        return true;
    }

    int AsmJsFunctionInfo::GetTotalSizeinBytes() const
    {
        int size = mDoubleByteOffset + (mDoubleConstCount + mDoubleVarCount + mDoubleTmpCount) * sizeof(double);
#ifdef SIMD_JS_ENABLED
        if (SIMD_JS_FLAG && GetSimdAllCount())
        {
            // No space for alignment is added.
            size += GetSimdAllCount()* sizeof(AsmJsSIMDValue);
        }
#endif
        return size;
    }

    Js::AsmJsType AsmJsArrayView::GetType() const
    {
        switch (mViewType)
        {
        case ArrayBufferView::TYPE_INT8:
        case ArrayBufferView::TYPE_INT16:
        case ArrayBufferView::TYPE_INT32:
        case ArrayBufferView::TYPE_UINT8:
        case ArrayBufferView::TYPE_UINT16:
        case ArrayBufferView::TYPE_UINT32:
            return AsmJsType::Intish;
        case ArrayBufferView::TYPE_FLOAT32:
            return AsmJsType::MaybeFloat;
        case ArrayBufferView::TYPE_FLOAT64:
            return AsmJsType::MaybeDouble;
        default:;
        }
        AssertMsg(false, "Unexpected array type");
        return AsmJsType::Intish;
    }

    bool AsmJsArrayView::isMutable() const
    {
        return false;
    }


    bool AsmJsImportFunction::SupportsArgCall( uint argCount, AsmJsType* args, AsmJsRetType& retType )
    {
        for (uint i = 0; i < argCount ; i++)
        {
            if (!args[i].isExtern())
            {
                return false;
            }
        }
        return true;
    }

    AsmJsImportFunction::AsmJsImportFunction(PropertyName name, PropertyName field, ArenaAllocator* allocator) :
        AsmJsFunctionDeclaration(name, AsmJsSymbol::ImportFunction, allocator)
        , mField(field)
    {
        CheckAndSetReturnType(AsmJsRetType::Void);
    }


    bool AsmJsFunctionTable::SupportsArgCall( uint argCount, AsmJsType* args, AsmJsRetType& retType )
    {
        if (mAreArgumentsKnown)
        {
            return AsmJsFunctionDeclaration::SupportsArgCall(argCount, args, retType);
        }

        Assert(GetArgCount() == Constants::UninitializedValue);
        SetArgCount( argCount );

        retType = this->GetReturnType();

        for (uint i = 0; i < argCount ; i++)
        {
            if (args[i].isInt())
            {
                this->SetArgType(AsmJsType::Int, i);
            }
            else if (args[i].isDouble())
            {
                this->SetArgType(AsmJsType::Double, i);
            }
            else if (args[i].isFloat())
            {
                this->SetArgType(AsmJsType::Float, i);
            }
            else
            {
                // Function tables can only have int, double or float as arguments
                return false;
            }
        }
        mAreArgumentsKnown = true;
        return true;
    }

    bool AsmJsFunctionTable::EnsureSignatureIsKnown(AsmJsModuleCompiler* module)
    {
        if (!mAreArgumentsKnown)
        {
            AsmJsFunctionDeclaration* asmFunc = module->LookupFunction(mFirstFuncName);
            if (asmFunc->GetSymbolType() != AsmJsSymbol::ModuleFunction)
            {
                return false;
            }
            AsmJsFunc* moduleFunc = (AsmJsFunc*)asmFunc;
            const int argCount = moduleFunc->GetArgCount();
            SetArgCount(argCount);
            for (int i = 0; i < argCount; i++)
            {
                SetArgType(moduleFunc->GetArgType(i), i);
            }
            mAreArgumentsKnown = true;
            return true;
        }
        return true;
    }

#ifdef SIMD_JS_ENABLED
    AsmJsSIMDFunction::AsmJsSIMDFunction(PropertyName name, ArenaAllocator* allocator, int argCount, AsmJsSIMDBuiltinFunction builtIn, OpCodeAsmJs op, AsmJsRetType retType, ...) :
        AsmJsFunctionDeclaration(name, AsmJsSymbol::SIMDBuiltinFunction, allocator)
        , mBuiltIn(builtIn)
        , mOverload(nullptr)
        , mOpCode(op)
    {
        bool ret = CheckAndSetReturnType(retType);
        Assert(ret);
        va_list arguments;

        SetArgCount(argCount);
        va_start(arguments, retType);
        for (int iArg = 0; iArg < argCount; iArg++)
        {
            SetArgType(va_arg(arguments, AsmJsType), iArg);
        }
        va_end(arguments);
    }

    bool AsmJsSIMDFunction::SupportsSIMDCall(int argCount, AsmJsType* args, OpCodeAsmJs& op, AsmJsRetType& retType)
    {
        if (AsmJsFunctionDeclaration::SupportsArgCall(argCount, args, retType))
        {
            op = mOpCode;
            return true;
        }
        return mOverload && mOverload->SupportsSIMDCall(argCount, args, op, retType);
    }

    bool AsmJsSIMDFunction::SupportsArgCall(uint argCount, AsmJsType* args, AsmJsRetType& retType)
    {
        return AsmJsFunctionDeclaration::SupportsArgCall(argCount, args, retType) || (mOverload && mOverload->SupportsArgCall(argCount, args, retType));
    }

    bool AsmJsSIMDFunction::CheckAndSetReturnType(Js::AsmJsRetType val)
    {
        return AsmJsFunctionDeclaration::CheckAndSetReturnType(val) || (mOverload && mOverload->CheckAndSetReturnType(val));
    }


    void AsmJsSIMDFunction::SetOverload(AsmJsSIMDFunction* val)
    {
#if DBG
        AsmJsSIMDFunction* over = val->mOverload;
        while (over)
        {
            if (over == this)
            {
                Assert(false);
                break;
            }
            over = over->mOverload;
        }
#endif
        Assert(val->GetSymbolType() == GetSymbolType());
        if (this->mOverload)
        {
            this->mOverload->SetOverload(val);
        }
        else
        {
            mOverload = val;
        }
    }

    bool AsmJsSIMDFunction::IsConstructor()
    {
        return mBuiltIn == AsmJsSIMDBuiltin_int32x4 || mBuiltIn == AsmJsSIMDBuiltin_float32x4 || mBuiltIn == AsmJsSIMDBuiltin_float64x2;
    }

    bool AsmJsSIMDFunction::IsConstructor(uint argCount)
    {
        if (!IsConstructor())
        {
            return false;
        }
        
        switch (mBuiltIn)
        {
        case AsmJsSIMDBuiltin_float64x2:
            return argCount == 2;
        case AsmJsSIMDBuiltin_float32x4:
        case AsmJsSIMDBuiltin_int32x4:
            return argCount == 4;
        };
        return false;
    }

    AsmJsVarType AsmJsSIMDFunction::GetConstructorVarType()
    {
        Assert(this->IsConstructor());
        return GetReturnType().toVarType();
    }
#endif

}