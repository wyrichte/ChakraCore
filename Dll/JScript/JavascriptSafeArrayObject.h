//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
namespace Js
{
class JavascriptSafeArrayObject : public DynamicObject
{
private:
    DEFINE_VTABLE_CTOR(JavascriptSafeArrayObject, DynamicObject);
    DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptSafeArrayObject); 
public:
    JavascriptSafeArrayObject(JavascriptSafeArray* safeArrayObj, DynamicType * type)
        : DynamicObject(type)
    {
        m_safeArray = safeArrayObj;
    }

    class EntryInfo
    {
    public:
        static FunctionInfo NewInstance;
        static FunctionInfo ToLbound;
        static FunctionInfo ToUbound;
        static FunctionInfo ToDimensions;
        static FunctionInfo ToGetItem;
        static FunctionInfo ToArray;
        static FunctionInfo ValueOf;
    };

    static Var NewInstance(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryToLbound(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryToUbound(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryToDimensions(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryToGetItem(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryToArray(RecyclableObject* function, CallInfo callInfo, ...);
    static Var EntryValueOf(RecyclableObject* function, CallInfo callInfo, ...);

    static bool Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_SafeArrayObject;
    }

    static JavascriptSafeArrayObject* FromVar(Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSafeArray'");

        return static_cast<JavascriptSafeArrayObject *>(RecyclableObject::FromVar(aValue));
    }
    
    void InitPrototype(ScriptSite* scriptSite);
    JavascriptSafeArray* GetJavascriptSafeArray(){return m_safeArray;}

    virtual BOOL GetDiagValueString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
    virtual BOOL GetDiagTypeString(StringBuilder<ArenaAllocator>* stringBuilder, ScriptContext* requestContext) override;
private:
    JavascriptSafeArray* m_safeArray;
};
}