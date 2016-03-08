#pragma once

class Engine
{
public:
    enum Kind { 
        Chakra, 
        LagunaContext, 
        LagunaRuntime, 
        LagunaSameContext, 
        V8, 
        V8Isolates
    };

    virtual ~Engine();
    virtual int ParseScript(char16 *str) = 0;
    virtual void SetupTest() = 0;
    virtual void RunTest() = 0;

    static Engine* CreateEngine(Kind kind);
};


class ChakraEngine : public Engine
{
public:
    ChakraEngine();
    ~ChakraEngine();
    int ParseScript(char16 *str);
    void SetupTest();
    void RunTest();
private:
    // for Chakra
    ScriptSite *m_pChakraSite;
};

class JsRTEngine : public Engine
{
    public:
    JsRTEngine(Engine::Kind engineKind);
    ~JsRTEngine();
    int ParseScript(char16 *str);
    void SetupTest();
    void RunTest();
private:
    JsContextRef m_Context;
    JsRuntimeHandle m_Runtime;
       
    static JsRuntimeHandle g_sharedRuntime;
    static JsContextRef g_sharedContext;

    void CallFunction(char16 *funcName);
};

#ifdef ALLOW_V8
class V8Engine : public Engine
{
public:
    V8Engine(bool isolate);
    ~V8Engine();
    int ParseScript(char16 *str);
    void SetupTest();
    void RunTest();
private:
    v8::Persistent<v8::Context> m_v8context;

    v8::Isolate *m_pIsolate;
};

#endif
