/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

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
        void ValidateThisInstance(Var thisObject);

        static JavascriptTypedObjectSlotAccessorFunction* FromVar(Var instance);
        static bool Is(Var instance);
        static void ValidateThis(Js::JavascriptTypedObjectSlotAccessorFunction* func, Var thisObject);

    private:
        int allowedTypeId;
    };
};
