/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

namespace Js
{
    typedef Var (__stdcall *StdCallJavascriptMethod)(RecyclableObject *callee, bool isConstructCall, Var *args, USHORT cargs, void *callbackState);

    class JavascriptExternalFunction : public RuntimeFunction
    {
    private:
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptExternalFunction);
    protected:
        DEFINE_VTABLE_CTOR(JavascriptExternalFunction, RuntimeFunction);
        JavascriptExternalFunction(DynamicType * type);
    public:
        JavascriptExternalFunction(JavascriptMethod nativeMethod, DynamicType* type);
        JavascriptExternalFunction(JavascriptMethod entryPoint, DynamicType* type, InitializeMethod method, unsigned short deferredSlotCount, bool accessors);
        JavascriptExternalFunction(JavascriptExternalFunction* wrappedMethod, DynamicType* type);
        JavascriptExternalFunction(StdCallJavascriptMethod nativeMethod, DynamicType* type);

        virtual BOOL IsExternalFunction() override {return TRUE; }
        inline void SetSignature(Var signature) { this->signature = signature; }
        Var GetSignature() { return signature; }
        inline void SetCallbackState(void *callbackState) { this->callbackState = callbackState; }
        void *GetCallbackState() { return callbackState; }

        static const int ETW_MIN_COUNT_FOR_CALLER = 0x100; // power of 2
        class EntryInfo
        {
        public:
            static FunctionInfo ExternalFunctionThunk;
            static FunctionInfo WrappedFunctionThunk;
            static FunctionInfo StdCallExternalFunctionThunk;
        };

        JavascriptMethod GetNativeMethod() { return nativeMethod; }
        BOOL SetLengthProperty(Var length);

        virtual JavascriptExternalFunction* MakeCopyOnWriteObject(ScriptContext* scriptContext) override;

        virtual bool CloneMethod(JavascriptFunction** pnewMethod, const Var newHome) override;

    private:
        Var signature;
        void *callbackState;
        union
        {
            JavascriptMethod nativeMethod;
            JavascriptExternalFunction* wrappedMethod;
            StdCallJavascriptMethod stdCallNativeMethod;
        };
        InitializeMethod initMethod;

        // Ensure GC doesn't pin
        unsigned int oneBit:1;
        // Count of type slots for
        unsigned int typeSlots:15;
        // Determines if we need are a dictionary type
        unsigned int hasAccessors:1;
        // This is used for etw tracking for now; potentially we can use this as heuristic
        // to determine if when to JIT the method.
        unsigned int callCount:15;

        static Var ExternalFunctionThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static Var WrappedFunctionThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static Var StdCallExternalFunctionThunk(RecyclableObject* function, CallInfo callInfo, ...);
        static void __cdecl DeferredInitializer(DynamicObject* instance, DeferredTypeHandlerBase* typeHandler, DeferredInitializeMode mode);

        void PrepareExternalCall(Arguments * args);
        Var FinishExternalCall(Var result);

        friend class JavascriptLibrary;
    };
}
