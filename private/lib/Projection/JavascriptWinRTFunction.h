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

    private:
        Var signature;
    };

    template <> inline bool VarIsImpl<JavascriptWinRTFunction>(RecyclableObject* object)
    {
        return VarIs<JavascriptFunction>(object) && UnsafeVarTo<JavascriptFunction>(object)->IsWinRTFunction();
    }

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
    };

    template <> inline bool VarIsImpl<JavascriptWinRTConstructorFunction>(RecyclableObject* object)
    {
        return Js::VarIs<Js::JavascriptFunction>(object) && Js::VarTo<Js::JavascriptFunction>(object)->IsWinRTFunction()
            && Js::VarTo<Js::JavascriptWinRTFunction>(object)->IsConstructorFunction();
    }
}
