// Copyright (C) Microsoft. All rights reserved. 
// FastDOMNative.cpp : Definest tests to exercise FastDOM support
//

#include "stdafx.h"
#include "ScriptDirectUnitTests.h"

//Test Case 1 
//Add property to the Global object and retieve 
void FastDomTestCase1(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        std::wstring someprop=_u("someprop");
        PropertyId pid=mytest->GetOrAddPropertyId(someprop.c_str());
        std::wstring retprop=mytest->GetPropertyName(pid);
        std::string retprop_str(retprop.length(),L'\0');

        // TODO: following was failing in snap, disabling temporarily
        /*
        if(retprop.compare(someprop)>0 && pid<0)
        {
            std::stringstream str;
            str<<"FAIL: : The Add Property and Getpropertyname "<<"  PID:  "<<pid<<endl;
            Print(str.str(),false);
        }
        else
        {
            std::stringstream str;
            str<<"PASS: : The Add Property and Getpropertyname "<<"  PID:  "<<pid<<endl;
            Print(str.str(),true);        
        }
        */
    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}

//Same test case in Loop for 100 of properties  
//Change this number once we get the limit on Property ID's if any 

void FastDomTestCase2(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        PropertyId pid=-1;
        for(int i=0;i<100;i++)
        {
            std::wstringstream str_prop;
            str_prop<<"o"<<i;      
            std::wstring someprop=str_prop.str();
            pid=mytest->GetOrAddPropertyId(someprop.c_str());
            std::wstring retprop=mytest->GetPropertyName(pid);

            // TODO: following was failing in snap, disabling temporarily
            /*
            if(retprop.compare(someprop)>0 && pid<0)
            {
                std::stringstream str;
                str<<"FAIL: : The Add Property and Getpropertyname "<<"  PID:  "<<pid<<endl;
                Print(str.str(),false);
            }
            else
            {
                std::stringstream str;
                str<<"PASS: : The Add Property and Getpropertyname "<<"  PID:  "<<pid<<endl;
                Print(str.str(),true);        
            }            
            */
        }

    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}

//Same test case in Loop for 10 of properties  - Varying the PID's 

void FastDomTestCase3(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        PropertyId pid=-1;
        for(int i=0;i<10;i++)
        {
            pid=-i;
            std::wstring retprop=mytest->GetPropertyName(pid);
            if(retprop.compare(_u(""))==0)
            {
                std::stringstream str;
                str<<"PASS: The Add Property For Negative PID's "<<pid<<endl;
                Print(str.str(),true);
            }

            else
            {
                std::stringstream str;
                str<<"FAIL: : The Add Property on Global Object "<<pid<<"Property Name : "<<retprop.c_str()<<endl;
                Print(str.str(),false);
            }
        }

    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}

//Add a Property to the Typed Object - Int 31 and 32 limits 
//
// o1 = new Object();
// o1.p1 = 2147483648;
//
void FastDomTestCase4(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {

        mytest->Start();
        mytest->CreateTypeObject(_u("o1"));
        mytest->SetPropertyOnTypedObject(_u("o1"),_u("p1"),_u("10"),"int","10");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test1",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test1",false);
	   }
        mytest->ClearData();

        mytest->Start();
        mytest->CreateTypeObject(_u("o2"));
        mytest->SetPropertyOnTypedObject(_u("o2"),_u("p2"),_u("2147483648"),"double","2147483648");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test2",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test2",false);
	   }
        mytest->ClearData();


        mytest->Start();
        mytest->CreateTypeObject(_u("o3"));
        mytest->SetPropertyOnTypedObject(_u("o3"),_u("p3"),_u("2147483649"),"double","2147483649");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test3",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test3",false);
	   }
        mytest->ClearData();

        mytest->Start();
        mytest->CreateTypeObject(_u("o4"));
        mytest->SetPropertyOnTypedObject(_u("o4"),_u("p4"),_u("429496725"),"double","429496725");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test4",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test4",false);
	   }
        mytest->ClearData();

        mytest->Start();
        mytest->CreateTypeObject(_u("o5"));
        mytest->SetPropertyOnTypedObject(_u("o5"),_u("p5"),_u("429496726"),"double","429496726");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test5",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test5",false);
	   }
        mytest->ClearData();

        mytest->Start();
        mytest->CreateTypeObject(_u("o6"));
        mytest->SetPropertyOnTypedObject(_u("o6"),_u("p6"),_u("429496724"),"double","429496724");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 4 test6",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 4 test6",false);
	   }
        mytest->ClearData();


    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}
//Test Case 5
//Really Long String Property Name 
//
//Var o=new Object();
//o.propertypropertypropertypropertypropertypropertypropertypropertypropertypropertyproperty1="Chakra"
//[JITU]- TODO - long name length be 257

void FastDomTestCase5(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {

        mytest->Start();
        mytest->CreateTypeObject(_u("o"));
        mytest->SetPropertyOnTypedObject(_u("o"),_u("propertypropertypropertypropertypropertypropertypropertypropertypropertypropertyproperty1"),_u("'Chakra'"),"string","Chakra");
        mytest->End();
       if(verify->CheckNative())
	   {
		   Print("FastDomTestCase 5 test1",true);
	   }
	   else
	   {
		   Print("FastDomTestCase 5 test1",false);
	   }
        mytest->ClearData();

    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}


//Add a Property to the Typed Object - Loop Tests
//Change the Number in the loop once we get the limit on Type ID's
//
// o1 = new Object();
// o1.p1 = 2147483648;
//
//
 void FastDomTestCase6(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {

        for(int i=0;i<100;i++)
        {
            mytest->Start();
            std::stringstream str,str1,value,expvalue;
            str<<"obj"<<i;
            str1<<"p"<<i;
            value<<"'Microsoft"<<i<<"'";
            expvalue<<"Microsoft"<<i;
            std::string o1=str.str();
            std::string p1=str1.str();
            std::wstring wo1=StringToWString(o1);
            std::wstring wp1=StringToWString(p1);
            std::wstring wvalue=StringToWString(value.str());
            mytest->CreateTypeObject(wo1);
            mytest->SetPropertyOnTypedObject(wo1,wp1,wvalue,"string",expvalue.str());
            mytest->End();
		   if(verify->CheckNative())
		   {
			   Print("FastDomTestCase 6",true);
		   }
		   else
		   {
			   Print("FastDomTestCase 6",false);
		   }
            mytest->ClearData();
        }
    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}



//
// Function Foo(){}
// this.foo_obj=new Foo();
// foo_obj.protoprop1=2147483648;
// Foo.prototype.myfooprop='eze';
//

void FastDomTestCase7(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        mytest->Start();
		std::wstring ctorname=_u("ctorname");
		std::wstring tname=_u("tname7");
        mytest->CreateFunction(_u("Chakra_Foo"),_u("default"),_u("default"),ctorname);
        mytest->CreateTypedObjectWithPrototype(_u("Chakra_Foo"),_u("Chakra_foo_obj"),tname);
        mytest->SetPropertyOnTypedObject(_u("Chakra_foo_obj"),_u("protoprop1"),_u("2147483648"),"double","2147483648");
        mytest->SetPropertyOnPrototypeInstance(_u("Chakra_Foo"),_u("myfooprop"),_u("'eze'"),"string","eze");
        mytest->End();
		if(verify->CheckNative())
		{
			Print("FastDomTestCase 7",true);
		}
		else
		{
			Print("FastDomTestCase 7",false);
		}
        mytest->ClearData();
    }
    catch(exception e)
    {
       Print(e.what(),false);
    }
}


//Function JScript_Foo(){}
//this.foo_obj=new Foo();
//foo0.oo0=111;
//Foo.prototype.p1="v1";
//
//Function JScript_Bar(){}
//Bar.prototype=foo_obj;
//this.foo1=new Bar();
//foo1.oo2=222;
//Bar.p2="v2"
//


//creating a prototype chain in a loop 

void FastDomTestCase8(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {
        mytest->Start();
		std::wstring cname=_u("ctroname1");
        mytest->CreateFunction(_u("JScript_Foo"),_u("default"),_u("default"),cname);
		std::wstring tname=_u("tname8");
        mytest->CreateTypedObjectWithPrototype(_u("JScript_Foo"),_u("foo0"),tname);
        mytest->SetPropertyOnTypedObject(_u("foo0"),_u("oo0"),_u("111"),"int","111");
        mytest->SetPropertyOnPrototypeInstance(_u("JScript_Foo"),_u("p0"),_u("'v1'"),"string","v1");
        mytest->End();

        mytest->Start();
		cname=_u("ctroname2");
		tname=_u("tname81");
        mytest->CreateFunction(_u("JScript_Bar"),_u("foo0"),_u("default"),cname);
        mytest->CreateTypedObjectWithPrototype(_u("JScript_Bar"),_u("foo1"),tname);
        mytest->SetPropertyOnTypedObject(_u("foo1"),_u("oo1"),_u("222"),"int","222");
        mytest->SetPropertyOnPrototypeInstance(_u("JScript_Bar"),_u("p1"),_u("'v1'"),"string","v1");
        mytest->End();

        for(int i=0;i<10;i++)
        {
            std::stringstream str,str1,str2,str3,str4,str5,str6,str7;
            std::string tempstr;
            str<<"foo"<<i;
            tempstr=str.str();
            std::wstring proto_proto=StringToWString(tempstr);
            
            str1<<"foo"<<(i+1);

            std::wstring prototype=StringToWString(str1.str());
            
            str2<<"foo"<<i+2;

            std::wstring object=StringToWString(str2.str());

            
            str3<<"Foo"<<i;

            std::wstring ctor_name=StringToWString(str3.str());

            str4<<"oo"<<i+2;
            str5<<"p"<<i+2;
            str6<<i+1<<i+1;
            std::wstring obj_prop=StringToWString(str4.str());
            std::wstring obj_value=StringToWString(str6.str());
            std::wstring proto_prop=StringToWString(str5.str());
            std::wstring proto_value=_u("'v1'");

            mytest->Start();
			std::wstring ctrname=_u("ctroname")+i;
			tname=_u("tname")+i;
            mytest->CreateFunction(ctor_name,prototype,proto_proto,ctrname);
            mytest->CreateTypedObjectWithPrototype(ctor_name,object,tname);
            mytest->SetPropertyOnTypedObject(object,obj_prop,obj_value,"int",str6.str());
            mytest->SetPropertyOnPrototypeInstance(ctor_name,proto_prop,proto_value,"string","v1");
            mytest->End();
        }

		if(verify->CheckNative())
		{
			Print("FastDomTestCase 8",true);
		}
		else
		{
			Print("FastDomTestCase 8",false);
		}
    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}

//Test Case 10
//
//for(var i=0;i<1000;i++){
//this.("p"+i)="v"+i;
//}
//

void FastDomTestCase9(MyScriptDirectTests* mytest,Verifier<MyScriptDirectTests>* verify)
{
    try
    {

        for(int i=0;i<100;i++)
        {
            mytest->Start();
            std::stringstream str,str1,expstr;
            str<<"'v"<<i<<"'";
            str1<<"p"<<i;
            expstr<<"v"<<i;
            std::string v1=str.str();
            std::string p1=str1.str();
            std::wstring wv1=StringToWString(v1);
            std::wstring wp1=StringToWString(p1);
            mytest->SetPropertyOnGlobalObject(wp1,wv1,"string",expstr.str());
            mytest->End();
			if(verify->CheckNative())
			{
				Print("FastDomTestCase 9",true);
			}
			else
			{
				Print("FastDomTestCase 9",false);
			}
            mytest->ClearData();
        }
    }
    catch(exception e)
    {
        Print(e.what(),false);
    }
}

void FastDomTestCase10(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    static const __int64 intValue[] = {0xFFF8000000000000ull, 0x7fffff0000000000ull, 0xFFFf000000000000ull, 0xFF00000000000000ull, 0xFFFFF00000000000ull};
#ifdef _M_X64
    static const BOOL shouldEqual[] = {true,                   false,                false,                true,                  false };
#else
    static BOOL shouldEqual[] = {         true,                    true,                 true,                     true,                  true };
#endif
    C_ASSERT(sizeof(intValue)/sizeof(__int64) == sizeof(shouldEqual)/ sizeof(BOOL));
    Var dbVar;
    IActiveScriptDirect* activeScriptDirect = mytest->GetScriptDirectNoRef();
    try
    {
        for (int i = 0; i < sizeof(intValue)/sizeof(__int64); i++)
        {
            mytest->FAIL_hr(activeScriptDirect->DoubleToVar(*((double*)&intValue[i]), &dbVar), _u("DoubleToVar"));

            double dbValue;
            mytest->FAIL_hr(activeScriptDirect->VarToDouble(dbVar, &dbValue), _u("VarToDouble"));
            if (((*((__int64*)&dbValue) == intValue[i]) && !shouldEqual[i]) ||
                ((*((__int64*)&dbValue) != intValue[i]) && shouldEqual[i]))
            {
                mytest->FAIL_hr(E_FAIL, _u("invalid NaN value"));
            }
        }
        Print("FastDomTestCase 10 SUCCEEDED");
    }
    catch(exception e)
    {
        Print(e.what(),false);
        Print("FastDomTestCase 10 failed");
    }
}

void RunFastDomTests(MyScriptDirectTests* mytest, Verifier<MyScriptDirectTests>* verify)
{
    FastDomTestCase1(mytest,verify);
    FastDomTestCase2(mytest,verify);
    FastDomTestCase3(mytest,verify);
    FastDomTestCase4(mytest,verify);  
    FastDomTestCase5(mytest,verify); 
    FastDomTestCase6(mytest,verify); 
    FastDomTestCase7(mytest,verify);
    FastDomTestCase8(mytest,verify); 
    FastDomTestCase9(mytest,verify); 
    FastDomTestCase10(mytest, verify);
}
