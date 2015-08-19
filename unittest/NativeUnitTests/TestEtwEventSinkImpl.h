#pragma once
#define TEST_ETW_EVENTS

#include "TestEtwEventSink.h"

class MethodEventNames
{
public:
    static const std::wstring Load;
    static const std::wstring Unload;
    static const std::wstring DCStart;
    static const std::wstring DCEnd;
};


class SourceEventNames
{
public:
    static const std::wstring Load;
    static const std::wstring Unload;
    static const std::wstring DCStart;
    static const std::wstring DCEnd;
};


class MethodInfo
{
public:
    void* scriptContextId;
    void* methodStartAddress;
    uint64 methodSize;
    uint methodID;
    uint16 methodFlags;
    uint16 methodAddressRangeID;
    DWORD_PTR sourceID;
    uint line;
    uint column;
    std::wstring name;

public:
    MethodInfo(void* scriptContextId,
               void* methodStartAddress, 
               uint64 methodSize, 
               uint methodID, 
               uint16 methodFlags, 
               uint16 methodAddressRangeID, 
               DWORD_PTR sourceID, 
               uint line, 
               uint column, 
               const wchar_t* methodName) :
            scriptContextId(scriptContextId),
            methodStartAddress(methodStartAddress),
            methodID(methodID),
            methodSize(methodSize),
            methodFlags(methodFlags),
            methodAddressRangeID(methodAddressRangeID),
            sourceID(sourceID),
            line(line),
            column(column)
        {
            this->name = std::wstring(methodName);
        }

    const wchar_t* Name() const
    {
        return name.c_str();
    }

};

class SourceInfo
{
public:
    void* scriptContextId;
    uint64 sourceContext;
    uint sourceFlags;
    std::wstring url;

public:
    SourceInfo(uint64 sourceContext,
        void* scriptContextId,
        uint sourceFlags,
        const wchar_t* url) :
            sourceContext(sourceContext),
            scriptContextId(scriptContextId),
            sourceFlags(sourceFlags)
            {
                this->url = std::wstring(url);
            }

    const wchar_t* Name() const
    {
        return url.c_str();
    }
};

class TestEtwEventSinkImpl : 
    public TestEtwEventSink
{
public:
    void WriteMethodEvent(const wchar_t* eventName, 
        void* scriptContextId,
        void* methodStartAddress, 
        uint64 methodSize, 
        uint methodID, 
        uint16 methodFlags, 
        uint16 methodAddressRangeID, 
        DWORD_PTR sourceID, 
        uint line, 
        uint column, 
        const wchar_t* methodName);

    void WriteSourceEvent(const wchar_t* eventName, uint64 sourceContext, void* scriptContextId, uint sourceFlags, const wchar_t* url);

    void UnloadInstance();

    static TestEtwEventSink* GetInstance(bool trace);

protected:
    template<class T>
    void VerifyEventMap(T eventMap)
    {
        if(eventMap)
        {
            auto it = eventMap->begin();
            if(eventMap->size() > 0)
            {
                this->m_fLastTestCaseFailed = true;
                printf("Unload events missing for:\n");
            }
            while(it != eventMap->end())
            {
                printf(("\t\t%s\n"), it->second->Name());
                delete it->second;
                it++;
            }
            eventMap->clear();
            delete eventMap;
        }
    }

    TestEtwEventSinkImpl(bool trace);

    bool IsTraceOn() const { return trace; }

private:
    typedef std::map<void*, MethodInfo*> MethodEventMap;
    typedef std::map<uint64, SourceInfo*> SourceEventMap;
    MethodEventMap* methodEventMap;
    SourceEventMap* sourceEventMap;
    CRITICAL_SECTION csMethodMap;
    bool trace;

protected:
    bool           m_fLastTestCaseFailed;

    static TestEtwEventSink* Instance;
};

TestEtwEventSink* __stdcall CREATE_EVENTSINK_PROC_NAME(bool trace);

class MockEtwController:
    public TestEtwEventSinkImpl
{
private:
    RundownFunc rundown;      // The PerformRundown callback, used to simulate ETW rundown callback

    HANDLE etwThread;               // Handle to the etw thread
    DWORD etwThreadId;

    static CRITICAL_SECTION s_csController;
    static HANDLE s_terminateRequestEvent;  // An event to signal terminate request to etw thread
    volatile bool etwThreadStarted;         // A flag to indicate the etw thread has actually started
    HANDLE terminatedEvent;                 // An event to signal back etw thread has terminated

private:
    MockEtwController(RundownFunc rundown, bool trace);

    static DWORD WINAPI EtwThreadProc(_In_ LPVOID lpParameter);
    DWORD Run();
    void StartEtwThread();
    void StopEtwThread();
    bool IsEtwThread() const;

public:
    virtual void WriteMethodEvent(const wchar_t* eventName, 
        void* scriptContextId,
        void* methodStartAddress, 
        uint64 methodSize, 
        uint methodID, 
        uint16 methodFlags, 
        uint16 methodAddressRangeID, 
        DWORD_PTR sourceID, 
        uint line, 
        uint column, 
        const wchar_t* methodName);
    virtual void WriteSourceEvent(const wchar_t* eventName, uint64 sourceContext, void* scriptContextId, uint sourceFlags, const wchar_t* url);
    virtual void UnloadInstance();

    static TestEtwEventSink* GetInstance(RundownFunc rundown, bool trace);
};
