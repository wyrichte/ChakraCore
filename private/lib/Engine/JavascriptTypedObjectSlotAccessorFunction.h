//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

namespace Js
{
    class JavascriptTypedObjectSlotAccessorFunction : public RuntimeFunction
    {
    private:      
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptTypedObjectSlotAccessorFunction); 
    protected:
        DEFINE_VTABLE_CTOR(JavascriptTypedObjectSlotAccessorFunction, RuntimeFunction);
    public:
        JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId);

        int GetAllowedTypeId() const {return allowedTypeId; }
        bool ValidateThisInstance(Var thisObject);
        bool InstanceOf(Var thisObj);

        static JavascriptTypedObjectSlotAccessorFunction* FromVar(Var instance);
        static bool Is(Var instance);

    private:
        int allowedTypeId;
    };
};
