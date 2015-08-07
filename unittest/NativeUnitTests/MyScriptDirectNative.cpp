// Copyright (C) Microsoft. All rights reserved.
#pragma once
#include "stdafx.h"
#include "edgescriptDirect.h"
#include <iostream>
#include <string>
#include "MyScriptDirectNative.h"
#include "Verifier.h"

// BUGBUG: we should use VarToExtension in constructor as well.
extern __int64 extensionOffset = -1;

using namespace std;

FN_JsVarAddRef MyScriptDirectTests::ptr_JsVarAddRef;
FN_JsVarRelease MyScriptDirectTests::ptr_JsVarRelease;

void PinObject(Var obj)
{
    MyScriptDirectTests::PinObject(obj);
}

void UnpinObject(Var obj)
{
    MyScriptDirectTests::UnpinObject(obj);
}

HRESULT MyScriptDirectTests::PinObject(Var obj)
{
    if (obj)
    {
        return ptr_JsVarAddRef(obj);
    }
    return S_OK;
}

HRESULT MyScriptDirectTests::UnpinObject(Var obj)
{
    if (obj)
    {
        return ptr_JsVarRelease(obj);
    }
    return S_OK;
}

MyScriptDirectTests::MyScriptDirectTests(JsHostNativeTestArguments* jsHostArgs) : 
    mptr_mydata(new Data()), mptr_EzeScriptDirect(nullptr), mptr_ActiveScriptParse(nullptr), mptr_ActiveScriptGC(nullptr),
    mptr_jsop(nullptr), mptr_typeop(nullptr)
{
    InitJScriptEngine(jsHostArgs);
    mptr_data_item=0;
}

Var MyScriptDirectTests::MyObjectConstructor(Var function, CallInfo callInfo, Var* args)
{
    Var msdt = args[1];
    MyScriptDirectTests* somethis=(MyScriptDirectTests*)*(void**)((byte*)msdt+extensionOffset);
    IActiveScriptDirect* iasd=somethis->mptr_mydata->activescriptdirect;
    Var typeObj;
    iasd->CreateTypedObject(somethis->mptr_data_item->GetType(),100,TRUE,&typeObj);
    somethis->mptr_data_item->SetTypeObj(typeObj);
    return somethis->mptr_data_item->GetTypeObj();
}


BOOL MyScriptDirectTests::FAIL_hr(HRESULT hres,std::wstring method, HRESULT expectedHR)
{
    try
    {
        if(FAILED(hres) && hres != expectedHR)
        {
            std::wstring fail_str=L"";
            fail_str=L"FAILED:  "+method;
            throw fail_str;
        }
        return true;
    }
    catch(exception e)
    {
        throw e;
    }
}
BOOL MyScriptDirectTests::GetJsop()
{
    FAIL_hr(mptr_EzeScriptDirect->GetJavascriptOperations(&mptr_jsop),L"GetJavascriptOperations");
    return true;
}
BOOL MyScriptDirectTests::GetTypeOp()
{
    FAIL_hr(mptr_EzeScriptDirect->GetDefaultTypeOperations(&mptr_typeop),L"GetDefaultTypeOperations");
    return true;
}

void MyScriptDirectTests::InitJScriptEngine(JsHostNativeTestArguments* jsHostArgs)
{

    BOOL check = 0;
    if (! jsHostArgs || ! jsHostArgs->activeScript || ! jsHostArgs->jscriptLibrary)
    {
        FAIL_hr(E_FAIL, L"Invalid jsHostArgs structure");
    }
    ptr_JsVarAddRef = (FN_JsVarAddRef) GetProcAddress(jsHostArgs->jscriptLibrary, "JsVarAddRef");
    if (!ptr_JsVarAddRef)
    {
        FAIL_hr(E_FAIL, L"GetProcAddress JsVarAddRef");
    }
    ptr_JsVarRelease = (FN_JsVarRelease) GetProcAddress(jsHostArgs->jscriptLibrary, "JsVarRelease");
    if (!ptr_JsVarRelease)
    {
        FAIL_hr(E_FAIL, L"GetProcAddress JsVarRelease");
    }
 
    mptr_jsHostArgs = jsHostArgs;
    IActiveScript* script = jsHostArgs->activeScript;
    FAIL_hr(script->QueryInterface(__uuidof(IActiveScriptDirect), (LPVOID*)&mptr_EzeScriptDirect),L"QI IActiveScriptDirect");
    FAIL_hr(script->QueryInterface(__uuidof(IActiveScriptParse), (LPVOID*)&mptr_ActiveScriptParse),L"QI IActiveScriptParse");
    FAIL_hr(script->QueryInterface(__uuidof(IActiveScriptGarbageCollector), (LPVOID*)&mptr_ActiveScriptGC),L"QI IActiveScriptGarbageCollector");
    
    mptr_mydata->activescriptdirect=mptr_EzeScriptDirect;
    check=GetJsop();
    if(check)
    {
        check=GetTypeOp();
        if(!check)
        {
            cout<<"FAILED TO SET THE TYPE OP"<<endl;
        }
    }
    else
    {
        cout<<"FAILED to set the mptr_jsop"<<endl;
    }
}

LPWSTR ConvertToLPWSTR( const std::string& s )
{
    WCHAR* ws = new wchar_t[s.size()+1]; // +1 for zero at the end
    memcpy( ws, s.c_str(), s.size());
    ws[s.size()] = 0; // zero at the end
    return ws;
}


void MyScriptDirectTests::Start()
{
    mptr_data_item=new InsideData();
    mbool_dataitem_init=1;

}

void MyScriptDirectTests::End()
{
    mptr_mydata->mydatalist.push_back(mptr_data_item);
    mbool_dataitem_init=0;
    mptr_jsHostArgs->onScriptErrorHelper->SetCallback(nullptr, nullptr);
}

void MyScriptDirectTests::Pass(std::string message)
{
    //TODO
}

void MyScriptDirectTests::ClearData()
{
    for each(InsideData* inside in mptr_mydata->mydatalist)
    {        
        delete inside;
    }
    mptr_mydata->mydatalist.clear();
    mptr_mydata->inheritance.clear();
    mptr_mydata->objmapping.clear();

}

HRESULT MyScriptDirectTests::LoadScriptFromFile(__in LPCWSTR fileName, void* jsHostScriptSite)
{
    return mptr_jsHostArgs->loadScriptFile(jsHostScriptSite ? jsHostScriptSite : mptr_jsHostArgs->jsHostScriptSite, fileName);
}

IActiveScriptDirect* MyScriptDirectTests::CreateNewEngine(void** jsHostScriptSite, bool freeAtShutdown)
{
    IActiveScriptDirect* scriptDirect = nullptr;
    return FAILED(mptr_jsHostArgs->createNewEngine(&scriptDirect, jsHostScriptSite, freeAtShutdown)) ? nullptr : scriptDirect;
}

HRESULT MyScriptDirectTests::ShutdownScriptSite(void* jsHostScriptSite)
{
    return mptr_jsHostArgs->shutdownScriptSite(jsHostScriptSite);
}

void MyScriptDirectTests::ParseAndExecute( __in LPCWSTR str, HRESULT expectedHR)
{
    EXCEPINFO excepInfo;
    VARIANT result;
    FAIL_hr(mptr_ActiveScriptParse->ParseScriptText(str, nullptr, nullptr, nullptr, 0xdeadbeef, 0, SCRIPTTEXT_HOSTMANAGESSOURCE, &result, &excepInfo), L"ParseScriptText", expectedHR);
}

void MyScriptDirectTests::CreateTypeObject(std::wstring objname)
{
    HTYPE newtyperef=0;
    Var globalObj=0;
    Var typeobj=0;
    BOOL setresult=false;
    PropertyId pid=-1;
    PropertyId exists_id=-1;
    std::string obname="";
    FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(objname.c_str(),&exists_id),L" Find Property Id Failed for Property: "+objname);
    if(exists_id>0)
    {
        FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObj),L"GetGlobalObject");
        BOOL results=0;
        FAIL_hr(mptr_jsop->HasProperty(mptr_EzeScriptDirect, globalObj,exists_id,&results),L"GetProperty for Property: "+objname);
        if(!results)
        {
            WCHAR* pn=L"xasd";
            PropertyId nameId=GetOrAddPropertyId(pn);
            FAIL_hr(mptr_EzeScriptDirect->CreateType((JavascriptTypeId)-1,nullptr,0,nullptr,nullptr,mptr_typeop, false, nameId,true,&newtyperef), L"Create Type");
            FAIL_hr(mptr_EzeScriptDirect->CreateTypedObject(newtyperef,100,TRUE,&typeobj),L"CreateTypedObject");
            FAIL_hr(mptr_typeop->SetProperty(mptr_EzeScriptDirect, globalObj,exists_id,typeobj,&results),L"SetProperty");
        }
    }
    else
    {
        WCHAR* pn=L"xasd1";
        PropertyId nameId=GetOrAddPropertyId(pn);
        FAIL_hr(mptr_EzeScriptDirect->CreateType((JavascriptTypeId)-1, nullptr, 0, nullptr, nullptr, mptr_typeop, false, nameId, true, &newtyperef), L"Create Type");
        FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(objname.c_str(),&pid),L"GetOrAddPropertyId");;
        if (pid<0)
        {
            std::string fail_str="";
            fail_str="Property ID is less than 0 ";
            throw fail_str;
        }
        FAIL_hr(mptr_EzeScriptDirect->CreateTypedObject(newtyperef, 100, true, &typeobj),L"CreateTypedObject");

        FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObj),L"GetGlobalObject");
        FAIL_hr(mptr_typeop->SetProperty(mptr_EzeScriptDirect, globalObj,pid,typeobj,&setresult),L"SetProperty");
        if (!setresult)
        {
            std::string fail_str="";
            fail_str="FAILED to set the Property";
            throw fail_str;
        }
    }
    if(typeobj!=0)
    {
        mptr_data_item->SetTypeObj(typeobj);
        mptr_data_item->typeobj_name=objname;
    }



}

void MyScriptDirectTests::SetPropertyOnTypedObject(std::wstring objname,std::wstring propname,std::wstring value,std::string type,std::string expval)
{
    try
    {
        std::wstring srcstr=L"this."+objname+L"."+propname+L"="+value;
        wchar_t* l_str=new wchar_t[srcstr.size()];
        l_str=const_cast<wchar_t*>(srcstr.c_str());
        ParseAndExecute(l_str);

        //Set State
        mptr_data_item->prop_name=propname;
        mptr_data_item->prop_value=expval;
        mptr_data_item->propval_type=type;
        mptr_data_item->is_globalobj="false";
        mptr_data_item->has_prototypeobj="false";

    }
    catch (exception e)
    {
        throw e.what();
    }
}



BOOL MyScriptDirectTests::SetPropertyOnGlobalObject(std::wstring propname,std::wstring value,std::string type,std::string expval)
{
    try
    {
        std::wstring srcstr=L"this."+propname+L"="+value;
        wchar_t* l_str=new wchar_t[srcstr.size()];
        l_str=const_cast<wchar_t*>(srcstr.c_str());

        ParseAndExecute(l_str);

        //Set State
        mptr_data_item->prop_name=propname;
        mptr_data_item->prop_value=expval;
        mptr_data_item->propval_type=type;
        mptr_data_item->is_globalobj="true";
        mptr_data_item->has_prototypeobj="false";

        return true;
    }
    catch (exception e)
    {
        throw e.what();
    }
}

void MyScriptDirectTests::CreateFunction(std::wstring function_name,std::wstring proto_objname,std::wstring proto_protoname,std::wstring cname/*,int chainnumber*/)
{
    Var proto_instance=0;
    Var protofunc=0;
    Var globalObject=0;
    FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObject),L"GetGlobalObject");
    std::string fname(function_name.length(),L'');

    char* str_c2=new char[function_name.length()];
    memcpy(str_c2,function_name.c_str(),function_name.size());
    fname=str_c2;

    WCHAR* funcname=const_cast<WCHAR*>(function_name.c_str());
    WCHAR* cnstrname=const_cast<WCHAR*>(cname.c_str());
    if(proto_objname.compare(L"default")==0)
    {
        //Create a prototype Object and set that object to the constructor function
        FAIL_hr(mptr_EzeScriptDirect->CreateObject(&proto_instance),L"CreateObject Failed ");
    }
    else
    {
        PropertyId pid_proto=-1;
        FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(proto_objname.c_str(),&pid_proto),L"FindPropertyId");

        if(proto_objname.compare(L"default")==0 && proto_protoname.compare(L"default")==0 )
        {
            FAIL_hr(mptr_jsop->GetProperty(mptr_EzeScriptDirect, globalObject,pid_proto,&proto_instance),L"GetProperty");
        }
        else if(proto_protoname.compare(L"default")==0 && proto_objname.compare(L"default")!=0)
        {
            InsideData* newdata_item=mptr_mydata->inheritance[L"default"];
            proto_instance=newdata_item->GetTypeObj();
        }
        else
        {
            InsideData* newdata_item=mptr_mydata->objmapping[proto_objname];
            proto_instance=newdata_item->GetTypeObj();
        }

    }
    PropertyId constructorId=GetOrAddPropertyId(cnstrname);
    FAIL_hr(mptr_EzeScriptDirect->CreateConstructor(proto_instance,MyObjectConstructor,constructorId,true,&protofunc),L"CreateConstructor Failed for the Function: "+function_name);

    PropertyId pid=GetOrAddPropertyId(funcname);
    BOOL result;
    FAIL_hr(mptr_typeop->SetProperty(mptr_EzeScriptDirect, globalObject,pid,protofunc,&result),L"SetProperty failed to set Property: "+function_name+L" to the Global Object");

    mptr_data_item->SetPrototypeObj(proto_instance);
    mptr_data_item->ctor_name=function_name;
    mptr_data_item->SetFuncPtrCtor(protofunc);

    mptr_data_item->is_globalobj="false";
    mptr_data_item->has_prototypeobj="true";
    mptr_data_item->ctor_name=function_name;
    mptr_data_item->prototypeobj_name=proto_objname;
    mptr_data_item->proto_protoname=proto_protoname;

    std::pair<std::wstring,bool>inheri_exists;
    inheri_exists.first=proto_objname;



}


BOOL MyScriptDirectTests::CreateTypedObjectWithPrototype(std::wstring ctor_name,std::wstring objname,std::wstring tname)
{
    /*Script Equivalent
    function Foo(somevar){}
    // "this" is the Global object which in case of dom would be window
    this.fooobj=new Foo(somethis);
    Foo.prototype.hello=5678;
    this.Fobj.hello=86597;
    */


    HTYPE newtyperef=0;
    PropertyId pid_obj=-1;
    Var globalObj=0;
    WCHAR* obj_name=const_cast<WCHAR*>(objname.c_str());
    WCHAR* type_name=const_cast<WCHAR*>(tname.c_str());

    //Create a type with the prototype object
    PropertyId nameId=GetOrAddPropertyId(type_name);
    FAIL_hr(mptr_EzeScriptDirect->CreateType(TypeId_Unspecified, nullptr, 0, mptr_data_item->GetPrototypeObj(), nullptr, mptr_typeop, false, nameId, true, &newtyperef), L"Create Type");

    pid_obj=GetOrAddPropertyId(obj_name);

    PropertyId pid_this=GetOrAddPropertyId(L"_this");
    Var _thisval;
    FAIL_hr(mptr_EzeScriptDirect->CreateTypedObject(newtyperef, sizeof(this), true, &_thisval),L"CreateTypeObject");
    void* offset;
    JavascriptTypeId typeId;
    FAIL_hr(mptr_EzeScriptDirect->VarToExtension(_thisval, &offset, &typeId),L"VarToExtension");
    *(void**)offset = this;
    if (extensionOffset == -1)
    {
        extensionOffset = (byte*)offset-(byte*)_thisval;
    }
    FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObj),L"GetGlobalObject");
    FAIL_hr(mptr_jsop->SetProperty(mptr_EzeScriptDirect, globalObj,pid_this,_thisval),L"SetProperty");

    // Add property to the prototype Object using Parse and Execute
    std::wstring w_script=L"this."+objname+L"= new "+ctor_name + L"(_this)";
    wchar_t* l_str=new wchar_t[w_script.size()];
    l_str=const_cast<wchar_t*>(w_script.c_str());

    mptr_data_item->SetType(newtyperef);

    ParseAndExecute(l_str);

    mptr_data_item->typeobj_name=objname;

    std::pair<std::wstring,InsideData*>inheri_obj;
    std::pair<std::wstring,InsideData*>current_obj;

    current_obj.first=mptr_data_item->typeobj_name;
    current_obj.second=mptr_data_item;
    mptr_mydata->objmapping.insert(current_obj);


    if(mptr_data_item->prototypeobj_name.compare(L"default")==0)
    {
        std::pair<std::wstring,InsideData*>inheri_obj1;
        inheri_obj1.first=mptr_data_item->typeobj_name;
        inheri_obj1.second=mptr_data_item;
        mptr_mydata->inheritance.insert(inheri_obj1);
        inheri_obj.first=L"default";
        inheri_obj.second=mptr_data_item;
    }
    else
    {
        inheri_obj.first=mptr_data_item->typeobj_name;
        inheri_obj.second=mptr_mydata->objmapping[mptr_data_item->prototypeobj_name];
    }
    mptr_mydata->inheritance.insert(inheri_obj);
    return true;

}

void MyScriptDirectTests::SetPropertyOnPrototypeInstance(std::wstring ctor_name,std::wstring prop_name,std::wstring value,std::string type,std::string exp_value)
{
    //Check if the prototype instance for the ctor exists
    PropertyId exists_id=-1;
    Var globalObj=0;
    FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObj),L"GetGlobalObject");
    FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(ctor_name.c_str(),&exists_id),L"FindPropertyId");
    if(exists_id>0)
    {
        Var ctorobj=0;
        FAIL_hr(mptr_jsop->GetProperty(mptr_EzeScriptDirect, globalObj,exists_id,&ctorobj),L"GetProperty");
        exists_id=-1;
        FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(L"prototype",&exists_id),L"FindPropertyId");
        if(exists_id>0)
        {
            BOOL result=0;
            FAIL_hr(mptr_typeop->HasOwnProperty(mptr_EzeScriptDirect, ctorobj,exists_id,&result),L"HasOwnProperty");
            if(!result)
            {
                cout<<"The Ctor does not have a prototype object"<<endl;

            }
        }
        else
        {
            cout<<"The Ctor does not have a prototype object"<<endl;

        }

    }
    else
    {
        cout<<"The Ctor does not exist"<<endl;

    }
    std::wstring srcstr=ctor_name+L".prototype."+prop_name+L"="+value;
    wchar_t* l_str=new wchar_t[srcstr.size()];
    l_str=const_cast<wchar_t*>(srcstr.c_str());

    ParseAndExecute(l_str);
    mptr_data_item->proto_prop_name=prop_name;
    mptr_data_item->proto_prop_value=exp_value;
    mptr_data_item->proto_propval_type=type;
    mptr_data_item->is_globalobj="false";
    mptr_data_item->has_prototypeobj="true";


}

std::wstring MyScriptDirectTests::GetPropertyName(PropertyId pid)
{
    //Get or Add Property ID's Various Values
    LPCWSTR prop_name;
    FAIL_hr(mptr_EzeScriptDirect->GetPropertyName(pid,&prop_name),L"GetPropertyName");
    std::wstring w_propname=prop_name;
    return w_propname;
}

void MyScriptDirectTests::DeleteProperty(std::wstring prop_name,std::wstring objname)
{
    try
    {
        Var globalobject=0;
        FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalobject),L"GetGlobalObject");
        if(objname.compare(L"global")==0)
        {
            PropertyId pid_prop=-1;
            FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(prop_name.c_str(),&pid_prop),L"FindPropertyId");
            FAIL_hr(mptr_jsop->DeleteProperty(mptr_EzeScriptDirect, globalobject,pid_prop),L"DeleteProperty");
            if(mbool_dataitem_init==1)
                mptr_data_item->prop_value="";
        }
        else
        {
            Var obj_name=0;
            PropertyId pid_obj=-1;
            PropertyId pid_prop=-1;
            FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(objname.c_str(),&pid_obj),L"FindPropertyId");
            FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(prop_name.c_str(),&pid_prop),L"FindPropertyId");
            FAIL_hr(mptr_jsop->GetProperty(mptr_EzeScriptDirect, globalobject,pid_obj,&obj_name),L"GetProperty");
            FAIL_hr(mptr_jsop->DeleteProperty(mptr_EzeScriptDirect, obj_name,pid_prop),L"DeleteProperty");
            mptr_data_item->prop_value="";
        }

    }
    catch(exception e)
    {
        throw e;
    }

}

void MyScriptDirectTests::ByteCount()
{
    //TODO

}

void MyScriptDirectTests::SetOnScriptError(JsHostOnScriptErrorCallback ptr_function, void* context)
{
    mptr_jsHostArgs->onScriptErrorHelper->SetCallback(ptr_function, context);
}

HRESULT MyScriptDirectTests::CreateErrorObject(JsErrorType errorType, HRESULT hCode, LPCWSTR message, Var *errorObject)
{
    return mptr_EzeScriptDirect->CreateErrorObject(errorType, hCode, message, errorObject);
}

HRESULT MyScriptDirectTests::CollectGarbage(SCRIPTGCTYPE scriptgctype)
{
    return mptr_ActiveScriptGC->CollectGarbage(scriptgctype);
}

Var MyScriptDirectTests::GetGlobalObject()
{
    Var globalObject;
    FAIL_hr(mptr_EzeScriptDirect->GetGlobalObject(&globalObject), L"GetGlobalObject");
    return globalObject;
}

PropertyId MyScriptDirectTests::GetOrAddPropertyId(const wchar_t *const name)
{
    PropertyId propertyId;
    FAIL_hr(mptr_EzeScriptDirect->GetOrAddPropertyId(name, &propertyId), L"GetOrAddPropertyId");
    return propertyId;
}

Var MyScriptDirectTests::GetProperty(const Var instance, const wchar_t *const name)
{
    Var value;
    FAIL_hr(mptr_jsop->GetProperty(mptr_EzeScriptDirect, instance, GetOrAddPropertyId(name), &value), L"GetProperty");
    return value;
}

void MyScriptDirectTests::SetProperty(const Var instance, const wchar_t *const name, const Var value)
{
    FAIL_hr(mptr_jsop->SetProperty(mptr_EzeScriptDirect, instance, GetOrAddPropertyId(name), value), L"SetProperty");
}

bool MyScriptDirectTests::ToBoolean(const Var value)
{
    BOOL converted;
    FAIL_hr(mptr_EzeScriptDirect->VarToBOOL(value, &converted), L"ToBoolean");
    return converted==TRUE;
}

double MyScriptDirectTests::ToDouble(const Var value)
{
    double converted;
    FAIL_hr(mptr_EzeScriptDirect->VarToDouble(value, &converted), L"ToDouble");
    return converted;
}

std::wstring MyScriptDirectTests::ToString(const Var value)
{
    const wchar_t *str;
    unsigned int length;
    FAIL_hr(mptr_EzeScriptDirect->VarToRawString(value, &str, &length), L"ToString");
    return std::wstring(str, length);
}

Var MyScriptDirectTests::ToVar(const bool value)
{
    Var converted;
    FAIL_hr(mptr_EzeScriptDirect->BOOLToVar(value, &converted), L"ToVar");
    return converted;
}

MyScriptDirectTests::~MyScriptDirectTests()
{
    if (mptr_EzeScriptDirect)
    {
        mptr_EzeScriptDirect->Release();
    }
    if (mptr_ActiveScriptParse)
    {
        mptr_ActiveScriptParse->Release();
    }
    if (mptr_ActiveScriptGC)
    {
        mptr_ActiveScriptGC->Release();
    }
    if (mptr_jsop)
    {
        mptr_jsop->Release();
    }
    if (mptr_typeop)
    {
        mptr_typeop->Release();
    }
    delete mptr_mydata;
}
