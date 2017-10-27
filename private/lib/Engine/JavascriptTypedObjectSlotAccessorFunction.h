//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//-------------------------------------------------------------------------------------------------------

typedef Js::Var(*ScriptMethod)(
    Js::Var method,
    CallInfo callInfo,
    Js::Var *args);

namespace Js
{
    class JavascriptTypedObjectSlotAccessorFunction : public RuntimeFunction
    {
    private:
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptTypedObjectSlotAccessorFunction); 
    protected:
        DEFINE_VTABLE_CTOR(JavascriptTypedObjectSlotAccessorFunction, RuntimeFunction);
    public:
        JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId, ScriptMethod fallBackTrampoline);

        int GetAllowedTypeId() const { return allowedTypeId; }
        bool ValidateThisInstance(Var thisObject);
        bool InstanceOf(Var thisObj);

        static JavascriptTypedObjectSlotAccessorFunction* FromVar(Var instance);
        static JavascriptTypedObjectSlotAccessorFunction* UnsafeFromVar(Var instance);
        static bool Is(Var instance);

        ScriptMethod GetFallBackTrampoline() const { return fallBackTrampoline; }

    private:
        int allowedTypeId;
        ScriptMethod fallBackTrampoline;
    };
};
