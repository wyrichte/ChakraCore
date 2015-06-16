#include "stdafx.h"

Engine::~Engine()
{
}

Engine* Engine::CreateEngine(Kind kind)
{
    if(kind == Kind::Chakra) 
    {
        return new ChakraEngine();
    } 
    else if(kind == Kind::LagunaContext || kind == Kind::LagunaSameContext || kind == Kind::LagunaRuntime) 
    {
        return new JsRTEngine(kind);
    }
#ifdef ALLOW_V8
    else if(kind == Kind::V8) 
    {
        return new V8Engine(false);
    }
    else if(kind == Kind::V8Isolates) 
    {
        return new V8Engine(true);
    }
#endif
    else 
    {
        Fail("Unknown engine type");
        return NULL; // To fix debug build warning about not all control paths return a value
    }
}

ChakraEngine::ChakraEngine() : m_pChakraSite(NULL)
{
    ScriptSite::Create(&m_pChakraSite);
}

ChakraEngine::~ChakraEngine()
{
    ScriptSite::Shutdown(m_pChakraSite);
    m_pChakraSite->Release();
}

int ChakraEngine::ParseScript(wchar_t *str)
{
    int val;
    HRESULT hr = m_pChakraSite->ParseScript(str);
    if(FAILED(hr))
        Fail("Chakra script execution failed");
    hr = m_pChakraSite->GetGlobalValue(L"output", &val);
    if(FAILED(hr))
        Fail("Chakra value retrieval failed");
    return val;
}

void ChakraEngine::SetupTest()
{
    if(FAILED(m_pChakraSite->InvokeGlobalFunction(L"setup")))
    {
        Fail("Chakra setup() call failed");
    }
}

void ChakraEngine::RunTest()
{
    if(FAILED(m_pChakraSite->InvokeGlobalFunction(L"runTest")))
    {
        Fail("Chakra runTest() call failed");
    }
}

JsRuntimeHandle JsRTEngine::g_sharedRuntime = NULL;
JsContextRef JsRTEngine::g_sharedContext = NULL;

JsRTEngine::JsRTEngine(Engine::Kind engineKind)
{
    JsContextRef tmpContext;

    if(engineKind == Kind::LagunaRuntime)
    {
        if(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, JsRuntimeVersionCurrent, NULL, &m_Runtime) != JsNoError)
            Fail("Failed to create Runtime");
    }
    else
    {
        // Note: not thread safe; relies on test app to reject multi-threaded test scenarios with a shared runtime.
        if(g_sharedRuntime == NULL)
        {
            if(JsCreateRuntime(JsRuntimeAttributeAllowScriptInterrupt, JsRuntimeVersionCurrent, NULL, &g_sharedRuntime) != JsNoError)
                Fail("Failed to create shared Runtime");
        }
        m_Runtime = g_sharedRuntime;
    }

    if(engineKind == Kind::LagunaContext || engineKind == Kind::LagunaRuntime)
    {
        if(JsCreateContext(m_Runtime, &tmpContext) != JsNoError)
            Fail("Failed to create context");

        
        JsAddRef(tmpContext, NULL);
        m_Context = tmpContext;
    }
    else
    {
        // Note: not thread safe; relies on test app to reject multi-threaded test scenarios with a shared context.
        if(g_sharedContext == NULL)
        {
            if(JsCreateContext(m_Runtime, &tmpContext) != JsNoError)
                Fail("Failed to create shared context");

            JsAddRef(tmpContext, NULL);
            g_sharedContext = tmpContext;
        }
        JsAddRef(g_sharedContext, NULL);
        m_Context = g_sharedContext;
    }
}

JsRTEngine::~JsRTEngine()
{
    // Correctly releases the reference whether same or different contexts are used.
    if(JsRelease(m_Context, NULL) != JsNoError)
        Fail("Failed to release Context");
   
    if(g_sharedRuntime == NULL)
    {
        if(JsDisposeRuntime(m_Runtime) != JsNoError)
            Fail("Failed to dispose Runtime");
    }
}

int JsRTEngine::ParseScript(wchar_t *str)
{
    if(JsSetCurrentContext(m_Context) != JsNoError)
        Fail("Failed to set current context");

    JsRef result;
    if(JsRunScript(str, &result) != JsNoError)
        Fail("Failed to run script");

    JsValueRef number;
    if(JsConvertValueToNumber(result, &number) != JsNoError)
        Fail("Failed to convert result to number");

    double val;
    if(JsNumberToDouble(number, &val) != JsNoError)
        Fail("Failed to convert number to double");

    if(JsSetCurrentContext(NULL) != JsNoError)
        Fail("Failed to reset current context");

    return (int)val;
}

void JsRTEngine::CallFunction(wchar_t *funcName)
{
    if(JsSetCurrentContext(m_Context) != JsNoError)
        Fail("Failed to set current context");

    JsValueRef globalObject;
    if(JsGetGlobalObject(&globalObject) != JsNoError)
        Fail("Failed to retrieve global object");

    JsPropertyId propertyId;
    if(JsGetPropertyId(funcName, &propertyId) != JsNoError)
        Fail("Failed to retrieve propertyId");

    JsValueRef property;
    if(JsGetProperty(globalObject, propertyId, &property) != JsNoError)
        Fail("Failed to retrieve property");

    JsValueRef result;
    // Call the function, passing in the global object as 'this'
    if(JsCallFunction(property, &globalObject, 1, &result) != JsNoError)
        Fail("Failed to call function");

    if(JsSetCurrentContext(NULL) != JsNoError)
        Fail("Failed to reset current context");
    
}

void JsRTEngine::SetupTest()
{
    CallFunction(L"setup");
}

void JsRTEngine::RunTest()
{
    CallFunction(L"runTest");
}

#ifdef ALLOW_V8
V8Engine::V8Engine(bool isolate) : m_pIsolate(NULL)
{
    if(isolate)
    {
        m_pIsolate = v8::Isolate::New();
        m_pIsolate->Enter();
        v8::V8::Initialize();
    }

    m_v8context = v8::Context::New();

    if(m_pIsolate)
    {
        m_pIsolate->Exit();
    }
}

V8Engine::~V8Engine()
{
    if(m_pIsolate)
    {
        m_pIsolate->Enter();
    }

    m_v8context.Dispose();
    m_v8context.Clear();

    if(m_pIsolate)
    {
        m_pIsolate->Exit();
        m_pIsolate->Dispose();
        m_pIsolate = NULL;
    }
}

int V8Engine::ParseScript(wchar_t *str)
{
    if(m_pIsolate)
    {
        m_pIsolate->Enter();
    }

    v8::HandleScope handle_scope;
    v8::Context::Scope scope(m_v8context);
    v8::Handle<v8::String> source = v8::String::New(str);
    v8::Handle<v8::Script> script = v8::Script::Compile(source);
    v8::Handle<v8::Value> result = script->Run();
    int val = result->ToInteger()->Value();

    if(m_pIsolate)
    {
        m_pIsolate->Exit();
    }

    return val;
}

void V8Engine::SetupTest()
{
    if(m_pIsolate)
    {
        m_pIsolate->Enter();
    }

    v8::HandleScope handle_scope;
    v8::Context::Scope scope(m_v8context);
    v8::Local<v8::Value> val = m_v8context->Global()->Get(v8::String::New("setup"));
    v8::Handle<v8::Function> func = val.As<v8::Function>();

    func->Call(m_v8context->Global(), 0, nullptr);

    if(m_pIsolate)
    {
        m_pIsolate->Exit();
    }
}

void V8Engine::RunTest()
{
    if(m_pIsolate)
    {
        m_pIsolate->Enter();
    }

    v8::HandleScope handle_scope;
    v8::Context::Scope scope(m_v8context);

    v8::Local<v8::Value> val = m_v8context->Global()->Get(v8::String::New("runTest"));
    v8::Handle<v8::Function> func = val.As<v8::Function>();

    func->Call(m_v8context->Global(), 0, nullptr);

    if(m_pIsolate)
    {
        m_pIsolate->Exit();
    }
}

#endif // ALLOW_V8