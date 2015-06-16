//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//  Implements typed array.
//----------------------------------------------------------------------------
#pragma once

namespace Js
{
    typedef Var (*PFNCreateTypedArray)(Js::ArrayBuffer* arrayBuffer, uint32 offSet, uint32 mappedLength, Js::JavascriptLibrary* javascirptLibrary);

    template<typename T> int __cdecl TypedArrayCompareElementsHelper(void* context, const void* elem1, const void* elem2);

    class TypedArrayBase : public ArrayBufferParent
    {
        friend ArrayBuffer;
    private:
        static PropertyId specialPropertyIds[];

    protected:
        DEFINE_VTABLE_CTOR_ABSTRACT(TypedArrayBase, ArrayBufferParent);

    private:
        BOOL GetPropertyBuiltIns(Js::PropertyId propertyId, Js::Var* value);

    public:
        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo Set;
            static FunctionInfo Subarray;

            static FunctionInfo From;
            static FunctionInfo Of;
            static FunctionInfo CopyWithin;
            static FunctionInfo Entries;
            static FunctionInfo Every;
            static FunctionInfo Fill;
            static FunctionInfo Filter;
            static FunctionInfo Find;
            static FunctionInfo FindIndex;
            static FunctionInfo ForEach;
            static FunctionInfo IndexOf;
            static FunctionInfo Join;
            static FunctionInfo Keys;
            static FunctionInfo LastIndexOf;
            static FunctionInfo Map;
            static FunctionInfo Reduce;
            static FunctionInfo ReduceRight;
            static FunctionInfo Reverse;
            static FunctionInfo Slice;
            static FunctionInfo Some;
            static FunctionInfo Sort;
            static FunctionInfo Values;

            static FunctionInfo GetterBuffer;
            static FunctionInfo GetterByteLength;
            static FunctionInfo GetterByteOffset;
            static FunctionInfo GetterLength;
            static FunctionInfo GetterSymbolToStringTag;
            static FunctionInfo GetterSymbolSpecies;
        };

        TypedArrayBase(ArrayBuffer* arrayBuffer, uint byteOffset, uint mappedLength, uint elementSize, DynamicType* type);

        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntrySet(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySubarray(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFrom(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryOf(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryCopyWithin(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryEntries(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryEvery(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFill(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFilter(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFind(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryFindIndex(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryForEach(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIndexOf(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryJoin(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryKeys(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryLastIndexOf(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryMap(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryReduce(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryReduceRight(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryReverse(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySlice(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySome(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySort(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryValues(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntryGetterBuffer(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterByteLength(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterByteOffset(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterLength(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterSymbolToStringTag(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterSymbolSpecies(RecyclableObject* function, CallInfo callInfo, ...);

        virtual BOOL HasProperty(Js::PropertyId propertyId) override;
        virtual BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL HasItem(uint32 index) override;
        virtual BOOL DeleteItem(uint32 index, Js::PropertyOperationFlags flags) override { return false; }
        virtual BOOL GetItem(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext * requestContext) override;
        virtual BOOL SetItem(__in uint32 index, __in Js::Var value, __in Js::PropertyOperationFlags flags = PropertyOperation_None) override;
        virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
        virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
        virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;
        virtual BOOL GetItemReference(Js::Var originalInstance, __in uint32 index, __out Js::Var* value, Js::ScriptContext * requestContext) override;
        virtual BOOL GetEnumerator(__in BOOL enumNonEnumerable, __out Js::Var* enumerator, Js::ScriptContext * requestContext, __in bool preferSnapshotSemantics = true, __in bool enumSymbols = false) override;

        virtual BOOL IsEnumerable(PropertyId propertyId)  override;
        virtual BOOL IsConfigurable(PropertyId propertyId)  override;
        virtual BOOL IsWritable(PropertyId propertyId)  override;
        virtual BOOL SetEnumerable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetWritable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetConfigurable(PropertyId propertyId, BOOL value) override;
        virtual BOOL SetAttributes(PropertyId propertyId, PropertyAttributes attributes) override;
        virtual BOOL SetAccessors(PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags) override;

        virtual BOOL GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext) override;
        virtual uint GetSpecialPropertyCount() const override;
        virtual PropertyId* GetSpecialPropertyIds() const override;
        virtual BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, PropertyOperationFlags flags = PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override;
        static BOOL Is(Var aValue);
        static BOOL Is(TypeId typeId);
        static TypedArrayBase* FromVar(Var aValue);
        //Returns false if this is not a TypedArray or it's not detached
        static BOOL IsDetachedTypedArray(Var aValue);
        static HRESULT GetBuffer(Var aValue, ArrayBuffer** outBuffer, uint32* outOffset, uint32* outLength);

        virtual BOOL DirectSetItem(__in uint32 index, __in Js::Var value, __in bool skipSetItem) = 0;
        virtual Var  DirectGetItem(__in uint32 index) = 0;

        uint32 GetByteLength() const { return length * BYTES_PER_ELEMENT; }
        uint32 GetByteOffset() const { return byteOffset; }
        uint32 GetBytesPerElement() const { return BYTES_PER_ELEMENT; }
        byte*  GetByteBuffer() const { return buffer; };
        bool IsDetachedBuffer() const { return this->GetArrayBuffer()->IsDetached(); }
        static Var CommonSet(Arguments& args);
        static Var CommonSubarray(Arguments& args);

        void SetObject(RecyclableObject* arraySource, uint32 targetLength, uint32 offset = 0);
        void Set(TypedArrayBase* typedArraySource, uint32 offset = 0);

        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;

        static bool TryGetLengthForOptimizedTypedArray(const Var var, uint32 *const lengthRef, TypeId *const typeIdRef);
        BOOL ValidateIndexAndDirectSetItem(__in Js::Var index, __in Js::Var value, __in bool * isNumericIndex);
        uint32 ValidateAndReturnIndex(__in Js::Var index, __in bool * skipOperation, __in bool * isNumericIndex);

        // objectArray support
        virtual BOOL SetItemWithAttributes(uint32 index, Var value, PropertyAttributes attributes) override;

    protected:
        inline BOOL IsBuiltinProperty(PropertyId);
        static Var CreateNewInstance(Arguments& args, ScriptContext* scriptContext, uint32 elementSize, PFNCreateTypedArray pfnCreateTypedArray );
        
        virtual void* GetCompareElementsFunction() = 0;

        virtual Var Subarray(uint32 begin, uint32 end) = 0;
        int32 BYTES_PER_ELEMENT;
        uint32 byteOffset;
        BYTE* buffer;   // beginning of mapped array.

    public:
        static uint32 GetOffsetOfBuffer()  { return offsetof(TypedArrayBase, buffer); }
        static uint32 GetOffsetOfLength()  { return offsetof(TypedArrayBase, length); }
    };

    template <typename TypeName, bool clamped = false, bool virtualAllocated = false>
    class TypedArray : public TypedArrayBase
    {
    protected:
        DEFINE_VTABLE_CTOR(TypedArray, TypedArrayBase);
        virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) 
        {
            Assert(this->GetScriptContext() != scriptContext);
            AssertMsg(VirtualTableInfo<TypedArray>::HasVirtualTable(this), "Derived class need to define marshal to script context");
            VirtualTableInfo<Js::CrossSiteObject<TypedArray<TypeName, clamped, virtualAllocated>>>::SetVirtualTable(this);
            ArrayBuffer* arrayBuffer = this->GetArrayBuffer();
            if (arrayBuffer && !arrayBuffer->IsCrossSiteObject())
            {
                arrayBuffer->MarshalToScriptContext(scriptContext);
            }
        }

        TypedArray(DynamicType *type): TypedArrayBase(null, 0, 0, sizeof(TypeName), type) { buffer = null; }

    public:
        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo Set;
            static FunctionInfo Subarray;
        };

        TypedArray(ArrayBuffer* arrayBuffer, uint32 byteOffset, uint32 mappedLength, DynamicType* type);

        virtual TypedArray<TypeName, clamped, virtualAllocated>* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;

        static Var Create(ArrayBuffer* arrayBuffer, uint32 byteOffSet, uint32 mappedLength, JavascriptLibrary* javascriptLibrary);
        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);

        static Var EntrySet(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySubarray(RecyclableObject* function, CallInfo callInfo, ...);

        Var Subarray(uint32 begin, uint32 end);

        static BOOL Is(Var aValue);
        static TypedArray<TypeName, clamped, virtualAllocated>* FromVar(Var aValue);

        __inline Var BaseTypedDirectGetItem(__in uint32 index)
        {
            if (this->IsDetachedBuffer()) //9.4.5.8 IntegerIndexedElementGet 
            {
                JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_DetachedTypedArray);
            }

            if (index < GetLength())
            {
                Assert((index + 1)* sizeof(TypeName)+GetByteOffset() <= GetArrayBuffer()->GetByteLength());
                TypeName* typedBuffer = (TypeName*)buffer;
                return JavascriptNumber::ToVar(typedBuffer[index], GetScriptContext());
            }
            if (BinaryFeatureControl::LanguageService())
                return GetScriptContext()->GetMissingItemResult(this, index);
            return GetLibrary()->GetUndefined();
        }

        __inline Var TypedDirectGetItemWithCheck(__in uint32 index)
        {
            if (this->IsDetachedBuffer()) //9.4.5.8 IntegerIndexedElementGet 
            {
                JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_DetachedTypedArray);
            }

            if (index < GetLength())
            {
                Assert((index + 1)* sizeof(TypeName)+GetByteOffset() <= GetArrayBuffer()->GetByteLength());
                TypeName* typedBuffer = (TypeName*)buffer;
                return JavascriptNumber::ToVarWithCheck(typedBuffer[index], GetScriptContext());
            }
            if (BinaryFeatureControl::LanguageService())
                return GetScriptContext()->GetMissingItemResult(this, index);
            return GetLibrary()->GetUndefined();
        }

        __inline BOOL BaseTypedDirectSetItem(__in uint32 index, __in Js::Var value, __in bool skipSetElement, TypeName (*convFunc)(Var value, ScriptContext* scriptContext))
        {
            // This call can potentially invoke user code, and may end up detaching the underlying array (this).
            // Therefore it was brought out and above the IsDetached check
            TypeName typedValue = convFunc(value, GetScriptContext());
            
            if (this->IsDetachedBuffer()) //9.4.5.9 IntegerIndexedElementSet 
            {
                JavascriptError::ThrowTypeError(GetScriptContext(), JSERR_DetachedTypedArray);
            }

            if (skipSetElement)
            {
                return FALSE;
            }

            AssertMsg(index < GetLength(), "Trying to set out of bound index for typed array.");
            Assert((index + 1)* sizeof(TypeName) + GetByteOffset() <= GetArrayBuffer()->GetByteLength());
            TypeName* typedBuffer = (TypeName*)buffer;

            typedBuffer[index] = typedValue;

            return TRUE;
            }

        virtual BOOL DirectSetItem(__in uint32 index, __in Js::Var value, __in bool skipSetItem) override sealed;
        virtual Var  DirectGetItem(__in uint32 index) override sealed;


        static BOOL DirectSetItem(__in TypedArray* arr, __in uint32 index, __in Js::Var value)
        {
            AssertMsg(arr != nullptr, "Array shouldn't be nullptr.");

            return arr->DirectSetItem(index, value, index >= arr->GetLength());
        }

    protected:
        void* GetCompareElementsFunction()
        {
            return &TypedArrayCompareElementsHelper<TypeName>;
        }
    };

    // in windows build environment, wchar_t is still not a intrinsic type, and we cannot do the type
    // specialized
    class CharArray : public TypedArrayBase
    {
    protected:
        DEFINE_VTABLE_CTOR(CharArray, TypedArrayBase);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(CharArray);

    public:
        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo Set;
            static FunctionInfo Subarray;
        };

        static Var EntrySet(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySubarray(RecyclableObject* function, CallInfo callInfo, ...);

        CharArray(ArrayBuffer* arrayBuffer, uint32 byteOffset, uint32 mappedLength, DynamicType* type) :
        TypedArrayBase(arrayBuffer, byteOffset, mappedLength, sizeof(wchar_t), type)
        {
            AssertMsg(arrayBuffer->GetByteLength() >= byteOffset, "invalid offset");
            AssertMsg(mappedLength*sizeof(wchar_t)+byteOffset <= GetArrayBuffer()->GetByteLength(), "invalid length");
            buffer = arrayBuffer->GetBuffer() + byteOffset;
        }

        static Var Create(ArrayBuffer* arrayBuffer, uint32 byteOffSet, uint32 mappedLength, JavascriptLibrary* javascirptLibrary);
        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);
        static BOOL Is(Var aValue);

        Var Subarray(uint32 begin, uint32 end);
        static CharArray* FromVar(Var aValue);

        virtual BOOL DirectSetItem(__in uint32 index, __in Js::Var value, __in bool skipSetItem) override;
        virtual Var  DirectGetItem(__in uint32 index) override;

    protected:
        void* GetCompareElementsFunction()
        {
            return &TypedArrayCompareElementsHelper<wchar_t>;
        }
    };
    
    typedef TypedArray<int8> Int8Array;
    typedef TypedArray<uint8,false> Uint8Array;
    typedef TypedArray<uint8,true> Uint8ClampedArray;
    typedef TypedArray<int16> Int16Array;
    typedef TypedArray<uint16> Uint16Array;
    typedef TypedArray<int32> Int32Array;
    typedef TypedArray<uint32> Uint32Array;
    typedef TypedArray<float> Float32Array;
    typedef TypedArray<double> Float64Array;
    typedef TypedArray<int64> Int64Array;
    typedef TypedArray<uint64> Uint64Array;
    typedef TypedArray<bool> BoolArray;
    typedef TypedArray<int8, false, true> Int8VirtualArray;
    typedef TypedArray<uint8, false, true> Uint8VirtualArray;
    typedef TypedArray<uint8, true, true> Uint8ClampedVirtualArray;
    typedef TypedArray<int16, false, true> Int16VirtualArray;
    typedef TypedArray<uint16, false, true> Uint16VirtualArray;
    typedef TypedArray<int32, false, true> Int32VirtualArray;
    typedef TypedArray<uint32, false, true> Uint32VirtualArray;
    typedef TypedArray<float, false, true> Float32VirtualArray;
    typedef TypedArray<double, false, true> Float64VirtualArray;
    
}
