// Copyright (C) Microsoft. All rights reserved. 
#pragma once
#include "stdafx.h"
#include "IMyScriptDirectTests.h"

typedef JsHostOnScriptErrorCallback FN_OnScriptError;

typedef HRESULT (STDAPICALLTYPE* FN_JsVarAddRef)(Var);
typedef HRESULT (STDAPICALLTYPE* FN_JsVarRelease)(Var);
class MyScriptDirectTests:public IMyScriptDirectTests
{
private:   
    static FN_JsVarAddRef ptr_JsVarAddRef;
    static FN_JsVarRelease ptr_JsVarRelease;
    JsHostNativeTestArguments* mptr_jsHostArgs;
    IActiveScriptDirect* mptr_EzeScriptDirect;
    IJavascriptThreadProperty* threadProperty;
    IActiveScriptParse* mptr_ActiveScriptParse;
    IActiveScriptGarbageCollector* mptr_ActiveScriptGC;
    Data* mptr_mydata;
    IJavascriptOperations* mptr_jsop;
    ITypeOperations* mptr_typeop;
    InsideData* mptr_data_item;
    BOOL mbool_dataitem_init;

    //Helper Methods 
private:
    void InitJScriptEngine(JsHostNativeTestArguments* jsHostArgs);
    static Var MyObjectConstructor(Var function, CallInfo callInfo, Var* args);  
    BOOL GetTypeOp();
    BOOL GetJsop();
    void Pass(std::string message);
    IJavascriptThreadProperty* CreateThreadService(HINSTANCE jscriptLibrary);

public:
    static HRESULT PinObject(Var obj);
    static HRESULT UnpinObject(Var obj);
    MyScriptDirectTests(JsHostNativeTestArguments* jsHostArgs);
    ~MyScriptDirectTests();
    void InitThreadService() { this->threadProperty = CreateThreadService(this->mptr_jsHostArgs->jscriptLibrary); }
    IJavascriptThreadProperty* GetThreadService() const { return threadProperty; }
    void SetPropertyOnTypedObject(std::wstring  objname,std::wstring propname,std::wstring value,std::string type,std::string expval);
    void SetPrototypeOnTypedObject();
    Data* GetData(){return mptr_mydata;};
    void GetFlags(int *flagCount, _Outptr_result_maybenull_ LPWSTR** flags) { *flagCount = mptr_jsHostArgs->flagCount; *flags = mptr_jsHostArgs->flags; }
    void ClearData();
    void Start();
    void End();
    void ParseAndExecute(__in LPCWSTR str, HRESULT expectedHR=S_OK);
    HRESULT LoadScriptFromFile(__in LPCWSTR fileName, void* jsHostScriptSite = NULL);
    void CreateTypeObject(std::wstring objname);
    void CreateFunction(std::wstring function_name,std::wstring proto_objname,std::wstring proto_protoname,std::wstring cname);
    BOOL CreateTypedObjectWithPrototype(std::wstring ctor_name,std::wstring objname,std::wstring tname);
    void SetPropertyOnPrototypeInstance(std::wstring ctor_name,std::wstring prop_name,std::wstring value,std::string type,std::string exp_value);
    BOOL SetPropertyOnGlobalObject(std::wstring propname,std::wstring value,std::string type,std::string expval);
    std::wstring GetPropertyName(PropertyId pid);
    void DeleteProperty(std::wstring prop_name,std::wstring objname);
    void MyScriptDirectTests::ByteCount();
    void SetOnScriptError(FN_OnScriptError ptr_function, void* context);
    HRESULT CreateErrorObject(JsErrorType errorType, HRESULT hCode, LPCWSTR message, Var *errorObject);
    HRESULT CollectGarbage(SCRIPTGCTYPE scriptgctype);
    // return an IActiveScriptDirect pointer; it's not addref'ed. 
    IActiveScriptDirect* GetScriptDirectNoRef() const {return mptr_EzeScriptDirect; }
    IActiveScriptDirect* CreateNewEngine(void** jsHostScriptSite, bool freeAtShutdown);
    HRESULT ShutdownScriptSite(void* jsHostScriptSite);
    BOOL FAIL_hr(HRESULT hres,std::wstring message, HRESULT expectedHR=S_OK);

    Var GetGlobalObject();
    PropertyId GetOrAddPropertyId(const wchar_t *const name);
    Var GetProperty(const Var instance, const wchar_t *const name);
    void SetProperty(const Var instance, const wchar_t *const name, const Var value);
    bool ToBoolean(const Var value);
    double ToDouble(const Var value);
    std::wstring ToString(const Var value);
    Var ToVar(const bool value);
};
