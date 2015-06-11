// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"

#ifdef RLEXE_XML_DATASOURCE
#include "XmlLite.h"
#include "wextestclass.h"
#include <shlwapi.h> 
#include <atlsafe.h>
#include <versionhelpers.h>

#pragma comment(lib, "xmllite.lib")

using namespace WEX::Logging;
using namespace WEX::TestExecution;

#define IFFAILRET(cond) if (FAILED(hr = (cond))) { return hr; }
#define LOGGED_IFFAILRET(cond, msg, ...) if (FAILED(hr = (cond))) { WEX::Common::String errorMessage; errorMessage.AppendFormat(msg, __VA_ARGS__); LogError(errorMessage); return hr; }

namespace ChakraNativeTaefTests
{
    template < typename InterfaceType >
    class ComObjectImpl : public InterfaceType
    {
    protected:
        ComObjectImpl() : m_cRef(0) {}
        virtual ~ComObjectImpl() {}

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
        {
            if (IsEqualIID(riid, __uuidof(InterfaceType)))
                *ppv = (InterfaceType *)this;
            else if (IsEqualIID(riid, IID_IUnknown))
                *ppv = (IUnknown *)this;
            else
            {
                *ppv = NULL;
                return E_NOINTERFACE;
            }

            AddRef();
            return NOERROR;
        }

        virtual ULONG STDMETHODCALLTYPE AddRef()
        {
            return InterlockedIncrement((long *)&m_cRef);
        }

        virtual ULONG STDMETHODCALLTYPE Release()
        {
            LONG cRef;

            cRef = InterlockedDecrement((long *)&m_cRef);
            if (cRef)
                return cRef;

            delete this;
            return 0;
        }
    private:
        ULONG m_cRef;
    };

    class RLExeXmlDataRow : public ComObjectImpl<IDataRow>
    {       
    public:
        RLExeXmlDataRow(CComBSTR& command, CComBSTR& files, CComBSTR& compileflags, CComBSTR& baseline, 
            CComBSTR& overrideTarget, CComBSTR& overrideOS, CComBSTR& overrideBaseline, CComBSTR& overrideCompileFlags,
            CComSafeArray<BSTR>& excludeVariant, CComSafeArray<BSTR>& excludeArch, CComSafeArray<BSTR>& excludeFlavor,
            CComSafeArray<BSTR>& excludeOS, CComSafeArray<BSTR>& excludeTags, CComSafeArray<BSTR>& includeTags, CComSafeArray<BSTR>& unknownTags,
            bool parallel, bool elevated, bool isEdit) : parallel(parallel), elevated(elevated), isEdit(isEdit)
        {
            this->command.Attach(command.Detach());
            this->files.Attach(files.Detach());
            this->compileflags.Attach(compileflags.Detach());
            this->baseline.Attach(baseline.Detach());
            this->overrideTarget.Attach(overrideTarget.Detach());
            this->overrideOS.Attach(overrideOS.Detach());
            this->overrideBaseline.Attach(overrideBaseline.Detach());
            this->overrideCompileFlags.Attach(overrideCompileFlags.Detach());
            if (excludeArch != NULL)
            {
                this->excludeArch.Attach(excludeArch.Detach());
            }
            if (excludeVariant != NULL)
            {
                this->excludeVariant.Attach(excludeVariant.Detach());
            }
            if (excludeOS != NULL)
            {
                this->excludeOS.Attach(excludeOS.Detach());
            }
            if (excludeFlavor != NULL)
            {
                this->excludeFlavor.Attach(excludeFlavor.Detach());
            }
            if (excludeTags != NULL)
            {
                this->excludeTags.Attach(excludeTags.Detach());
            }
            if (includeTags != NULL)
            {
                this->includeTags.Attach(includeTags.Detach());
            }
            if (unknownTags != NULL)
            {
                this->unknownTags.Attach(unknownTags.Detach());
            }
        }

        virtual HRESULT STDMETHODCALLTYPE GetTestData(
            BSTR pszName,
            _COM_Outptr_result_maybenull_ SAFEARRAY** ppData)
        {
            CComSafeArray<BSTR> testData;
            
            if (wcscmp(pszName, L"Command") == 0)
            {
                if (command != NULL)
                {
                    testData.Add(command);
                }
            }
            else if (wcscmp(pszName, L"FileName") == 0)
            {
                if (files != NULL)
                {
                    testData.Add(files);
                }                            
            }
            else if (wcsncmp(pszName, L"Baseline", 8) == 0)
            {
                if (pszName[8] == L'\0')
                {
                    if (baseline != NULL)
                    {
                        testData.Add(baseline);
                    }
                }
                else if (pszName[8] == L'-')
                {
                    if (overrideBaseline && 
                        ((overrideTarget && ::_wcsicmp(pszName + 9, overrideTarget) == 0)
                        || (overrideOS && ::_wcsicmp(pszName + 9, overrideOS) == 0)))
                    {
                        testData.Add(overrideBaseline);
                    }                    
                }               
                else
                {
                    return E_FAIL;
                }
            }
            else if (wcsncmp(pszName, L"Flags", 5) == 0)
            {
                if (pszName[5] == L'\0')
                {
                    if (compileflags != NULL)
                    {
                        testData.Add(compileflags);
                    }
                }
                else if (pszName[5] == L'-')
                {
                    if (overrideCompileFlags &&
                        ((overrideTarget && ::_wcsicmp(pszName + 6, overrideTarget) == 0)
                        || (overrideOS && ::_wcsicmp(pszName + 6, overrideOS) == 0)))
                    {
                        testData.Add(overrideCompileFlags);
                    }
                }
                else
                {
                    return E_FAIL;
                }               
            }
            else if (wcscmp(pszName, L"ExcludeTags") == 0)
            {
                if (excludeTags != NULL)
                {
                    testData.CopyFrom(excludeTags);
                }
            }
            else if (wcscmp(pszName, L"IncludeTags") == 0)
            {
                if (includeTags != NULL)
                {
                    testData.CopyFrom(includeTags);
                }
            }
            else if (wcscmp(pszName, L"UnknownTags") == 0)
            {
                if (unknownTags != NULL)
                {
                    testData.CopyFrom(unknownTags);
                }
            }
            else if (wcscmp(pszName, L"ExcludeVariant") == 0)
            {
                if (excludeVariant != NULL)
                {
                    testData.CopyFrom(excludeVariant);
                }
            }
            else if (wcscmp(pszName, L"ExcludeArch") == 0)
            {
                if (excludeArch != NULL)
                {
                    testData.CopyFrom(excludeArch);
                }
            }
            else if (wcscmp(pszName, L"ExcludeOS") == 0)
            {
                if (excludeOS != NULL)
                {
                    testData.CopyFrom(excludeOS);
                }
            }
            else if (wcscmp(pszName, L"ExcludeFlavor") == 0)
            {
                if (excludeFlavor != NULL)
                {
                    testData.CopyFrom(excludeFlavor);
                }
            }
            else
            {
                return E_FAIL;
            }

            if (testData != NULL)
            {
                *ppData = testData.Detach();
                return S_OK;
            }
            *ppData = NULL;
            return S_FALSE;
        }

        virtual HRESULT STDMETHODCALLTYPE GetMetadataNames(
            _COM_Outptr_result_maybenull_ SAFEARRAY** ppMetadataNames)
        {
            CComSafeArray<BSTR> metadataNames;
            if (!parallel)
            {                
                metadataNames.Add(L"Parallel");
            }
            if (elevated)
            {
                metadataNames.Add(L"RunAs");              
            }
            if (isEdit)
            {
                metadataNames.Add(L"Edit");
            }
            if (compileflags)
            {
                HRESULT hr = S_OK;
                CComBSTR groupName;
                IFFAILRET(GetExecutionGroup(&groupName));
                if (groupName)
                {
                    metadataNames.Add(L"ExecutionGroup");
                }
            }

            if (metadataNames != NULL)
            {
                *ppMetadataNames = metadataNames.Detach();
                return S_OK;
            }
            *ppMetadataNames = NULL;
            return S_FALSE;
        }

        virtual HRESULT STDMETHODCALLTYPE GetMetadata(
            BSTR pszName,
            _COM_Outptr_result_maybenull_ SAFEARRAY** ppData)
        {
            if (!parallel && wcscmp(pszName, L"Parallel") == 0)
            {
                CComSafeArray<BSTR> data;
                data.Add(L"false");
                *ppData = data.Detach();
                return S_OK;
            }
            if (elevated && wcscmp(pszName, L"RunAs") == 0)
            {
                CComSafeArray<BSTR> data;
                data.Add(L"Elevated");
                *ppData = data.Detach();
                return S_OK;
            }
            if (isEdit && wcscmp(pszName, L"Edit") == 0)
            {
                CComSafeArray<BSTR> data;
                data.Add(L"1");
                *ppData = data.Detach();
                return S_OK;
            }
            if (compileflags && wcscmp(pszName, L"ExecutionGroup") == 0)
            {
                HRESULT hr = S_OK;
                CComSafeArray<BSTR> data;
                CComBSTR groupName;
                IFFAILRET(GetExecutionGroup(&groupName));
                data.Add(groupName);
                *ppData = data.Detach();
                return S_OK;
            }
            return E_FAIL;
        }

        virtual HRESULT STDMETHODCALLTYPE GetName(
            _COM_Outptr_result_maybenull_ BSTR* ppszRowName)
        {
            // Use the file name as the name of the test
            *ppszRowName = files ? files.Copy() : command.Copy();
            if (*ppszRowName == NULL)
            {
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

    private:
        bool parallel;
        bool elevated;
        bool isEdit;        // if this test has "edit" tag
        CComBSTR command;
        CComBSTR files;        
        CComBSTR compileflags;
        CComBSTR baseline;
        CComBSTR overrideTarget;
        CComBSTR overrideOS;
        CComBSTR overrideBaseline;
        CComBSTR overrideCompileFlags;
        CComSafeArray<BSTR> excludeVariant;
        CComSafeArray<BSTR> excludeArch;
        CComSafeArray<BSTR> excludeOS;
        CComSafeArray<BSTR> excludeFlavor;       
        CComSafeArray<BSTR> excludeTags;
        CComSafeArray<BSTR> includeTags;
        CComSafeArray<BSTR> unknownTags;

        HRESULT GetExecutionGroup(_Outptr_result_maybenull_z_ BSTR* pbstrGroupName)
        {
            HRESULT hr = S_OK;
            *pbstrGroupName = nullptr;

            // If a test produces or consumes custom dynamic profile, put them into the same execution group
            CComBSTR flags = this->compileflags;
            IFFAILRET(flags.ToLower());

            const wchar_t PREFIX[] = L"-dynamicprofile";
            PCWSTR p = flags;
            for (;;)
            {
                p = wcsstr(p, PREFIX);
                if (!p) break; // No more -dynamicprofile like switches

                p += _countof(PREFIX) - 1; // Skip "-dynamicprofile"
                if (wcsncmp(p, L"cache:", 6) == 0 || wcsncmp(p, L"input:", 6) == 0)
                {
                    p += 6; // Skip "cache:" or "input:". Use the shared dynamicprofile file name as execution group name.
                    PCWSTR first = p;
                    while (*p && *p != L' ') p++;
                    *pbstrGroupName = ::SysAllocStringLen(first, p - first);
                    if (!*pbstrGroupName)
                    {
                        return E_OUTOFMEMORY;
                    }

                    break; // Done
                }
            }

            return hr;
        }
    };

    class RLExeXmlReader : public ComObjectImpl<IDataSource>
    {
        void LogError(wchar_t const * msg)
        {
            WEX::Common::String errorMessage;
            UINT lineNumber = 0;
            UINT linePosition = 0;
            pReader->GetLineNumber(&lineNumber);
            pReader->GetLinePosition(&linePosition);
            errorMessage.AppendFormat(L"%d,%d:", lineNumber, linePosition);
            errorMessage.Append(msg);
            Log::Error(errorMessage);
        }

    public:

        RLExeXmlReader(IXmlReader * pReader, wchar_t const * dirtags, DataSourceFlags flags) : pReader(pReader), dirtags(dirtags), flags(flags) { }
        
        virtual HRESULT STDMETHODCALLTYPE Advance(
            _COM_Outptr_result_maybenull_ IDataRow** ppDataRow)
        {            

            XmlNodeType nodeType;
            HRESULT hr;
            LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'regress-exe'");
            if (nodeType == XmlNodeType_EndElement)
            {
                // End of file;
                LOGGED_IFFAILRET(VerifyLocalName(L"regress-exe"), L"Mismatch end tag 'regress-exe'");
                if (ReadWithSkipCommentAndWhiteSpace(&nodeType) != S_FALSE)
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Extra character end of file")
                }
                *ppDataRow = NULL;
                return S_OK;
            }
            if (nodeType != XmlNodeType_Element)
            {
                LOGGED_IFFAILRET(E_FAIL, L"Unexpect node type %d", nodeType);
            }
            LOGGED_IFFAILRET(VerifyLocalName(L"test"), L"Expect start tag 'test'");
            LOGGED_IFFAILRET(ReadNextNodeType(XmlNodeType_Element), L"Expect start tag 'default'");
            LOGGED_IFFAILRET(VerifyLocalName(L"default"), L"Expect start tag 'default'");

            CComBSTR tags;
            CComBSTR files;
            CComBSTR baseline;
            CComBSTR compileflags;
            CComBSTR command;
            CComBSTR overrideTarget;
            CComBSTR overrideOS;
            CComBSTR overrideBaseline;
            CComBSTR overrideCompileflags;
            while (true)
            {
                LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'default'");

                if (nodeType == XmlNodeType_EndElement)
                {
                    break;
                }

                wchar_t const * pwszLocalName;
                if (nodeType != XmlNodeType_Element)
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Unexpected node type %d", nodeType);
                }

                LOGGED_IFFAILRET(pReader->GetLocalName(&pwszLocalName, NULL), L"Unable to get local name for element");

                if (wcscmp(pwszLocalName, L"tags") == 0)
                {
                    IFFAILRET(ParseValue(L"tags", tags));
                }
                else if (wcscmp(pwszLocalName, L"files") == 0)
                {
                    IFFAILRET(ParseValue(L"files", files));
                    if (wcscmp(L"dotest.cmd", files) == 0)
                    {
                        if (command != NULL)
                        {
                            LOGGED_IFFAILRET(E_FAIL, L"Duplicate command for dotest.cmd");
                        }
                        
                        command.Attach(files.Detach());
                    }
                }
                else if (wcscmp(pwszLocalName, L"baseline") == 0)
                {
                    IFFAILRET(ParseValue(L"baseline", baseline));
                }
                else if (wcscmp(pwszLocalName, L"compile-flags") == 0)
                {
                    IFFAILRET(ParseValue(L"compile-flags", compileflags));
                }
                else if (wcscmp(pwszLocalName, L"command") == 0)
                {
                    IFFAILRET(ParseValue(L"command", command));
                }
                else
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Unknown tag '%s'", pwszLocalName);
                }
            }

            // End of Row
            LOGGED_IFFAILRET(VerifyLocalName(L"default"), L"Mismatch closing tag 'default'");
            LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'test'");
            if (nodeType == XmlNodeType_Element)
            {
                IFFAILRET(ParseOverrides(overrideTarget, overrideOS, overrideBaseline, overrideCompileflags));
                LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'test'");
            }
            if (nodeType != XmlNodeType_EndElement)
            {
                LOGGED_IFFAILRET(E_FAIL, L"Missing end tag 'test' for the test entry");
            }

            LOGGED_IFFAILRET(VerifyLocalName(L"test"), L"Mismatch closing tag 'test' for the test entry");

            {
                bool parallel = true;
                bool elevated = false;
                bool isEdit = false;
                CComSafeArray<BSTR> excludeVariant;
                CComSafeArray<BSTR> excludeArch;
                CComSafeArray<BSTR> excludeOS;
                CComSafeArray<BSTR> excludeFlavor;
                CComSafeArray<BSTR> excludeTags;
                CComSafeArray<BSTR> unknownTags;
                CComSafeArray<BSTR> includeTags;
                if (*dirtags)
                {
                    if (tags.Length() != 0)
                    {
                        tags.Append(L",");
                    }
                    tags.Append(dirtags);
                }
                // Parse the tags
                if (tags != NULL)
                {
                    wchar_t const * current = tags;

                    while (*current != 0)
                    {
                        CComBSTR tag;
                        wchar_t const * endCurrent = wcschr(current, L',');
                        wchar_t const * nextCurrent;
                        if (endCurrent == NULL)
                        {
                            endCurrent = current + wcslen(current);
                            nextCurrent = endCurrent;
                        }
                        else
                        {
                            nextCurrent = endCurrent + 1;
                        }
                        size_t count = endCurrent - current;
                        tag.Append(current, count);
                        if (wcscmp(tag, L"exclude_x86") == 0
                            || wcscmp(tag, L"exclude_arm") == 0
                            || wcscmp(tag, L"exclude_amd64") == 0
                            || wcscmp(tag, L"exclude_arm64") == 0)
                        {
                            IFFAILRET(excludeArch.Add(tag + 8));
                        }
                        else if (wcscmp(tag, L"exclude_win7") == 0
                            || wcscmp(tag, L"exclude_win8") == 0
                            || wcscmp(tag, L"exclude_winBlue") == 0)
                        {
                            IFFAILRET(excludeOS.Add(tag + 8));
                        }
                        else if (wcscmp(tag, L"exclude_chk") == 0
                            || wcscmp(tag, L"exclude_fre") == 0)
                        {
                            IFFAILRET(excludeFlavor.Add(tag + 8));
                        }
                        else if (wcscmp(tag, L"exclude_interpreted") == 0
                            || wcscmp(tag, L"fails_interpreted") == 0)
                        {
                            if (this->IsProjection())
                            {
                                // Projection renamed Interpreted to NoNative
                                IFFAILRET(excludeVariant.Add(L"NoNative"));
                            }
                            else
                            {
                                IFFAILRET(excludeVariant.Add(L"Interpreted"));
                            }
                        }
                        else if (this->IsProjection() && wcscmp(tag, L"exclude_native") == 0)
                        {
                            // Projection renamed Native to ForceNative
                            IFFAILRET(excludeVariant.Add(L"ForceNative"));
                        }
                        else if (wcscmp(tag, L"exclude_dynapogo") == 0
                            || wcscmp(tag, L"fails_dynapogo") == 0)
                        {
                            // Remap dynapogo as foracenative
                            IFFAILRET(excludeVariant.Add(L"DynaPogo"));
                        }
                        else if (wcscmp(tag, L"exclude_serialized") == 0)
                        {
                            // serilaized imply all other serialized
                            IFFAILRET(excludeVariant.Add(L"ByteCodeSerialized"));
                            IFFAILRET(excludeVariant.Add(L"ForceSerialized"));
                        }
                        else if (wcscmp(tag, L"exclude_snap") == 0
                            || wcscmp(tag, L"exclude_snap") == 0)
                        {
                            // Default on, unless user specify Snap=1
                            IFFAILRET(excludeTags.Add(tag + 8));
                        }
                        else if (wcscmp(tag, L"exclude_nostress") == 0)
                        {
                            IFFAILRET(includeTags.Add(L"RecyclerStress"));
                        }
                        else if (wcscmp(tag, L"JsEtwConsole") == 0)
                        {
                            parallel = false;
                            // elevated = true;
                            // Default off, unless user specify JsEtwConsole=1
                            IFFAILRET(includeTags.Add(tag));
                        }
                        else if (wcscmp(tag, L"fail") == 0)
                        {
                            // Default off, unless user specify <tag>=1
                            IFFAILRET(includeTags.Add(tag));
                        }
                        else if (wcscmp(tag, L"edit") == 0)
                        {
                            isEdit = true;
                        }
                        else if (wcscmp(tag, L"exclude_ship") == 0
                            || wcscmp(tag, L"exclude_default") == 0
                            || wcscmp(tag, L"exclude_inprocdebug") == 0)
                        {
                            // Deprecated tags, ignore
                        }
                        else
                        {
                            unknownTags.Add(tag);
                        }
                        current = nextCurrent;
                    }
                }
                *ppDataRow = new RLExeXmlDataRow(command, files, compileflags, baseline,
                    overrideTarget, overrideOS, overrideBaseline, overrideCompileflags,
                    excludeVariant, excludeArch, excludeFlavor, excludeOS, excludeTags, includeTags, unknownTags, 
                    parallel, elevated, isEdit);
                (*ppDataRow)->AddRef();
                return S_OK;
            }
        }
                
        virtual HRESULT STDMETHODCALLTYPE Reset()
        {
            return E_FAIL;
        }

        virtual HRESULT STDMETHODCALLTYPE GetTestDataNames(
            _COM_Outptr_result_maybenull_ SAFEARRAY** ppTestDataNames)
        {
            HRESULT hr;
            *ppTestDataNames = NULL;

            CComSafeArray<BSTR> testDataNames;
            IFFAILRET(testDataNames.Create());

            for (int i = 0; i < _countof(TestDataNamesAndTypes); i++)
            {
                // Add the test data names that the data row recognizes.
                CComBSTR name;
                IFFAILRET(name.Append(TestDataNamesAndTypes[i][0]));
                IFFAILRET(testDataNames.Add(name));                
            }

            *ppTestDataNames = testDataNames.Detach();
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE GetTestDataType(
            BSTR pszName,
            _COM_Outptr_result_maybenull_ BSTR* ppszTypeName)
        {
            for (int i = 0; i < _countof(TestDataNamesAndTypes); i++)
            {
                if (::_wcsicmp(pszName, TestDataNamesAndTypes[i][0]) != 0)
                {
                    continue;
                }
                *ppszTypeName = ::SysAllocString(TestDataNamesAndTypes[i][1]);
                if (*ppszTypeName == NULL)
                {
                    return E_OUTOFMEMORY;
                }

                return S_OK;
            }
            *ppszTypeName = NULL;
            return E_INVALIDARG;
        }

        static wchar_t const * const TestDataNamesAndTypes[17][2];
    private:        
        CComPtr<IXmlReader> pReader;       
        wchar_t const * dirtags;
        DataSourceFlags flags;

        bool IsProjection() { return (flags && DataSourceFlags::ProjectionTests) != 0; }        

        // Xml read helper functions
        HRESULT ParseValue(wchar_t const * pwszLocalName, CComBSTR& bstrValue)
        {            
            if (pReader->IsEmptyElement()) { bstrValue = L""; return S_OK; }
            HRESULT hr;
            wchar_t const * value;
            XmlNodeType nodeType;
            LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing value for tag '%s'", pwszLocalName);
            if (nodeType == XmlNodeType_Text)
            {
                LOGGED_IFFAILRET(pReader->GetValue(&value, NULL), L"Failed to get value for tag '%s'", pwszLocalName);
                // Ignore duplicated values
                if (bstrValue == NULL)
                {
                    IFFAILRET(bstrValue.Append(value));
                }
                LOGGED_IFFAILRET(ReadNextNodeType(XmlNodeType_EndElement), L"Missing end tag '%s'", pwszLocalName);
            }
            LOGGED_IFFAILRET(VerifyLocalName(pwszLocalName), L"Mismatch closing tag '%s'", pwszLocalName);
            return S_OK;
        }

        HRESULT ParseOverrides(CComBSTR& target, CComBSTR& os, CComBSTR& baseline, CComBSTR& compileflags)
        {
            HRESULT hr;
            LOGGED_IFFAILRET(VerifyLocalName(L"condition"), L"Unexpected element after default");
            CComBSTR conditionBaseline;
            CComBSTR conditionCompileFlags;
            do
            {
                XmlNodeType nodeType;
                LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'condition'");
                if (nodeType == XmlNodeType_Element)
                {
                    wchar_t const * pwszLocalName;
                    LOGGED_IFFAILRET(pReader->GetLocalName(&pwszLocalName, NULL), L"Unable to get local name for element");
                    if (wcscmp(pwszLocalName, L"target") == 0)
                    {
                        IFFAILRET(ParseValue(L"target", target));
                    }
                    else if (wcscmp(pwszLocalName, L"os") == 0)
                    {
                        IFFAILRET(ParseValue(L"os", os));
                    }
                    else if (wcscmp(pwszLocalName, L"override") == 0)
                    {
                        do
                        {
                            LOGGED_IFFAILRET(ReadWithSkipCommentAndWhiteSpace(&nodeType), L"Missing end tag 'condition'");
                            if (nodeType == XmlNodeType_Element)
                            {
                                LOGGED_IFFAILRET(pReader->GetLocalName(&pwszLocalName, NULL), L"Unable to get local name for element");
                                if (wcscmp(pwszLocalName, L"baseline") == 0)
                                {
                                    IFFAILRET(ParseValue(L"baseline", conditionBaseline));
                                }
                                else if (wcscmp(pwszLocalName, L"compile-flags") == 0)
                                {
                                    IFFAILRET(ParseValue(L"compile-flags", conditionCompileFlags));
                                }
                                else
                                {
                                    LOGGED_IFFAILRET(E_FAIL, L"Unknown condition tag '%s'", pwszLocalName);
                                }
                                continue;
                            }
                            if (nodeType != XmlNodeType_EndElement)
                            {
                                LOGGED_IFFAILRET(E_FAIL, L"Missing end tag 'override'")
                            }
                            break;
                        } while (true);

                        LOGGED_IFFAILRET(VerifyLocalName(L"override"), L"Mismatch end tag 'override'");
                    }
                    continue;
                }
                else if (nodeType == XmlNodeType_EndElement)
                {
                    LOGGED_IFFAILRET(VerifyLocalName(L"condition"), L"Mismatch end tag 'condition'");                    
                    break;
                }
                LOGGED_IFFAILRET(E_FAIL, L"Unexpect node type in condition %d", nodeType);
            } while (true);            
            if (target != NULL)
            {
                if (os != NULL)
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Condition with both target and os");
                }

                if (!(::_wcsicmp(target, L"amd64") == 0
                    || ::_wcsicmp(target, L"arm") == 0))               
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Invalid target '%s' for condition", target);
                }
                
            
            }
            else if (os != NULL)
            {                
                if (!(::_wcsicmp(L"win7", os) == 0 || ::_wcsicmp(L"win8", os) == 0 || ::_wcsicmp(L"wp8", os) == 0))
                {
                    LOGGED_IFFAILRET(E_FAIL, L"Unknown os '%s'", os);
                }
            }
            else
            {
                LOGGED_IFFAILRET(E_FAIL, L"Condition without target or os");
            }


            if (conditionBaseline != NULL)
            {
                baseline = conditionBaseline;
            }
            if (conditionCompileFlags != NULL)
            {
                compileflags = conditionCompileFlags;
            }
            return S_OK;
        }
        
        HRESULT ReadWithSkipCommentAndWhiteSpace(XmlNodeType * pNodeType)
        {
            HRESULT hr;
            XmlNodeType nodeType;
            while (S_OK == (hr = pReader->Read(&nodeType)))
            {
                switch (nodeType)
                {
                case XmlNodeType_Whitespace:
                case XmlNodeType_Comment:
                    break;
                default:
                    *pNodeType = nodeType;
                    return S_OK;
                }
            }
            return hr;
        }
        
        HRESULT VerifyLocalName(wchar_t const * expectedLocalName)
        {
            HRESULT hr;
            wchar_t const * pwszLocalName;
            if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) { return hr; }
            return (wcscmp(pwszLocalName, expectedLocalName) == 0) ? S_OK : E_FAIL;            
        }

        HRESULT ReadNextNodeType(XmlNodeType expectNodeType)
        {
            HRESULT hr;
            XmlNodeType nodeType;            
            if (FAILED(hr = ReadWithSkipCommentAndWhiteSpace(&nodeType))) { return hr; }
            return (nodeType == expectNodeType) ? S_OK : E_FAIL;          
        }
    };

    wchar_t const * const RLExeXmlReader::TestDataNamesAndTypes[17][2] =
    {
        { L"FileName", L"String" },
        { L"Flags", L"String" },
        { L"Baseline", L"String" },
        { L"ExcludeVariant", L"String[]" },
        { L"ExcludeArch", L"String[]" },
        { L"ExcludeOS", L"String[]" },
        { L"ExcludeFlavor", L"String[]" },
        { L"ExcludeTags", L"String[]" },
        { L"IncludeTags", L"String[]" },
        { L"UnknownTags", L"String[]" },
        { L"Command", L"String" },

        // Override parameters
        { L"Flags-win8", L"String" },
        { L"Baseline-amd64", L"String" },
        { L"Baseline-arm", L"String" },
        { L"Baseline-wp8", L"String" },
        { L"Baseline-win7", L"String" },
        { L"Baseline-win8", L"String" }
    };


    HRESULT
    RLExeXmlDataSource::Open(wchar_t const * filename, wchar_t const * dirtags, DataSourceFlags flags, WEX::TestExecution::IDataSource ** dataSource)
    {
        HRESULT hr;
        CComPtr<IStream> pFileStream;
        CComPtr<IXmlReader> pReader;
        if (FAILED(hr = SHCreateStreamOnFile(filename, STGM_READ, &pFileStream)))
        {
            WEX::Common::String errorMessage;
            errorMessage.AppendFormat(L"Error creating file reader, error is %08.8lx", hr);
            Log::Error(errorMessage);
            return hr;
        }

        if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
        {
            WEX::Common::String errorMessage;
            errorMessage.AppendFormat(L"Error creating xml reader, error is %08.8lx", hr);
            Log::Error(errorMessage);
            return hr;
        }

        if (FAILED(hr = pReader->SetInput(pFileStream)))
        {
            WEX::Common::String errorMessage;
            errorMessage.AppendFormat(L"Error setting input for reader, error is %08.8lx", hr);
            Log::Error(errorMessage);
            return hr;
        }

        // verify the regress header
        XmlNodeType nodeType;
        while (S_OK == (hr = pReader->Read(&nodeType)))
        {
            switch (nodeType)
            {

            case XmlNodeType_Element:
                wchar_t const * pwszLocalName;
                IFFAILRET(pReader->GetLocalName(&pwszLocalName, NULL));
                if (wcscmp(pwszLocalName, L"regress-exe") == 0)
                {
                    *dataSource = new RLExeXmlReader(pReader, dirtags, flags);
                    (*dataSource)->AddRef();
                    return S_OK;
                }
                break;
            case XmlNodeType_Whitespace:
            case XmlNodeType_XmlDeclaration:
            case XmlNodeType_Comment:
                // Skip
                break;
            default:
                // Invalid node type
                return E_FAIL;
            }
        }
        return E_FAIL;       
    }

    // ===========================================================
    //  Code to convert RLEXE.XML to Taef's XML file
    // ===========================================================
    static HRESULT WriteToFile(HANDLE hFile, wchar_t const * buffer, DWORD len)
    {
        DWORD written;
        DWORD bytes = len * sizeof(wchar_t);
        if (!WriteFile(hFile, buffer, bytes, &written, nullptr) || written != bytes)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        return S_OK;
    }

    template <DWORD len>
    static HRESULT WriteToFile(HANDLE hFile, wchar_t const (&buffer)[len])
    {        
        return WriteToFile(hFile, buffer, len - 1);     
    }
    static HRESULT WriteToFile(HANDLE hFile, WEX::Common::String& str)
    {
        return WriteToFile(hFile, str, str.GetLength());      
    }
    
    static wchar_t const header[] =
        L"\uFEFF<?xml version=\"1.0\"?>\r\n"
        L"<Data>\r\n"
        L"  <Table Id=\"Tests\">\r\n"
        L"    <ParameterTypes>\r\n";

    static wchar_t const closeFile[] = 
        L"  </Table>\r\n"
        L"</Data>\r\n";
    
    HRESULT RLExeXmlDataSource::GenerateXmlDataSource(wchar_t const * rlexexml, wchar_t const * dirtags, DataSourceFlags flags, wchar_t const * outputFileName)
    {
        HRESULT hr;
        CComPtr<IDataSource> dataSource;
        IFFAILRET(RLExeXmlDataSource::Open(rlexexml, dirtags, flags, &dataSource));
     
        AutoHANDLE hFile = ::CreateFile(outputFileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        
        IFFAILRET(WriteToFile(hFile, header));

        for (int i = 0; i < _countof(RLExeXmlReader::TestDataNamesAndTypes); i++)
        {
            // TODO: Only support Strings
            WEX::Common::String o;
            if (wcscmp(L"String[]", RLExeXmlReader::TestDataNamesAndTypes[i][1]) == 0)
            {
                o.Format(L"      <ParameterType Name=\"%s\" Array=\"true\">String</ParameterType>\r\n", RLExeXmlReader::TestDataNamesAndTypes[i][0]);
            }
            else
            {
                o.Format(L"      <ParameterType Name=\"%s\">String</ParameterType>\r\n", RLExeXmlReader::TestDataNamesAndTypes[i][0]);
            }
            
            IFFAILRET(WriteToFile(hFile, o));            
        }

        
        IFFAILRET(WriteToFile(hFile, L"    </ParameterTypes>\r\n"));

        while (true)
        {
            CComPtr<IDataRow> dataRow;
            IFFAILRET(dataSource->Advance(&dataRow));
            if (dataRow == nullptr)
            {
                break;
            }

            CComBSTR name;
            IFFAILRET(dataRow->GetName(&name));
            WEX::Common::String rowStart;
            rowStart.Format(L"    <Row Name=\"%s\"", name);
            IFFAILRET(WriteToFile(hFile, rowStart));

            SAFEARRAY * psaMetadataNames;
            IFFAILRET(dataRow->GetMetadataNames(&psaMetadataNames));
                        
            if (psaMetadataNames != nullptr)
            {                
                CComSafeArray<BSTR> metadataNames;
                metadataNames.Attach(psaMetadataNames);
                int itemCount = metadataNames.GetCount();
                for (int i = 0; i < itemCount; i++)
                {
                    SAFEARRAY *psaMetadataValue;
                    IFFAILRET(dataRow->GetMetadata(metadataNames[i], &psaMetadataValue));
                    CComSafeArray<BSTR> metadataValue;
                    metadataValue.Attach(psaMetadataValue);
                    if (metadataValue.GetCount() != 1) { return E_FAIL; }
                    WEX::Common::String metadata;
                    metadata.Format(L" %s=\"%s\"", metadataNames[i], metadataValue[0]);
                    IFFAILRET(WriteToFile(hFile, metadata));
                }                
            }
            IFFAILRET(WriteToFile(hFile, L">\r\n"));

            for (int i = 0; i < _countof(RLExeXmlReader::TestDataNamesAndTypes); i++)
            {
                SAFEARRAY * psa;
                
                // TODO: I know we don't use the name as BSTR in GetTestData for our implementation, just cast it
                IFFAILRET(dataRow->GetTestData((BSTR)RLExeXmlReader::TestDataNamesAndTypes[i][0], &psa));
                if (psa == nullptr) { continue; }

                CComSafeArray<BSTR> safeArray;
                safeArray.Attach(psa);

                // TODO: Only support Strings
                if (wcscmp(L"String[]", RLExeXmlReader::TestDataNamesAndTypes[i][1]) == 0)
                {
                    WEX::Common::String param;
                    param.Format(L"      <Parameter Name=\"%s\">\r\n", RLExeXmlReader::TestDataNamesAndTypes[i][0]);
                    IFFAILRET(WriteToFile(hFile, param));

                    int itemCount = safeArray.GetCount();
                    for (int j = 0; j < itemCount; j++)
                    {
                        param.Format(L"        <Value>%s</Value>\r\n", safeArray[j]);
                        IFFAILRET(WriteToFile(hFile, param));
                    }
                    IFFAILRET(WriteToFile(hFile, L"      </Parameter>\r\n"));                    
                }
                else
                {
                    if (safeArray.GetCount() != 1) { return E_FAIL; }
                    WEX::Common::String param;
                    param.Format(L"      <Parameter Name=\"%s\">%s</Parameter>\r\n", RLExeXmlReader::TestDataNamesAndTypes[i][0], safeArray[0]);
                    IFFAILRET(WriteToFile(hFile, param));
                }
            }
            IFFAILRET(WriteToFile(hFile, L"    </Row>\r\n"));
        }
        IFFAILRET(WriteToFile(hFile, closeFile));
        return S_OK;
    }
};
#endif // #ifdef RLEXE_XML_DATASOURCE