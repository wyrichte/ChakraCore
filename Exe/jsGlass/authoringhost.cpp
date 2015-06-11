/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "ieisos.h"

template<class T>
inline void ReleasePtr(T*& ptr) 
{
    T* t = ptr;
    ptr = NULL;
    if (t) t->Release();
}

template<class T>
inline void ReleaseArray(T**& a, int length)
{
    if (a)
    {
        T** t = a;
        a = NULL;
        for (int i = 0; i < length; i++)
            ReleasePtr(t[i]);
        delete [] t;
    }
}

wchar_t * ToText(int number)
{
    if (number == 0) return L"0";
    static wchar_t buffer[100];
    buffer[99] = 0;
    wchar_t *p = &buffer[98];
    bool isNeg = number < 0;
    if (isNeg) number = -number;
    while (number > 0 && p >= buffer)
    {
        *p-- = number % 10 + 0x30;
        number = number / 10;
    }
    if (isNeg) *p-- = '-';
    return p + 1;
}

class JsonBuilder
{
    CComBSTR m_json;
public:
    void BeginObject()
    {
        m_json += "{ ";
    }
    void EndObject()
    {
        m_json += " }";
    }
    void BeginList()
    {
        m_json += "[ ";
    }
    void EndList()
    {
        m_json += "] ";
    }
    void AddField(LPCWSTR name)
    {
        m_json += "\"";
        m_json += name;
        m_json += "\":";
    }
    void AddField(LPCWSTR name, LPCWSTR value)
    {
        AddField(name);
        m_json += "\"";
        AddEscapedText(value);
        m_json += "\"";
    }
    void AddField(LPCWSTR name, int value)
    {
        AddField(name);
        m_json += ToText(value);
    }
    void AddField(LPCWSTR name, bool value)
    {
        AddField(name);
        m_json += value ? L"true" : L"false";
    }
    void AddField(LPCWSTR name, VARIANT_BOOL value)
    {
        AddField(name, value == VARIANT_TRUE ? true : false);
    }
    void AddComma()
    {
        m_json += ", ";
    }
    void NewLine()
    {
        m_json += L"\r\n";
    }
    LPCWSTR Psz() { return m_json; }
private:
    void AddEscapedText(LPCWSTR text)
    {
        LPCWSTR p = text;
        wchar_t ch;

        while (p !=NULL && (ch = *p) != NULL)
        {
            switch (ch)
            {
                case '\n':  m_json += L"\\n";  break;
                case '\r':  m_json += L"\\r";  break;
                case '\t':  m_json += L"\\t";  break;
                case '\v':  m_json += L"\\v";  break;
                case '\'':  m_json += L"\\'";  break;
                case '\"':  m_json += L"\\\"";  break;
                case '\\':  m_json += L"\\\\";  break;
                default:    m_json.Append(p,1);  break;
            };
            p++;
        }
    }
};

CAuthoringHost::CAuthoringHost(JsGlass* jsGlass)
    : refCount(1),
    _jsGlass(jsGlass),
    _message(this, GetCurrentThreadId()),
    _timer(NULL)
{
    _threadId = GetCurrentThreadId();
}

CAuthoringHost::~CAuthoringHost()
{
}

STDMETHODIMP CAuthoringHost::QueryInterface(REFIID riid, __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (riid == _uuidof(IUnknown))
    {
        *ppvObject =  static_cast<IUnknown*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG CAuthoringHost::AddRef()
{
    return InterlockedIncrement(&refCount);
}

ULONG CAuthoringHost::Release()
{
    long currentCount = InterlockedDecrement(&refCount);
    if (currentCount == 0)
    {
        delete this;
    }
    return currentCount;
}

static CAuthoringHost *gHost = NULL;

VOID CALLBACK CAuthoringHost::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (gHost)
        gHost->Timer();
}

void CAuthoringHost::Timer()
{
    if (_callback) 
    {
        KillTimer(NULL, _timer);
        void (*callback)(void *) = _callback;
        _callback = NULL;
        void *data = _callbackData;
        _callbackData = NULL;
        callback(data);
    }
}

HRESULT CAuthoringHost::SetCallbackTimer(DWORD dwTime)
{
    Message<CAuthoringHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&CAuthoringHost::SetCallbackTimer, dwTime, &result))
    {
        // This makes this call synchronous.  That is alright for Adding a script, and allows
        // us to just use the LPCSTRING rather than put it in a BSTR.
        return result.BlockOnResult();
    }
    gHost = this;
    _timer = SetTimer(NULL, NULL, dwTime, TimerCallback);
    return S_OK;
}

void CAuthoringHost::CallAfter(DWORD dwTime, void (*callback)(void *), void *data)
{
    _callback = callback;
    _callbackData = data;
    SetCallbackTimer(dwTime);
}

void CAuthoringHost::KillCallback()
{
    _callback = NULL;
    if (_timer)
        KillTimer(NULL, _timer);
    _timer = NULL;
}

unsigned int CAuthoringHost::HostLoop(void* args)
{
    HRESULT hr = S_OK;
    CAuthoringHost* host;
    HostLoopArgs* loopArgs = (HostLoopArgs*)args;

    CoInitializeEx(NULL, IsOs_OneCoreUAP() ? COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED);

    host = new CAuthoringHost(loopArgs->jsGlass);
    loopArgs->jsGlass->_authoringHost = host;

    IfFailGo(host->Initialize(loopArgs->jscriptPath));

    // TODO this is a hack for getting commands from the main thread
    // to the scriptHost thread.
    // I need to consider how to make this affordable for expanding the command set
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_QUIT)
        {
            break;
        }
        if (Message<CAuthoringHost>::TryDispatch(msg))
        {
            continue;
        }
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }

Error:
    CoUninitialize();
    _endthreadex(0);
    return 0;
}

HRESULT CAuthoringHost::Initialize(LPCWSTR targetdll)
{
    if (_threadId != GetCurrentThreadId())
    {
        AssertMsg(TRUE, "Called host initialize on the wrong thread");
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    return hr;
}

static size_t CountLines(LPCWSTR ptr)
{
    size_t lines = 1; // An empty string still has one line (although a very short one).
    for (;;)
    {
        switch (*ptr++)
        {
        case '\r': if (*ptr == '\n') ptr++;
        case '\n': lines++; break;
        case '\0': return lines;
        }
    }
    // Never gets here.
}

static LPCWSTR* BreakIntoLines(LPCWSTR ptr, size_t cLines)
{
    LPCWSTR* result = static_cast<LPCWSTR*>(malloc((cLines + 2) * sizeof(LPCWSTR)));
    LPCWSTR* current = result;
    
    *current++ = ptr;
    
    while (cLines > 0)
    {
        switch (*ptr++)
        {
        case '\r': if (*ptr == '\n') ptr++; 
        case '\n': *current++ = ptr; cLines--;
        default: continue;
        case '\0': break;
        }
        break;
    }
    *current = ptr - 1;

    return result;
}

#ifdef DIRECT_AUTHOR_TEST
static const wchar_t *KindTextOf(AuthorTokenKind kind)
{
    switch (kind)
    {
    case atkEnd:                        return L"end";
    case atkText:                       return L"text";
    case atkIdentifier:                 return L"identifier";
    case atkComment:                    return L"comment";
    case atkNumber:                     return L"number";
    case atkString:                     return L"string";
    case atkRegexp:                     return L"regex";
    case atkScanError:                  return L"scanError";
    case atkComma:                      return L",";
    case atkDArrow:                     return L"=>";
    case atkAsg:                        return L"=";
    case atkAsgAdd:                     return L"+=";
    case atkAsgSub:                     return L"-=";
    case atkAsgMul:                     return L"*=";
    case atkAsgDiv:                     return L"/=";
    case atkAsgMod:                     return L"%=";
    case atkAsgAnd:                     return L"&=";
    case atkAsgXor:                     return L"^=";
    case atkAsgOr:                      return L"|=";
    case atkAsgLsh:                     return L"<<=";
    case atkAsgRsh:                     return L">>=";
    case atkAsgRs2:                     return L">>>=";
    case atkQMark:                      return L"?";
    case atkColon:                      return L":";
    case atkLogOr:                      return L"||";
    case atkLogAnd:                     return L"&&";
    case atkOr:                         return L"|";
    case atkXor:                        return L"^";
    case atkAnd:                        return L"&";
    case atkEQ:                         return L"==";
    case atkNE:                         return L"!=";
    case atkEqv:                        return L"===";
    case atkNEqv:                       return L"!==";
    case atkLT:                         return L"<";
    case atkLE:                         return L"<=";
    case atkGT:                         return L">";
    case atkGE:                         return L">=";
    case atkLsh:                        return L"<<";
    case atkRsh:                        return L">>";
    case atkRs2:                        return L">>>";
    case atkAdd:                        return L"+";
    case atkSub:                        return L"-";
    case atkStar:                       return L"*";
    case atkDiv:                        return L"/";
    case atkPct:                        return L"%";
    case atkTilde:                      return L"~";
    case atkBang:                       return L"!";
    case atkInc:                        return L"++";
    case atkDec:                        return L"--";
    case atkLParen:                     return L"(";
    case atkLBrack:                     return L"[";
    case atkDot:                        return L".";
    case atkScope:                      return L"::";
    case atkSColon:                     return L";";
    case atkRParen:                     return L")";
    case atkRBrack:                     return L"]";
    case atkLCurly:                     return L"{";
    case atkRCurly:                     return L"}";
    case atkBreak:                      return L"break";
    case atkCase:                       return L"case";
    case atkCatch:                      return L"catch";
    case atkClass:                      return L"class";
    case atkConst:                      return L"const";
    case atkContinue:                   return L"continue";
    case atkDebugger:                   return L"debugger";
    case atkDefault:                    return L"default";
    case atkDelete:                     return L"delete";
    case atkDo:                         return L"do";
    case atkElse:                       return L"else";
    case atkEnum:                       return L"enum";
    case atkExport:                     return L"export";
    case atkExtends:                    return L"extends";
    case atkFalse:                      return L"false";
    case atkFinally:                    return L"finally";
    case atkFor:                        return L"for";
    case atkFunction:                   return L"function";
    case atkIf:                         return L"if";
    case atkImport:                     return L"import";
    case atkIn:                         return L"in";
    case atkInstanceof:                 return L"instanceof";
    case atkNew:                        return L"new";
    case atkNull:                       return L"null";
    case atkReturn:                     return L"return";
    case atkSuper:                      return L"super";
    case atkSwitch:                     return L"switch";
    case atkThis:                       return L"this";
    case atkThrow:                      return L"throw";
    case atkTrue:                       return L"true";
    case atkTry:                        return L"try";
    case atkTypeof:                     return L"typeof";
    case atkVar:                        return L"var";
    case atkVoid:                       return L"void";
    case atkWhile:                      return L"while";
    case atkWith:                       return L"with";
    case atkImplements:                 return L"implements";
    case atkInterface:                  return L"interface";
    case atkLet:                        return L"let";
    case atkPackage:                    return L"package";
    case atkPrivate:                    return L"private";
    case atkProtected:                  return L"protected";
    case atkPublic:                     return L"public";
    case atkStatic:                     return L"static";
    case atkYield:                      return L"yield";
    case atkOf:                         return L"of";
    case atkStrTemplate:                return L"string";
    default:                            return L"unknown";
    }
}

HRESULT CAuthoringHost::GetTokenRanges(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    Message<CAuthoringHost>::FunctionCallResult result;
    if (S_OK == _message.AsyncCall(&CAuthoringHost::GetTokenRanges, scriptEngine, scriptCode, &result))
    {
        // This makes this call synchronous.  That is alright for Adding a script, and allows
        // us to just use the LPCWSTR rather than put it in a BSTR.
        return result.BlockOnResult();
    }

    HRESULT hr = S_OK;

    CComBSTR json = L"{ \"lines\": [";
  
    // Count the number of lines
    size_t cLines = CountLines(scriptCode);
    LPCWSTR* lines = BreakIntoLines(scriptCode, cLines);

    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorColorizeText> colorizer;

    IfFailGo(scriptEngine->QueryInterface(&authorServices));
    IfFailGo(authorServices->GetColorizer(&colorizer));
    
    // Produce token ranges for each line and add it lines.
    AuthorSourceState state = SOURCE_STATE_INITIAL;
    for (size_t i = 0; i < cLines; i++)
    {
        if (i > 0) json += L", ";

        json += L"{ \"line\" : ";
        json += ToText(i);

        json += L", \"tokens\": [";
        CComPtr<IAuthorTokenEnumerator> tokens;
        IfFailGo(colorizer->Colorize(lines[i], lines[i + 1] - lines[i], state, &tokens));
        bool first = true;
        for (;;)
        {
            AuthorTokenColorInfo tokenInfo;
            IfFailGo(tokens->Next(&tokenInfo));
            if (tokenInfo.Kind == atkEnd)
            {
                state = tokenInfo.State;
                break;
            }
            
            if (!first) json += L", ";
            first = false;

            json += L"{ \"start\": ";
            json += ToText(tokenInfo.StartPosition);
            json += L", \"end\": ";
            json += ToText(tokenInfo.EndPosition);
            json += L", \"kind\": \"";
            json += KindTextOf(tokenInfo.Kind);
            json += L"\"}";
        }
        json += "] }";
        tokens = NULL;
    }
    
    json += L"] }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    return hr;
}

#define QI_IMPL(intf)\
    if (IsEqualIID(riid, _uuidof(intf)))\
{   \
    *ppvObj = static_cast<intf *>(this); \
    AddRef();\
    return NOERROR;\
}\

#define ValidatePtr(expr, err) \
    do { \
      if (!(expr)) { \
        hr = err; \
        goto Error; \
      } \
    } while (0)

#define ValidateAlloc(expr) ValidatePtr(expr, E_OUTOFMEMORY);
#define ValidateArg(expr) ValidatePtr(expr, E_INVALIDARG);

template <class Interface>
class SimpleOleClass : public Interface
{
private:
    long m_refCount;
public:
    SimpleOleClass(): m_refCount(1) { }

    // *** IUnknown ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
    {
        QI_IMPL(IUnknown);
        QI_IMPL(Interface);

        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG,AddRef)(void)
    {
        return ++m_refCount;
    }

    STDMETHOD_(ULONG,Release)(void)
    {
        long lw = --m_refCount;
        if (0 == lw)
        {
            delete this;
        }
        return lw;
    }
};

typedef const __nullterminated wchar_t *LPCSTRING;
typedef __nullterminated wchar_t *LPSTRING;

class StringFileReader : public SimpleOleClass<IAuthorFileReader>
{
private:
    LPCSTRING string;
    size_t stringLength;
    size_t offset;
public:
    StringFileReader(LPCSTRING string, size_t stringLength): string(string), stringLength(stringLength), offset(0) { }

    STDMETHOD(Read)(long length, __out_ecount(length) wchar_t *buffer, __out_ecount(1) long *read)
    {
        HRESULT hr = S_OK;
        ValidateArg((length >= 0));
        ValidateArg(read);

        long copyLength = length;
        if (static_cast<size_t>(length) + this->offset > stringLength)
            copyLength = stringLength - this->offset;
        if (copyLength > 0) 
        {
            memcpy(buffer, &(((LPWSTR)string)[this->offset]), sizeof(wchar_t) * copyLength);
            *read = copyLength;
            this->offset += copyLength;
        }
        else
            *read = 0;

Error:
        return hr;
    }

    STDMETHOD(Seek)(long offset)
    {
        HRESULT hr = S_OK;
        ValidateArg((offset >= 0));
        this->offset = offset;
Error:
        return hr;
    }

    STDMETHOD(Close)()
    {
        return S_OK;
    }
};

class AuthoringFile : public SimpleOleClass<IAuthorFile>
{
private:
    LPCSTRING text;
    size_t length;
    AuthorFileStatus status;
public:
    AuthoringFile(LPCSTRING text, size_t length): text(text), length(length) { }

    AuthorFileStatus LastStatus() { return this->status; }

    void AdjustLength(int adjustment) { length = (size_t)((int)length + adjustment); }
    size_t Length() { return length; }
    LPCSTRING Text() { return text; }

    STDMETHOD(GetDisplayName)(BSTR *name)
    {
        *name = NULL;
        return S_OK;
    }
        
    STDMETHOD(GetLength)(long *length)
    {
        if (length)
            *length = this->length;

        return S_OK;
    }
        
    STDMETHOD(GetReader)(IAuthorFileReader **result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = new StringFileReader(text, length);

Error:
        return hr;
    }
        
    STDMETHOD(StatusChanged)( AuthorFileStatus newStatus)
    {
        status = newStatus;
        return S_OK;
    }

};

class SingleFileContext : public SimpleOleClass<IAuthorFileContext>
{
private:
    AuthoringFile *file;
    IAuthorFileHandle *handle;

public:
    SingleFileContext(IAuthorServices *services, LPCSTRING text, size_t length)
    { 
        file = new AuthoringFile(text, length);
        services->RegisterFile(file, &handle);
    }

    ~SingleFileContext()
    {
        ReleasePtr(file);
        ReleasePtr(handle);
    }

    AuthorFileStatus LastStatus() { return file->LastStatus(); }
    
    void AdjustLength(int adjustment) 
    { 
        if (file) file->AdjustLength(adjustment);
    }

    IAuthorFileHandle *Handle() { return handle; }
    AuthoringFile *File() { return file; }

    STDMETHOD(GetPrimaryFile)(IAuthorFileHandle **result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = handle;
        handle->AddRef();

Error:
        return hr;
    }

    STDMETHOD(GetContextFileCount)(int *result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = 0;

Error:
        return hr;
    }

    STDMETHOD(GetContextFiles)(int startIndex, int count, __out_ecount(count) IAuthorFileHandle **handles)
    {
        HRESULT hr = S_OK;

        ValidateArg(count == 0);

Error:
        return hr;
    }

    STDMETHOD(GetHostType)(AuthorHostType *type)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(type, E_INVALIDARG);

        *type = AuthorHostType::ahtBrowser;

Error:
        return hr;
    }

    STDMETHOD(GetScriptVersion)(AuthorScriptVersion *scriptVersion)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(scriptVersion, E_INVALIDARG);

        *scriptVersion = AuthorScriptVersion::ScriptVersion12;

Error:
        return hr;
    }

    STDMETHOD(FileAuthoringPhaseChanged)(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle *executingFile)
    {
        HRESULT hr = S_OK;

        ValidateArg(phase >= afpDormant && phase <= afpExecuting);

Error:
        return hr;
    }

    STDMETHOD(LogMessage)(BSTR message)
    {
        HRESULT hr = S_OK;

//Error:
        return hr;
    }

    STDMETHOD(VerifyScriptInContext)(BSTR url, BSTR charset, BSTR type, IAuthorFileHandle *sourceFile, VARIANT_BOOL insertBeforeSourceFile, AuthorFileAsynchronousScriptStatus* status)
    {
        HRESULT hr = S_OK;

        *status = AuthorFileAsynchronousScriptStatus::afassIgnored;

//Error:
        return hr;
    }
};

class SplatterFileContext : public SimpleOleClass<IAuthorFileContext>
{
private:
    size_t size;
    size_t length;
    wchar_t *text;
    AuthoringFile *file;
    IAuthorFileHandle *handle;

public:
    SplatterFileContext(IAuthorServices *services, __in_ecount(size) wchar_t *text, size_t length, size_t size): text(text), length(length), size(size)
    { 
        file = new AuthoringFile(text, 0);
        services->RegisterFile(file, &handle);
    }

    ~SplatterFileContext()
    {
        ReleasePtr(file);
        ReleasePtr(handle);
    }

    AuthorFileStatus LastStatus() { return file->LastStatus(); }
    
    void AdjustLength(int adjustment) 
    { 
        if (file) file->AdjustLength(adjustment);
    }

    IAuthorFileHandle *Handle() { return handle; }
    AuthoringFile *File() { return file; }

    HRESULT ExtendView()
    {
        HRESULT hr = S_OK;
        size_t previousLength = file->Length();
        ValidateArg(previousLength < length);

        file->AdjustLength(1);
        AuthorFileChange change;
        change.charactersInserted = 1;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(handle->FileChanged(1, &change));
Error:
        return hr;
    }

    HRESULT ViewAll()
    {
        HRESULT hr = S_OK;

        size_t previousLength = file->Length();
        ValidateArg(previousLength < length);

        size_t exposed = length - previousLength;
        file->AdjustLength(exposed);
        AuthorFileChange change;
        change.charactersInserted = exposed;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(handle->FileChanged(1, &change));
Error:
        return hr;
    }

    HRESULT RetractView()
    {
        HRESULT hr = S_OK;

        size_t previousLength = file->Length();
        ValidateArg(previousLength > 0);

        file->AdjustLength(-1);
        AuthorFileChange change;
        change.charactersInserted = 1;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(handle->FileChanged(1, &change));

Error:
        return hr;
    }

    HRESULT InsertText(size_t offset, LPCWSTR insertText)
    {
        HRESULT hr = S_OK;
        size_t insertLen = 0;
        size_t newLength = 0;
        
        ValidateArg(insertText);

        insertLen = ::wcslen(insertText);
        newLength = length + insertLen;

        ValidateArg(newLength >= length);
        ValidateArg(file->Length() == length);
        ValidateArg(offset <= length);
        ValidateArg(newLength <= size);

        // Physically insert the new characters
        for (size_t i = newLength; i > offset; i--)
            text[i] = text[i - insertLen];
        text[offset] = text[offset - insertLen];
        for (size_t i = 0; i < insertLen; i++)
            text[i + offset] = insertText[i];

        // Update the lengths
        file->AdjustLength(insertLen);
        length = newLength;

        // Notify the file handle the file was changed.
        AuthorFileChange change;
        change.charactersInserted = insertLen;
        change.charactersRemoved = 0;
        change.offset = (long)offset;
        IfFailGo(handle->FileChanged(1, &change));

Error:
        return hr;
    }

    HRESULT DeleteText(size_t offset, size_t deleteLen)
    {
        HRESULT hr = S_OK;
        size_t newLength = 0;

        newLength = length - deleteLen;
        ValidateArg(newLength < length);
        ValidateArg(file->Length() == length);
        ValidateArg(offset < length);

        // Physically remove the characters
        for (size_t i = offset; i < length; i++)
            text[i] = text[i + deleteLen];

        // Update the lengths
        file->AdjustLength(-(long)deleteLen);
        length = newLength;

        // Notify the file handle the file was changed
        AuthorFileChange change;
        change.charactersInserted = 0;
        change.charactersRemoved = deleteLen;
        change.offset = (long)offset;
        IfFailGo(handle->FileChanged(1, &change));

Error:
        return hr;
    }

    STDMETHOD(GetPrimaryFile)(IAuthorFileHandle **result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = handle;
        handle->AddRef();

Error:
        return hr;
    }

    STDMETHOD(GetContextFileCount)(int *result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = 0;

Error:
        return hr;
    }

    STDMETHOD(GetContextFiles)(int startIndex, int count, __out_ecount(count) IAuthorFileHandle **handles)
    {
        HRESULT hr = S_OK;

        ValidateArg(count == 0);

Error:
        return hr;
    }

    STDMETHOD(GetHostType)(AuthorHostType *type)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(type, E_INVALIDARG);

        *type = AuthorHostType::ahtBrowser;

Error:
        return hr;
    }

    STDMETHOD(GetScriptVersion)(AuthorScriptVersion *scriptVersion)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(scriptVersion, E_INVALIDARG);

        *scriptVersion = AuthorScriptVersion::ScriptVersion12;

Error:
        return hr;
    }

    STDMETHOD(FileAuthoringPhaseChanged)(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle *executingFile)
    {
        HRESULT hr = S_OK;

        ValidateArg(phase >= afpDormant && phase <= afpExecuting);

Error:
        return hr;
    }

    STDMETHOD(LogMessage)(BSTR message)
    {
        HRESULT hr = S_OK;

//Error:
        return hr;
    }

    STDMETHOD(VerifyScriptInContext)(BSTR url, BSTR charset, BSTR type, IAuthorFileHandle *sourceFile, VARIANT_BOOL insertBeforeSourceFile, AuthorFileAsynchronousScriptStatus* status)
    {
        HRESULT hr = S_OK;

        *status = AuthorFileAsynchronousScriptStatus::afassIgnored;

//Error:
        return hr;
    }
};

class MultiFileContext : public SimpleOleClass<IAuthorFileContext>
{
private:
    typedef AuthoringFile *AuthoringFilePtr;
    typedef IAuthorFileHandle *IAuthorFileHandlePtr;
    int m_count;
    AuthoringFile *m_primary;
    IAuthorFileHandle *m_primaryHandle;
    AuthoringFile **m_sources;
    IAuthorFileHandle **m_sourceHandles;
    IAuthorServices *m_services;
    AuthorHostType m_hostType;
    AuthorScriptVersion m_scriptVersion;

public:
    MultiFileContext(IAuthorServices *services, LPCSTRING primary, size_t length, int count, LPCWSTR sources[], AuthorHostType hostType = AuthorHostType::ahtBrowser, AuthorScriptVersion scriptVersion = AuthorScriptVersion::ScriptVersion12)
        : m_count(count), m_services(services), m_hostType(hostType), m_scriptVersion(scriptVersion), m_sources(nullptr), m_sourceHandles(nullptr)
    {
        m_primary = new AuthoringFile(primary, length);
        services->RegisterFile(m_primary, &m_primaryHandle);
        if (count > 0)
        {
            m_sources = new AuthoringFilePtr[count + 1];
            m_sourceHandles = new IAuthorFileHandlePtr[count + 1];
            for (int i = 0; i < count; i++)
            {
                m_sources[i] = new AuthoringFile(sources[i], ::wcslen(sources[i]));
                services->RegisterFile(m_sources[i], &m_sourceHandles[i]);
            }
        }
    }

    ~MultiFileContext()
    {
        ReleasePtr(m_primary);
        ReleasePtr(m_primaryHandle);
        ReleaseArray(m_sources, m_count);
        ReleaseArray(m_sourceHandles, m_count);
    }

    void AdjustLength(int adjustment) 
    { 
        if (m_primary) m_primary->AdjustLength(adjustment);
    }

    IAuthorFileHandle *Handle() { return m_primaryHandle; }
    AuthoringFile *File() { return m_primary; }
    AuthorFileStatus LastStatus() { return m_primary->LastStatus(); }

    HRESULT LoadAuthoringFile(LPCWSTR filename, AuthoringFile** authoringFile)
    {
        HRESULT hr = S_OK;

        *authoringFile = NULL;
        FILE* file;
        if(!_wfopen_s(&file, filename, L"rb"))
        {
            LPWSTR content = FileSystem::GetContents(file);
            fclose(file);

            *authoringFile = new AuthoringFile(content, ::wcslen(content));
        }
        else
        {
            hr = ::GetLastError();
        }

//Error:
        return hr;
    }

    HRESULT AddContextFile(LPCWSTR filename)
    {
        return AddContextFile(filename, m_count);
    }

    HRESULT AddContextFile(LPCWSTR filename, int index)
    {
        HRESULT hr = S_OK;

        AuthoringFile* authoringFile;
        IfFailGo(LoadAuthoringFile(filename, &authoringFile));
        if (authoringFile == NULL)
        {
            hr = E_FAIL;
            return hr;
        }

        AddContextFile(authoringFile, index);

Error:
        return hr;
    }

    HRESULT AddContextFile(AuthoringFile* authoringFile, int index)
    {
        HRESULT hr = S_OK;

        if (index > m_count || index < 0 || authoringFile == NULL)
        {
            hr = E_FAIL;
            return hr;
        }

        AuthoringFile **newSources = new AuthoringFilePtr[m_count + 1];
        ValidateAlloc(newSources);
        IAuthorFileHandle **newHandles = new IAuthorFileHandlePtr[m_count + 1];
        ValidateAlloc(newHandles);

        for (int i = 0; i < index; i++)
        {
            newSources[i] = m_sources[i];
            newHandles[i] = m_sourceHandles[i];
        }

        newSources[index] = authoringFile;
        IfFailGo(m_services->RegisterFile(newSources[index], &newHandles[index]));

        for (int i = index; i < m_count; i++)
        {
            newSources[i + 1] = m_sources[i];
            newHandles[i + 1] = m_sourceHandles[i];
        }

        // Clean the old pointer container.
        if (m_sources != nullptr)
        {
            delete [] m_sources;
            delete [] m_sourceHandles;
        }

        m_sources = newSources;
        m_sourceHandles = newHandles;
        m_count++;

Error:
        return hr;
    }

    HRESULT RemoveContextFile(int index)
    {
        HRESULT hr = S_OK;

        ValidateArg(index < m_count);

        ReleasePtr(m_sources[index]);
        ReleasePtr(m_sourceHandles[index]);
        m_count--;
        for (int i = index; i < m_count; i++)
        {
            m_sources[i] = m_sources[i + 1];
            m_sourceHandles[i] = m_sourceHandles[i + 1];
        }
Error:
        return hr;
    }

    int Count() { return m_count; }

    STDMETHOD(GetPrimaryFile)(IAuthorFileHandle **result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = m_primaryHandle;
        m_primaryHandle->AddRef();

Error:
        return hr;
    }

    STDMETHOD(GetContextFileCount)(int *result)
    {
        HRESULT hr = S_OK;
        ValidateArg(result);

        *result = m_count;

Error:
        return hr;
    }

    STDMETHOD(GetContextFiles)(int startIndex,int count, __out_ecount(count) IAuthorFileHandle **handles)
    {
        HRESULT hr = S_OK;

        ValidateArg(startIndex + count <= m_count);
        
        for (int i = 0; i < count; i++)
        {
            handles[i] = m_sourceHandles[i];
            if (m_sourceHandles[i]) m_sourceHandles[i]->AddRef();
        }
Error:
        return hr;
    }

    STDMETHOD(GetHostType)(AuthorHostType *type)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(type, E_INVALIDARG);

        *type = m_hostType;

Error:
        return hr;
    }

    STDMETHOD(GetScriptVersion)(AuthorScriptVersion *scriptVersion)
    {
        HRESULT hr = S_OK;
        
        ValidatePtr(scriptVersion, E_INVALIDARG);

        *scriptVersion = AuthorScriptVersion::ScriptVersion12;

Error:
        return hr;
    }

    STDMETHOD(FileAuthoringPhaseChanged)(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle *executingFile)
    {
        HRESULT hr = S_OK;

        ValidateArg(phase >= afpDormant && phase <= afpExecuting);

Error:
        return hr;
    }

    STDMETHOD(LogMessage)(BSTR message)
    {
        HRESULT hr = S_OK;

//Error:
        return hr;
    }

    STDMETHOD(VerifyScriptInContext)(BSTR url, BSTR charset, BSTR type, IAuthorFileHandle *sourceFile, VARIANT_BOOL insertBeforeSourceFile, AuthorFileAsynchronousScriptStatus* status)
    {
        HRESULT hr = S_OK;
        
        AuthoringFile* authoringFile;

        if (url && *url)
        {
            IfFailGo(LoadAuthoringFile((LPCWSTR)url, &authoringFile));
        }
        else
        {
            // script is empty, ignore the call
            *status = AuthorFileAsynchronousScriptStatus::afassIgnored;
            return hr;
        }

        // find the source file index
        int index;
        if (sourceFile == m_primaryHandle)
        {
            index = m_count;
        }
        else
        {
            for(index = 0; index < m_count; index++)
            {
                if (m_sourceHandles[index] == sourceFile)
                    break;
            }
            AssertMsg(index != m_count); // source file has to be in the context
        }

        // check if the file exists in the context
        for(int i = 0; i < m_count; i++)
        {
            if (m_sources[i]->Length() == authoringFile->Length() && 
                ::wcsncmp(m_sources[i]->Text(), authoringFile->Text(), m_sources[i]->Length()) ==  0)
            {
                delete authoringFile;

                //TODO: handle the case where the file is not before the sourceFile

                // found the file in the list
                *status = AuthorFileAsynchronousScriptStatus::afassLoaded;
                return hr;
            }
        }

        // add the file 
        IfFailGo(AddContextFile(authoringFile, index));

        // the file does not exist
        *status = AuthorFileAsynchronousScriptStatus::afassAdded;

Error:
        return hr;
    }
};

void RecordRegions(CComBSTR &json, AuthorFileStatus status, AuthorFileRegion *regions, int length)
{
    // Add status
    json += "{ \"status\": \"";
    if (!status) json += "no errors";
    if (status & afsErrors) json += "parse errors";
    if (status & afsWarnings) json += " warnings";
    if (status & afsInvalidRegions) json += ", invalid regions";
    json += "\"";

    // Add regions
    json += L", \"ranges\": [";
    for (int i = 0; i < length; i++)
    {
        if (i > 0) json += ", ";
        json += "{ \"offset\": ";
        json += ToText(regions[i].offset);
        json += ", \"length\": ";
        json += ToText(regions[i].length);
        json += "} ";
    }
    json += "] }";
}

HRESULT CAuthoringHost::GetRegions(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorRegionSet> regionSet;
    SingleFileContext *singleFileContext;
    CComPtr<IAuthorFileContext> fileContext;
    CComBSTR json;
    AuthorFileRegion *regions = NULL;

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine);

    IfFailGo(scriptEngine->QueryInterface(&authorServices));
    singleFileContext = new SingleFileContext(authorServices, scriptCode, ::wcslen(scriptCode));
    fileContext.Attach(singleFileContext);
    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));
    IfFailGo(authoring->GetRegions(&regionSet));
    int length;
    IfFailGo(regionSet->get_Count(&length)); 
    regions = static_cast<AuthorFileRegion *>(malloc(sizeof(AuthorFileRegion) * length));
    ValidateAlloc(regions);
    regionSet->GetItems(0, length, regions);
    
    AuthorFileStatus status = singleFileContext->LastStatus();
    
    RecordRegions(json, status, regions, length);

    IfFailGo(_jsGlass->EventCallback(json));

Error:
    if (regions) free(regions);
    return hr;
}

static void FinalizeBSTR(BSTR& str)
{
    if (str) 
    {
        ::SysFreeString(str);
        str = NULL;
    }
}

static void FinalizeCompletion(AuthorCompletion& completion)
{
    FinalizeBSTR(completion.name);
    FinalizeBSTR(completion.displayText);
    FinalizeBSTR(completion.insertionText);
}

static void CallHurry(void *authoring)
{
   ((IAuthorFileAuthoring *)authoring)->Hurry(0);
}


// Collect offsets where the '|' is in the code but don't include the '|'
static HRESULT ParseOffsets(LPCSTRING scriptCode, __out_ecount(maxOffset) long* offsets, size_t maxOffset, __out_ecount(1) int *count, __out_ecount(1) LPSTRING *source)
{
    HRESULT hr = S_OK;
    wchar_t *code;

    size_t len = wcslen(scriptCode);
    code = static_cast<wchar_t *>(malloc((len + 1) * sizeof(wchar_t)));
    ValidateAlloc(code);

    // Collect offsets where the '|' is in the code but don't include the '|'
    size_t cOffsets = 0;
    size_t j = 0;
    for (size_t i = 0; i < len && cOffsets < maxOffset; i++)
    {
        wchar_t ch = scriptCode[i];
        if (ch == '|' 
            // Don't consider " |" or "||" a cursor location because it is most likely the | or || operators.
            && (i == 0 || (scriptCode[i - 1] != ' ' && scriptCode[i - 1] != '|')))
            offsets[cOffsets++] = j;
        else
            code[j++] = ch;
    }
    code[j++] = 0;
    
    *count = (int)cOffsets;
    *source = code;

Error:
    return hr;
}

void RecordCompetion(CComBSTR &json, AuthorFileRegion extent, AuthorCompletionSetKind kind, AuthorCompletion *completions, size_t count)
{
    json += "{ \"extent\": { \"offset\":";
    json += ToText(extent.offset);
    json += ", \"length\":";
    json += ToText(extent.length);
    json += "}, \"kind\": \"";
    switch (kind)
    {
    case acskMember: json += "member"; break;
    case acskStatement: json += "statement"; break;
    default: json += "unknown"; break;
    }
    json += "\", \"completions\": [";
    for (unsigned int i = 0; i < count; i++)
    {
        if (i != 0) json += ", ";
        json += "{ \"kind\": \"";
        switch (completions[i].kind)
        {
        case ackMethod: json += "method"; break;
        case ackField: json += "field"; break;
        case ackProperty: json += "property"; break;
        case ackReservedWord: json += "reserved word"; break;
        case ackIdentifier: json += "identifier"; break;
        case ackParameter: json += "parameter"; break;
        case ackVariable: json += "variable"; break;
        case ackLabel: json += "label"; break;
        default: json += "unknown"; break;
        }
        json += "\", \"group\": \"";
        wchar_t const *separator = L"";
        AuthorCompletionFlags group = completions[i].group;
        if (group & acfMembersFilter) { json += "members"; separator = L", "; }
        if (group & acfFileIdentifiersFilter) { json += separator; json += "file identifiers"; separator = L", "; }
        if (group & acfSyntaxElementsFilter) { json += separator; json += "syntax"; separator = L", "; }
        json += "\", \"name\": \"";
        json += completions[i].name;
        json += "\", \"displayText\": \"";
        json += completions[i].displayText;
        json += "\", \"insertionText\": \"";
        json += completions[i].insertionText;
        json += "\"}";
    }
    json += "] }";
}

HRESULT CAuthoringHost::GetCompletions(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[])
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    CComBSTR json = L"{ \"completions\": [";
    LPSTRING code = NULL;

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scripts && count > 0);

    long offsets[100];
    int cOffsets;
    LPCWSTR scriptCode = scripts[count - 1]; // Assumes the last file is the primary script.
    IfFailGo(ParseOffsets(scriptCode, offsets, sizeof(offsets)/sizeof(offsets[0]), &cOffsets, &code));

    IfFailGo(scriptEngine->QueryInterface(&authorServices));
    fileContext.Attach(new MultiFileContext(authorServices, code, ::wcslen(code), count - 1, scripts));
    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    bool first = true;
    for (int i = 0; i < cOffsets; i++)
    {
        if (!first) json += ", ";
        first = false;

        PerformGetCompletionRequest(authoring, offsets[i], AuthorCompletionFlags::acfAny, json);
    }
    json += "] }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    if (code) free(code);
    return hr;
}

HRESULT CAuthoringHost::GetErrors(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    CComPtr<IAuthorMessageSet> messageSet;
    CComBSTR json = L"{ \"errors\": [";
    wchar_t *code = NULL;
    AuthorFileMessage messages[1000];
    memset(messages, 0, sizeof(messages));

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scriptCode);

    IfFailGo(scriptEngine->QueryInterface(&authorServices));

    SingleFileContext *context = new SingleFileContext(authorServices, scriptCode, ::wcslen(scriptCode));
    fileContext.Attach(context);

    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    IfFailGo(authoring->Update());
    int count;
    
    IfFailGo(context->Handle()->GetMessageSet(&messageSet));
    IfFailGo(messageSet->get_Count(&count));
    ValidatePtr(count < 1000, E_FAIL);        
    IfFailGo(messageSet->GetItems(0, count, messages));
    
    for (int i = 0; i < count; i++)
    {
        if (i) json += ", ";
        json += "{ \"position\": ";
        json += ToText(messages[i].position);
        json += ", \"length\": ";
        json += ToText(messages[i].length);
        json += ", \"message\": \"";
        json += messages[i].message;
        json += "\", \"messageID\": ";
        json += ToText(messages[i].messageID);
        json += "}";
    }
    json += "] }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    if (code) free(code);
    return hr;
}

HRESULT CAuthoringHost::GetAst(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    CComBSTR json;

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scriptCode);

    IfFailGo(scriptEngine->QueryInterface(&authorServices));

    fileContext.Attach(new SingleFileContext(authorServices, scriptCode, ::wcslen(scriptCode)));

    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    IfFailGo(authoring->GetASTAsJSON(&json));
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    return hr;
}

void DumpReturnValue(IAuthorReturnValue* returnValue, JsonBuilder& json)
{
    json.BeginObject();

    if (returnValue)
    {
        CComBSTR type;
        returnValue->get_Type(&type);
        json.AddField(L"type", type);

        json.AddComma();

        CComBSTR description;
        returnValue->get_Description(&description);
        json.AddField(L"description", description);

        json.AddComma();

        CComBSTR locid;
        returnValue->get_Locid(&locid);
        json.AddField(L"locid", locid);

        json.AddComma();

        CComBSTR elementType;
        returnValue->get_ElementType(&elementType);
        json.AddField(L"elementType", elementType);

        json.AddComma();

        CComBSTR helpKeyword;
        returnValue->get_HelpKeyword(&helpKeyword);
        json.AddField(L"helpKeyword", helpKeyword);
    }

    json.EndObject(); 
}

void DumpSignature(IAuthorSignature* signature, JsonBuilder& json)
{
    json.BeginObject();

    CComBSTR description;
    signature->get_Description(&description);
    json.AddField(L"description", description);

    json.AddComma();

    CComBSTR locid;
    signature->get_Locid(&locid);
    json.AddField(L"locid", locid);

    json.AddComma();

    CComBSTR externalFile;
    signature->get_ExternalFile(&externalFile);
    json.AddField(L"externalFile", externalFile);

    json.AddComma();

    CComBSTR externalid;
    signature->get_Externalid(&externalid);
    json.AddField(L"externalid", externalid);

    json.AddComma();

    CComBSTR helpKeyword;
    signature->get_HelpKeyword(&helpKeyword);
    json.AddField(L"helpKeyword", helpKeyword);

    json.AddComma();

    // Dump return value
    CComPtr<IAuthorReturnValue> returnValue;
    signature->get_ReturnValue(&returnValue);
    json.AddField(L"returnValue");
    DumpReturnValue(returnValue, json);
    returnValue = nullptr;

    // Get parameters collection
    CComPtr<IAuthorParameterSet> params;
    signature->get_Parameters(&params);

    // Get parameters count
    int paramsCount = 0;
    params->get_Count(&paramsCount);

    json.AddComma();
    json.AddField(L"parameters");
    json.BeginList();

    // Dump parameters
    for(int pindex = 0; pindex<paramsCount; pindex++)
    {
        if(pindex > 0) json.AddComma();
        json.BeginObject();

        // Get parameter object
        CComPtr<IAuthorParameter> paramInfo;
        params->GetItems(pindex, 1, &paramInfo);

        CComBSTR name;
        paramInfo->get_Name(&name);
        json.AddField(L"name", name);

        json.AddComma();

        CComBSTR description;
        paramInfo->get_Description(&description);
        json.AddField(L"description", description);

        json.AddComma();

        CComBSTR type;
        paramInfo->get_Type(&type);
        json.AddField(L"type", type);

        json.AddComma();

        CComBSTR locid;
        paramInfo->get_Locid(&locid);
        json.AddField(L"locid", locid);

        json.AddComma();

        CComBSTR elementType;
        paramInfo->get_ElementType(&elementType);
        json.AddField(L"elementType", elementType);

        json.AddComma();

        json.AddField(L"signature");

        CComPtr<IAuthorSignature> paramSignature;
        paramInfo->get_FunctionParamSignature(&paramSignature);
        if(paramSignature != nullptr)
        {
            DumpSignature(paramSignature, json);
        } 
        else
        {
            json.BeginObject();
            json.EndObject();
        }

        json.EndObject();
    } // Parameters

    params = nullptr;

    json.EndList(); // End parameters list
            
    json.EndObject(); // End signature object
}

HRESULT DumpFunctionHelp(IAuthorFunctionHelp* funcHelp, JsonBuilder& json)
{
    HRESULT hr = S_OK;

    CComBSTR functionName;
    CComPtr<IAuthorSignatureSet> signatures;

    json.AddField(L"functionHelp");
    json.BeginObject();

    if(funcHelp != nullptr)
    {
        IfFailGo(funcHelp->get_FunctionName(&functionName));
        json.AddField(L"functionName", functionName);
        
        // Get signatures collection
        funcHelp->get_Signatures(&signatures);
        
        // Get signatures count
        int signaturesCount = 0;
        IfFailGo(signatures->get_Count(&signaturesCount));

        json.AddComma();
        json.AddField(L"signatures");
        json.BeginList(); // Begin signatures list

        for (int sindex = 0; sindex <signaturesCount; sindex++)
        {
            // Get a signature 
            CComPtr<IAuthorSignature> signature;
            IfFailGo(signatures->GetItems(sindex, 1, &signature));

            if (sindex != 0) json.AddComma();
            DumpSignature(signature, json);

        } // Signatures

        json.EndList(); // End signatures list
    }

    json.EndObject();
Error:
    return hr;
}

HRESULT CAuthoringHost::GetFunctionHelp(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[])
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    JsonBuilder json; 
    wchar_t *code = NULL;

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scripts && count > 0);

    long offsets[1000];
    int cOffsets;
    LPCWSTR scriptCode = scripts[count - 1]; // Assumes the last file is the primary script.
    IfFailGo(ParseOffsets(scriptCode, offsets, sizeof(offsets)/sizeof(offsets[0]), &cOffsets, &code));

    IfFailGo(scriptEngine->QueryInterface(&authorServices));

    fileContext.Attach(new MultiFileContext(authorServices, code, ::wcslen(code), count - 1, scripts));

    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    json.BeginObject();
    json.AddField(L"functionHelpTests");
    json.BeginList();

    bool first = true;
    for (int i = 0; i < cOffsets; i++)
    {
        CallAfter(5000, CallHurry, authoring);
        CComPtr<IAuthorFunctionHelp> funcHelp = NULL;
        DWORD currentParamIndex = 0;
        AuthorFileRegion extent;
        AuthorDiagStatus diagStatus;
        IfFailGo(authoring->GetFunctionHelp(offsets[i], afhfDefault, &currentParamIndex, &extent, &diagStatus, &funcHelp));
        KillCallback();

        if (!first) json.AddComma();
        first = false;

        json.NewLine();
        json.BeginObject(); // Begin test result object
        if(funcHelp == NULL) 
        {
            json.AddField(L"diagStatus", (int)diagStatus);
            json.EndObject();
            continue;
        }

        json.AddField(L"currentParameterIndex", (int)currentParamIndex);

        json.AddComma();
        json.AddField(L"extentOffset", (int)extent.offset);

        json.AddComma();
        json.AddField(L"extentLength", (int)extent.length);

        json.AddComma();
        DumpFunctionHelp(funcHelp, json);

        json.EndObject(); // End test result object
        
        // Release the function help object
        funcHelp = nullptr;
    }

    json.EndList(); // End test results list
    json.EndObject(); // End functionHelpTests object

    IfFailGo(_jsGlass->EventCallback(json.Psz()));

Error:
    if (code) free(code);
    return hr;
}

static LPCWSTR AuthorTypeToText(AuthorType authorType)
{
    switch(authorType)
    {
    case atUnknown:
        return L"atUnknown";
    case atBoolean:
        return L"atBoolean";
    case atNumber:
        return L"atNumber";
    case atString:
        return L"atString";
    case atObject:
        return L"atObject";
    case atFunction:
        return L"atFunction";
    case atArray:
        return L"atArray";
    case atDate:
        return L"atDate";
    case atRegEx:
        return L"atRegEx";
    }
    return L"";
}

static LPCWSTR AuthorScopeToText(AuthorScope authorScope)
{
    switch(authorScope)
    {
    case ascopeUnknown:
        return L"ascopeUnknown";
    case ascopeGlobal:
        return L"ascopeGlobal";
    case ascopeClosure:
        return L"ascopeClosure";
    case ascopeLocal:
        return L"ascopeLocal";
    case ascopeParameter:
        return L"ascopeParameter";
    case ascopeMember:
        return L"ascopeMember";
    }
    return L"";
}

HRESULT CAuthoringHost::GetQuickInfo(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    JsonBuilder json; 
    
    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scriptCode);

    long offsets[100];
    int cOffsets;
    wchar_t *code;
    IfFailGo(ParseOffsets(scriptCode, offsets, sizeof(offsets)/sizeof(offsets[0]), &cOffsets, &code));

    IfFailGo(scriptEngine->QueryInterface(&authorServices));

    fileContext.Attach(new SingleFileContext(authorServices, code, ::wcslen(code)));

    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));
    AuthorFileRegion extent;

    json.BeginObject();
    json.AddField(L"quickInfos");
    json.BeginList();


    for (int i = 0; i < cOffsets; i++)
    {
        CComPtr<IAuthorSymbolHelp> symbolHelp;

        if (i) json.AddComma();
        json.NewLine();
        json.BeginObject(); // Begin test result object
        json.AddField(L"offset", (int)offsets[i]);

        IfFailGo(authoring->GetQuickInfo(offsets[i], &extent, &symbolHelp));
        if (symbolHelp != nullptr)
        {
            AuthorType type;
            AuthorScope scope;
            CComBSTR typeName;
            CComBSTR name;
            CComPtr<IAuthorFunctionHelp> funcHelp;
            IfFailGo(symbolHelp->get_Type(&type));
            IfFailGo(symbolHelp->get_Scope(&scope));
            IfFailGo(symbolHelp->get_TypeName(&typeName));
            IfFailGo(symbolHelp->get_Name(&name));
            IfFailGo(symbolHelp->get_FunctionHelp(&funcHelp));

            json.AddComma();
            json.AddField(L"extentOffset", (int)extent.offset);
            json.AddComma();
            json.AddField(L"extentLength", (int)extent.length);
            json.AddComma();
            json.AddField(L"Name", name);
            json.AddComma();
            json.AddField(L"Type", AuthorTypeToText(type));
            json.AddComma();
            json.AddField(L"TypeName", typeName);
            json.AddComma();
            json.AddField(L"Scope", AuthorScopeToText(scope));
            json.AddComma();
            DumpFunctionHelp(funcHelp, json);
        }

        json.EndObject();
    }

    json.EndList(); // End test results list
    json.EndObject(); // End functionHelpTests object

    IfFailGo(_jsGlass->EventCallback(json.Psz()));

Error:
    return hr;
}

struct Operation 
{
    size_t offset;
protected:
    Operation(size_t offset): offset(offset) { }
public:
    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t *text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first) { return S_OK; }
    virtual int GetAdjustment() { return 0; }
};

struct InsertOperation : public Operation
{
    LPCWSTR insertText;

    InsertOperation(size_t offset, LPCWSTR text): Operation(offset), insertText(text) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first)
    {
        HRESULT hr = S_OK;

        size_t insertLen = ::wcslen(insertText);
        size_t textLen = ::wcslen(text);
        for (size_t i = textLen + insertLen; i >= offset; i--)
            text[i] = text[i - insertLen];
        for (size_t i = 0; i < insertLen; i++)
            text[i + offset] = insertText[i];
        
        fileContext->AdjustLength((int)insertLen);

        AuthorFileChange change;
        change.offset = this->offset;
        change.charactersRemoved = 0;
        change.charactersInserted = insertLen;

        IfFailGo(fileContext->Handle()->FileChanged(1, &change));

Error:
        return hr;
    }

    virtual int GetAdjustment() 
    {
        return ::wcslen(insertText);
    }
};

struct StatusOperation : public Operation
{
    size_t length;

    StatusOperation(size_t offset, size_t length): Operation(offset), length(length) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first)
    {
        HRESULT hr = S_OK;

        size_t textLen = ::wcslen(text);
        for (size_t i = offset; i < textLen; i++)
            text[i] = text[i + length];

        fileContext->AdjustLength(-(int)length);

        AuthorFileChange change;
        change.offset = this->offset;
        change.charactersRemoved = length;
        change.charactersInserted = 0;
        
        IfFailGo(fileContext->Handle()->FileChanged(1, &change));

Error:
        return hr;
    }

    virtual int GetAdjustment() 
    {
        return -(int)length;
    }
};

struct CompletionsOperation : public Operation
{
    AuthorCompletionFlags flags;

    CompletionsOperation(size_t offset, AuthorCompletionFlags flags = acfMembersFilter): Operation(offset), flags(flags) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first)
    {
        HRESULT hr = S_OK;

        CComPtr<IAuthorCompletionSet> completionSet;

        AuthorCompletion completions[1000];
        int count;
        AuthorFileRegion extent = { 0 };
        
        memset(completions, 0, sizeof(completions));

        //host->CallAfter(1000, CallHurry, authoring);
        IfFailGo(authoring->GetCompletions(offset, flags, &completionSet));
        //host->KillCallback();

        AuthorCompletionSetKind kind = acskMember;
        if (completionSet)
        {
            IfFailGo(completionSet->get_Count(&count));
            ValidatePtr(count < 1000, E_FAIL);
            IfFailGo(completionSet->GetItems(0, count, completions));
            IfFailGo(completionSet->GetExtent(&extent));
            IfFailGo(completionSet->get_Kind(&kind));
        }
        else
            count = 0;

        if (!first) json += ", ";
        first = false;
        RecordCompetion(json, extent, kind, completions, count);

Error:
        return hr;
    }
};

struct RegionsOperation : public Operation
{
    RegionsOperation(size_t offset) : Operation(offset) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first)
    {
        AuthorFileRegion *regions = NULL;
        HRESULT hr = S_OK;
        CComPtr<IAuthorRegionSet> regionSet;

        int length;
        IfFailGo(authoring->GetRegions(&regionSet));
        IfFailGo(regionSet->get_Count(&length));
        regions = static_cast<AuthorFileRegion *>(malloc(sizeof(AuthorFileRegion) * length));
        ValidateAlloc(regions);
        IfFailGo(regionSet->GetItems(0, length, regions));
        AuthorFileStatus status = fileContext->LastStatus();

        if (!first) json += ", ";
        first = false;
        RecordRegions(json, status, regions, length);

Error:
        if (regions) free(regions);
        return hr;
    }
    
};

struct FunctionHelpOperation : public Operation
{
    FunctionHelpOperation(size_t offset) : Operation(offset) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first)
    {
        HRESULT hr = S_OK;

        CComPtr<IAuthorFunctionHelp> funcHelp = NULL;
        DWORD currentParamIndex = 0;
        AuthorFileRegion extent;
        AuthorDiagStatus diagStatus;
        IfFailGo(authoring->GetFunctionHelp(this->offset, afhfDefault, &currentParamIndex, &extent, &diagStatus, &funcHelp));

        if (!first) json += ", ";
        first = false;

        {
            JsonBuilder jsonBuilder;
            jsonBuilder.BeginObject(); // Begin test result object
            if(funcHelp == NULL) 
            {
                jsonBuilder.AddField(L"diagStatus", (int)diagStatus);
            }
            else
            {
                jsonBuilder.AddField(L"currentParameterIndex", (int)currentParamIndex);

                jsonBuilder.AddComma();
                jsonBuilder.AddField(L"extentOffset", (int)extent.offset);

                jsonBuilder.AddComma();
                jsonBuilder.AddField(L"extentLength", (int)extent.length);

                jsonBuilder.AddComma();
                DumpFunctionHelp(funcHelp, jsonBuilder);
            }
            jsonBuilder.EndObject(); // End test result object

            json += jsonBuilder.Psz();
        }
Error:
        return hr;
    }
};

struct AddContextFileOperation : public Operation
{
    LPCWSTR m_filename;

    AddContextFileOperation(size_t offset, LPCWSTR filename): Operation(offset), m_filename(filename) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first) override
    {
        HRESULT hr = S_OK;

        IfFailGo(fileContext->AddContextFile(m_filename));
        IfFailGo(authoring->ContextChanged());

Error:
        return hr;
    }
};

struct RemoveLastContextFileOperation : public Operation
{
    RemoveLastContextFileOperation(size_t offset): Operation(offset) { }

    virtual HRESULT Perform(CAuthoringHost *host, __nullterminated wchar_t * text, CComPtr<IAuthorFileAuthoring> authoring, MultiFileContext *fileContext, CComBSTR &json, bool &first) override
    {
        HRESULT hr = S_OK;

        IfFailGo(fileContext->RemoveContextFile(fileContext->Count() - 1));
        IfFailGo(authoring->ContextChanged());

Error:
        return hr;
    }
};


const size_t NOINDEX = 0xFFFFFFFFU;

static HRESULT AddOperation(Operation *operation, size_t index, Operation** operations, size_t maxOffset, size_t &cOperations)
{
    HRESULT hr = E_FAIL;

    if (index == NOINDEX) index = cOperations;
    if (index >= maxOffset) goto Error;
    if (operations[index] != NULL) goto Error;

    operations[index] = operation;
    if (index >= cOperations) cOperations = index + 1;

    hr = S_OK;

Error:
    return hr;
}

static HRESULT ParseIndex(LPCWSTR scriptCode, size_t &i, size_t &index)
{
    HRESULT hr = S_OK;

    index = NOINDEX;

    if (scriptCode[i] == '#')
    {
        i++;
        size_t newIndex = 0;
        while (true)
        {
            wchar_t ch = scriptCode[i];
            if (ch >= '0' && ch <= '9')
            {
                i++;
                newIndex = newIndex * 10 + (ch - '0');
            }
            else
                break;
        }
        index = newIndex;
    }

    if (scriptCode[i] != ':')
    {
        hr = E_FAIL;
        goto Error;
    }
    i++;

Error:
    return hr;
}

static HRESULT SkipToEndOfOperation(LPCWSTR scriptCode, size_t &i)
{
    HRESULT hr = S_OK;

    size_t li = i;
    while (true)
    {
        wchar_t ch = scriptCode[li++];
        switch (ch)
        {
        case '!': 
            if (scriptCode[li] == '!') 
            {
                i = li - 1;
                goto Done;
            }
            break;
        case '\0':
            hr = E_FAIL;
            goto Error;
        }
    }

Done:
Error:
    return hr;
}

static HRESULT ParseOperations(LPCSTRING scriptCode, __out_ecount(maxOffset) Operation **operations, size_t maxOffset, size_t &count, __nullterminated wchar_t *&source, LPCWSTR defaultPath)
{
    HRESULT hr = S_OK;

    if (!defaultPath) defaultPath = L"";

    size_t len = wcslen(scriptCode);
    wchar_t *code = static_cast<wchar_t *>(malloc((len + 1) * sizeof(wchar_t)));
    ValidateAlloc(code);

    for (size_t i = 0; i < maxOffset; i++) operations[i] = NULL;

    // Collect the operations -
    // Insert:              !+: < insert text > !!   
    // Delete:              !-: < text to delete > !!
    // Completions:         !|:
    // Regions:             !R:
    // Insert ordered:      !+#1: < insert text > !!
    // Delete ordered:      !+#2: < text to delete > !!
    // Completions ordered: !|#3:
    // Regions ordered:     !R#4:

    size_t cOperations = 0;
    size_t j = 0;
    size_t index;
    
    for (size_t i = 0; i <= len; i++)
    {
        wchar_t ch = scriptCode[i];

        if (ch == '!' && i + 1 < len)
        {
            ch = scriptCode[i + 1];
            switch (ch)
            {
            case '+':
                {
                    // Insert operation
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    size_t start = i;
                    IfFailGo(SkipToEndOfOperation(scriptCode, i));
                    size_t end = i;
                    i++;
                    size_t insertLen = end - start;
                    wchar_t *insertText = static_cast<wchar_t *>(malloc((insertLen + 1) * sizeof(wchar_t)));
                    ValidateAlloc(insertText);
                    for (size_t k = 0; k < insertLen; k++) insertText[k] = scriptCode[start + k];
                    insertText[insertLen] = '\0';
                    InsertOperation *insertOperation = new InsertOperation(j, insertText);
                    ValidateAlloc(insertOperation);
                    IfFailGo(AddOperation(insertOperation, index, operations, maxOffset, cOperations));
                    continue;
                }

            case '-':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    size_t start = i;
                    IfFailGo(SkipToEndOfOperation(scriptCode, i));
                    size_t end = i;
                    i++;
                    size_t deleteLen = end - start;
                    for (size_t k = 0; k < deleteLen; k++) code[j++] = scriptCode[start + k];
                    StatusOperation *deleteOperation = new StatusOperation(j, deleteLen);
                    ValidateAlloc(deleteOperation);
                    IfFailGo(AddOperation(deleteOperation, index, operations, maxOffset, cOperations));
                    continue;
                }

            case 'I':
            case '|':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    i--;
                    CompletionsOperation *completionsOperation = new CompletionsOperation(j, ch == '|' ? acfMembersFilter : AuthorCompletionFlags(acfMembersFilter | acfImplicitRequest));
                    ValidateAlloc(completionsOperation);
                    IfFailGo(AddOperation(completionsOperation, index, operations, maxOffset, cOperations));
                    continue;
                }
            case '(':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    i--;
                    FunctionHelpOperation *functionHelpOperation = new FunctionHelpOperation(j);
                    ValidateAlloc(functionHelpOperation);
                    IfFailGo(AddOperation(functionHelpOperation, index, operations, maxOffset, cOperations));
                    continue;
                }
            case 'R':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    i--;
                    RegionsOperation *regionsOperation = new RegionsOperation(j);
                    ValidateAlloc(regionsOperation);
                    IfFailGo(AddOperation(regionsOperation, index, operations, maxOffset, cOperations));
                    continue;
                }
            case 'A':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    size_t start = i;
                    IfFailGo(SkipToEndOfOperation(scriptCode, i));
                    size_t end = i;
                    i++;
                    size_t nameLen = end - start;
                    size_t defaultPathLen = ::wcslen(defaultPath);
                    wchar_t *filename = static_cast<wchar_t *>(malloc((defaultPathLen + 1 + nameLen + 1) * sizeof(wchar_t)));
                    ValidateAlloc(filename);
                    for (size_t k = 0; k < defaultPathLen; k++) filename[k] = defaultPath[k];
                    for (size_t k = 0; k < nameLen; k++) filename[defaultPathLen + k] = scriptCode[start + k];
                    filename[defaultPathLen + nameLen] = '\0';
                    AddContextFileOperation *addContextFileOperation = new AddContextFileOperation(j, filename);
                    ValidateAlloc(addContextFileOperation);
                    IfFailGo(AddOperation(addContextFileOperation, index, operations, maxOffset, cOperations));
                    continue;
                }
            case 'D':
                {
                    i += 2;
                    IfFailGo(ParseIndex(scriptCode, i, index));
                    i--;
                    RemoveLastContextFileOperation *removeLastContextFileOperation = new RemoveLastContextFileOperation(j);
                    ValidateAlloc(removeLastContextFileOperation);
                    IfFailGo(AddOperation(removeLastContextFileOperation, index, operations, maxOffset, cOperations));
                    continue;
                }
            }
        }

        code[j++] = ch;
    }

    for (size_t i = 0; i < cOperations; i++)
        ValidatePtr(operations[i], E_FAIL);

    // Adjust offsets to operations assuming they will be performed in order.
    for (size_t i = 0; i < cOperations; i++)
    {
        Operation *operation = operations[i];
        size_t offset = operation->offset;
        int adjustment = operation->GetAdjustment();
        if (adjustment)
        {
            for (size_t j = i + 1; j < cOperations; j++)
            {
                Operation *op = operations[j];
                if (op->offset >= offset) op->offset = (size_t)((int)op->offset + adjustment);
            }
        }
    }

    source = code;
    count = cOperations;

Error:
    return hr;
}

HRESULT CAuthoringHost::ProcessCompletionsSession(IUnknown *scriptEngine, LPCWSTR scriptCode, LPCWSTR defaultPath)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    CComBSTR json = L"{ \"session\": [";

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scriptCode);

    Operation *operations[50];
    size_t cOperations;
    wchar_t *code;
    bool first = true;

    IfFailGo(ParseOperations(scriptCode, operations, sizeof(operations)/sizeof(operations[0]), cOperations, code, defaultPath));

    IfFailGo(scriptEngine->QueryInterface(&authorServices));
    MultiFileContext *multiFileContext = new MultiFileContext(authorServices, code, ::wcslen(code), 0, NULL);
    fileContext.Attach(multiFileContext);
    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    for (size_t i = 0; i < cOperations; i++)
    {
        Operation *operation = operations[i];
        IfFailGo(operation->Perform(this, code, authoring, multiFileContext, json, first));
    }
    json += L"] }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    return hr;
}

HRESULT CAuthoringHost::SplatterSession(IUnknown *scriptEngine, LPCWSTR scriptCode)
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    CComBSTR json = L"";

    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scriptCode);

//    size_t cOperations;
//    bool first = true;
    size_t len = ::wcslen(scriptCode);
    size_t size = len + 1000;
    ValidateArg(size > len);

    wchar_t *code = static_cast<wchar_t *>(malloc(size * sizeof(wchar_t)));
    ::wcscpy_s(code, size, scriptCode);
    IfFailGo(scriptEngine->QueryInterface(&authorServices));
    SplatterFileContext *splatterFileContext = new SplatterFileContext(authorServices, code, len, size);
    fileContext.Attach(splatterFileContext);
    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    // Expose the file character by character asking for region information and a completion set
    for (size_t i = 0; i < len; i++)
    {
        CComPtr<IAuthorRegionSet> regions;
        CComPtr<IAuthorCompletionSet> completions;

        IfFailGo(splatterFileContext->ExtendView());
        IfFailGo(authoring->GetRegions(&regions));
        IfFailGo(authoring->GetCompletions((long)i, acfAny, &completions));
    }

    // Get a completion list for every character in the file.
    for (size_t i = 0; i < len; i++)
    {
        CComPtr<IAuthorCompletionSet> completions;
        IfFailGo(authoring->GetCompletions((long)i, acfAny, &completions));
    }

    // Insert a "foo." at every 10 characters location in the file, get regions then completions, then delete the text.
    for (size_t i = 0; i < len; i += 10)
    {
        CComPtr<IAuthorRegionSet> regions;
        CComPtr<IAuthorCompletionSet> completions;

        IfFailGo(splatterFileContext->InsertText(i, L"foo."));
        IfFailGo(authoring->GetRegions(&regions));
        CallAfter(200, CallHurry, authoring);
        IfFailGo(authoring->GetCompletions(i + 4, acfAny, &completions));
        KillCallback();
        IfFailGo(splatterFileContext->DeleteText(i, 4));
    }

    // Retract every character in the file
    for (size_t i = len; i > 0; i--)
    {
        CComPtr<IAuthorRegionSet> regions;
        CComPtr<IAuthorCompletionSet> completions;

        IfFailGo(splatterFileContext->RetractView());
        IfFailGo(authoring->GetRegions(&regions));
        IfFailGo(authoring->GetCompletions((long)i, acfAny, &completions));
    }

    json += L"{ \"success\": true }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    return hr;
}

HRESULT CAuthoringHost::PerformGetCompletionRequest(IAuthorFileAuthoring* authoring, long offset, AuthorCompletionFlags flags, CComBSTR &json)
{
    CComPtr<IAuthorCompletionSet> completionSet;
    AuthorCompletion completions[1000];
    AuthorFileRegion extent;
    AuthorCompletionSetKind kind;
    memset(completions, 0, sizeof(completions));
    int count = 0;

    HRESULT hr = S_OK;

    CallAfter(1000, CallHurry, authoring);
    hr = authoring->GetCompletions(offset, flags, &completionSet);
    KillCallback();
    IfFailGo(hr);

    if (NULL != completionSet)
    {
        IfFailGo(completionSet->get_Count(&count));
        ValidatePtr(count < 1000, E_FAIL);
        IfFailGo(completionSet->GetItems(0, count, completions));
        IfFailGo(completionSet->GetExtent(&extent));
        IfFailGo(completionSet->get_Kind(&kind));
    }
    else
    {
        extent.length = 0;
        extent.offset = 0;
        kind = (AuthorCompletionSetKind)( AuthorCompletionSetKind::acskStatement + 1 );
    }
    RecordCompetion(json, extent, kind, completions, count);

Error:
    for (int i = 0; i < sizeof(completions)/sizeof(completions[0]); i++)
        FinalizeCompletion(completions[i]);

    return hr;
}

HRESULT CAuthoringHost::MultipleHostTypeCompletion(IUnknown *scriptEngine, int count, __in_ecount(count) LPCWSTR scripts[])
{
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> browserAuthoring;
    CComPtr<IAuthorFileAuthoring> applicationAuthoring;
    CComPtr<IAuthorFileContext> browserFileContext;
    CComPtr<IAuthorFileContext> applicationFileContext;
    CComBSTR json = L"{ \"completions\": [";
    LPSTRING code = NULL;
    HRESULT hr = S_OK;
    ValidateArg(scriptEngine && scripts && count > 0);

    long offsets[100];
    int cOffsets;
    LPCWSTR scriptCode = scripts[count - 1]; // Assumes the last file is the primary script.
    IfFailGo(ParseOffsets(scriptCode, offsets, sizeof(offsets)/sizeof(offsets[0]), &cOffsets, &code));

    IfFailGo(scriptEngine->QueryInterface(&authorServices));

    // Initialize the browser host context
    browserFileContext.Attach(new MultiFileContext(authorServices, code, ::wcslen(code), count - 1, scripts, AuthorHostType::ahtBrowser));
    IfFailGo(authorServices->GetFileAuthoring(browserFileContext, &browserAuthoring));

    // Initialize the application host context
    applicationFileContext.Attach(new MultiFileContext(authorServices, code, ::wcslen(code), count - 1, scripts, AuthorHostType::ahtApplication));
    IfFailGo(authorServices->GetFileAuthoring(applicationFileContext, &applicationAuthoring));

    bool first = true;
    for (int i = 0; i < cOffsets; i++)
    {
        if (!first) json += ", ";
            first = false;
        json += L"{ \"browserCompletions\": ";
        PerformGetCompletionRequest(browserAuthoring, offsets[i], AuthorCompletionFlags::acfMembersFilter, json);
        json += L", \"applicationCompletions\": ";
        PerformGetCompletionRequest(applicationAuthoring, offsets[i], AuthorCompletionFlags::acfMembersFilter, json);
        json += "}";
    }

    json += "] }";
    IfFailGo(_jsGlass->EventCallback(json));

Error:
    if (code) free(code);
    return hr;
}

#endif