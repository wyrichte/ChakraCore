//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//  Implements ArrayBuffer according to Khronos spec.
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
    class ArrayBufferParent;
    class ArrayBuffer : public DynamicObject
    {
    private:
        static PropertyId specialPropertyIds[];

    public:
        // we need to install cross-site thunk on the nested arraybuffer when marshalling
        // typed array.
        DEFINE_VTABLE_CTOR(ArrayBuffer, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ArrayBuffer);
#define MAX_ASMJS_ARRAYBUFFER_LENGTH 0x100000000 //4GB
    private:
        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result);
        void ClearParentsLength(ArrayBufferParent* parent);
    public:
        template <typename FreeFN>
        class ArrayBufferDetachedState : public ArrayBufferDetachedStateBase
        {
        public:
            FreeFN* freeFunction;

            ArrayBufferDetachedState(BYTE* buffer, uint32 bufferLength, FreeFN* freeFunction, ArrayBufferAllocationType allocationType)
                : ArrayBufferDetachedStateBase(TypeIds_ArrayBuffer, buffer, bufferLength, allocationType),
                freeFunction(freeFunction)
            {}

            virtual void ClearSelfOnly() override
            {
                HeapDelete(this);
            }

            virtual void DiscardState() override
            {
                if (this->buffer != nullptr)
                {                    
                    freeFunction(this->buffer);
                    this->buffer = nullptr;
                }
                this->bufferLength = 0;
            }

            virtual void Discard() override
            {
                ClearSelfOnly();
            }
        };

        template <typename Allocator>
        ArrayBuffer(uint32 length, DynamicType * type, Allocator allocator);

        ArrayBuffer(byte* buffer, uint32 length, DynamicType * type);

        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
            static FunctionInfo Slice;
            static FunctionInfo IsView;
            static FunctionInfo GetterByteLength;
            static FunctionInfo GetterSymbolSpecies;
        };
        
        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntrySlice(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryIsView(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterByteLength(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryGetterSymbolSpecies(RecyclableObject* function, CallInfo callInfo, ...);

        static bool Is(Var aValue);
        static ArrayBuffer* NewFromDetachedState(DetachedStateBase* state, JavascriptLibrary *library);
        static ArrayBuffer* FromVar(Var aValue);

        static bool IsValidAsmJsBufferLength(uint length, bool forceCheck = false)
        {
#if _WIN64 
            /*
            1. length >= 2^16
            2. length is power of 2 or (length > 2^24 and length is multiple of 2^24)
            3. length is a multiple of 4K
            */
            return ((length >= 0x10000) &&
                ( ((length & (~length + 1)) == length) || 
                (length >= 0x1000000 && ((length & 0xFFFFFF) == 0))
                ) && 
                ((length % AutoSystemInfo::PageSize) == 0)
                );

#else
            if (forceCheck)
            {
                return ( (length >= 0x10000) && 
                         ( ((length & (~length + 1)) == length) || 
                           (length >= 0x1000000 && ((length & 0xFFFFFF) == 0))
                         ) && 
                         ((length & 0x0FFF) == 0)
                        );
            }
            return false;
#endif

        }

        static bool IsValidVirtualBufferLength(uint length)
        {
            
#if _WIN64 
            /*
            1. length >= 2^16
            2. length is power of 2 or (length > 2^24 and length is multiple of 2^24)
            3. length is a multiple of 4K
            */           
            return ( !PHASE_OFF1(Js::TypedArrayVirtualPhase) &&
                     (length >= 0x10000) &&
                     ( ( (length & (~length + 1)) == length) || 
                       ( length >= 0x1000000 && 
                         ((length & 0xFFFFFF) == 0)
                       )
                     ) &&
                     ((length % AutoSystemInfo::PageSize) == 0)
                   );

#else
            return false;
#endif
        }

        virtual BOOL HasProperty(Js::PropertyId propertyId) override;
        virtual BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
        virtual BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
        virtual BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
        virtual BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;

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
        virtual BOOL SetPropertyWithAttributes(PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None , SideEffects possibleSideEffects = SideEffects_Any) override;

        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;

        virtual ArrayBufferDetachedStateBase* DetachAndGetState();
        bool IsDetached() { return this->isDetached; }
        void SetIsAsmJsBuffer(){ mIsAsmJsBuffer = true; }
        uint32 GetByteLength() const { return bufferLength; }
        BYTE* GetBuffer() const { return buffer; }

        static int GetByteLengthOffset() { return offsetof(ArrayBuffer, bufferLength); }
        static int GetIsDetachedOffset() { return offsetof(ArrayBuffer, isDetached); }
        static int GetBufferOffset() { return offsetof(ArrayBuffer, buffer); }

        void AddParent(ArrayBufferParent* parent);
        void RemoveParent(ArrayBufferParent* parent);
        static void FreeMemAlloc(Var ptr)
        {
           BOOL fSuccess = VirtualFree((LPVOID)ptr, 0, MEM_RELEASE);
           Assert(fSuccess);
        }
#if _WIN64
        //maximum 2G -1  for amd64
        static const uint32 MaxArrayBufferLength = 0x7FFFFFFF;
#else
        // maximum 1G to avoid arithmatic overflow.
        static const uint32 MaxArrayBufferLength = 1 << 30;
#endif
    protected:
        typedef void __cdecl FreeFn(void* ptr);   
        virtual ArrayBufferDetachedStateBase* CreateDetachedState(BYTE* buffer, uint32 bufferLength);

        inline BOOL IsBuiltinProperty(PropertyId);
        static uint32 GetIndexFromVar(Js::Var arg, uint32 length, ScriptContext* scriptContext);

        //In most cases, the ArrayBuffer will only have one parent
        RecyclerWeakReference<ArrayBufferParent>* primaryParent;
        JsUtil::List<RecyclerWeakReference<ArrayBufferParent>*>* otherParents;


        BYTE  *buffer;             // Points to a heap allocated RGBA buffer, can be null
        uint32 bufferLength;       // Number of bytes allocated

        // When an ArrayBuffer is detached, the TypedArray and DataView objects pointing to it must be made aware, for this purpose the ArrayBuffer needs to hold WeakReferences to them
        bool isDetached;
        bool mIsAsmJsBuffer;
        bool isBufferCleared;

    };

    class ArrayBufferParent : public ArrayObject
    {
        friend ArrayBuffer;

    private:
        ArrayBuffer* arrayBuffer;

    protected:
        DEFINE_VTABLE_CTOR_ABSTRACT(ArrayBufferParent, ArrayObject);

        ArrayBufferParent(DynamicType * type, uint32 length, ArrayBuffer* arrayBuffer)
            : ArrayObject(type, /*initSlots*/true, length),
            arrayBuffer(arrayBuffer)
        {
            arrayBuffer->AddParent(this);
        }

        void ClearArrayBuffer()
        {
            if (this->arrayBuffer != nullptr)
            {
                this->arrayBuffer->RemoveParent(this);
                this->arrayBuffer = nullptr;
            }
        }

        void SetArrayBuffer(ArrayBuffer* arrayBuffer)
        {
            this->ClearArrayBuffer();

            if (arrayBuffer != nullptr)
            {
                this->arrayBuffer->AddParent(this);
                this->arrayBuffer = arrayBuffer;
            }
        }

    public:
        ArrayBuffer* GetArrayBuffer() const
        {
            return this->arrayBuffer;
        }
    };

    // Normally we use malloc/free; for ArrayBuffer created from projection we need to use different allocator.
    class JavascriptArrayBuffer : public ArrayBuffer
    {
    protected:
        DEFINE_VTABLE_CTOR(JavascriptArrayBuffer, ArrayBuffer);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptArrayBuffer);

    public:
        static JavascriptArrayBuffer* Create(uint32 length, DynamicType * type);        
        static JavascriptArrayBuffer* Create(byte* buffer, uint32 length, DynamicType * type);
        virtual void Dispose(bool isShutdown) override;
        virtual void Finalize(bool isShutdown) override;
        static void*__cdecl  AllocWrapper(size_t length)
        {
#if _WIN64
            LPVOID address = VirtualAlloc(null, MAX_ASMJS_ARRAYBUFFER_LENGTH, MEM_RESERVE, PAGE_NOACCESS);
            //throw out of memory
            if (!address)
            {
                Js::Throw::OutOfMemory();
            }
            LPVOID arrayAddress = VirtualAlloc(address, length, MEM_COMMIT, PAGE_READWRITE);
            if (!arrayAddress)
            {
                Js::Throw::OutOfMemory();
            }
            return arrayAddress;
#else
            Assert(false);
            return nullptr;
#endif
        };

    protected:
        JavascriptArrayBuffer(DynamicType * type);
    private:
        JavascriptArrayBuffer(uint32 length, DynamicType * type);
        JavascriptArrayBuffer(byte* buffer, uint32 length, DynamicType * type);
        virtual DynamicObject* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;
    };

    // the memory must be allocated via CoTaskMemAlloc.
    class ProjectionArrayBuffer : public ArrayBuffer
    {
    protected:
        DEFINE_VTABLE_CTOR(ProjectionArrayBuffer, ArrayBuffer);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ProjectionArrayBuffer);
        typedef void __stdcall FreeFn(LPVOID ptr);
        virtual ArrayBufferDetachedStateBase* CreateDetachedState(BYTE* buffer, uint32 bufferLength) override
        { 
            return HeapNew(ArrayBufferDetachedState<FreeFn>, buffer, bufferLength, CoTaskMemFree, ArrayBufferAllocationType::CoTask);
        }

    public:
        // Create constructor. script engine creates a buffer allocated via CoTaskMemAlloc.
        static ProjectionArrayBuffer* Create(uint32 length, DynamicType * type);
        // take over ownership. a CoTaskMemAlloc'ed buffer passed in via projection.
        static ProjectionArrayBuffer* Create(byte* buffer, uint32 length, DynamicType * type);
        virtual void Dispose(bool isShutdown) override;
        virtual void Finalize(bool isShutdown) override {};
    private:
        ProjectionArrayBuffer(uint32 length, DynamicType * type);
        ProjectionArrayBuffer(byte* buffer, uint32 length, DynamicType * type);
    };

    // non-owning ArrayBuffer used for wrapping external data
    class ExternalArrayBuffer : public ArrayBuffer
    {
    protected:
        DEFINE_VTABLE_CTOR(ExternalArrayBuffer, ArrayBuffer);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ExternalArrayBuffer);
    public:
        ExternalArrayBuffer(byte *buffer, uint32 length, DynamicType *type);
    };
}

