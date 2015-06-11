/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"

//*****************************************************************************
//  Function:       CreateScriptEngine
//  Purpose:        Demonstrate and test creating a script engine
//  Returns:        true if success, false on failure
//  Notes:          A script engine implements IScriptEngine and allows a
//                  host to interact with scripts and vice versa
//*****************************************************************************
bool CreateScriptEngine()
{
    CComPtr<IScriptEngine> pEngine;
    HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
    IFRET(hr);
    return true;
}

//*****************************************************************************
//  Function:       NativeCallScriptExpression
//  Purpose:        Host calling a script expression and getting results
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of execution contexts, script
//                  parsing, script execution, and return values
//*****************************************************************************
bool NativeCallScriptExpression()
{
    CComPtr<IScriptEngine> pEngine;
    HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
    IFRET(hr);

    CComPtr<IScope> context;
    hr=pEngine->CreateContext(0,&context);
    IFRET(hr);

    CComPtr<IValue> script;
    hr=context->CreateString(L"f=98; c=(5/9)*(f-32);",&script);
    IFRET(hr);

    CComPtr<IObject> func;


    double result;

    //
    // Script text is given to the engine to parse and produce parse nodess
    // In the future, it will be possible to QueryInterface on additional
    // interfaces that expose the parse structures such as parse nodes
    // and associated tokens
    //
    hr = pEngine->Parse(script, context, &func, NULL);
    IFRET(hr);

    CComPtr<IValue> ret;
    hr = func->Call(NULL,NULL,&ret);
    IFRET(hr);

    hr = ret->GetDouble(&result);
    IFRET(hr);

    return (ceil(result) == ceil(36.6));
}

//*****************************************************************************
//  Function:       NativeGetScriptProperty
//  Purpose:        Host gets a property value from script code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of global objects and property access
//*****************************************************************************
bool NativeGetScriptProperty()
{
    CComPtr<IScriptEngine> pEngine;
    HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
    IFRET(hr);

    CComPtr<IScope> context;
    hr=pEngine->CreateContext(0,&context);
    IFRET(hr);

    CComPtr<IValue> script;
    hr=context->CreateString(L"c=25; f=(9/5)*c+32;",&script);
    IFRET(hr);

    CComPtr<IObject> func;


    double result;

    hr = pEngine->Parse(script, context, &func, NULL);
    IFRET(hr);

    CComPtr<IValue> ret;
    hr = func->Call(NULL,NULL,&ret);
    IFRET(hr);

    hr = ret->GetDouble(&result);
    IFRET(hr)

    //
    // Get the global object associated with this context
    //
    CComPtr<IObject> pGlobal;
    hr = context->GetGlobalObject(&pGlobal);
    IFRET(hr);

    CComPtr<IValue> pStringF;
    hr = context->CreateString(L"f", &pStringF);
    IFRET(hr);

    //
    // Get the property named "f" on the global object
    //

    CComPtr<IValue> fVal;
    hr = pGlobal->Get(pStringF, &fVal);
    IFRET(hr);

    hr = fVal->GetDouble(&result);
    IFRET(hr);

    return (result == 77.0);
}

//*****************************************************************************
//  Function:       NativeSetScriptProperty
//  Purpose:        Host sets a property value from script code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of global objects and property access
//*****************************************************************************
bool NativeSetScriptProperty()
{
    CComPtr<IScriptEngine> pEngine;
    HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
    IFRET(hr);

    double result;

    CComPtr<IScope> context;
    hr=pEngine->CreateContext(0,&context);
    IFRET(hr);

    //
    // Get the global object associated with this context
    //
    CComPtr<IObject> pGlobal;
    hr = context->GetGlobalObject(&pGlobal);
    IFRET(hr);

    //
    // Create string for value "f"
    //
    CComPtr<IValue> pStringZ;
    hr = context->CreateString(L"f", &pStringZ);
    IFRET(hr);

    //
    // Create value 99.0
    //
    CComPtr<IValue> dval;
    hr = context->CreateDouble(99.0, &dval);
    IFRET(hr);

    //
    // Set the value of "f" to be equal to 99.0
    //
    hr = pGlobal->Set(pStringZ, dval);
    IFRET(hr);

    //
    // Execute script to get c value
    //

    CComPtr<IValue> script;
    hr=context->CreateString(L"c=(5/9)*(f-32);",&script);
    IFRET(hr);

    CComPtr<IObject> func;
    hr = pEngine->Parse(script, context, &func, NULL);
    IFRET(hr);

    dval.Release();
    hr = func->Call(NULL,NULL,&dval);
    IFRET(hr);
    
    //
    // Get the return result
    //
    hr = dval->GetDouble(&result);
    IFRET(hr);

    return (ceil(result) == ceil(37.2));
}

//*****************************************************************************
//  Function:       ScriptGetNativeProperty
//  Purpose:        Script code to read a native property value
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of native objects
//*****************************************************************************
bool ScriptGetNativeProperty()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  double result;

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  // ........................................................................
  // Native objects are one option for native hosts to inject objects that 
  // can be accessed by scripting. In this approach the object is custom 
  // built and does not require the script engine to create or manage 
  // anything else. 
  //
  // However, native objects require the host to implement more code and have 
  // more control. Custom objects require less  implementation logic and 
  // allow a host to have a single object implement method or property behavior 
  // on behalf of one or more existing scritp objects
  //
  // Custom objects will be explained in other functions and tests
  // ........................................................................
  struct NativeCelsiusObject : public IObject, public IValue
  {
    volatile LONG m_cRef;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObject))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0)
    {
      delete this;
    }
      return cRefs;
    }

    CComPtr<IScriptEngine> m_spEngine;
    CComPtr<IScope> m_context;

    NativeCelsiusObject(IScriptEngine *pEngine)
    {
      m_spEngine = pEngine;
      m_spEngine->CreateContext(0,&m_context);
    }

    HRESULT __stdcall GetContext(IScope** contextRef) {
      *contextRef=m_context;
      return S_OK;
    }
    //
    // This native object only implements IScriptObject::GetValue()
    // which is used to return a variant value based on a string name
    //
    HRESULT __stdcall Get(IValue* name, IValue **valueRef)
    {
      HRESULT hr;
      BOOL isEqual;

      CComPtr<IValue> propName;
      m_context->CreateString(L"degree",&propName);

      CComPtr<IString> propString;
      propName->GetString(&propString);

      hr = propString->IsEqual(name, &isEqual);

      IFRET_ARG(hr);
      if (!isEqual)
    return E_INVALIDARG;
            
      //
      // Create the number variant 34.5 as a value of 'degree'
      //
      return m_context->CreateDouble(34.5, valueRef);
    }

    HRESULT __stdcall Set(IValue* name, IValue *valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyCount(int *pCount)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByName(IValue* name, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByIndex(int index, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetType(TypeID* piType) { 
      *piType=TypeID_Object;
      return S_OK;
    }
    HRESULT __stdcall IsEqual(IValue *pvOther, BOOL *pbIsEqual) { return E_NOTIMPL; }
    HRESULT __stdcall GetBool(BOOL *pBool) { return E_NOTIMPL; }
    HRESULT __stdcall GetDouble(double *pNumber) { return E_NOTIMPL; }
    HRESULT __stdcall GetInt(int *intRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetString(IString **ppString) { return E_NOTIMPL; }
    HRESULT __stdcall Call(IValue* thisArg,IArray* args,IValue** returnValueRef) { return E_NOTIMPL; }
  };

  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);

  //
  // Create a native object
  //
  CComPtr<IValue> poCelsius = new NativeCelsiusObject(pEngine);

  //
  // Set the global property "Celsius" to be the object
  //
  CComPtr<IValue> psCelsius;
  hr = context->CreateString(L"Celsius", &psCelsius);
  IFRET(hr);

  hr = pGlobal->Set(psCelsius, poCelsius);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"w=Celsius.degree;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> dval;
  hr = func->Call(NULL,NULL,&dval);
  IFRET(hr);

    //
    // Get the return result
    //
  hr = dval->GetDouble(&result);
  IFRET(hr); 

  return (result == 34.5);
}

//*****************************************************************************
//  Function:       NativeCallScriptFunction
//  Purpose:        Native code calls a script function and passes it
//                  arguments
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of functions as objects
//*****************************************************************************
bool NativeCallScriptFunction()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  double result;

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);


  CComPtr<IValue> script;
  hr=context->CreateString(L"function ftoc(f) { return (5/9)*(f-32);}",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);


    // ........................................................................
    // Functions are objects in JavaScript. the function declaration in the 
    // script above causes a property on the global object called ftoc
    // to be created. This property should have the type object
    // and can be called by native code
    // ........................................................................
  CComPtr<IValue> pvFtoc;
  CComPtr<IValue> psFtoc;

  hr = context->CreateString(L"ftoc", &psFtoc);
  IFRET(hr);

  hr = pGlobal->Get(psFtoc, &pvFtoc);
  IFRET(hr);

  CComPtr<IObject> poFtoc;
  hr = pvFtoc->QueryInterface(__uuidof(IObject),(void**)&poFtoc);
  IFRET(hr);

  //
  // Create a single number variant to pass to the function
  //
  CComPtr<IValue> pvn97;
  hr = context->CreateDouble(97.0, &pvn97);
  IFRET(hr);

  //
  // Create an arguments array of one to hold the value
  //
  CComPtr<IArray> pvoArgs;
  hr = context->CreateArray(1, NULL, &pvoArgs);
  IFRET(hr);

  hr = pvoArgs->Set(0, pvn97);
  IFRET(hr);

  //
  // Now call the script function from native code passing
  // in the arguments
  //
  CComPtr<IValue> pvReturn;
  hr = poFtoc->Call(NULL, pvoArgs, &pvReturn);
  IFRET(hr);

    //
    // Get the return result
    //
  hr = pvReturn->GetDouble(&result);
  IFRET(hr);

  return (ceil(result) == ceil(36.1));
}

//*****************************************************************************
//  Function:       ScriptCallNativeFunction
//  Purpose:        Script code to call a native function
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of native objects
//*****************************************************************************
bool ScriptCallNativeFunction()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  double result;

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);



    // ........................................................................
    // This uses a native object although we'll see later that using
    // a custom object is much easier and requires less work
    // ........................................................................
  struct CtoF : public IObject, IValue {
        volatile LONG m_cRef;

        HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
        {
            *ppvObject = NULL;

            if (IsEqualGUID(riid, IID_IUnknown)) 
            {
                *ppvObject = reinterpret_cast<void **> (this);
            } 

            if (IsEqualGUID(riid, __uuidof(IObject))) 
            {
                *ppvObject = reinterpret_cast<void **> (this);
            } 

            if (IsEqualGUID(riid, __uuidof(IValue))) 
            {
                *ppvObject = reinterpret_cast<void **> (this);
            } 

            if (*ppvObject) 
            {
                ((IUnknown*)*ppvObject)->AddRef();
                return S_OK;
            }
            return E_NOINTERFACE;
        }

        DWORD __stdcall AddRef()   
        { 
            return InterlockedIncrement(&m_cRef);  
        }
        
        DWORD __stdcall Release()  
        {
            LONG cRefs = 0;
            cRefs = InterlockedDecrement(&m_cRef);
            if (cRefs == 0) 
            {
                delete this;
            }
            return cRefs;
        }

        CComPtr<IScriptEngine> m_spEngine;
        CComPtr<IScope> m_context;

        CtoF(IScriptEngine *pEngine)
        {
          m_spEngine = pEngine;
          m_spEngine->CreateContext(0,&m_context);
        }

    HRESULT __stdcall GetContext(IScope** contextRef) {
      *contextRef=m_context;
      return S_OK;
    }

    HRESULT __stdcall Get(IValue* name, IValue **valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Set(IValue* name, IValue *valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyCount(int *pCount)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByName(IValue* name, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByIndex(int index, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetType(TypeID* piType) { 
      *piType=TypeID_Object;
      return S_OK;
    }
    HRESULT __stdcall IsEqual(IValue *pvOther, BOOL *pbIsEqual) { return E_NOTIMPL; }
    HRESULT __stdcall GetBool(BOOL *pBool) { return E_NOTIMPL; }
    HRESULT __stdcall GetDouble(double *pNumber) { return E_NOTIMPL; }
    HRESULT __stdcall GetInt(int *intRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetString(IString **ppString) { return E_NOTIMPL; }

    HRESULT __stdcall Call(IValue* thisArg,IArray* args,IValue** returnValueRef) { 
      int len;

      // Set the default return to be a null variant
      m_context->GetNull(returnValueRef);
            
      // Ensure we have the right number of arguments
      HRESULT hr = args->GetLength(&len);
      if(FAILED(hr) || len < 1) {
      return hr;
      }

      double c;
      CComPtr<IValue> pvC;
      hr = args->Get(0, &pvC);
      IFRET(hr);

      hr = pvC->GetDouble(&c);
      IFRET(hr);

      double f = (9.0/5.0)*c+32.0;
      hr = m_context->CreateDouble(f, returnValueRef);
      return hr;
    }
  };


  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);

  //
  // Create a native object
  //
  CComPtr<IValue> poCtoF = new CtoF(pEngine);

  //
  // Set the global property "ctof" to be the object
  //
  CComPtr<IValue> psCtoF;
  hr = context->CreateString(L"ctof", &psCtoF);
  IFRET(hr);

  hr = pGlobal->Set(psCtoF, poCtoF);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"f=ctof(22);",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  //
  // Get the return result
  //
  hr = val->GetDouble(&result);
  IFRET(hr);

  return (ceil(result) == ceil(71.6));
}



//*****************************************************************************
//  Function:       ScriptSetNativeProperty
//  Purpose:        Script code to set a native property
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of native objects
//*****************************************************************************
bool ScriptSetNativeProperty()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);


  // ........................................................................
  // This uses a native object although we'll see later that using
  // a custom object is much easier and requires less work
  // ........................................................................
  struct Fahrenheit : public IObject, public IValue
  {
    double f;
    volatile LONG m_cRef;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObject))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0) 
    {
      delete this;
    }
      return cRefs;
    }

    CComPtr<IScriptEngine> m_spEngine;
    CComPtr<IScope> m_context;

    Fahrenheit(IScriptEngine *pEngine)
    {
      m_spEngine = pEngine;
      f = 0.0;
      m_spEngine->CreateContext(0,&m_context);
    }

    HRESULT __stdcall GetContext(IScope** contextRef) {
      *contextRef=m_context;
      return S_OK;
    }
    //
    // This native object only implements IScriptObject::GetValue()
    // which is used to return a variant value based on a string name
    //
    HRESULT __stdcall Get(IValue* name, IValue **valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Set(IValue* name, IValue *value)
    {
      HRESULT hr;
      BOOL isEqual;

      CComPtr<IValue> propName;
      m_context->CreateString(L"f",&propName);

      CComPtr<IString> propString;
      propName->GetString(&propString);

      hr = propString->IsEqual(name, &isEqual);

      IFRET_ARG(hr);
      if (!isEqual)
    return E_INVALIDARG;

      //
      // Create the number variant 34.5 as a value of test
      //
      return value->GetDouble(&f);
    }

    HRESULT __stdcall GetPropertyCount(int *pCount)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByName(IValue* name, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByIndex(int index, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetType(TypeID* piType) { 
      *piType=TypeID_Object;
      return S_OK;
    }
    HRESULT __stdcall IsEqual(IValue *pvOther, BOOL *pbIsEqual) { return E_NOTIMPL; }
    HRESULT __stdcall GetBool(BOOL *pBool) { return E_NOTIMPL; }
    HRESULT __stdcall GetDouble(double *pNumber) { return E_NOTIMPL; }
    HRESULT __stdcall GetInt(int *intRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetString(IString **ppString) { return E_NOTIMPL; }
    HRESULT __stdcall Call(IValue* thisArg,IArray* args,IValue** returnValueRef) { return E_NOTIMPL; }
  };


  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);

  Fahrenheit *poF = new Fahrenheit(pEngine);

  CComPtr<IValue> psFahrenheit;
  hr = context->CreateString(L"Fahrenheit", &psFahrenheit);
  IFRET(hr);

  hr = pGlobal->Set(psFahrenheit, poF);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"Fahrenheit.f=98.3;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  return (poF->f == 98.3);
}


//*****************************************************************************
//  Function:       ScriptGetCustomProperty
//  Purpose:        Script code to get a custom property from native code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of custom objects
//*****************************************************************************
bool ScriptGetCustomProperty()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  //.........................................................................
  // An object type is a native code object that can be used to handle
  // property set / get / call() on one or more properties on one
  // or more real JavaScript objects. This makes it easier for native
  // code to use a script engine to implement basic property and 
  // IObject mechanics while allowing for native extensibility
  //
  // Object types are the easiest (and recommended) approach for native
  // extensibility
  //.........................................................................
  struct CelsiusObjectType : public IObjectType {
    volatile LONG m_cRef;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObjectType))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0) 
    {
      delete this;
    }
      return cRefs;
    }

    CComPtr<IScriptEngine> m_spEngine;
    CComPtr<IScope> m_context;

    CelsiusObjectType(IScriptEngine *pEngine) {
    m_spEngine = pEngine;
    m_spEngine->CreateContext(0,&m_context);
    }

    HRESULT __stdcall GetPolicy(int* policyRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetPropertyCount(int* countRef) { *countRef=1; return S_OK; }
    HRESULT __stdcall GetPropertyInfo(int index,IValue** nameRef,IValue** type) {
      if (index==0) {
    m_context->CreateString(L"ftoc_factor",nameRef);
    if (type!=NULL) {
      *type=NULL;
    }
    return S_OK;
      }
      else return E_INVALIDARG;
    }

    HRESULT __stdcall Get(IObject *instance,  int index, IValue* name,IValue **valueRef) {
      if(index != 0) {
    return E_FAIL;
      }

      m_context->CreateDouble(9.0/5.0, valueRef);
      return S_OK;
    }

    HRESULT __stdcall Set(IObject *instance, int index, IValue* name,IValue* valueRef) {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Call(IObject *instance, 
         int index, 
         IValue* name,
         IValue *thisArg,
         IArray *args,
         IValue **valueRef) { return E_NOTIMPL; }
  };

  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);


  CComPtr<IObjectType> pCustom = new CelsiusObjectType(pEngine);

  CComPtr<IObject> customizedObject;
  hr = context->CreateObject(pCustom,&customizedObject);
  IFRET(hr);

  //
  // Create string for global property "custom"
  //
  CComPtr<IValue> pStringZ;
  hr = context->CreateString(L"custom", &pStringZ);
  IFRET(hr);

  CComPtr<IValue> customVal;
  customizedObject->QueryInterface(__uuidof(IValue),(void**)&customVal);

  //
  // Set the value of "custom" to be the custom object
  //
  hr = pGlobal->Set(pStringZ, customVal);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"x=custom.ftoc_factor;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  double dReturn;
  hr = val->GetDouble(&dReturn);

  return (dReturn == (9.0/5.0));
}

//*****************************************************************************
//  Function:       ScriptSetCustomProperty
//  Purpose:        Script code to set a custom property from native code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of custom objects
//*****************************************************************************
bool ScriptSetCustomProperty()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  //.........................................................................
  // An object type is a native code object that can be used to handle
  // property set / get / call() on one or more properties on one
  // or more real JavaScript objects. This makes it easier for native
  // code to use a script engine to implement basic property and 
  // IObject mechanics while allowing for native extensibility
  //
  // Object types are the easiest (and recommended) approach for native
  // extensibility
  //.........................................................................
  struct CelsiusObjectType : public IObjectType {
    volatile LONG m_cRef;
    double factor;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObjectType))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0) 
    {
      delete this;
    }
      return cRefs;
    }

    CComPtr<IScriptEngine> m_spEngine;
    CComPtr<IScope> m_context;

    CelsiusObjectType(IScriptEngine *pEngine) {
    m_spEngine = pEngine;
        factor = 9.0/5.0;
    m_spEngine->CreateContext(0,&m_context);
    }

    HRESULT __stdcall GetPolicy(int* policyRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetPropertyCount(int* countRef) { *countRef=1; return S_OK; }
    HRESULT __stdcall GetPropertyInfo(int index,IValue** nameRef,IValue** type) {
      if (index==0) {
    m_context->CreateString(L"ftoc_factor",nameRef);
    if (type!=NULL) {
      *type=NULL;
    }
    return S_OK;
      }
      else return E_INVALIDARG;
    }

    HRESULT __stdcall Get(IObject *instance,  int index, IValue* name,IValue **valueRef) {
      return E_NOTIMPL;
      return S_OK;
    }

    HRESULT __stdcall Set(IObject *instance, int index, IValue* name,IValue* value) {
      if(index != 0) {
    return E_FAIL;
      }

      return value->GetDouble(&factor);
    }

    HRESULT __stdcall Call(IObject *instance, 
         int index, 
         IValue* name,
         IValue *thisArg,
         IArray *args,
         IValue **valueRef) { return E_NOTIMPL; }
  };

  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);


  CelsiusObjectType* pCustom = new CelsiusObjectType(pEngine);

  CComPtr<IObject> customizedObject;
  hr = context->CreateObject(pCustom,&customizedObject);
  IFRET(hr);

  //
  // Create string for global property "custom"
  //
  CComPtr<IValue> pStringZ;
  hr = context->CreateString(L"custom", &pStringZ);
  IFRET(hr);

  CComPtr<IValue> customVal;
  customizedObject->QueryInterface(__uuidof(IValue),(void**)&customVal);

  //
  // Set the value of "custom" to be the custom object
  //
  hr = pGlobal->Set(pStringZ, customVal);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"custom.ftoc_factor=1.83;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  double f = pCustom->factor;
  delete pCustom;
  return (f == 1.83);
}

//*****************************************************************************
//  Function:       ScriptCallCustomMethod
//  Purpose:        Script code to call a custom method
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of custom objects
//*****************************************************************************
bool ScriptCallCustomMethod()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  struct HandlerWrapper: public IObject, public IValue {
    volatile LONG m_cRef;
    IObject* pObject;
    int id;
    IObjectType* handler;

    HandlerWrapper(IObject* pObject,IObjectType* handler,int id) {
      this->id=id;
      this->pObject=pObject;
      pObject->AddRef();
      this->handler=handler;
      handler->AddRef();
    }

    ~HandlerWrapper() { pObject->Release(); handler->Release(); }

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObject))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0) 
    {
      delete this;
    }
      return cRefs;
    }

    HRESULT __stdcall GetContext(IScope** contextRef) {
      return pObject->GetContext(contextRef);
      return S_OK;
    }
    //
    // This native object only implements IScriptObject::GetValue()
    // which is used to return a variant value based on a string name
    //
    HRESULT __stdcall Get(IValue* name, IValue **valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Set(IValue* name, IValue *valueRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyCount(int *pCount)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByName(IValue* name, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetPropertyByIndex(int index, IProperty **propertyRef)
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall GetType(TypeID* piType) { 
      *piType=TypeID_Object;
      return S_OK;
    }
    HRESULT __stdcall IsEqual(IValue *pvOther, BOOL *pbIsEqual) { return E_NOTIMPL; }
    HRESULT __stdcall GetBool(BOOL *pBool) { return E_NOTIMPL; }
    HRESULT __stdcall GetDouble(double *pNumber) { return E_NOTIMPL; }
    HRESULT __stdcall GetInt(int *intRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetString(IString **ppString) { return E_NOTIMPL; }
    HRESULT __stdcall Call(IValue* thisArg,IArray* args,IValue** returnValueRef) {
      return handler->Call(pObject,id,NULL,thisArg,args,returnValueRef);
    }
  };

  struct ConvertCustomObject : public IObjectType {
    volatile LONG m_cRef;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject)
    {
      *ppvObject = NULL;

      if (IsEqualGUID(riid, IID_IUnknown)) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (IsEqualGUID(riid, __uuidof(IObjectType))) 
    {
      *ppvObject = reinterpret_cast<void **> (this);
    } 

      if (*ppvObject) 
    {
      ((IUnknown*)*ppvObject)->AddRef();
      return S_OK;
    }
      return E_NOINTERFACE;
    }

    DWORD __stdcall AddRef()   
    { 
      return InterlockedIncrement(&m_cRef);  
    }
        
    DWORD __stdcall Release()  
    {
      LONG cRefs = 0;
      cRefs = InterlockedDecrement(&m_cRef);
      if (cRefs == 0) 
    {
      delete this;
    }
      return cRefs;
    }

    CComPtr<IScriptEngine> m_spEngine;
    CComPtr<IScope> m_context;

    ConvertCustomObject(IScriptEngine *pEngine) {
      m_spEngine = pEngine;
      m_spEngine->CreateContext(0,&m_context);  
    }

    HRESULT __stdcall GetPolicy(int* policyRef) { return E_NOTIMPL; }
    HRESULT __stdcall GetPropertyCount(int* countRef) { *countRef=2; return S_OK; }
    HRESULT __stdcall GetPropertyInfo(int index,IValue** nameRef,IValue** type) {
      if (index==0) {
    m_context->CreateString(L"ctof",nameRef);
    if (type!=NULL) {
      *type=NULL;
    }
    return S_OK;
      }
      else if (index==1) {
    m_context->CreateString(L"ftoc",nameRef);
    if (type!=NULL) {
      *type=NULL;
    }
    return S_OK;
      }
      else return E_INVALIDARG;
    }

    HRESULT __stdcall Get(IObject *instance,int index, IValue* name,IValue **valueRef) {
      *valueRef=new HandlerWrapper(instance,this,index);
      return S_OK;
    }

    HRESULT __stdcall Set(IObject *instance, int index, IValue* name,IValue* valueRef) {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Call(IObject *instance, 
         int index, 
         IValue* name,
         IValue *thisArg,
         IArray *args,
         IValue **valueRef) {
      if((index != 0) && (index != 1)) {
    return E_FAIL;
      }

      int count;
      HRESULT hr;

      hr = args->GetLength(&count);
      if(FAILED(hr) || count < 1)
    {
      return E_FAIL;
    }

      CComPtr<IValue> pArg;
      hr =args->Get(0, &pArg);
      IFRET_ARG(hr);

      double dArg;
      hr = pArg->GetDouble(&dArg);
      IFRET_ARG(hr);

      switch(index) {
    case    0:      // ctof
      {
        double f = (9.0/5.0)*dArg+32.0;
        hr = m_context->CreateDouble(f, valueRef);
        return hr;
      }

    case    1:      // ftoc
      {
        double c = (dArg-32.0)*5.0/9.0;
        hr = m_context->CreateDouble(c, valueRef);
        return hr;
      }
    }
      return E_FAIL;
    }
  };

  //
  // Get the global object associated with this context
  //
  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);


  CComPtr<IObjectType> pCustom = new ConvertCustomObject(pEngine);

  CComPtr<IObject> customizedObject;
  hr = context->CreateObject(pCustom,&customizedObject);
  IFRET(hr);

  CComPtr<IValue> customVal;
  customizedObject->QueryInterface(__uuidof(IValue),(void**)&customVal);

  //
  // Create string for global property "custom"
  //
  CComPtr<IValue> pStringZ;
  hr = context->CreateString(L"custom", &pStringZ);
  IFRET(hr);

  //
  // Set the value of "custom" to be the custom object
  //
  hr = pGlobal->Set(pStringZ, customVal);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"f=custom.ctof(33.4);c=custom.ftoc(f);z=c;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  double dReturn;
  hr = val->GetDouble(&dReturn);
  IFRET(hr);

  return (ceil(dReturn) == ceil(33.4));
}

//*****************************************************************************
//  Function:       NativeGetScriptArrayValue
//  Purpose:        Native code get array value from script code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of array access
//*****************************************************************************
bool NativeGetScriptArrayValue()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"x = [ 3, 2, 1 ];",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  int count;
  CComPtr<IArray> paReturn;
  hr = val->QueryInterface(__uuidof(IArray),(void**)&paReturn);
  IFRET(hr);

  hr = paReturn->GetLength(&count);
  if(FAILED(hr) || count < 3) {
    return false;
  }

  CComPtr<IValue> v1, v2, v3;
  hr = paReturn->Get(0, &v1);
  IFRET(hr);

  hr = paReturn->Get(1, &v2);
  IFRET(hr);

  hr = paReturn->Get(2, &v3);
  IFRET(hr);

  double d1, d2, d3;
  v1->GetDouble(&d1);
  v2->GetDouble(&d2);
  v3->GetDouble(&d3);

  return (d1 == 3.0 && d2 == 2.0 && d3 == 1.0);
}

//*****************************************************************************
//  Function:       NativeSetScriptArrayValue
//  Purpose:        Native code set array value from script code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of array access
//*****************************************************************************
bool NativeSetScriptArrayValue()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"x = [ 3, 2, 1 ];",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  int count;
  CComPtr<IArray> paReturn;
  hr = val->QueryInterface(__uuidof(IArray),(void**)&paReturn);
  IFRET(hr);

  hr = paReturn->GetLength(&count);
  if(FAILED(hr) || count < 3) {
    return false;
  }

  CComPtr<IValue> v2;
  hr = context->CreateDouble(9.0, &v2);
  IFRET(hr);

  hr = paReturn->Set(1, v2);

  CComPtr<IValue> script2;
  hr=context->CreateString(L"z = x[1];",&script2);
  IFRET(hr);

  CComPtr<IObject> func2;
  hr = pEngine->Parse(script2, context, &func2, NULL);
  IFRET(hr);

  CComPtr<IValue> val2;
  hr = func2->Call(NULL,NULL,&val2);
  IFRET(hr);

  double d2;
  hr = val2->GetDouble(&d2);
  IFRET(hr);

  return (d2 == 9.0);
}

//*****************************************************************************
//  Function:       ScriptGetNativeArrayValue
//  Purpose:        Script code get array value from native code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of native array access
//*****************************************************************************
bool ScriptGetNativeArrayValue()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  CComPtr<IArray> pArray;
  hr = context->CreateArray(2, NULL, &pArray);
  IFRET(hr);

  CComPtr<IValue> v0, v1;

  hr = context->CreateDouble(10.0, &v0);
  IFRET(hr);

  hr = context->CreateDouble(24.0, &v1);
  IFRET(hr);

  hr = pArray->Set(0, v0);
  IFRET(hr);

  hr = pArray->Set(1, v1);
  IFRET(hr);


  CComPtr<IValue> psArray;
  hr = context->CreateString(L"MyArray", &psArray);
  IFRET(hr);

  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);

  CComPtr<IValue> arrayVal;
  hr=pArray->QueryInterface(__uuidof(IValue),(void**)&arrayVal);
  IFRET(hr);

  hr = pGlobal->Set(psArray, arrayVal);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"x = MyArray[0] + MyArray[1];",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  double dReturn;
  hr = val->GetDouble(&dReturn);
  IFRET(hr);

  return (dReturn == 34.0);
}

//*****************************************************************************
//  Function:       ScriptSetNativeArrayValue
//  Purpose:        Script code set array value from native code
//  Returns:        true if success, false on failure
//  Notes:          Demonstrates use of native array access
//*****************************************************************************
bool ScriptSetNativeArrayValue()
{
  CComPtr<IScriptEngine> pEngine;
  HRESULT hr = pEngine.CoCreateInstance(__uuidof(StandardScriptEngine));
  IFRET(hr);

  CComPtr<IScope> context;
  hr=pEngine->CreateContext(0,&context);
  IFRET(hr);

  CComPtr<IArray> pArray;
  hr = context->CreateArray(2, NULL, &pArray);
  IFRET(hr);

  CComPtr<IValue> v0, v1;
  hr = context->CreateDouble(10.0, &v0);
  IFRET(hr);

  hr = context->CreateDouble(24.0, &v1);
  IFRET(hr);

  hr = pArray->Set(0, v0);
  IFRET(hr);

  hr = pArray->Set(1, v1);
  IFRET(hr);

  CComPtr<IValue> psArray;
  hr = context->CreateString(L"MyArray", &psArray);
  IFRET(hr);

  CComPtr<IObject> pGlobal;
  hr = context->GetGlobalObject(&pGlobal);
  IFRET(hr);

  CComPtr<IValue> arrayVal;
  hr=pArray->QueryInterface(__uuidof(IValue),(void**)&arrayVal);
  IFRET(hr);

  hr = pGlobal->Set(psArray, arrayVal);
  IFRET(hr);

  CComPtr<IValue> script;
  hr=context->CreateString(L"MyArray[0]=17.7;",&script);
  IFRET(hr);

  CComPtr<IObject> func;
  hr = pEngine->Parse(script, context, &func, NULL);
  IFRET(hr);

  CComPtr<IValue> val;
  hr = func->Call(NULL,NULL,&val);
  IFRET(hr);

  v0.Release();
  hr = pArray->Get(0, &v0);
  IFRET(hr);

  double d0;
  hr = v0->GetDouble(&d0);
  IFRET(hr);

  return (d0 == 17.7);
}

int _tmain(int argc, _TCHAR* argv[]) {
  CoInitialize(NULL);

    typedef bool (*TestFunction)();
    struct Test { LPCSTR name; TestFunction function; };

    Test tests[] = {
        { "CreateScriptEngine", CreateScriptEngine },
        { "NativeCallScriptExpression", NativeCallScriptExpression },
        { "NativeGetScriptProperty", NativeGetScriptProperty },
        { "NativeSetScriptProperty", NativeSetScriptProperty },
        { "ScriptGetNativeProperty", ScriptGetNativeProperty },
        { "NativeCallScriptFunction", NativeCallScriptFunction },
        { "ScriptCallNativeFunction", ScriptCallNativeFunction },
        { "ScriptSetNativeProperty", ScriptSetNativeProperty },
        { "ScriptGetCustomProperty", ScriptGetCustomProperty },
        { "ScriptSetCustomProperty", ScriptSetCustomProperty },
        { "ScriptCallCustomMethod", ScriptCallCustomMethod },
        { "NativeGetScriptArrayValue", NativeGetScriptArrayValue },
        { "NativeSetScriptArrayValue", NativeSetScriptArrayValue },
        { "ScriptGetNativeArrayValue", ScriptGetNativeArrayValue },
        { "ScriptSetNativeArrayValue", ScriptSetNativeArrayValue },
    // TODO: Test chain global object
    // TODO: Native -> script -> Native -> script chain call test
    // TODO: Test garbage collection of native objects
    // TODO: Test parse errors
    // TODO: Test tokenization errors
    // TODO: Test excution errors
    // TODO: Test multiple parse sequences on same engine

    };

  int count = sizeof(tests)/sizeof(tests[0]);
  int errcount = 0;
  for(int t=0; t < count; t++) {
      bool b;
      b = tests[t].function();
      printf("\n%2d - %s (%s)", t+1, tests[t].name, (b) ? "passed" : "failed");
      if(!b) {
    errcount++;
      }
        
  }

  printf("\n");
  if(errcount > 0)
    return -1;

  return 0;

}
