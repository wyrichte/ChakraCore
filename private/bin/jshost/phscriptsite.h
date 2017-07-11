/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include <cor.h>
#include "DelayLoadLibrary.h"
#include "JsHostScriptSite.h"
#include "docobj.h"
#include <string>

// Global buffer for VirtualAlloc used by a test, instantiation is in phscriptsite.cpp; while clean up in final GC.
extern LPVOID UTF8BoundaryTestBuffer;

class SimpleSourceMapper;
typedef SimpleSourceMapper * SimpleSourceMapperRef;
extern SimpleSourceMapperRef UTF8SourceMapper;

class DelayLoadWinRt;
class DelayLoadWinRtTypeResolution;
class Debugger;

struct JsFile
{
    JsFile();
    ~JsFile();
public:
    std::wstring m_fileName;
    std::wstring m_sourceContent; // Will be populated when under debugger.
    CComPtr<IDebugDocumentHelper> m_debugDocumentHelper;
};

class ExceptionData
{
public:
    JsErrorType errorType;
    char16 *description;
    Var thrownObject;

    ExceptionData()
    {
        this->errorType = JavascriptError;
        this->description = nullptr;
        this->thrownObject = nullptr;
    }

    ~ExceptionData()
    {
        delete[] this->description;
        this->thrownObject = nullptr;
    }
};

class JsHostActiveScriptSite : IActiveScriptSite, IActiveScriptSiteDebug, IActiveScriptSiteDebugHelper, IJsHostScriptSite, ISCAHost, IHeapEnumHost, IOleCommandTarget, 
    IActiveScriptDirectSite, IActiveScriptDirectHost
{
    PREVENT_COPY(JsHostActiveScriptSite)

public:
    struct MappingInfo
    {
        HANDLE fileHandle;
        HANDLE fileMapping;
        LPVOID base;
    };

private:
    unsigned long refCount;
    DWORD jsHostScriptSiteCookie;
    DWORD activeScriptCookie;
    DWORD byteCodeGenCookie;
    DWORD globalObjectIDispatchExCookie;
    DWORD javascriptDispatchCookie;
    DelayLoadWinRt * m_WinRTLibrary;
    static const DWORD freeOnShutdownCount = 128;
    static DWORD nextFreeOnShutdownSlot;
    static LPVOID freeOnShutdown[freeOnShutdownCount];
    static DWORD nextUnmapOnShutdownSlot;
    static MappingInfo unmapOnShutdown[freeOnShutdownCount];
    DWORD nextDeleteSimpleSourceMappersOnShutDown;
    SimpleSourceMapper* deleteSimpleSourceMappersOnShutDown[freeOnShutdownCount];
    std::map<std::wstring, ModuleRecord> moduleRecordMap;
    std::map<std::wstring, std::wstring> moduleSourceMap;
    std::map<ModuleRecord, std::wstring> moduleDirMap;
    std::map<DWORD_PTR, std::wstring> scriptDirMap;

    Js::DelayLoadWinRtString * m_WinRTStringLibrary;
    DelayLoadWinRtTypeResolution * m_WinRTTypeResolutionLibrary;
    JsHostOnScriptErrorHelper m_onScriptErrorHelper;
    std::map<DWORD_PTR,JsFile> m_fileMap;
    DWORD_PTR m_dwNextSourceCookie;  // A simple counter when not debugging, otherwise holds source cookie
    DWORD_PTR m_dwCurrentSourceCookie; // This will be used to for sending notification when on ParseScriptText.
    BOOL wasClosed;
    BOOL isEvalAllowed;
    IActiveScript* activeScript;

    // Tracks whether we have performed source rundown or not.
    bool m_didSourceRundown;

    // Tracks if the host is in debug mode or not.
    bool m_isHostInDebugMode;

    bool m_isDiagnosticsScriptSite;

    static const DWORD s_profilerCookie = 111;
    WORD domainId; // Number to identify domain of the file, different number mean coming from different doamins, used for CMDID_SCRIPTSITE_SID checks

    JsHostActiveScriptSite();
    ~JsHostActiveScriptSite();

    HRESULT CreateScriptEngine(bool isPrimary = true);

    HRESULT LoadScriptFromFile(LPCOLESTR filename, Var* errorObject = nullptr, bool isModuleCode = false);
    HRESULT LoadScriptFromString(LPCOLESTR contents, _In_opt_bytecount_(cbBytes) LPBYTE pbUtf8, UINT cbBytes, _Out_opt_ bool* pUsedUtf8, char16 *fullPath = nullptr);

    HRESULT LoadModuleFromString(bool isUtf8, 
        LPCWSTR fileName, UINT fileNameLength, LPCWSTR contentRaw, UINT byteLength, Var* errorObject, LPCWSTR fullName = nullptr);

    HRESULT StopScriptEngine();

    void RegisterSimpleSourceMapperToDeleteOnShutdown(SimpleSourceMapper *mapper);
    void RegisterToFreeOnShutdown(LPVOID mem);
    void RegisterToUnmapOnShutdown(MappingInfo info);

    DWORD_PTR AddUrl(_In_z_ LPCWSTR url);

    HRESULT RegisterDebugDocuments(CComPtr<IActiveScript>& activeScript, std::vector<SourceContextPair>& cookiePairs);
    void UpdateFileMapTable(std::vector<SourceContextPair>& cookiePairs, SourceContextPair* outPairArray);

    static char16* GetDir(LPCWSTR fullPath, __out char16* const fullDir);

    STDMETHODIMP FetchImportedModuleHelper(
        /* [in] */ ModuleRecord referencingModule,
        /* [in] */ LPCWSTR specifier,
        /* [in] */ unsigned long specifierLength,
        /* [out] */ ModuleRecord *dependentModuleRecord,
        /* [in] */ LPCWSTR refdir = nullptr);

public:

    bool delegateErrorHandling = false;
    ExceptionData *lastException = nullptr;

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IActiveScriptSite interfaces
    STDMETHODIMP GetLCID(LCID *plcid) { return E_NOTIMPL; }
    STDMETHODIMP GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    STDMETHODIMP GetDocVersionString(BSTR *pbstrVersion) { return E_NOTIMPL; }
    STDMETHODIMP OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo) { return E_NOTIMPL; }
    STDMETHODIMP OnStateChange(SCRIPTSTATE ssScriptState) { return E_NOTIMPL; }
    STDMETHODIMP OnScriptError(IActiveScriptError *pscripterror);
    STDMETHODIMP OnEnterScript();
    STDMETHODIMP OnLeaveScript();

    // IActiveScriptSiteDebug interfaces
    STDMETHODIMP GetDocumentContextFromPosition(DWORD_PTR dwSourceContext, ULONG uCharacterOffset,
                                                             ULONG uNumChars, IDebugDocumentContext** ppsc);
    STDMETHODIMP GetApplication(IDebugApplication**  ppda);
    STDMETHODIMP GetRootApplicationNode(IDebugApplicationNode**  ppdanRoot);
    STDMETHODIMP OnScriptErrorDebug(IActiveScriptErrorDebug*  pErrorDebug, BOOL* pfEnterDebugger,
                                                  BOOL* pfCallOnScriptErrorWhenContinuing);

    // IActiveScriptSiteDebugHelper interfaces
    STDMETHODIMP IsInDebugMode(BOOL *pfDebugMode);
    STDMETHODIMP GetApplicationNode(IDebugApplicationNode **ppdan);

    // IJsHostScriptSite interfaces
    STDMETHODIMP LoadScriptFile(LPCOLESTR filename);
    STDMETHODIMP LoadModuleFile(LPCOLESTR filename, BOOL useExistingModuleRecord, byte** errorObject, DWORD_PTR referenceModuleRecord);
    STDMETHODIMP LoadScript(LPCOLESTR script);
    STDMETHODIMP LoadModule(LPCOLESTR script, byte** errorObject);
    STDMETHODIMP RegisterModuleSource(LPCOLESTR moduleIdentifier, LPCOLESTR script);
    STDMETHODIMP InitializeProjection();
    STDMETHODIMP RegisterCrossThreadInterface();

    // ISCAHost interfaces
    STDMETHODIMP CreateObject(ISCAContext* context, SCATypeId typeId, Var *instance);
    STDMETHODIMP DiscardProperties(ISCAContext* context, SCATypeId typeId, ISCAPropBag* propbag);

    // IHeapEnumHost interfaces
    STDMETHODIMP BeginEnumeration();

    // IOleCommandTarget interfaces
    STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut);

    // IActiveScriptDirectHost interface
    STDMETHODIMP ValidateCall(
        /* [in] */ __RPC__in_opt IActiveScriptSite *src,
        /* [in] */ __RPC__in_opt IActiveScriptSite *dest,
        /* [in] */ __RPC__in Var instance) {
        return E_NOTIMPL;
    }

    void __stdcall EnqueuePromiseTask(Var task);

    STDMETHODIMP FetchImportedModule(
        /* [in] */ __RPC__in ModuleRecord referencingModule,
        /* [in] */ __RPC__in LPCWSTR specifier,
        /* [in] */ unsigned long specifierLength,
        /* [out] */ __RPC__deref_out_opt ModuleRecord *dependentModuleRecord);

    STDMETHODIMP FetchImportedModuleFromScript(
        /* [in] */ __RPC__in DWORD_PTR dwReferencingSourceContext,
        /* [in] */ __RPC__in LPCWSTR specifier,
        /* [in] */ unsigned long specifierLength,
        /* [out] */ __RPC__deref_out_opt ModuleRecord *dependentModuleRecord);

    STDMETHODIMP NotifyModuleReady(
        /* [in] */ __RPC__in ModuleRecord referencingModule,
        /* [in] */ __RPC__in Var exceptionVar);

    HRESULT GetActiveScript(IActiveScript ** activeScript);
    HRESULT GetActiveScriptDirect(IActiveScriptDirect ** activeScriptDirect);

    HRESULT GetGlobalObjectDispatchEx(IDispatchEx ** globalObject);
    HRESULT GetByteCodeGenerator(IActiveScriptByteCode ** byteCodeGen);

    JsHostOnScriptErrorHelper* GetOnScriptErrorHelper() { return &m_onScriptErrorHelper; }
    HRESULT InitializeDebugDocument(LPCWSTR scriptCode, BSTR filename);

    // This functions checks if the WScript.Attach() have called before, in that case it will go the debug mode.
    HRESULT CheckDynamicAttach();

    // This functions checks if WScript.Detach() was called, in which case it will leave debug mode.
    HRESULT CheckDynamicDetach();

    HRESULT StartScriptProfiler();
    HRESULT StopScriptProfiler();

    // Performs a rundown of sources and puts the engine into source rundown mode.
    HRESULT CheckPerformSourceRundown();

    HRESULT STDMETHODCALLTYPE CheckEvalRestriction();
    HRESULT STDMETHODCALLTYPE HostExceptionFromHRESULT(HRESULT hr, Var *outError);
    void SetEvalAllowed(BOOL result) { this->isEvalAllowed = result; }

    void SetDiagnosticsScriptSite(bool set) { this->m_isDiagnosticsScriptSite = true; }
    bool IsDiagnosticsScriptSite() const { return this->m_isDiagnosticsScriptSite; }

    static HRESULT Create(JsHostActiveScriptSite ** scriptSite, bool actAsDiagnosticsHost, bool isPrimary, WORD domainId);
    static HRESULT Shutdown(JsHostActiveScriptSite * scriptSite);
    static LPCWSTR MapExternalToES6ErrorText(JsErrorType externalErrorType);   
    static void Terminate();
    void RegisterScriptDir (DWORD_PTR sourceContext, char16* const fullDir);

    void SetDomainId(WORD domainId) { this->domainId = domainId; }
};

class SimpleSourceMapper : public IActiveScriptByteCodeSource
{
private:
    BYTE* sourceCode;
    DWORD sourceCodeSize;
    ULONG currentRefCount;

public:
    void Initialize(BYTE* sourceCode, DWORD sourceCodeSize)
    {
        this->sourceCode = sourceCode;
        this->sourceCodeSize = sourceCodeSize;
        this->currentRefCount = 0;
    }

    ULONG GetRefCount()
    {
        return currentRefCount;
    }

    virtual HRESULT STDMETHODCALLTYPE MapSourceCode( 
            /* [size_is][size_is][out] */ BYTE **sourceCode,
            /* [out] */ DWORD *pdwSourceCodeSize) override
    {
         *sourceCode = this->sourceCode;
         *pdwSourceCodeSize = this->sourceCodeSize;
         return S_OK;
    }
        
    virtual void STDMETHODCALLTYPE UnmapSourceCode( void) override
    {
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override
    {
        *ppvObject = (IActiveScriptByteCodeSource *)this;
        return S_OK;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( void) override
    {
        return ++currentRefCount;
    }

    virtual ULONG STDMETHODCALLTYPE Release( void) override
    {
        AssertMsg(currentRefCount > 0, "IActiveScriptByteCodeSource is being released more times than addref was called.");

        return --currentRefCount;
    }
};


class ScriptOperations : public ITypeOperations
{
private:
    long refCount;
    static OperationUsage defaultUsage;
    ITypeOperations* defaultOperations;

public:
    ScriptOperations(ITypeOperations* defaultScriptOperations);
    ~ScriptOperations();

    STDMETHODIMP QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) ;
    STDMETHODIMP_(ULONG) AddRef( void) ;

    STDMETHODIMP_(ULONG) Release( void);

    STDMETHODIMP GetOperationUsage( 
        /* [out] */ OperationUsage *flags);

    STDMETHODIMP HasOwnProperty( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result);

    STDMETHODIMP GetOwnProperty( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent);

    STDMETHODIMP GetPropertyReference( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var *value,
        /* [out] */ BOOL *propertyPresent);

    STDMETHODIMP SetProperty( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var value,
        /* [out] */ BOOL *result);

    STDMETHODIMP DeleteProperty( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result);

    STDMETHODIMP GetOwnItem( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ Var *value,
        /* [out] */ BOOL *itemPresent);

    STDMETHODIMP SetItem( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [in] */ Var value,
        /* [out] */ BOOL *result);

    STDMETHODIMP DeleteItem( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var index,
        /* [out] */ BOOL *result);

    STDMETHODIMP GetEnumerator( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [out] */ IVarEnumerator **enumerator);

    STDMETHODIMP IsEnumerable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result);

    STDMETHODIMP IsWritable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result);

    STDMETHODIMP IsConfigurable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ BOOL *result);

    STDMETHODIMP SetEnumerable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value);

    STDMETHODIMP SetWritable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value);

    STDMETHODIMP SetConfigurable( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ BOOL value);

    STDMETHODIMP SetAccessors( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [in] */ Var getter,
        /* [in] */ Var setter);

    STDMETHODIMP GetSetter( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ PropertyId propertyId,
        /* [out] */ Var* setter,
        /* [out] */ ::DescriptorFlags* flags);

    STDMETHODIMP Equals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result);
        
    STDMETHODIMP StrictEquals( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ Var other,
        /* [out] */ BOOL *result);
        
    STDMETHODIMP QueryObjectInterface( 
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var instance,
        /* [in] */ REFIID riid,
        /* [out] */ void** ppvObj);

    STDMETHODIMP GetInitializer(
        /* [out] */ InitializeMethod * initializer,
        /* [out] */ int * initSlotCapacity,
        /* [out] */ BOOL * hasAccessors);

    STDMETHODIMP GetFinalizer(
        /* [out] */ FinalizeMethod * finalizer);

    STDMETHODIMP HasInstance(
        /* [in] */ __RPC__in_opt IActiveScriptDirect *scriptDirect,
        /* [in] */ Var constructor,
        /* [in] */ Var instance, 
        /* [out] */ BOOL* result);

    STDMETHODIMP GetNamespaceParent(
        /* [in] */ Var instance,
        /* [out] */ Var* namespaceParent);

};

class DelayLoadWinRt : public Js::DelayLoadLibrary
{
private:
    // Winrt specific functions
    typedef HRESULT FNCActivateInstance(HSTRING, IInspectable**);
    typedef FNCActivateInstance* PFNCActivateInstance;
    PFNCActivateInstance m_pfnActivateInstance;

    typedef HRESULT FNGetActivationFactory(HSTRING, REFIID, PVOID*);
    typedef FNGetActivationFactory* PFNGetActivationFactory;
    PFNGetActivationFactory m_pfnGetActivationFactory;

public:
    DelayLoadWinRt() : Js::DelayLoadLibrary(_u("api-ms-win-core-winrt-l1-1-0.dll")), m_pfnActivateInstance(NULL), m_pfnGetActivationFactory(NULL) { }
    virtual ~DelayLoadWinRt() { }

    HRESULT ActivateInstance(HSTRING activatableClassId, __deref_out IInspectable ** instance);
    HRESULT GetActivationFactory(__in HSTRING activatableClassId, __in REFIID id, __deref_out PVOID* factory);
};

class DelayLoadWinRtTypeResolution : public Js::DelayLoadLibrary
{
private:
    typedef HRESULT FNCRoResolveNamespace(const HSTRING, const HSTRING, const DWORD, const HSTRING*, DWORD*, HSTRING**, DWORD*, HSTRING**);
    typedef FNCRoResolveNamespace* PFNCRoResolveNamespace;
    PFNCRoResolveNamespace m_pfnRoResolveNamespace;

public:
    DelayLoadWinRtTypeResolution() : Js::DelayLoadLibrary(_u("api-ms-win-ro-typeresolution-l1-1-0.dll")), m_pfnRoResolveNamespace(NULL) { }
    virtual ~DelayLoadWinRtTypeResolution() { }

    HRESULT RoResolveNamespace(
        __in_opt const HSTRING namespaceName, 
        __in_opt const HSTRING windowsMetaDataPath,
        __in const DWORD packageGraphPathsCount,
        __in_opt const HSTRING *packageGraphPaths,
        __out DWORD *metaDataFilePathsCount,
        HSTRING **metaDataFilePaths,
        __out DWORD *subNamespacesCount,
        HSTRING **subNamespaces);
};

typedef enum
{
    TRO_NONE = 0x00,
    TRO_RESOLVE_TYPE = 0x01,
    TRO_RESOLVE_NAMESPACE = 0x02,
    TRO_RESOLVE_TYPE_AND_NAMESPACE = TRO_RESOLVE_TYPE | TRO_RESOLVE_NAMESPACE
} TYPE_RESOLUTION_OPTIONS;

// This is the base path where to look for *.winmd files
// Will leverage the WIN_JSHOST_METADATA_BASE_PATH env. var if present.
// (note: the '.' is intended, to support seemlessly the scenarios where the env. var is defined or not, with or w/o trailing '\')
#define WIN_JSHOST_METADATA_BASE_PATH _u("%WIN_JSHOST_METADATA_BASE_PATH%.\\")

class ActiveScriptDirectHost : public IActiveScriptProjectionHost, IActiveScriptProjectionTelemetryHost
{
private:
    long _refCount;
    bool _closeWasCalled;

#if DBG
    bool _isWin8; // True if we're running on Win8 or later, false otherwise
#endif

    DelayLoadWinRt m_WinRTLibrary;
    Js::DelayLoadWinRtString m_WinRTStringLibrary;
    DelayLoadWinRtTypeResolution m_WinRTTypeResolutionLibrary;

    // Private REXT metadata helper methods
    STDMETHODIMP ValidateNameFormat(__in const HSTRING hstrFullName);

    STDMETHODIMP FindTypeInMetaDataFile(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __in PCWSTR pszCandidateFilePath,
    __in TYPE_RESOLUTION_OPTIONS resolutionOptions,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef);

    STDMETHODIMP FindTypeInDirectory(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __in PCWSTR pszDirectoryPath,
    __out_opt HSTRING *phstrMetaDataFilePath,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef);

    STDMETHODIMP FindType(
    __in IMetaDataDispenserEx *pMetaDataDispenser,
    __in PCWSTR pszFullName,
    __out_opt HSTRING *phstrMetaDataFilePath,
    __deref_out_opt IMetaDataImport2 **ppMetaDataImport,
    __out_opt mdTypeDef *pmdTypeDef);

    STDMETHODIMP TelemetryIncrement(DWORD dataID, DWORD count);   
    // set the SQM data for given dataID; corresponds to calling SqmSet(HSESSION, DWORD, DWORD)
    STDMETHODIMP TelemetrySet(DWORD dataID, DWORD value);   


public:
    ActiveScriptDirectHost();
    ~ActiveScriptDirectHost();

    STDMETHODIMP GetTypeMetaDataInformation(LPCWSTR typeName, IUnknown** metaDataInformation, DWORD* typeDefToken, BOOL* isVersioned);
    STDMETHODIMP CreateTypeInstance(LPCWSTR typeName, IUnknown** instance);
    STDMETHODIMP CreateTypeFactoryInstance(LPCWSTR typeName, IID factoryID, IUnknown** instance);
    STDMETHODIMP TypeInstanceCreated(IUnknown* instance);
    STDMETHODIMP Close();
    STDMETHODIMP GetNamespaceChildren( 
        __in LPCWSTR fullNamespace,
        __out DWORD *metaDataImportCount,
        __deref_out_ecount_full(*metaDataImportCount) IUnknown ***metaDataImport,
        __out DWORD *childrenNamespacesCount,
        __deref_out_ecount_full(*childrenNamespacesCount) LPWSTR **childrenNamespaces,
        __deref_out_ecount_full(*metaDataImportCount) BOOL **metaDataImportIsVersioned);


    // Code from REX to get metadata info
    STDMETHODIMP RoGetMetaDataFile(
    __in const HSTRING name,
    __in_opt IMetaDataDispenserEx *metaDataDispenser,
    __out_opt HSTRING *metaDataFilePath,
    __deref_out_opt IMetaDataImport2 **metaDataImport,
    __out_opt mdTypeDef *typeDefToken);

    // IUnknown members
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
};
