//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
#if JS_PROFILE_DATA_INTERFACE
namespace Js
{
    // This class allows read/write Javascript access to a function's profile data.
    class ProfileDataObject : public DynamicObject
    {
    protected:
        DEFINE_VTABLE_CTOR(ProfileDataObject, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(ProfileDataObject);
    public:
        ProfileDataObject(DynamicType *type, FunctionBody *funcBody);
        virtual PropertyQueryFlags HasPropertyQuery(PropertyId propertyId) override;
        virtual PropertyQueryFlags GetPropertyQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual PropertyQueryFlags GetPropertyQuery(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual PropertyQueryFlags GetPropertyReferenceQuery(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL IsEnumerable(PropertyId propertyId) override;

    private:
        FunctionBody *funcBody;

        void LoadPropertyId(LPCWSTR str, PropertyId *propertyId);

        void BuildConstants();
        void SetConstantProperty(LPCWSTR str, uint16 value, Var object);
        
        template<class T> 
        void AddWrappedArray(PropertyId propertyId, byte *buffer, uint32 length);

        PropertyId loopCount;
        PropertyId implicitCallFlags;
        PropertyId loopImplicitCallFlags;
        PropertyId returnTypeInfo;
        PropertyId parameterInfo;
        // PropertyId callSiteInfo; // TODO
        PropertyId propValueType;
        PropertyId propImplicitCallFlags;
    };
}
#endif
