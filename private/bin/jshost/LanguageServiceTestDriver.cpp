//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

#ifdef LANGUAGE_SERVICE_TEST

void CallHurry(void *authoring)
{
    ((IAuthorFileAuthoring *)authoring)->Hurry(0);
}

void Logger::LogMessage(__in __nullterminated wchar_t *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vwprintf(msg, args);
    va_end(args);
}

void Logger::LogVerbose(__in __nullterminated wchar_t *msg, ...)
{
    if(m_level == Level::Verbose)
    {
        va_list args;
        va_start(args, msg);
        vwprintf(msg, args);
        va_end(args);
    }
}

void Logger::LogError(__in __nullterminated wchar_t *msg, ...)
{
    va_list args;
    va_start(args, msg);
    wprintf(L"ERROR: ");
    vwprintf(msg, args);
    va_end(args);
}

template <class Fn>
void MapCompletions(IAuthorCompletionSet* completionSet, Fn fn)
{
    HRESULT hr = S_OK;
    Assert(completionSet != nullptr);

    AuthorCompletion *completions = nullptr;

    int numCompletions = 0;
    IfFailGo(completionSet->get_Count(&numCompletions));

    completions = new AuthorCompletion[numCompletions];
    IfFailGo(completionSet->GetItems(0, numCompletions, completions));

    AuthorFileRegion extent;
    IfFailGo(completionSet->GetExtent(&extent));

    AuthorCompletionSetKind kind;
    IfFailGo(completionSet->get_Kind(&kind));

    for (int i = 0; i < numCompletions; ++i)
    {
        fn(completions[i]);
    }

Error:
    if (completions != nullptr)
    {
        for (int i = 0; i < numCompletions; ++i)
            OperationBase::FinalizeCompletion(completions[i]);
        delete[] completions;
    }
}

template <class ParamFn, class DescriptionFn>
void MapFunctionHelp(IAuthorFunctionHelp* funcHelp, ParamFn fn, DescriptionFn descriptionFn)
{
    HRESULT hr = S_OK;
    Assert(funcHelp != nullptr);

    CComPtr<IAuthorSignatureSet> signatures;
    IfFailGo(funcHelp->get_Signatures(&signatures));

    int signaturesCount = 0;
    IfFailGo(signatures->get_Count(&signaturesCount));

    for (int sindex = 0; sindex < signaturesCount; sindex++)
    {
        // Get a signature 
        CComPtr<IAuthorSignature> signature;
        IfFailGo(signatures->GetItems(sindex, 1, &signature));

        CComPtr<IAuthorParameterSet> params;
        signature->get_Parameters(&params);

        // Get parameters count
        int paramsCount = 0;
        params->get_Count(&paramsCount);
        
        for(int pindex = 0; pindex < paramsCount; pindex++)
        {
            // Get parameter object
            CComPtr<IAuthorParameter> paramInfo;
            IfFailGo(params->GetItems(pindex, 1, &paramInfo));
            if (paramInfo != nullptr)
            {
                fn(paramInfo.p);
            }
        }

        CComBSTR description;
        if (signature->get_Description(&description) == S_OK && description)
        {
            descriptionFn(description.m_str);
        }
    }

Error:
    return;
}

template<class CountFn, class RegionFn>
HRESULT MapRegions(IAuthorRegionSet* authorRegionSet, CountFn countFn, RegionFn regionFn)
{
    HRESULT hr = S_OK;
    int count;
    AuthorFileRegion* authorFileRegions = nullptr;

    IfFailGo(authorRegionSet->get_Count(&count));
    countFn(count);
    authorFileRegions = new AuthorFileRegion[count];
    IfFailGo(authorRegionSet->GetItems(0, count, authorFileRegions));
    for (int i = 0; i < count; i++)
    {
        regionFn(i, authorFileRegions[i]);
    }
Error:
    if (authorFileRegions != nullptr)
    {
        delete[] authorFileRegions;
    }

    return hr;
}

template<class CountFn, class TaskCommentFn>
HRESULT MapTaskComments(IAuthorTaskCommentSet* authorTaskCommentSet, CountFn countFn, TaskCommentFn taskCommentFn)
{
    HRESULT hr = S_OK;
    int count;
    AuthorFileTaskComment* authorFileTaskComments = nullptr;
    IfFailGo(authorTaskCommentSet->get_Count(&count));
    countFn(count);
    authorFileTaskComments = new AuthorFileTaskComment[count];
    IfFailGo(authorTaskCommentSet->GetItems(0, count, authorFileTaskComments));
    for (int i = 0; i < count; i++)
    {
        taskCommentFn(authorFileTaskComments[i]);
    }
Error:
    if (authorFileTaskComments != nullptr)
    {
        delete[] authorFileTaskComments;
    }

    return hr;
}

template<class CountAndNameFn, class ReferencesFn>
HRESULT MapGetReferences(IAuthorReferenceSet* authorReferencesSet, CountAndNameFn countAndNameFn, ReferencesFn referencesFn)
{
    Assert(authorReferencesSet != nullptr);

    HRESULT hr = S_OK;
    int count = 0;
    CComBSTR identifier;
    AuthorSymbolReference * authorSymbolsReferences = nullptr;

    IfFailGo(authorReferencesSet->get_Count(&count));
    IfFailGo(authorReferencesSet->get_Identifier(&identifier));

    countAndNameFn(count, identifier);

    authorSymbolsReferences = new AuthorSymbolReference[count];
    IfFailGo(authorReferencesSet->GetItems(0, count, authorSymbolsReferences));
    for (int i = 0; i < count; i++)
    {
        referencesFn(authorSymbolsReferences[i]);
    }

Error:
    if (authorSymbolsReferences != nullptr)
    {
        delete[] authorSymbolsReferences;
    }

    return hr;
}


LPCWSTR GetAuthorParseNodeEdgeText(AuthorParseNodeEdge edge)
{
    switch (edge)
    {
    case apneNone: return L"apneNone";
    case apneOperand: return L"apneOperand";
    case apneLeft: return L"apneLeft";
    case apneRight: return L"apneRight";
    case apneCondition: return L"apneCondition";
    case apneThen: return L"apneThen";
    case apneElse: return L"apneElse";
    case apneInitialization: return L"apneInitialization";
    case apneIncrement: return L"apneIncrement";
    case apneBody: return L"apneBody";
    case apneBlockBody: return L"apneBlockBody";
    case apneValue: return L"apneValue";
    case apneTarget: return L"apneTarget";
    case apneArgument: return L"apneArgument";
    case apneArguments: return L"apneArguments";
    case apneMembers: return L"apneMembers";
    case apneVariable: return L"apneVariable";
    case apneObject: return L"apneObject";
    case apneTry: return L"apneTry";
    case apneCatch: return L"apneCatch";
    case apneFinally: return L"apneFinally";
    case apneCase: return L"apneCase";
    case apneElements: return L"apneElements";
    case apneListItem: return L"apneListItem";
    case apneMember: return L"apneMember";
    case apneType: return L"apneType";
    case apneExtends: return L"apneExtends";
    case apneCtor: return L"apneCtor";
    case apneStaticMembers: return L"apneStaticMembers";
    case apneStringLiterals: return L"apneStringLiterals";
    case apneSubstitutionExpression: return L"apneSubstitutionExpression";
    case apneStringRawLiterals: return L"apneStringRawLiterals";
    default: return L"unknown";
    }
}

LPCWSTR GetAuthorParseNodeKindText(AuthorParseNodeKind kind)
{
    switch (kind)
    {
    case apnkEmptyNode: return L"apnkEmptyNode";
    case apnkNone: return L"apnkNone";
    case apnkName: return L"apnkName";
    case apnkInt: return L"apnkInt";
    case apnkFlt: return L"apnkFlt";
    case apnkStr: return L"apnkStr";
    case apnkRegExp: return L"apnkRegExp";
    case apnkThis: return L"apnkThis";
    case apnkNull: return L"apnkNull";
    case apnkFalse: return L"apnkFalse";
    case apnkTrue: return L"apnkTrue";
    case apnkEmpty: return L"apnkEmpty";
    case apnkLdFncSlot: return L"apnkLdFncSlot";
    case apnkArgRef: return L"apnkArgRef";
    case apnkHelperCall3: return L"apnkHelperCall3";
    case apnkNot: return L"apnkNot";
    case apnkNeg: return L"apnkNeg";
    case apnkPos: return L"apnkPos";
    case apnkLogNot: return L"apnkLogNot";
    case apnkEllipsis: return L"apnkEllipsis";
    case apnkIncPost: return L"apnkIncPost";
    case apnkDecPost: return L"apnkDecPost";
    case apnkIncPre: return L"apnkIncPre";
    case apnkDecPre: return L"apnkDecPre";
    case apnkTypeof: return L"apnkTypeof";
    case apnkVoid: return L"apnkVoid";
    case apnkDelete: return L"apnkDelete";
    case apnkArray: return L"apnkArray";
    case apnkObject: return L"apnkObject";
    case apnkTempRef: return L"apnkTempRef";
    case apnkComputedName: return L"apnkComputedName";
    case apnkYieldLeaf: return L"apnkYieldLeaf";
    case apnkYield: return L"apnkYield";
    case apnkYieldStar: return L"apnkYieldStar";
    case apnkStFncSlot: return L"apnkStFncSlot";
    case apnkAdd: return L"apnkAdd";
    case apnkSub: return L"apnkSub";
    case apnkMul: return L"apnkMul";
    case apnkDiv: return L"apnkDiv";
    case apnkMod: return L"apnkMod";
    case apnkOr: return L"apnkOr";
    case apnkXor: return L"apnkXor";
    case apnkAnd: return L"apnkAnd";
    case apnkEq: return L"apnkEq";
    case apnkNe: return L"apnkNe";
    case apnkLt: return L"apnkLt";
    case apnkLe: return L"apnkLe";
    case apnkGe: return L"apnkGe";
    case apnkGt: return L"apnkGt";
    case apnkCall: return L"apnkCall";
    case apnkDot: return L"apnkDot";
    case apnkAsg: return L"apnkAsg";
    case apnkInstOf: return L"apnkInstOf";
    case apnkIn: return L"apnkIn";
    case apnkEqv: return L"apnkEqv";
    case apnkNEqv: return L"apnkNEqv";
    case apnkComma: return L"apnkComma";
    case apnkLogOr: return L"apnkLogOr";
    case apnkLogAnd: return L"apnkLogAnd";
    case apnkLsh: return L"apnkLsh";
    case apnkRsh: return L"apnkRsh";
    case apnkRs2: return L"apnkRs2";
    case apnkNew: return L"apnkNew";
    case apnkIndex: return L"apnkIndex";
    case apnkQmark: return L"apnkQmark";
    case apnkAsgAdd: return L"apnkAsgAdd";
    case apnkAsgSub: return L"apnkAsgSub";
    case apnkAsgMul: return L"apnkAsgMul";
    case apnkAsgDiv: return L"apnkAsgDiv";
    case apnkAsgMod: return L"apnkAsgMod";
    case apnkAsgAnd: return L"apnkAsgAnd";
    case apnkAsgXor: return L"apnkAsgXor";
    case apnkAsgOr: return L"apnkAsgOr";
    case apnkAsgLsh: return L"apnkAsgLsh";
    case apnkAsgRsh: return L"apnkAsgRsh";
    case apnkAsgRs2: return L"apnkAsgRs2";
    case apnkScope: return L"apnkScope";
    case apnkMember: return L"apnkMember";
    case apnkSetMember: return L"apnkSetMember";
    case apnkGetMember: return L"apnkGetMember";
    case apnkList: return L"apnkList";
    case apnkVarDecl: return L"apnkVarDecl";
    case apnkTemp: return L"apnkTemp";
    case apnkFncDecl: return L"apnkFncDecl";
    case apnkClassDecl: return L"apnkClassDecl";
    case apnkProg: return L"apnkProg";
    case apnkEndCode: return L"apnkEndCode";
    case apnkDebugger: return L"apnkDebugger";
    case apnkFor: return L"apnkFor";
    case apnkIf: return L"apnkIf";
    case apnkWhile: return L"apnkWhile";
    case apnkDoWhile: return L"apnkDoWhile";
    case apnkForIn: return L"apnkForIn";
    case apnkForOf: return L"apnkForOf";
    case apnkBlock: return L"apnkBlock";
    case apnkStrTemplate: return L"apnkStrTemplate";
    case apnkWith: return L"apnkWith";
    case apnkBreak: return L"apnkBreak";
    case apnkContinue: return L"apnkContinue";
    case apnkLabel: return L"apnkLabel";
    case apnkSwitch: return L"apnkSwitch";
    case apnkCase: return L"apnkCase";
    case apnkTryCatch: return L"apnkTryCatch";
    case apnkCatch: return L"apnkCatch";
    case apnkReturn: return L"apnkReturn";
    case apnkTry: return L"apnkTry";
    case apnkThrow: return L"apnkThrow";
    case apnkFinally: return L"apnkFinally";
    case apnkTryFinally: return L"apnkTryFinally";
    case apnkStruct: return L"apnkStruct";
    case apnkEnum: return L"apnkEnum";
    case apnkTyped: return L"apnkTyped";
    case apnkVarDeclList: return L"apnkVarDeclList";
    case apnkDefaultCase: return L"apnkDefaultCase";
    case apnkConstDecl: return L"apnkConstDecl";
    case apnkLetDecl: return L"apnkLetDecl";
    case apnkConstDeclList: return L"apnkConstDeclList";
    case apnkLetDeclList: return L"apnkLetDeclList";
    case apnkSuper: return L"apnkSuper";
    default: return L"unknown";
    }
}

LanguageServiceTestDriver::LanguageServiceTestDriver(std::wstring const& jslsPath)
    : m_jslsPath(jslsPath)
{
}

LanguageServiceTestDriver::~LanguageServiceTestDriver()
{
    for each(OperationBase *op in m_testOperations)
    {
        delete op;
    }

    delete m_authoringHost;

    HINSTANCE hInstance = LoadLibraryEx(m_jslsPath.c_str(), NULL, 0);
    FARPROC pDllCanUnloadNow = GetProcAddress(hInstance, "DllCanUnloadNow");
    if (pDllCanUnloadNow != NULL)
    {
        pDllCanUnloadNow();
    }
    FreeLibrary(hInstance);
}

HRESULT LanguageServiceTestDriver::InitializeHost()
{
    HRESULT hr = S_OK;
    m_authoringHost = new CAuthoringHost();
    IfFailGo(m_authoringHost->Initialize(m_jslsPath.c_str()));

Error:
    if(FAILED(hr))
    {
        m_logger.LogError(L"initialization of CAuthoringHost failed\n");
    }
    return hr;
}

HRESULT LanguageServiceTestDriver::AddFile(std::wstring const& fileName)
{
    std::wstring code;
    LPCOLESTR contents = nullptr;
    HRESULT hr = JsHostLoadScriptFromFile(fileName.c_str(), contents, nullptr/*isUtf8Out*/, nullptr/*contentsRawOut*/, nullptr/*lengthBytesOut*/, false/*printFileOpenError*/);

    if (hr == S_OK && contents)
    {
        code = contents;
        HeapFree(GetProcessHeap(), 0, (void*)contents);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        m_logger.LogError(L"failed to open file %ls.\n", fileName.c_str());
        goto Error;
    }

    m_primarySourceFileName = fileName;
    IfFailGo(ParseAnnotations(code, m_primarySourceFileText));

Error:
    return hr;
}

HRESULT LanguageServiceTestDriver::ParseAnnotations(std::wstring const& code, std::wstring& newCode)
{
    static const wchar_t* const beginMarker = L"/**";
    static const wchar_t* const endMarker = L"**/";

    static const wchar_t* const completionString = L"ml";
    static const wchar_t* const gotoDefString = L"gd";
    static const wchar_t* const defTargetString = L"target";
    static const wchar_t* const refFileString = L"ref";
    static const wchar_t* const paramListString = L"pl";
    static const wchar_t* const getRegionString = L"gr";
    static const wchar_t* const getTaskCommentsString = L"gt";
    static const wchar_t* const getStructureString = L"gs";
    static const wchar_t* const getReferencesString = L"getref";
    static const wchar_t* const getSubtreeString = L"subtree";

    static const size_t beginMarkerLen = wcslen(beginMarker);
    static const size_t endMarkerLen = wcslen(endMarker);
    static const size_t completionStringLen = wcslen(completionString);
    static const size_t gotoDefStringLen = wcslen(gotoDefString);
    static const size_t defTargetStringLen = wcslen(defTargetString);

    HRESULT hr = S_OK;
//	int opCount = 0;
    newCode.clear();
    wchar_t const* codePtr = code.c_str();
    size_t codeLen = wcslen(codePtr);
    for(size_t i = 0; i < codeLen; ++i)
    {
        if(wcsncmp(&codePtr[i], beginMarker, beginMarkerLen) == 0)
        {
            i += beginMarkerLen;
//			opCount++;

            int beginPos = i;
            int endPos = -1;
            int sourceOffset = newCode.size();

            std::wstring annotationTag;

            // Parse out the annotation tag - find the colon
            while(i < codeLen && codePtr[i] != L':' && codePtr[i] != L'(')
                annotationTag += codePtr[i++];

            // Skip past the colon
            ++i;

            if(annotationTag == completionString || annotationTag == paramListString)
            {
                bool isCompletion = annotationTag == completionString;
                bool hasStrictFlag = false;
                CompletionOperation *op = nullptr;
                if (isCompletion)
                {
                    op = new CompletionOperation(m_logger);
                }
                else
                {
                    FunctionHelpOperation *fOp = new FunctionHelpOperation(m_logger);
                    if (codePtr[i - 1] == L'(')
                    {
                        // Parameter list has some arguments
                        std::wstring isStrict;
                        while(i < codeLen && codePtr[i] != L')')
                            isStrict += (wchar_t)tolower(codePtr[i++]);
                        if (isStrict == L"forceorder")
                        {
                            fOp->m_isStrictCompare = true;
                            hasStrictFlag = true;
                        }
                        else
                        {
                            fOp->m_isStrictCompare = false;
                        }
                        while(i < codeLen && codePtr[i] != L':')
                            ++i; // Skip )
                        ++i; // Skip colon
                    }
                    else
                    {
                        fOp->m_isStrictCompare = false;
                    }
                    op = fOp;
                }

                bool foundBang = false;
                int wordBeginPos = -1;
                op->m_offset = sourceOffset;

                m_logger.LogVerbose(L"Found %ls operation at offset %d\n", isCompletion ? L"completion": L"functionHelp",  op->m_offset);

                for(; i < codeLen; ++i)
                {
                    auto RecordWordToOperation = [&](int start, int end) 
                    {
                        std::wstring word(&codePtr[start], &codePtr[end]);
                        if (foundBang)
                        {
                            op->m_unexpected.push_back(word);
                            m_logger.LogVerbose(L"  Does not expect: %ls\n", word.c_str());
                        }
                        else
                        {
                            op->m_expected.push_back(word);
                            m_logger.LogVerbose(L"  Expects: %ls\n", word.c_str());
                        }
                    };

                    if(codePtr[i] == '\n' || codePtr[i] == '\r')
                    {
                        break;
                    }
                    else if(wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
//						m_logger.LogMessage(L"Setting endPos to %d for %d the operation\n", i, opCount);
                        endPos = i;
                        i += endMarkerLen - 1; // We need to save one character, since i will increment on the next iteration
                        if (!op->dumpWholeMemberList && !op->ignoreMemberList && wordBeginPos != -1 && wordBeginPos != endPos)
                        {
                            RecordWordToOperation(wordBeginPos, endPos);
                        }
                        break;
                    }
                    else if(codePtr[i] == '!')
                    {
                        foundBang = true;
                    }
                    else if (codePtr[i] == '*')
                    {
                        op->dumpWholeMemberList = true;
                    }
                    else if (codePtr[i] == '-')
                    {
                        op->ignoreMemberList = true;
                    }
                    else
                    {
                        if(codePtr[i] == ',')
                        {
                            RecordWordToOperation(wordBeginPos, i);
                            foundBang = false;
                            wordBeginPos = -1;
                        }
                        else if(wordBeginPos == -1)
                        {
                            wordBeginPos = i;
                        }
                    }
                }

                if(endPos == -1 || (!op->dumpWholeMemberList && !op->ignoreMemberList && !hasStrictFlag && op->m_expected.size() == 0 && op->m_unexpected.size() == 0))
                {
//					m_logger.LogMessage(L"dumpWholeMemberList %ls\n", op->dumpWholeMemberList ? L"true" : L"false");
//					m_logger.LogMessage(L"ignoreMemberList %ls\n", op->ignoreMemberList ? L"true" : L"false");
//					m_logger.LogMessage(L"hasStrictFlag %ls\n", hasStrictFlag ? L"true" : L"false");
//					m_logger.LogMessage(L"op->m_expected.size() %d\n",  op->m_expected.size());
//					m_logger.LogMessage(L"op->m_unexpected.size() %d\n", op->m_unexpected.size());
                    m_logger.LogError(L"parsing annotations!\n");
                    delete op;
                    IfFailGo_NoLog(E_FAIL);
                }

                m_testOperations.push_back(op);
            }
            else if(annotationTag == gotoDefString)
            {
                // Handle GotoDef
                GotoDefOperation *op = new GotoDefOperation(m_logger);
                op->m_offset = sourceOffset;

                m_logger.LogVerbose(L"Found gotodef operation at offset %d\n", op->m_offset);

                // The target id is just a single identifier.  Anything else is an error.
                int beginPos = i;
                for(; i < codeLen; ++i)
                {
                    if(wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        op->m_id = std::wstring(&codePtr[beginPos], &codePtr[i]);
                        i += endMarkerLen - 1;
                        break;
                    }
                    else if(!(isalnum(codePtr[i])))
                    {
                        m_logger.LogError(L"%ls annotation requires a single alphanumeric string", annotationTag.c_str());
                        delete op;
                        IfFailGo_NoLog(E_FAIL);
                    }
                }
                m_testOperations.push_back(op);
            }
            else if(annotationTag == defTargetString)
            {
                // Handle Targets

                // The target id is just a single identifier.  Anything else is an error.
                Target target;
                target.m_offset = sourceOffset;
                m_logger.LogVerbose(L"Found code target at offset %d\n", target.m_offset);
                int beginPos = i;
                for(; i < codeLen; ++i)
                {
                    if(wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        target.m_id = std::wstring(&codePtr[beginPos], &codePtr[i]);
                        i += endMarkerLen - 1;
                        break;
                    }
                    else if(!(isalnum(codePtr[i])))
                    {
                        m_logger.LogError(L"%ls annotation requires a single alphanumeric string", annotationTag.c_str());
                        IfFailGo_NoLog(E_FAIL);
                    }
                }

                m_targets.push_back(target);
            }
            else if(annotationTag == getReferencesString)
            {
                GetReferencesOperation *op = new GetReferencesOperation(m_logger, sourceOffset);

                m_logger.LogVerbose(L"Found GetReferences operation at offset %d\n", op->m_offset);

                // Supported scenarios  /**getref:**/, /**getref:<number>**/ this number can be negative or positive.
                // This tells how much relatively we need to change the current offset.
                int relativeOffset = 0;
                int beginPos = i;
                for(; i < codeLen; ++i)
                {
                    if(wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        std::wstring token = std::wstring(&codePtr[beginPos], &codePtr[i]);
                        relativeOffset = _wtoi(token.c_str());
                        i += endMarkerLen - 1;
                        break;
                    }
                }
                op->m_offset += relativeOffset;
                m_testOperations.push_back(op);
            }
            else if (annotationTag == getSubtreeString)
            {
                GetSubtreeOperation *op = new GetSubtreeOperation(m_logger, sourceOffset);

                m_logger.LogVerbose(L"Found Subtree operation at offset %d\n", op->m_offset);

                int depth = 0;
                int beginPos = i;
                for (; i < codeLen; ++i)
                {
                    if (wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        std::wstring token = std::wstring(&codePtr[beginPos], &codePtr[i]);
                        depth = _wtoi(token.c_str());
                        i += endMarkerLen - 1;
                        break;
                    }
                }
                op->m_depth = depth;
                m_testOperations.push_back(op);
            }
            else if (annotationTag == refFileString)
            {
                int beginPos = i;
                for (; i < codeLen; ++i)
                {
                    if (wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0) // Currently only one file per ref supported. 
                    {
                        std::wstring fileName = std::wstring(&codePtr[beginPos], &codePtr[i]);
                        m_referenceSourceFiles.push_back(fileName);
                        i += endMarkerLen - 1;
                        break;
                    }
                }
            } 
            else if (annotationTag == getRegionString)
            {
                // no parameter for getRegion
                for (; i < codeLen; ++i)
                {
                    if (wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        i += endMarkerLen - 1;
                        break;
                    }
                }

                GetRegionOperation *op = new GetRegionOperation(m_logger);
                m_testOperations.push_back(op);
            }
            else if (annotationTag == getStructureString)
            {
                // Currently no arguments supported, we are dumping either whole or nothing structure.
                // TODO -> take parameter in the gs and do customized logging/matching
                // Supported /**gs:**/ and /**gs:-**/
                GetStructureOperation *op = new GetStructureOperation(m_logger, sourceOffset);

                for (; i < codeLen; ++i)
                {                    
                    if (wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        i += endMarkerLen - 1;
                        break;
                    }
                    else if (codePtr[i] == '-')
                    {
                        op->shouldPrintStructure = false;
                    }
                }
                m_testOperations.push_back(op);
            }
            else if (annotationTag == getTaskCommentsString)
            {
                list<wstring> prefixes;

                for (; i < codeLen; ++i)
                {
                    if (wcsncmp(&codePtr[i], endMarker, endMarkerLen) == 0)
                    {
                        i += endMarkerLen - 1;
                        break;
                    }

                    int tokenStart = i;
                    for (; i < codeLen; ++i)
                    {
                        if (codePtr[i] == ',' || codePtr[i] == '*')
                        {                            
                            break;
                        }
                    }

                    prefixes.push_back(wstring(codePtr + tokenStart, codePtr + i));
                    if (codePtr[i] == '*')
                    {
                        i--; // The last '*' character should not be consumed to make sure end comment matching will work
                    }
                }

                GetTaskCommentOperation *op = new GetTaskCommentOperation(m_logger, prefixes);
                m_testOperations.push_back(op);
            }
            else
            {
                // Keep the existing comment, as this is NOT an testoperation
                i = beginPos;
                newCode += beginMarker;
                newCode += codePtr[i];
            }

            // Done with this annotation.
        }
        else
        {
            newCode += codePtr[i];
        }
    }

Error:
return hr;
}

HRESULT LanguageServiceTestDriver::RunStress()
{
    return this->RunTyping(TypingMode::Stress);
}

HRESULT LanguageServiceTestDriver::RunPerf()
{   
    HRESULT result = this->RunTyping(TypingMode::Perf);
    PROCESS_MEMORY_COUNTERS_EX memCounters;
    memCounters.cb = sizeof(memCounters);
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memCounters, memCounters.cb);
    wprintf(L"### peakWorkingSet: %u\n", memCounters.PeakWorkingSetSize);
    wprintf(L"### workingSet: %u\n", memCounters.WorkingSetSize);
    wprintf(L"### privateMemory: %u\n", memCounters.PrivateUsage);
    return result;
}

#define BEGIN_MEASURING_OPERATION                                                                       \
if (mode == TypingMode::Perf)                                                                           \
{                                                                                                       \
    GetSystemTimeAsFileTime(&before);                                                                   \
}

#define END_MEASURING_OPERATION(operation)                                                              \
if (mode == TypingMode::Perf)                                                                           \
{                                                                                                       \
    GetSystemTimeAsFileTime(&after);                                                                    \
    beforeLargeInteger.HighPart = before.dwHighDateTime;                                                \
    beforeLargeInteger.LowPart = before.dwLowDateTime;                                                  \
    afterLargeInteger.HighPart = after.dwHighDateTime;                                                  \
    afterLargeInteger.LowPart = after.dwLowDateTime;                                                    \
    elapsedMilliseconds = (double)(afterLargeInteger.QuadPart - beforeLargeInteger.QuadPart) / 10000.0; \
    operation ## Sum += elapsedMilliseconds;                                                            \
    operation ## Max = max(operation ## Max, elapsedMilliseconds);                                      \
    operation ## Count++;                                                                               \
}

#define DECLARE_PERF_STATISTICS(operation)                                                              \
double operation ## Max = -1;                                                                           \
double operation ## Sum = 0;                                                                            \
int operation ## Count = 0;

#define REPORT_PERF_STATISTICS(operation)                                                               \
m_logger.LogMessage(L"### %ls Max: %.2f ms\n", L#operation, operation ## Max);                          \
m_logger.LogMessage(L"### %ls Avg: %.2f ms\n", L#operation, operation ## Sum / operation ## Count);

#define FOREACH_MEASURING_OPERATIONS                                                                    \
PERFORM_ON_OPERATION(getFunctionHelp)                                                                   \
PERFORM_ON_OPERATION(getCompletions)                                                                    \
PERFORM_ON_OPERATION(getStructure)                                                                      \
PERFORM_ON_OPERATION(getRegions)                                                                        \
PERFORM_ON_OPERATION(getTaskComments)

HRESULT LanguageServiceTestDriver::RunTyping(TypingMode mode)
{
    HRESULT hr = S_OK;
    CComPtr<IAuthorServices> authorServices;
    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorFileContext> fileContext;
    SplatterFileContext *splatterFileContext = NULL;
    FILETIME before = { 0 };
    FILETIME after = { 0 };
    ULARGE_INTEGER beforeLargeInteger;
    ULARGE_INTEGER afterLargeInteger;
    double elapsedMilliseconds;

#define PERFORM_ON_OPERATION DECLARE_PERF_STATISTICS
    FOREACH_MEASURING_OPERATIONS
#undef PERFORM_ON_OPERATION

    // TODO: generalize
    std::wstring code = m_primarySourceFileText;

    // Provide a new code buffer for the SplatterContext
    size_t codeLength = code.size();
    size_t bufSize = codeLength + sizeof(wchar_t)*1000;
    wchar_t *codeBuf = new wchar_t[bufSize];
    wcscpy_s(codeBuf, bufSize, code.c_str());

    authorServices = GetHost()->_authoringServices;
    splatterFileContext = new SplatterFileContext(authorServices, m_primarySourceFileName.c_str(), codeBuf, codeLength, bufSize, this);
    fileContext.Attach(splatterFileContext);

    for each(std::wstring fileName in m_referenceSourceFiles)
    {
        splatterFileContext->AddContextFile(fileName.c_str());
    }

    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));
    size_t start = HostConfigFlags::flags.StressStartIndex <= 0 ? 1 : (size_t)HostConfigFlags::flags.StressStartIndex;
    size_t end = HostConfigFlags::flags.StressEndIndex <= 0 ? codeLength : (size_t)HostConfigFlags::flags.StressEndIndex;
   
    if (start > codeLength)
    {
        // Not supported scenario,
        if (mode == TypingMode::Stress)
        {
            m_logger.LogMessage(L"Failure, StressStartIndex (%d) > codeLength (%d)\n", start, codeLength);
        }
        hr = E_INVALIDARG;
        goto Error;
    }

    if (end > codeLength)
    {
        // Not supported scenario,
        if (mode == TypingMode::Stress)
        {
            m_logger.LogMessage(L"Failure, StressEndIndex (%d) > codeLength (%d)\n", end, codeLength);
        }
        hr = E_INVALIDARG;
        goto Error;
    }

    if (start > 1)
    {
        splatterFileContext->ExtendView(start - 1);
    }

    for (size_t i = start; i <= end; i++)
    {
        if (mode == TypingMode::Stress)
        {
            m_logger.LogMessage(L"Intellisense progress : %d / %d\n", i, codeLength);
        }

        splatterFileContext->ExtendView();
        authoring->Update();
        wchar_t lastChar = splatterFileContext->GetLastChar();
        if (lastChar == L'(' || lastChar == L'.' || lastChar == L'\n' || lastChar == L',' || lastChar == L'{')
        {
            CComPtr<IAuthorFunctionHelp> funcHelp = nullptr;
            CComPtr<IAuthorCompletionSet> completions;
            CComPtr<IAuthorStructure> fileStructure;
            CComPtr<IAuthorStructureNodeSet> allNodes;

            // triggering functionHelp
            DWORD currentParamIndex = 0;
            AuthorFileRegion extent;
            AuthorDiagStatus diagStatus;

            if (mode == TypingMode::Stress)
            {
                m_logger.LogMessage(L"Triggering GetFunctionHelp\n");
            }
            m_authoringHost->CallAfter(HostConfigFlags::flags.Hurry, CallHurry, authoring);
            BEGIN_MEASURING_OPERATION
            IfFailGo(authoring->GetFunctionHelp((long)i, afhfDefault, &currentParamIndex, &extent, &diagStatus, &funcHelp));
            END_MEASURING_OPERATION(getFunctionHelp)
            
            m_authoringHost->KillCallback();

            if (funcHelp != nullptr)
            {
                MapFunctionHelp(funcHelp.p, [&](IAuthorParameter *paramInfo) {
                    // do nothing.
                }, [&](BSTR description) {
                    // do nothing
                });
            }

            // triggering completions 
            m_authoringHost->CallAfter(HostConfigFlags::flags.Hurry, CallHurry, authoring);
            if (mode == TypingMode::Stress)
            {
                m_logger.LogMessage(L"Triggering GetCompletions\n");
            }
            BEGIN_MEASURING_OPERATION
            IfFailGo(authoring->GetCompletions((long)i, acfAny, &completions));
            END_MEASURING_OPERATION(getCompletions)
            m_authoringHost->KillCallback();
            if (completions != nullptr)
            {
                MapCompletions(completions, [&](AuthorCompletion &completion) {
                    // do nothing.
                });
            }

            // triggering GetStructure
            if (mode == TypingMode::Stress)
            {
                m_logger.LogMessage(L"Triggering GetStructure\n");
            }
            
            m_authoringHost->CallAfter(HostConfigFlags::flags.Hurry, CallHurry, authoring);
            BEGIN_MEASURING_OPERATION
            IfFailGo(authoring->GetStructure(&fileStructure));
            END_MEASURING_OPERATION(getStructure)
            m_authoringHost->KillCallback();
            if (fileStructure)
            {
                IfFailGo(fileStructure->GetAllNodes(&allNodes));
            }
        }
        
        // triggering GetRegions
        CComPtr<IAuthorRegionSet> authorRegionSet;
        BEGIN_MEASURING_OPERATION
        IfFailGo(authoring->GetRegions(&authorRegionSet));
        END_MEASURING_OPERATION(getRegions)
        IfFailGo(MapRegions(authorRegionSet, [&](int count){/* no-op */}, [&](int index, AuthorFileRegion authorFileRegion){/* no-op */}));

        // triggering GetTaskComments
        CComPtr<IAuthorTaskCommentSet> authorTaskCommentSet;
        AuthorTaskCommentPrefix prefixes[1];
        prefixes[0].taskCommentPrefixText = L"TODO";
        prefixes[0].taskCommentPrefixLength = 4;
        BEGIN_MEASURING_OPERATION
        IfFailGo(authoring->GetTaskComments(prefixes, 1, &authorTaskCommentSet));
        END_MEASURING_OPERATION(getTaskComments)
        IfFailGo(MapTaskComments(authorTaskCommentSet, [&](int count){/* no-op */}, [&](AuthorFileTaskComment authorFileTaskComment){/* no-op */}));

        if (i%100 == 0)
        {
            // Lets call cleanup to do kick off some dispose. VS generally does that on idle time, so 1 in 100 seems good enough.
            authorServices->Cleanup(VARIANT_FALSE);
        }
    }

    if (mode == TypingMode::Stress)
    {
        // Helps in determining the success of the stress run.
        m_logger.LogMessage(L"Passed\n");
    }

    if (mode == TypingMode::Perf)
    {
#define PERFORM_ON_OPERATION REPORT_PERF_STATISTICS
        FOREACH_MEASURING_OPERATIONS
#undef PERFORM_ON_OPERATION
    }

Error:
    delete[] codeBuf;
    return hr;
}

#undef BEGIN_MEASURING_OPERATION
#undef END_MEASURING_OPERATION
#undef DECLARE_PERF_STATISTICS
#undef REPORT_PERF_STATISTICS
#undef FOREACH_MEASURING_OPERATIONS

HRESULT LanguageServiceTestDriver::ExecuteTests()
{
    HRESULT hr = S_OK;

    CComPtr<IAuthorFileAuthoring> authoring;
    CComPtr<IAuthorServices> authorServices = m_authoringHost->_authoringServices;
    CComPtr<IAuthorFileContext> fileContext;
    int numTests = 0;
    int numFailures = 0;

    std::wstring code = m_primarySourceFileText;

    fileContext.Attach(new MultiFileContext(authorServices, m_primarySourceFileName.c_str(), code.c_str(), code.size(), this));
    IfFailGo(authorServices->GetFileAuthoring(fileContext, &authoring));

    // This is to support the Hurry callback.
    ((MultiFileContext*)fileContext.p)->SetFileAuthoring(authoring);

    for each(std::wstring fileName in m_referenceSourceFiles)
    {
        ((MultiFileContext*)fileContext.p)->AddContextFile(fileName.c_str());
    }

    if (code.size() > 0 && HostConfigFlags::flags.AutoGS)
    {
        GetStructureOperation *op = new GetStructureOperation(m_logger, code.size() - 1);
        m_testOperations.push_back(op);
    }

    for each(OperationBase *m_testOperation in m_testOperations)
    {
        ++numTests;
        m_testOperation->SetPrimaryText(&this->m_primarySourceFileText);
        if(FAILED(m_testOperation->Execute(this, authoring)))
        {
            numFailures++;
            m_logger.LogMessage(L"Failed\n");
        }
        else
        {
            m_logger.LogMessage(L"Passed\n");
        }
        // Remove any pending hurry.
        this->GetHost()->KillCallback();
    }

Error:
    return hr;
}

HRESULT MultiFileContext::LoadAuthoringFile(LPCWSTR filename, AuthoringFile** authoringFile)
{
    *authoringFile = nullptr;
    LPCOLESTR contents = nullptr;
    HRESULT hr = JsHostLoadScriptFromFile(filename, contents, nullptr/*isUtf8Out*/, nullptr/*contentsRawOut*/, nullptr/*lengthBytesOut*/, false/*printFileOpenError*/);

    if (hr == S_OK && contents)
    {
        *authoringFile = new AuthoringFile(filename, contents, ::wcslen(contents), true/*shouldFreeText*/);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
    }

    return hr;
}

HRESULT MultiFileContext::FileAuthoringPhaseChanged(AuthorFileAuthoringPhase phase, int phaseId, IAuthorFileHandle *executingFile)
{
    HRESULT hr = S_OK;

    ValidateArg(phase >= afpDormant && phase <= afpExecuting);

    if (HostConfigFlags::flags.HurryIsEnabled
        && phase == afpExecuting
        && executingFile == this->m_primaryHandle
        && this->m_authoring != nullptr)
    {
        // Kick off Hurry.
        Assert(this->m_driver != nullptr);
        this->m_driver->GetHost()->CallAfter(HostConfigFlags::flags.Hurry, CallHurry, this->m_authoring);
    }

Error:
    return hr;
}

void OperationBase::FinalizeBSTR(BSTR& str)
{
    if (str)
    {
        ::SysFreeString(str);
        str = NULL;
    }
}

void OperationBase::FinalizeCompletion(AuthorCompletion& completion)
{
    FinalizeBSTR(completion.name);
    FinalizeBSTR(completion.displayText);
    FinalizeBSTR(completion.insertionText);
}

HRESULT GetRegionOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    HRESULT hr = S_OK;
    CComPtr<IAuthorRegionSet> authorRegionSet;

    IfFailGo(authoring->GetRegions(&authorRegionSet));
    IfFailGo(MapRegions(authorRegionSet, 
        [&](int count)
        {
            this->m_logger.LogMessage(L"%d regions found.\n", count); 
        }, 
        [&](int index, AuthorFileRegion authorFileRegion)
        {
            this->m_logger.LogMessage(L"{ index : %d, offset : %d, length : %d}\n", index, authorFileRegion.offset, authorFileRegion.length);
        }
    ));

Error:
    return hr;
}

HRESULT GetTaskCommentOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    HRESULT hr = S_OK;
    CComPtr<IAuthorTaskCommentSet> authorTaskCommentSet;
    AuthorTaskCommentPrefix* prefixes = new AuthorTaskCommentPrefix[this->m_prefixes.size()];

    int index = 0;
    for (list<wstring>::iterator iter = this->m_prefixes.begin(); iter != this->m_prefixes.end(); iter++)
    {
        prefixes[index].taskCommentPrefixText = iter->c_str();
        prefixes[index].taskCommentPrefixLength = iter->length();
        index++;
    }

    IfFailGo(authoring->GetTaskComments(prefixes, this->m_prefixes.size(), &authorTaskCommentSet));
    IfFailGo(MapTaskComments(authorTaskCommentSet,
        [&](int count)
        {
            this->m_logger.LogMessage(L"%d task comments found.\n", count);
        },
        [&](AuthorFileTaskComment authorFileTaskComment)
        {
            this->m_logger.LogMessage(L"%ls\n", this->m_primarySourceFileText->substr(authorFileTaskComment.offset, authorFileTaskComment.length).c_str());
        }
    ));
Error:

    if (prefixes != nullptr)
    {
        delete[] prefixes;
    }

    return hr;
}

HRESULT CompletionOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    CComPtr<IAuthorCompletionSet> completionSet;
    HRESULT hr = S_OK;

    // Issue the completion call
    hr = authoring->GetCompletions(m_offset, AuthorCompletionFlags::acfMembersFilter, &completionSet);
    IfFailGo(hr);

    m_logger.LogVerbose(L"Running completion at offset %d\n", m_offset);

    if(completionSet != NULL)
    {
        bool failed = false;
        std::map<std::wstring, bool> found;
        MapCompletions(completionSet, [&](AuthorCompletion &completion) {
            if (dumpWholeMemberList)
            {
                m_logger.LogMessage(L"    %ls\n", completion.name);
            }
            else if (ignoreMemberList)
            {
                // no-op
            }
            else
            {
                // Check all the completions for the expected/unexpected values.
                std::wstring curr(completion.name);
                if (completion.group != AuthorCompletionFlags::acfMembersFilter)
                {
                    m_logger.LogError(L"  IdentifierList returned\n");
                    failed = true;
                }

                m_logger.LogVerbose(L"  Completion contained: %ls\n", curr.c_str());

                if (std::find(m_expected.begin(), m_expected.end(), curr) != m_expected.end())
                {
                    found[curr] = true;
                }
                else if (std::find(m_unexpected.begin(), m_unexpected.end(), curr) != m_unexpected.end())
                {
                    m_logger.LogError(L"found unexpected '%ls' in completion list at (offset %d)\n", curr.c_str(), m_offset);
                    failed = true;
                }
            }
        });

        if (!this->ignoreMemberList)
        {
            for each(std::wstring str in m_expected)
            {
                if (found.find(str) == found.end())
                {
                    m_logger.LogError(L"expected '%ls' in the completion list, but it was not found (offset %d)\n", str.c_str(), m_offset);
                    failed = true;
                }
            }
        }

        IfFailGo_NoLog(failed ? E_FAIL : S_OK);

    }


Error:
    return hr;
}

HRESULT GotoDefOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    CComPtr<IAuthorFileHandle> fileHandle;
    long location;
    HRESULT hr = S_OK;
    
    m_logger.LogVerbose(L"Running gotodef at offset %d\n", m_offset);
    
    IfFailGo(authoring->GetDefinitionLocation(m_offset, &fileHandle, &location));

    m_logger.LogVerbose(L"  Result: id '%ls', offset %d\n", m_id.c_str(), location);

    // Loop through the known targets to find a match
    bool found = false;
    for each(Target target in driver->GetTargets())
    {
        m_logger.LogVerbose(L"  Searching target {%ls, %d}\n", target.m_id.c_str(), target.m_offset);
        if(target.m_id == m_id)
        {
            if(target.m_offset == location)
            {
                m_logger.LogVerbose(L"  Found expected target '%ls' (offset %d)\n", m_id.c_str(), location);
                found = true;
            }
            else
            {
                found = false; 
                break;
            }
        }
    }

    if(!found)
    {
        m_logger.LogError(L"Failed to gotodef for offset %d\n", m_offset);
        IfFailGo_NoLog(E_FAIL);
    }

Error:
    return hr;
}

void PrintSymbolReferenceKind(AuthorSymbolReferenceFlags flags, Logger *logger)
{
    logger->LogMessage(L"kind : ");

    if (flags & asrfAuthoritative)
    {
        logger->LogMessage(L"asrfAuthoritative ");
    }
    if (flags & asrfSuggestive)
    {
        logger->LogMessage(L"asrfSuggestive ");
    }
    if (flags & asrfLValue)
    {
        logger->LogMessage(L"asrfLValue ");
    }
    if (flags & asrfRValue)
    {
        logger->LogMessage(L"asrfRValue ");
    }
    if (flags & asrfLocalReference)
    {
        logger->LogMessage(L"asrfLocalReference ");
    }
    if (flags & asrfGlobalReference)
    {
        logger->LogMessage(L"asrfGlobalReference ");
    }
    if (flags & asrfMemberReference)
    {
        logger->LogMessage(L"asrfMemberReference ");
    }
    if (flags & asrfScopeInDoubt)
    {
        logger->LogMessage(L"asrfScopeInDoubt ");
    }
}

HRESULT GetReferencesOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    CComPtr<IAuthorReferenceSet> referenceSet;
    HRESULT hr = S_OK;

    m_logger.LogVerbose(L"Running GetReferences at offset %d\n", m_offset);

    IfFailGo(authoring->GetReferences(m_offset, &referenceSet));

    IfFailGo(MapGetReferences(referenceSet,
        [&](int count, BSTR identifier)
        {
            this->m_logger.LogMessage(L"Identifier : %ls and references found : %d\n", identifier, count);
        },
        [&](AuthorSymbolReference authorSymbolReference)
        {
            this->m_logger.LogMessage(L"\t{ range : [%d, %d], ", authorSymbolReference.position, authorSymbolReference.position + authorSymbolReference.length);
            PrintSymbolReferenceKind(authorSymbolReference.flags, &this->m_logger);
            this->m_logger.LogMessage(L"}\n");
        }
    ));

Error:
    return hr;
}

void GetSubtreeOperation::PrintParseNodeSet(IAuthorParseNodeSet *set)
{
    int count = 0;
    if (set && set->get_Count(&count) == S_OK)
    {
        m_logger.LogMessage(L"\tTotal nodes : %d\n", count);

        AuthorParseNode * authorParseNodes = new AuthorParseNode[count];
        if (set->GetItems(0, count, authorParseNodes) == S_OK)
        {
            for (int i = 0; i < count; i++)
            {
                AuthorParseNode *node = &authorParseNodes[i];
                m_logger.LogMessage(L"\t%ls : {kind : %ls, min : %d, lim : %d, level : %d}\n", 
                    GetAuthorParseNodeEdgeText(node->edgeLabel), 
                    GetAuthorParseNodeKindText(node->details.kind), 
                    node->details.startOffset, 
                    node->details.endOffset, 
                    node->level);
            }
        }

        if (authorParseNodes != nullptr)
        {
            delete[] authorParseNodes;
        }
    }
}

HRESULT GetSubtreeOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    CComPtr<IAuthorParseNodeCursor> cursor;
    CComPtr<IAuthorParseNodeSet> nodeSet;
    HRESULT hr = S_OK;

    m_logger.LogVerbose(L"Running GetSubtreeOperation at offset %d\n", m_offset);

    IfFailGo(authoring->GetASTCursor(&cursor));
    AuthorParseNodeDetails result;
    IfFailGo(cursor->SeekToOffset(m_offset, VARIANT_TRUE, &result));
    result; // not used
    IfFailGo(cursor->GetSubTree(m_depth, &nodeSet));

    m_logger.LogMessage(L"Print subtree at offset : %d\n", m_offset);
    PrintParseNodeSet(nodeSet);

Error:
    return hr;
}

HRESULT FunctionHelpOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    HRESULT hr = S_OK;
    CComPtr<IAuthorFunctionHelp> funcHelp = nullptr;
    DWORD currentParamIndex = 0;
    AuthorFileRegion extent;
    AuthorDiagStatus diagStatus;

    IfFailGo(authoring->GetFunctionHelp(m_offset, afhfDefault, &currentParamIndex, &extent, &diagStatus, &funcHelp));
    m_logger.LogVerbose(L"Running function help test at offset %d\n", m_offset);

    if (funcHelp != nullptr)
    {
        bool failed = false;
        bool first = true;
        size_t index = 0;
        std::map<std::wstring, bool> found;
        std::list<std::wstring>::const_iterator iterator = m_expected.begin();
        if (dumpWholeMemberList)
        {
            m_logger.LogMessage(L"    ");
        }
        MapFunctionHelp(funcHelp.p, [&](IAuthorParameter *paramInfo) {
            CComBSTR name;
            paramInfo->get_Name(&name);
            CComBSTR type;
            paramInfo->get_Type(&type);
            if (dumpWholeMemberList)
            {
                if (type.m_str == nullptr)
                {
                    m_logger.LogMessage(L"%ls%ls", first ? L"" : L", ", name);
                }
                else
                {
                    m_logger.LogMessage(L"%ls%ls %ls", first ? L"" : L", ", type, name);
                }
            }
            else if (this->m_isStrictCompare)
            {
                if (iterator != m_expected.end())
                {
                    std::wstring curr(name);
                    if (iterator != m_expected.end() && curr != *iterator)
                    {
                        m_logger.LogError(L"in strict comparison mode expected to find '%ls' at index %d but found %ls in function help list at (offset %d)\n", (*iterator).c_str(), index, curr.c_str(), m_offset);
                        failed = true;
                    }
                    else
                    {
                        found[curr] = true;
                    }
                    do 
                    { 
                        ++iterator; 
                    } while (iterator != m_expected.end() && (*iterator)[0] == '!');
                }
                ++index;
            }
            else
            {
                std::wstring curr(name);
                if (std::find(m_expected.begin(), m_expected.end(), curr) != m_expected.end())
                {
                    found[curr] = true;
                }
                else if (std::find(m_unexpected.begin(), m_unexpected.end(), curr) != m_unexpected.end())
                {
                    m_logger.LogError(L"found unexpected '%ls' in function help list at (offset %d)\n", curr.c_str(), m_offset);
                    failed = true;
                }
            }
            first = false;
        }, [&](BSTR description) {
            if (dumpWholeMemberList)
            {
                Assert(description);
                m_logger.LogMessage(L"\n%ls\n    ", description);

                // Reset the first variable so the next signature will able to indent nicely.
                first = true;
            }
        });

        if (this->m_isStrictCompare && m_expected.size() != index)
        {
            m_logger.LogError(L"in strict comparison mode expected to find '%d' parameters for the function but found %d in function help list at (offset %d)\n", m_expected.size(), index, m_offset);
            failed = true;
        }

        if (dumpWholeMemberList)
        {
            m_logger.LogMessage(L"\n");
        }

        for each(std::wstring str in m_expected)
        {
            if (found.find(str) == found.end())
            {
                m_logger.LogError(L"expected '%ls' in the function help list, but it was not found (offset %d)\n", str.c_str(), m_offset);
                failed = true;
            }
        }

        IfFailGo(failed ? E_FAIL : S_OK);
    }

Error:
    return hr;
}

HRESULT GetStructureOperation::Execute(LanguageServiceTestDriver *driver, CComPtr<IAuthorFileAuthoring> authoring)
{
    HRESULT hr = S_OK;
    Assert(driver != nullptr);

    CComPtr<IAuthorStructure> fileStructure;
    CComPtr<IAuthorStructureNodeSet> allNodes;
    std::wstring indent(L"");

    m_logger.LogVerbose(L"Running getstructure at offset %d\n", m_offset);
    IfFailGo(authoring->GetStructure(&fileStructure));

    IfFailGo(fileStructure->GetAllNodes(&allNodes));

    if (shouldPrintStructure)
    {
        m_logger.LogMessage(L"GetStructure : getting all nodes\n");

        PrintStructureNodes(allNodes);

        m_logger.LogMessage(L"GetStructure : Displaying structure graph\n");
        PrintStructureSubTree(fileStructure, -1/*start with main container*/, indent);
    }

Error:
    return hr;
}

void GetStructureOperation::PrintStructureKind(AuthorStructureNodeKind kind)
{
    switch(kind)
    {
    case asnkCustom:
        m_logger.LogMessage(L"asnkCustom");
        break;

    case asnkGlobal:
        m_logger.LogMessage(L"asnkGlobal");
        break;

    case asnkObjectLiteral:
        m_logger.LogMessage(L"asnkObjectLiteral");
        break;

    case asnkFunction:
        m_logger.LogMessage(L"asnkFunction");
        break;

    case asnkVariable:
        m_logger.LogMessage(L"asnkVariable");
        break;

    case asnkField:
        m_logger.LogMessage(L"asnkField");
        break;

    case asnkProperty:
        m_logger.LogMessage(L"asnkProperty");
        break;

    case asnkClass:
        m_logger.LogMessage(L"asnkClass");
        break;

    case asnkNamespace:
        m_logger.LogMessage(L"asnkNamespace");
        break;

    case asnkModule:
        m_logger.LogMessage(L"asnkModule");
        break;

    case asnkRegion:
        m_logger.LogMessage(L"asnkRegion");
        break;
    }
}

void GetStructureOperation::PrintStructureSubTree(IAuthorStructure * structure, int key, std::wstring indent)
{
    Assert(structure != nullptr);

    CComPtr<IAuthorStructureNodeSet> childrenNodes;
    AuthorStructureNode *structureNodes = nullptr;
    int count = 0;
    HRESULT hr = S_OK;

    if (key < 0)
    {
        IfFailGo(structure->GetContainerNodes(&childrenNodes));
    }
    else
    {
        IfFailGo(structure->GetChildrenOf(key, &childrenNodes));
    }

    IfFailGo(childrenNodes->get_Count(&count));
    if (count == 0)
    {
        return;
    }

    structureNodes = new AuthorStructureNode[count];
    IfFailGo(childrenNodes->GetItems(0, count, structureNodes));

    for (int i = 0; i < count; i++)
    {
        AuthorStructureNode *structureNode = &structureNodes[i];

        if (key < 0 && structureNode->container > 0)
        {
            // This is to iterate through main container nodes only.
            continue;
        }

        m_logger.LogMessage(L"%ls", indent.c_str());

        if (structureNode->itemName)
        {
            m_logger.LogMessage(L"%ls",structureNode->itemName);
        }
        else if (structureNode->containerName)
        {
            m_logger.LogMessage(L"%ls",structureNode->containerName);
        }
        else
        {
            PrintStructureKind(structureNode->kind);
        }
        m_logger.LogMessage(L"\n");

        if (structureNode->hasChildren)
        {
            std::wstring newIndent = indent + L"    ";
            PrintStructureSubTree(structure, structureNode->key, newIndent);
        }
    }

Error:
    if(structureNodes != nullptr)
    {
        for(int i = 0; i < count; ++i)
            CleanupAuthorStructureNode( &structureNodes[i]);
        delete[] structureNodes;
    }

}

void GetStructureOperation::PrintStructureNodes(IAuthorStructureNodeSet * nodes)
{
    Assert(nodes != nullptr);

    AuthorStructureNode *structureNodes = nullptr;
    int count;
    HRESULT hr = S_OK;

    IfFailGo(nodes->get_Count(&count));
    m_logger.LogMessage(L"Total nodes : %d\n", count);

    if (count == 0)
    {
        return;
    }

    structureNodes = new AuthorStructureNode[count];
    IfFailGo(nodes->GetItems(0, count, structureNodes));

    for (int i = 0; i < count; i++)
    {
        m_logger.LogMessage(L"{ ");
        AuthorStructureNode *structureNode = &structureNodes[i];
        PrintStructureKind(structureNode->kind);
        m_logger.LogMessage(L", ");

        m_logger.LogMessage(L"containerName : %ls, ", structureNode->containerName != nullptr ? structureNode->containerName : L"");
        m_logger.LogMessage(L"itemName : %ls, ",  structureNode->itemName != nullptr ? structureNode->itemName : L"");

        m_logger.LogMessage(L"range : {%d : %d}, ", structureNode->region.offset, structureNode->region.offset + structureNode->region.length);
        m_logger.LogMessage(L"HasChildren : %ls", structureNode->hasChildren ? L"yes" : L"no");
        m_logger.LogMessage(L" }\n");
    }

Error:
    if(structureNodes != nullptr)
    {
        for(int i = 0; i < count; ++i)
            CleanupAuthorStructureNode( &structureNodes[i]);
        delete[] structureNodes;
    }

    return;
}

void GetStructureOperation::CleanupAuthorStructureNode(AuthorStructureNode *structureNode)
{
    if(structureNode != nullptr)
    {
        FinalizeBSTR(structureNode->containerName);
        FinalizeBSTR(structureNode->itemName);
        FinalizeBSTR(structureNode->customKind);
        FinalizeBSTR(structureNode->glyph);
    }
}

#endif
