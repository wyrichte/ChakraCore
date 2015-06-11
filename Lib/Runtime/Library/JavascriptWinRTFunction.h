/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

namespace Js
{
    class WinRTFunctionInfo : public FunctionInfo
    {
    public:
        WinRTFunctionInfo(JavascriptMethod entryPoint, Attributes attributes = None)
            : FunctionInfo(entryPoint, attributes)
        { }

        void SetOriginalEntryPoint(JavascriptMethod entryPoint)
        {
            this->originalEntryPoint = entryPoint;
        }
    };

    class JavascriptWinRTFunction : public RuntimeFunction
    {
    protected:
        DEFINE_VTABLE_CTOR(JavascriptWinRTFunction, RuntimeFunction);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptWinRTFunction);
    public:
        JavascriptWinRTFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature)
            : RuntimeFunction(type, functionInfo), signature(signature)
        { }
        virtual BOOL IsWinRTFunction() override sealed {return TRUE; }
        virtual bool IsConstructorFunction() { return false; }
        inline Var GetSignature() { return signature; }
        inline void ChangeSignature(Var replacement) { signature = replacement; }
        inline WinRTFunctionInfo * GetWinRTFunctionInfo() { return static_cast<WinRTFunctionInfo*>(this->GetFunctionInfo()); }
        inline static JavascriptWinRTFunction * FromVar(Var var)
        {
#if DBG
            auto function = JavascriptFunction::FromVar(var);
            Assert(function->IsWinRTFunction());
#endif
            return static_cast<JavascriptWinRTFunction*>(var);
        }

    private:
        Var signature;
    };

    class JavascriptWinRTConstructorFunction : public JavascriptWinRTFunction
    {
    private:
        DEFINE_VTABLE_CTOR(JavascriptWinRTConstructorFunction, JavascriptWinRTFunction);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptWinRTConstructorFunction);

        Var typeInformation;

    public:
        JavascriptWinRTConstructorFunction(DynamicType * type, WinRTFunctionInfo * functionInfo, Var signature)
            : JavascriptWinRTFunction(type, functionInfo, signature), typeInformation(nullptr)
        { }

        virtual bool IsConstructorFunction() override { return true; }

        void SetTypeInformation(Var typeInformation)
        {
            Assert(this->typeInformation == nullptr);
            Assert(typeInformation != nullptr);
            this->typeInformation = typeInformation;
        }

        Var GetTypeInformation() { return typeInformation; }

        inline static bool Is(Var var)
        {
            return Js::JavascriptFunction::Is(var) && Js::JavascriptFunction::FromVar(var)->IsWinRTFunction()
                && Js::JavascriptWinRTFunction::FromVar(var)->IsConstructorFunction();
        }

        inline static JavascriptWinRTConstructorFunction * FromVar(Var var)
        {
#if DBG
            Assert(Js::JavascriptWinRTConstructorFunction::Is(var));
#endif
            return static_cast<JavascriptWinRTConstructorFunction*>(var);
        }
    };
}
