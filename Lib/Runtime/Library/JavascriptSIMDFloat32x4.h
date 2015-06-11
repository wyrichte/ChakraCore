//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef SIMD_JS_ENABLED

class JavascriptSIMDInt32x4;
class JavascriptSIMDFloat64x2;

namespace Js
{
    class JavascriptSIMDFloat32x4 sealed : public RecyclableObject
    {
    private:
        SIMDValue value;

        DEFINE_VTABLE_CTOR(JavascriptSIMDFloat32x4, RecyclableObject);


    public:
        class EntryInfo
        {
        public:
            static FunctionInfo ToString;
            
        };

        JavascriptSIMDFloat32x4(SIMDValue *val, StaticType *type);
        static JavascriptSIMDFloat32x4* New(SIMDValue *val, ScriptContext* requestContext);
        static bool Is(Var instance);
        static JavascriptSIMDFloat32x4* FromVar(Var aValue);

        static JavascriptSIMDFloat32x4* FromFloat64x2(JavascriptSIMDFloat64x2 *instance, ScriptContext* requestContext);
        static JavascriptSIMDFloat32x4* FromFloat64x2Bits(JavascriptSIMDFloat64x2 *instance, ScriptContext* requestContext);
        static JavascriptSIMDFloat32x4* FromInt32x4(JavascriptSIMDInt32x4   *instance, ScriptContext* requestContext);
        static JavascriptSIMDFloat32x4* FromInt32x4Bits(JavascriptSIMDInt32x4   *instance, ScriptContext* requestContext);

        __inline SIMDValue GetValue() { return value; }

        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;


        // Entry Points
        /*
        There is one toString per SIMD type. The code is entrant from value objects (e.g. a.toString()) or on arithmetic operations.
        It will also be a property of SIMD.float32x4.prototype for SIMD dynamic objects.
        */
        static Var EntryToString(RecyclableObject* function, CallInfo callInfo, ...);
        // End Entry Points

        Var  Copy(ScriptContext* requestContext);
        Var  CopyAndSetLane(uint index, float value, ScriptContext* requestContext);

    private:
        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, ScriptContext* requestContext);
        Var  GetLaneAsNumber(uint index, ScriptContext* requestContext);
        Var  GetSignMask();
        
    };
}

#endif