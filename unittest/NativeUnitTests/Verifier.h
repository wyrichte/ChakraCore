// Copyright (C) Microsoft. All rights reserved.
#pragma once
#include "MyScriptDirectNative.h"
#include<cstdlib>
#include<assert.h>

using namespace std;

template <class T>
class Verifier
{
private:
    MyScriptDirectTests* m_scriptdirect;
    Data* m_data;
    CComPtr<IJavascriptOperations> m_jsop;
    CComPtr<ITypeOperations> m_typeop;
    unsigned int currentAssertionNumber;

    void Print(std::string message)
    {
        cout<<message<<endl;
    }

public:
    BOOL FAIL_hr(HRESULT hres,std::string method)
    {
        if(FAILED(hres))
        {
            std::stringstream fail_str;
            fail_str<<"FAILED:  "<<method<<"  failed HRESULT is :"<<hres;
            throw fail_str.str();
        }
        return true;
    }

    void ResetAssertionCounter()
    {
        currentAssertionNumber = 0;
    }

    template<class T>
    void Assert(const T truth, std::string method)
    {
        ++currentAssertionNumber;
        if(!truth)
        {
            std::stringstream fail_str;
            fail_str
                << "FAILED:  "
                << method
                << " - Assert, #"
                << currentAssertionNumber;
            throw fail_str.str();
        }
    }

    template<class T>
    void AssertEqual(const T actual, const T expected, std::string method)
    {
        ++currentAssertionNumber;
        if(actual != expected)
        {
            std::stringstream fail_str;
            fail_str
                << "FAILED:  "
                << method
                << " - Assert, #"
                << currentAssertionNumber
                << " - Expected: "
                << expected
                << ", Actual: "
                << actual;
            throw fail_str.str();
        }
    }

    template<class T>
    void AssertNotEqual(const T actual, const T notExpected, std::string method)
    {
        ++currentAssertionNumber;
        if(actual == notExpected)
        {
            std::stringstream fail_str;
            fail_str
                << "FAILED:  "
                << method
                << " - Assert, #"
                << currentAssertionNumber
                << " - Not expected: "
                << notExpected;
            throw fail_str.str();
        }
    }

private:
    //Helper Methods
    //CheckVal: verifies the expected and the actual value for each of the types
    BOOL Checkval(Var actval,std::string expval,std::string type)
    {
        try
        {
            HRESULT hres=E_INVALIDARG;
            BOOL result=false;

            if(type.compare("double")==0)
            {

                double actintval=0;
                hres=m_data->activescriptdirect->VarToDouble(actval,&actintval);
                double expintval=atof(expval.c_str());


                if(actintval==expintval)
                {
                    cout<<"PASS: Expected Value: " <<expval <<"    Actual obj value: "<<actintval<< endl;
                    cout<<"\n";
                    std::stringstream str;
                    str<<"PASS: Expected Value: " <<expval <<"    Actual value: "<<actintval<< endl;
                    str<<"\n";
                    Print(str.str());
                    result=true;
                }
                else
                {
                    std::stringstream str;
                    str<<"FAIL: Expected Value: " <<expval <<"    Actual value: "<<actintval<< endl;
                    str<<"\n";
                    Print(str.str());
                    result= false;
                }
            }
            else if(type.compare("int")==0)
            {

                int actintval=0;
                hres=m_data->activescriptdirect->VarToInt(actval,&actintval);
                int expintval=atoi(expval.c_str());


                if(actintval==expintval)
                {
                    cout<<"PASS: Expected Value: " <<expval <<"    Actual obj value: "<<actintval<< endl;
                    cout<<"\n";
                    std::stringstream str;
                    str<<"PASS: Expected Value: " <<expval <<"    Actual value: "<<actintval<< endl;
                    str<<"\n";
                    Print(str.str());
                    result=true;
                }
                else
                {
                    std::stringstream str;
                    str<<"FAIL: Expected Value: " <<expval <<"    Actual value: "<<actintval<< endl;
                    str<<"\n";
                    Print(str.str());
                    result=false;
                }
            }
            else if(type.compare("string")==0)
            {
                BSTR actstrval=L"";
                FAIL_hr(m_data->activescriptdirect->VarToString(actval,&actstrval),"VarToString");
                std::wstring wstrval(actstrval);
                std::string strval(wstrval.begin(),wstrval.end());
                if(strval.compare(expval)==0)
                {
                    std::stringstream str;
                    str<<"PASS: Actual Value: " <<strval.c_str() <<"    Expected obj value: "<<expval.c_str() << endl;
                    str<<"\n";
                    Print(str.str());
                    result=true;
                }
                else
                {
                    std::stringstream str;
                    str<<"FAIL: Actual Value: " <<strval.c_str() <<"    Expected obj value: "<<expval.c_str() << endl;
                    str<<"\n";
                    Print(str.str());
                    result=false;
                }
            }
            else if(type.compare("bool")==0)
            {
                //TODO
            }
            else
            {
                std::stringstream str;
                str<<"Not a Known Type is passed , please check the property type passed"<<endl;
                Print(str.str());
                result=false;

            }
            return result;
        }
        catch(exception e)
        {
            throw e;
        }
    }

    //Typed Object and its properties are verified , does not have a prototype object

    BOOL CheckTypedObject(InsideData* inside_data)
    {
        try
        {
            BOOL result=false;
            std::wstring name=inside_data->prop_name;
            PropertyId pid=-1;
            Var retobjval=0;  // property values of typed obj
            LPCWSTR lname=name.c_str();
            FAIL_hr(m_data->activescriptdirect->GetOrAddPropertyId(lname,&pid),"Find PropertyId");
            if( pid<0)
            {
                std::stringstream fail_str;
                fail_str<<"FAILED: Find Property ID returned a negative pid, PID is: "<<pid<<endl;
                throw fail_str.str();
            }
            FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, inside_data->GetTypeObj(),pid,&retobjval),"Get Property");
            //Verification of all the values
            std::stringstream str_stream;
            std::string propname_str(name.begin(),name.end());
            std::string objname_str(inside_data->typeobj_name.begin(),inside_data->typeobj_name.end());
            str_stream<<"Object Name is:  "<<objname_str <<"  Property Name is: "<<propname_str;
            Print(str_stream.str());
            result=Checkval(retobjval,inside_data->prop_value,inside_data->propval_type);
            return result;
        }
        catch(exception e)
        {
            throw e;
        }

    }

    BOOL CheckGlobalObject(InsideData* inside_data)
    {
        try
        {
            BOOL result=false;
            std::wstring name=inside_data->prop_name;
            PropertyId pid=-1;
            Var globalobjval=0;
            Var retobjval=0;  // property values of typed obj
            LPCWSTR lname=name.c_str();
            FAIL_hr(m_data->activescriptdirect->GetOrAddPropertyId(lname,&pid),"Find PropertyId");
            if( pid<0)
            {
                std::stringstream fail_str;
                fail_str<<"FAILED: Find Property ID returned a negative pid, PID is: "<<pid<<endl;
                throw fail_str.str();
            }
            FAIL_hr(m_data->activescriptdirect->GetGlobalObject(&globalobjval),"GetGlobalObject");

            FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, globalobjval,pid,&retobjval),"Get Property");

            //Verification of all the values
            std::stringstream str_stream;
            std::string propname_str(name.begin(),name.end());
            std::string objname_str(inside_data->typeobj_name.begin(),inside_data->typeobj_name.end());
            str_stream<<"Object Name is:  "<<objname_str <<"  Property Name is: "<<propname_str;
            Print(str_stream.str());
            result=Checkval(retobjval,inside_data->prop_value,inside_data->propval_type);
            return result;
        }
        catch(exception e)
        {
            throw e;
        }

    }


    BOOL CheckPrototypeObj(InsideData* newinside_data,InsideData* inside_data)
    {
        try
        {
            PropertyId pid_obj=-1;
            BOOL objfinal_result=0;
            Var obj_propval=0;

            std::string objname_str(inside_data->typeobj_name.begin(),inside_data->typeobj_name.end());
            std::string propname_str(newinside_data->prop_name.begin(),newinside_data->prop_name.end());
            //Check the property of the Object
            if(inside_data->prop_value.compare("")!=0)
            {
                FAIL_hr(m_data->activescriptdirect->GetOrAddPropertyId(newinside_data->prop_name.c_str(),&pid_obj),"FindPropertyId on Object");
                if(pid_obj<0)
                {
                    std::stringstream fail_str;
                    fail_str<<"FAILED: Find Property ID returned a negative pid, PID is:  "<<pid_obj<<"\n";
                    throw fail_str.str();
                }
                FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, inside_data->GetTypeObj(),pid_obj,&obj_propval),"GetProperty on Object property");
                std::stringstream str_stream;
                str_stream<<"Object Name is:  "<<objname_str <<"  Property Name is: "<<propname_str;
                Print(str_stream.str());
               objfinal_result= Checkval(obj_propval,newinside_data->prop_value,inside_data->propval_type);
            }
            if(objfinal_result)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        catch(exception e)
        {
            throw e;
        }
    }

    //Typed object has a prototype objects, properties aof both the typed object and the prototype are verified

    BOOL CheckPrototypeObj(InsideData* inside_data)
    {
        //check the property of the value for the typeobject
        //check the property for the prototype object
        //do hasproperty on the typed object -true
        //Do get own property on the typed object -false/fail
        try
        {
            PropertyId pid_obj=-1;
            PropertyId pid_proto=-1;
            // property values of typed obj and prototype obj
            Var obj_propval=0;
            Var proto_propval=0;
            Var proto_obj_val=0;

            BOOL hasprop_result=0;
            BOOL objfinal_result=0;
            BOOL protofinal_result=0;
            BOOL proto_objfinal_result=0;

            std::string objname_str(inside_data->typeobj_name.begin(),inside_data->typeobj_name.end());
            std::string propname_str(inside_data->prop_name.begin(),inside_data->prop_name.end());

            std::string proto_objname_str(inside_data->ctor_name.begin(),inside_data->ctor_name.end());
            std::string proto_propname_str(inside_data->proto_prop_name.begin(),inside_data->proto_prop_name.end());
            //Check the property of the Object
            if(inside_data->prop_value.compare("")!=0)
            {
                FAIL_hr(m_data->activescriptdirect->GetOrAddPropertyId(inside_data->prop_name.c_str(),&pid_obj),"FindPropertyId on Object");
                if(pid_obj<0)
                {
                    std::stringstream fail_str;
                    fail_str<<"FAILED: Find Property ID returned a negative pid, PID is:  "<<pid_obj<<"\n";
                    throw fail_str.str();
                }
                FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, inside_data->GetTypeObj(),pid_obj,&obj_propval),"GetProperty on Object property");
                std::stringstream str_stream;
                str_stream<<"Object Name is:  "<<objname_str <<"  Property Name is: "<<propname_str;
                Print(str_stream.str());
                objfinal_result= Checkval(obj_propval,inside_data->prop_value,inside_data->propval_type);


            }
            else
            {
                //TODO
            }

            if(inside_data->proto_prop_value.compare("")!=0)
            {
                FAIL_hr(m_data->activescriptdirect->GetOrAddPropertyId(inside_data->proto_prop_name.c_str(),&pid_proto),"FindPropertyId on Prototype");
                if(pid_proto<0)
                {
                    std::stringstream fail_str;
                    fail_str<<"FAILED: Find Property ID returned a negative pid, PID is:  "<<pid_proto<<"\n";
                    throw fail_str.str();
                }
                FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, inside_data->GetTypeObj(),pid_proto,&proto_obj_val),"GetProperty on Object for proto property");
                FAIL_hr(m_jsop->GetProperty(m_data->activescriptdirect, inside_data->GetPrototypeObj(),pid_proto,&proto_propval),"Get Property on Prototype");
                FAIL_hr(m_jsop->HasProperty(m_data->activescriptdirect, inside_data->GetTypeObj(),pid_proto,&hasprop_result) ,"Has Property on Object");
                if(!hasprop_result)
                {
                    std::stringstream fail_str;
                    fail_str<<"FAILED: Has Property Failed ,Expaected :true  Actual Result:  "<<hasprop_result<<"\n";
                    throw fail_str.str();
                }
                //Check
                std::stringstream str_stream,str_stream1;
                str_stream<<"Object Name is:  "<<objname_str <<"  Property Name is: "<<proto_propname_str;
                Print(str_stream.str());
               proto_objfinal_result= Checkval(proto_obj_val,inside_data->proto_prop_value,inside_data->proto_propval_type);

                str_stream1<<"Object Name is:  "<<proto_objname_str <<"  Property Name is: "<<proto_propname_str;
                Print(str_stream1.str());
                protofinal_result=Checkval(proto_propval,inside_data->proto_prop_value,inside_data->proto_propval_type);
            }
            std::wstring obname_w=inside_data->typeobj_name;
            std::string default_check(obname_w.begin(),obname_w.end());
            do
            {

                InsideData* newinsidedata=m_data->inheritance[obname_w];
                obname_w=newinsidedata->typeobj_name;
                CheckPrototypeObj(newinsidedata,inside_data);
                std::string newobname=WStringToString(obname_w.c_str());
                if(newobname.compare(default_check)!=0)
                {
                    default_check=WStringToString(obname_w.c_str());
                }
                else
                {
                    default_check="default";
                }

            } while(default_check.compare("default")!=0);

            if(objfinal_result && protofinal_result && proto_objfinal_result)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        catch(exception e)
        {
            throw e;
        }
    }

    BOOL ClearData()
    {
        myscriptdirect->mydata->clearall();
        return true;
    }

public:
    Verifier(T* iscriptdirect) : currentAssertionNumber(0)
    {
        try
        {
            m_scriptdirect=dynamic_cast<MyScriptDirectTests*>(iscriptdirect);
            if(!m_scriptdirect)
            {
                std::cout<<"The Object is not a valid MyScriptDirect object";
                throw "Invalid MyScriptDirect Object is passed";
            }

        }
        catch(std::exception e)
        {
            std::cout<<"The Object is not a valid MyScriptDirect object"<<e.what();
            throw e;
        }

    }

    BOOL CheckNative()
    {
        try
        {
            m_data= m_scriptdirect->GetData();
            if (m_jsop)
            {
                m_jsop.Release();
            }
            if (m_typeop)
            {
                m_typeop.Release();
            }
            FAIL_hr(m_data->activescriptdirect->GetJavascriptOperations(&m_jsop), "GetJavascriptOperations");
            FAIL_hr(m_data->activescriptdirect->GetDefaultTypeOperations(&m_typeop), "GetDefaultTypeOperations");
            std::list<InsideData*> inside_data_list=m_data->mydatalist;
            std::list<InsideData*>::iterator it_insidedata;
            it_insidedata=inside_data_list.begin();
            BOOL retval=0;
            for(;it_insidedata!=inside_data_list.end();it_insidedata++)
            {
                InsideData* inside_data=*it_insidedata;

                //Check if it is for a Global Object
                if(inside_data->is_globalobj.compare("true")==0)
                {
                    retval=CheckGlobalObject(inside_data);
                }
                else if(inside_data->has_prototypeobj.compare("true")==0)
                {
                    retval=CheckPrototypeObj(inside_data);
                }
                else //this is just a typed object
                {
                    retval=CheckTypedObject(inside_data);
                }
            }
            return retval;
        }

        catch(exception e)
        {
            throw e;
        }
    }
    ~Verifier()
    {
    }

};

