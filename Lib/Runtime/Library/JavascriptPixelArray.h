//----------------------------------------------------------------------------
// Copyright (C) 2008 - 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    // Implementation for Canvas pixel array
    

    class JavascriptPixelArray : public DynamicObject
    {
    private:
        static PropertyId specialPropertyIds[];

    protected:
        DEFINE_VTABLE_CTOR(JavascriptPixelArray, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptPixelArray);
    public:    
        JavascriptPixelArray(DynamicType * type);
        JavascriptPixelArray(uint32 length, DynamicType * type);

        virtual void Dispose(bool isShutdown) override;
        virtual void Finalize(bool isShutdown) {};

        // Accessors used by fast operators
        Var DirectGetItem(uint32 index);        
        void DirectSetItem(uint32 index, Var value);
        static void DirectSetItem(JavascriptPixelArray *array, uint32 index, Var value);
        
        BYTE *GetBufferPointer() { return buffer; }
        UINT GetBufferLength() { return bufferlength; }
        int GetLengthAsSignedInt() { Assert((int) bufferlength < MaxPixelArrayLength); return (int) bufferlength; }
        
        static bool Is(Var aValue);
        static JavascriptPixelArray* FromVar(Var aValue);

        virtual BOOL GetItem(Var originalInstance, uint32 index, Var* value, ScriptContext * requestContext) override;
        virtual BOOL SetItem(uint32 index, Var value, PropertyOperationFlags flags) override;
        virtual BOOL DeleteProperty(PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL HasProperty(PropertyId propertyId) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;
        virtual BOOL IsConfigurable(PropertyId propertyId) override;
        virtual BOOL GetSpecialPropertyName(uint32 index, Var *propertyName, ScriptContext * requestContext) override;
        virtual uint GetSpecialPropertyCount() const override;
        virtual PropertyId* GetSpecialPropertyIds() const override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL GetItemReference(Var originalInstance, uint32 index, Var* value, ScriptContext* requestContext) override;
        virtual BOOL DeleteItem(uint32 index, PropertyOperationFlags flags) override;
        virtual BOOL GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContxt, bool preferSnapshotSemantics, bool enumSymbols = false) override;
        virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
        virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;        

        virtual DynamicObject* MakeCopyOnWriteObject(ScriptContext* scriptContext);
        
        class EntryInfo
        {
        public:
            static FunctionInfo NewInstance;
        };

        static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);

        static int GetOffsetOfBuffer()          { return offsetof(JavascriptPixelArray, buffer); }
        static int GetOffsetOfBufferlength()    { return offsetof(JavascriptPixelArray, bufferlength); }

    private:
        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, BOOL* result);
        bool SetPropertyBuiltIns(PropertyId propertyId, BOOL* result);

        void SetIntegerValue(uint32 index, int32 value);
        void SetDoubleValue(uint32 index, double d);
        
        BYTE  *buffer;             // Points to a heap allocated RGBA buffer, can be null
        uint32 bufferlength;       // Number of bytes allocated
        
        static const uint32 MaxPixelArrayLength;
    };
}
