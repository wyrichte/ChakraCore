// Copyright (C) Microsoft. All rights reserved. 
#include "edgescriptDirect.h"
#include "activscp.h"
#include "activscp_private.h"
#include "activprof.h"
#include "edgejshostnativetest.h"
#include <atlbase.h>
#include <dispex.h>
#include <tchar.h>
#include <list>

void PinObject(Var);
void UnpinObject(Var);

class InsideData
{
  public:
    std::wstring typeobj_name;
    std::wstring prop_name;
    std::string propval_type;

    std::wstring prototypeobj_name;
    std::wstring proto_prop_name;
    std::string proto_propval_type;
    std::wstring ctor_name;
    std::wstring proto_protoname;  

    std::string is_globalobj;
    std::string has_prototypeobj;

    std::string prop_value;
    std::string proto_prop_value;
    
    std::string sscripttotest;

    InsideData() : type(nullptr), typeobj(nullptr), prototypeobj(nullptr), funcptr_ctor(nullptr) {}
    ~InsideData()
    {
        UnpinObject(type);
        UnpinObject(typeobj);
        UnpinObject(prototypeobj);
        UnpinObject(funcptr_ctor);
    }
    HTYPE GetType()
    {
        return type;
    }
    void SetType(HTYPE typeIn)
    {
        UnpinObject(type);
        PinObject(type = typeIn);
    }
    Var GetTypeObj()
    {
        return typeobj;
    }

    void SetTypeObj(Var objIn)
    {
        UnpinObject(typeobj);
        PinObject(typeobj = objIn);
    }

    Var GetPrototypeObj()
    {
        return prototypeobj;
    }

    void SetPrototypeObj(Var objIn)
    {
        UnpinObject(prototypeobj);
        PinObject(prototypeobj = objIn); 
    }

    Var GetFuncPtrCtor()
    {
        return funcptr_ctor;
    }

    void SetFuncPtrCtor(Var objIn)
    {
        UnpinObject(funcptr_ctor);
        PinObject(funcptr_ctor = objIn);
    }


private: 
    HTYPE type;
    Var typeobj;
    Var prototypeobj;
    Var funcptr_ctor;

};
struct Data
{
    CComPtr<IActiveScriptDirect> activescriptdirect; 
    std::list<InsideData*> mydatalist;
    std::map<std::wstring ,InsideData*> inheritance;
    std::map<std::wstring,InsideData*> objmapping;
};

class IMyScriptDirectTests
{
public:
    virtual Data* GetData()=0;
    ~IMyScriptDirectTests(){}
};

