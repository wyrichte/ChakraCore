/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once
#ifdef LANGUAGE_SERVICE_TEST

#define ValidatePtr(expr, err) \
do { \
if (!(expr)) { \
    hr = err; \
    goto Error; \
} \
} while (0)

#define ValidateAlloc(expr) ValidatePtr(expr, E_OUTOFMEMORY);
#define ValidateArg(expr) ValidatePtr(expr, E_INVALIDARG);

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

class Logger
{

public:
    enum Level { Simple, Verbose };
    Level m_level;

    Logger() : m_level(Level::Simple){ }
    
    void SetLevel(Level level) { m_level = level; }
    void LogMessage(__in __nullterminated wchar_t *msg, ...);
    void LogVerbose(__in __nullterminated wchar_t *msg, ...);
    void LogError(__in __nullterminated wchar_t *msg, ...);
};

class LanguageServiceTestDriver;

class OperationBase
{
protected:
    Logger m_logger;
    std::wstring* m_primarySourceFileText;
public:
    OperationBase(Logger logger) : m_logger(logger), m_primarySourceFileText(0) { }
    enum Kind { Completion, GotoDef };
    Kind kind;
    virtual ~OperationBase() { }
    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring) = 0;
    
    static void FinalizeBSTR(BSTR& str);
    static void FinalizeCompletion(AuthorCompletion& completion);

    void SetPrimaryText(std::wstring* primarySourceFileText)
    {
        this->m_primarySourceFileText = primarySourceFileText;
    }
};

class GetRegionOperation : public OperationBase
{
public:
    GetRegionOperation(Logger logger) : OperationBase(logger)
    {

    }

    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
};

class GetTaskCommentOperation : public OperationBase
{
public:
    GetTaskCommentOperation(Logger logger, std::list<std::wstring> prefixes) : OperationBase(logger), m_prefixes(prefixes)
    {

    }

    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
private:

    std::list<std::wstring> m_prefixes;
};

class CompletionOperation : public OperationBase
{
public:
    CompletionOperation(Logger logger) : OperationBase(logger), dumpWholeMemberList(false), ignoreMemberList(false) { }

    bool dumpWholeMemberList; // This is used for dumping whole list, in order to see the insight of that object
    bool ignoreMemberList;    // This is to test DocCommentsParsing
    long m_offset;
    
    std::list<std::wstring> m_expected;
    std::list<std::wstring> m_unexpected;

    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
};

class GotoDefOperation : public OperationBase
{
public:
    GotoDefOperation(Logger logger) : OperationBase(logger) { }

    long m_offset;
    std::wstring m_id;

    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
};

class FunctionHelpOperation : public CompletionOperation
{
public:
    FunctionHelpOperation(Logger logger) : CompletionOperation(logger), m_isStrictCompare(false) { }
    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);

    bool m_isStrictCompare;
};

class GetStructureOperation : public OperationBase
{
public:
    GetStructureOperation(Logger logger, long offset) : OperationBase(logger), m_offset(offset), shouldPrintStructure(true) {}

    long m_offset;
    bool shouldPrintStructure;
    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);

private:
    void PrintStructureNodes(IAuthorStructureNodeSet * nodes);
    void PrintStructureKind(AuthorStructureNodeKind kind);
    void PrintStructureSubTree(IAuthorStructure * structure, int key, std::wstring indent);

    void CleanupAuthorStructureNode(AuthorStructureNode *structureNode);
};

class GetReferencesOperation : public OperationBase
{
public:
    GetReferencesOperation(Logger logger, long offset) : OperationBase(logger), m_offset(offset) { }

    long m_offset;

    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
};

class GetSubtreeOperation : public OperationBase
{
public:
    GetSubtreeOperation(Logger logger, long offset) : OperationBase(logger), m_offset(offset), m_depth(0) { }

    long m_offset;
    long m_depth;
    virtual HRESULT Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring);
private:
    void PrintParseNodeSet(IAuthorParseNodeSet *set);
};

struct Target
{
    long m_offset;
    std::wstring m_id;
};

enum TypingMode
{
    Perf,
    Stress
};

class LanguageServiceTestDriver
{
    CAuthoringHost            *m_authoringHost;
    std::wstring              m_jslsPath;
    Logger                    m_logger;
    std::list<Target>         m_targets;
    std::list<OperationBase*> m_testOperations;
    
    std::wstring m_primarySourceFileName;
    std::wstring m_primarySourceFileText;
    std::list<std::wstring> m_referenceSourceFiles;

    HRESULT ParseAnnotations(std::wstring const& code, std::wstring& newCode);

public:
    LanguageServiceTestDriver(std::wstring const& jslsPath);
    ~LanguageServiceTestDriver();

    std::list<Target> const& GetTargets() { return m_targets; }
    HRESULT InitializeHost();
    HRESULT AddFile(std::wstring const& fileName);
    HRESULT ExecuteTests();
    HRESULT RunStress();
    HRESULT RunPerf();
    HRESULT RunTyping(TypingMode mode);
    CAuthoringHost *GetHost() { return m_authoringHost; }
    void SetVerbose() { m_logger.SetLevel(Logger::Level::Verbose); }
};

template <class Interface>
class SimpleOleClass : public Interface
{
private:
    long m_refCount;
public:
    SimpleOleClass() : m_refCount(1) { }
    virtual ~SimpleOleClass() { }

    // *** IUnknown ***
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj)
    {
        QI_IMPL_INTERFACE(IUnknown);
        QI_IMPL_INTERFACE(Interface);

        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)(void)
    {
        return ++m_refCount;
    }

    STDMETHOD_(ULONG, Release)(void)
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
    StringFileReader(LPCSTRING string, size_t stringLength) : string(string), stringLength(stringLength), offset(0) { }

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
            memcpy(buffer, &(((LPWSTR)string)[this->offset]), sizeof(wchar_t)* copyLength);
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
    std::wstring fileName;
    AuthorFileStatus status;
    bool shouldFreeText;
public:
    AuthoringFile(LPCWSTR _fileName, LPCSTRING text, size_t length, bool _shouldFreeText = false) : fileName(_fileName), text(text), length(length), shouldFreeText(_shouldFreeText) { }
    ~AuthoringFile()
    {
        if (shouldFreeText)
        {
            HeapFree(GetProcessHeap(), 0, (void*)text);
        }
    }
    AuthorFileStatus LastStatus() { return this->status; }

    void AdjustLength(int adjustment) { length = (size_t)((int)length + adjustment); }
    size_t Length() { return length; }
    LPCSTRING Text() { return text; }

    STDMETHOD(GetDisplayName)(BSTR *name)
    {
        if (name == nullptr) return E_POINTER;

        if (fileName.size() > 0)
        {
            *name = ::SysAllocStringLen(fileName.c_str(), fileName.size());
        }
        else
        {
            *name = nullptr;
        }
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

    STDMETHOD(StatusChanged)(AuthorFileStatus newStatus)
    {
        status = newStatus;
        return S_OK;
    }

};


class MultiFileContext : public SimpleOleClass<IAuthorFileContext>
{
protected:
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
    LanguageServiceTestDriver *m_driver;
    IAuthorFileAuthoring *m_authoring;
public:
    MultiFileContext(IAuthorServices *services,
        const wchar_t* fileName,
        const wchar_t* primaryFileText,
        size_t length,
        LanguageServiceTestDriver *driver)
        : m_count(0),
          m_services(services),
          m_hostType(AuthorHostType::ahtBrowser),
          m_scriptVersion(AuthorScriptVersion::ScriptVersion12),
          m_sources(nullptr),
          m_sourceHandles(nullptr),
          m_driver(driver),
          m_authoring(nullptr)
    {
        m_primary = new AuthoringFile(fileName, primaryFileText, length);
        services->RegisterFile(m_primary, &m_primaryHandle);
    }

    virtual ~MultiFileContext()
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

    void SetAuthorHostType(AuthorHostType hostType)
    {
        m_hostType = hostType;
    }

    void SetAuthorScriptVersion(AuthorScriptVersion scriptVersion)
    {
        m_scriptVersion = scriptVersion;
    }

    HRESULT LoadAuthoringFile(LPCWSTR filename, AuthoringFile** authoringFile);

    HRESULT AddContextFile(LPCWSTR filename)
    {
        return AddContextFile(filename, m_count);
    }

    void SetFileAuthoring(IAuthorFileAuthoring *authoring)
    {
        this->m_authoring = authoring;
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

    STDMETHOD(GetContextFiles)(int startIndex, int count, __out_ecount(count) IAuthorFileHandle **handles)
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

        *scriptVersion = m_scriptVersion;

    Error:
        return hr;
    }

    STDMETHOD(FileAuthoringPhaseChanged)(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle *executingFile);

    STDMETHOD(LogMessage)(BSTR message)
    {
        HRESULT hr = S_OK;
        if (message)
        {
            fwprintf(stdout, L"%ls", message);
        }
        //Error:
        return hr;
    }

    STDMETHOD(VerifyScriptInContext)(BSTR url, BSTR charset, BSTR type, IAuthorFileHandle *sourceFile, VARIANT_BOOL insertBeforeSourceFile, AuthorFileAsynchronousScriptStatus* status)
    {
        HRESULT hr = S_OK;

        AuthoringFile* authoringFile;

        if (url && *url)
        {
            if (LoadAuthoringFile((LPCWSTR)url, &authoringFile) != S_OK)
            {
                // If not able to locate the file - just ignore and proceed.
                *status = AuthorFileAsynchronousScriptStatus::afassIgnored;
                return S_OK;
            }
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
            for (index = 0; index < m_count; index++)
            {
                if (m_sourceHandles[index] == sourceFile)
                    break;
            }
            Assert(index != m_count); // source file has to be in the context
        }

        // check if the file exists in the context
        for (int i = 0; i < m_count; i++)
        {
            if (m_sources[i]->Length() == authoringFile->Length() &&
                ::wcsncmp(m_sources[i]->Text(), authoringFile->Text(), m_sources[i]->Length()) == 0)
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


class SingleFileContext : public SimpleOleClass<IAuthorFileContext>
{
private:
    AuthoringFile *file;
    IAuthorFileHandle *handle;

public:
    SingleFileContext(IAuthorServices *services, const wchar_t* fileName, LPCSTRING text, size_t length)
    {
        file = new AuthoringFile(fileName, text, length);
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
        if (message)
        {
            fwprintf(stdout, L"%ls", message);
        }

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

class SplatterFileContext : public MultiFileContext
{
private:
    size_t m_splatterTextSize;
    size_t m_actualLength;
    wchar_t *m_splatterText;

public:
    SplatterFileContext(IAuthorServices *services,
        const wchar_t *fileName,
        const wchar_t *text,
        size_t length,
        size_t size,
        LanguageServiceTestDriver *driver) 
        : MultiFileContext(services, fileName, text, 0, driver),
        m_actualLength(length), m_splatterTextSize(size)
    {
        m_splatterText = (wchar_t*)text;
    }

    ~SplatterFileContext()
    {
    }

    wchar_t GetLastChar()
    {
        size_t textLen = m_primary->Length();
        if (textLen > 0 && textLen <= m_actualLength)
        {
            return m_splatterText[textLen - 1];
        }
        return L'\0';
    }

    wchar_t GetCharAt(size_t index)
    {
        if (index < m_actualLength)
        {
            return m_splatterText[index];
        }
        return L'\0';
    }

    HRESULT ExtendView(int numChar = 1)
    {
        HRESULT hr = S_OK;
        size_t previousLength = m_primary->Length();
        ValidateArg(previousLength < m_actualLength);
        ValidateArg(numChar >= 1);
        ValidateArg(previousLength + numChar <= m_actualLength);

        m_primary->AdjustLength(numChar);
        AuthorFileChange change;
        change.charactersInserted = numChar;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(m_primaryHandle->FileChanged(numChar, &change));
    Error:
        return hr;
    }

    HRESULT ViewAll()
    {
        HRESULT hr = S_OK;

        size_t previousLength = m_primary->Length();
        ValidateArg(previousLength < m_actualLength);

        size_t exposed = m_actualLength - previousLength;
        m_primary->AdjustLength(exposed);
        AuthorFileChange change;
        change.charactersInserted = exposed;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(m_primaryHandle->FileChanged(1, &change));
    Error:
        return hr;
    }

    HRESULT RetractView()
    {
        HRESULT hr = S_OK;

        size_t previousLength = m_primary->Length();
        ValidateArg(previousLength > 0);

        m_primary->AdjustLength(-1);
        AuthorFileChange change;
        change.charactersInserted = 1;
        change.charactersRemoved = 0;
        change.offset = (long)previousLength;
        IfFailGo(m_primaryHandle->FileChanged(1, &change));

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
        newLength = m_actualLength + insertLen;

        ValidateArg(newLength >= m_actualLength);
        ValidateArg(m_primary->Length() == m_actualLength);
        ValidateArg(offset <= m_actualLength);
        ValidateArg(newLength <= m_splatterTextSize);

        // Physically insert the new characters
        //for (size_t i = newLength; i > offset; i--)
        //    text[i] = text[i - insertLen];
        memmove(&m_splatterText[offset + insertLen], &m_splatterText[offset], m_actualLength - offset);
        memcpy(&m_splatterText[offset], insertText, insertLen);

        //text[offset] = text[offset - insertLen];
        //for (size_t i = 0; i < insertLen; i++)
        //    text[i + offset] = insertText[i];

        // Update the lengths
        m_primary->AdjustLength(insertLen);
        m_actualLength = newLength;

        // Notify the file handle the file was changed.
        AuthorFileChange change;
        change.charactersInserted = insertLen;
        change.charactersRemoved = 0;
        change.offset = (long)offset;
        IfFailGo(m_primaryHandle->FileChanged(1, &change));

    Error:
        return hr;
    }

    HRESULT DeleteText(size_t offset, size_t deleteLen)
    {
        HRESULT hr = S_OK;
        size_t newLength = 0;

        newLength = m_actualLength - deleteLen;
        ValidateArg(newLength < m_actualLength);
        ValidateArg(m_primary->Length() == m_actualLength);
        ValidateArg(offset < m_actualLength);

        // Physically remove the characters
        for (size_t i = offset; i < m_actualLength; i++)
            m_splatterText[i] = m_splatterText[i + deleteLen];
        memcpy(&m_splatterText[offset], &m_splatterText[offset + deleteLen], m_actualLength - deleteLen - offset);

        // Update the lengths
        m_primary->AdjustLength(-(long)deleteLen);
        m_actualLength = newLength;

        // Notify the file handle the file was changed
        AuthorFileChange change;
        change.charactersInserted = 0;
        change.charactersRemoved = deleteLen;
        change.offset = (long)offset;
        IfFailGo(m_primaryHandle->FileChanged(1, &change));

    Error:
        return hr;
    }

};

#endif

