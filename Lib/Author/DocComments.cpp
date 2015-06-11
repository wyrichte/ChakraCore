//---------------------------------------------------------------------------
// Copyright (C) 2010 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "JsDocCommentsParser.h"

using namespace Authoring;

#define HTMLELMENT  L"HTMLElement"

static bool IsXmlError(HRESULT hr)
{
    return hr >= MX_E_MX  && hr <= MX_E_MX + 0xFFF;
}

CommentBuffer::CommentBuffer(ArenaAllocator* alloc) : TextBuffer(alloc)
{
    this->commentType = commenttypeNone;
}

CommentBuffer::~CommentBuffer()
{
    // no-op - there is no other cleanup necessary
}

CommentBuffer* CommentBuffer::New(ArenaAllocator *alloc)
{
    return Anew(alloc, CommentBuffer, alloc);
}

void CommentBuffer::SetCommentType(CommentType commentType)
{
    this->commentType = commentType;
}

CommentType CommentBuffer::GetCommentType()
{
    return this->commentType;
}

class VsDocCommentsParser
{
    class Scanner
    {
    public:
        enum dcToken 
        { 
            dctNone,
            dctRoot,
            dctSummary, 
            dctSignature, 
            dctParam, 
            dctName, 
            dctReturns, 
            dctType, 
            dctValue, 
            dctEndElement, 
            dctText, 
            dctInvalid, 
            dctUnknownAttribute,
            dctUnknownElement,
            dclCData,
            dclWhiteSpace,
            dctEOF,
            dctVar,
            dctField,
            dctHelpKeyword,
            dctLocid,
            dctDomElement,
            dctElementDomElement,
            dctElementType,
            dctOptional,
            dctExternalid,
            dctExternalFile,
            dctValueElement,
            dctStatic,
            dctBr,
            dctDeprecated,
            dctCompatibleWith,
            dctPlatform,
            dctMinVersion
        };

    private:
        enum ScanState
        {
            ssNormal = 1,
            ssInAttributes = 2,
            ssPastFirstAttribute = 4,
            ssInEmptyElement = 8
        };

        CComPtr<IXmlReader> m_reader;
        dcToken m_token;
        ScanState m_state;
        HRESULT m_lastError;
        uint m_errorLine;
        charcount_t m_errorPos;

        inline HRESULT WriteToStream(IStream* stream, LPCWSTR text, uint length, bool convertNewLineToBr = false)
        {
            METHOD_PREFIX;
            Assert(stream);
            if(length) 
            {
                if (convertNewLineToBr)
                {
                    // Replace &#10; with <br/>
                    const wchar_t encodedNewLine[] = L"&#10;";
                    const size_t encodedNewLineLen = LengthOfLiteral(encodedNewLine);
                    LPCWSTR spanStart;
                    for(spanStart = text; spanStart < text + length;)
                    {
                        auto newLine = wcsstr(spanStart, encodedNewLine);
                        if(!newLine)
                            break;
                        // Write the current span followed by <br/>
                        IfFailGo(WriteToStream(stream, spanStart, static_cast<uint>(newLine - spanStart)));
                        IfFailGo(WriteToStream(stream, L"<br/>"));
                        spanStart = newLine + encodedNewLineLen;
                    }
                    IfFailGo(WriteToStream(stream, spanStart, static_cast<uint>((text + length) - spanStart)));
                }
                else
                {
                    stream->Write(text, length * sizeof(wchar_t), nullptr);
                }
            }
            METHOD_POSTFIX;
        }

        template< size_t N >
        HRESULT WriteToStream( IStream* stream, const wchar_t (&w)[N] )
        {
            return WriteToStream(stream, w, (N - 1)); // -1 because the size contains the null terminator
        }

    public:
        Scanner()
        {
            SetState(ssNormal);
            SetToken(dctEOF);
            SetLastError(S_OK);
        }

        HRESULT SetText(LPCWSTR text)
        {
            Assert(text != NULL);

            HRESULT hr = S_OK;

            //
            // Allocate a stream and write the doc comments xml into it, add a root element.
            // 

            CComPtr<IStream> stream; 
            hr = ::CreateStreamOnHGlobal(NULL, TRUE, &stream);
            IfFailedReturn(hr);

            hr = WriteToStream(stream, L"<doccomment>");
            IfFailedReturn(hr);
            hr =  WriteToStream(stream, text, ::wcslen(text), true /* convertNewLineToBr */);
            IfFailedReturn(hr);
            hr = WriteToStream(stream, L"</doccomment>");
            IfFailedReturn(hr);

            // Reset the stream
            LARGE_INTEGER pos = {0};
            hr = stream->Seek(pos, STREAM_SEEK_SET, NULL);
            IfFailedReturn(hr);

            // Create reader
            hr = CreateXmlReader(IID_IXmlReader, reinterpret_cast<void**>(&m_reader), NULL);
            IfFailedReturn(hr);
            hr = m_reader->SetInput(stream);

            SetToken(dctNone);

            return hr;
        }

        dcToken Scan()
        {
            HRESULT hr = S_OK;

            if(m_token == dctEOF)
            {
                return dctEOF;
            }

            if(m_state == ssNormal)
            {
                XmlNodeType nodeType;
                hr = m_reader->Read(&nodeType);
                if(FAILED(hr))
                    goto Error;

                if(hr == S_FALSE)
                {
                    return SetToken(dctEOF);
                }

                switch(nodeType)
                {
                    case XmlNodeType_Element:
                        LPCWSTR name;
                        hr = m_reader->GetLocalName(&name, NULL);
                        if(FAILED(hr)) 
                            goto Error;
                        SetState(ssInAttributes);
                        if(m_reader->IsEmptyElement())
                            SetState(static_cast<ScanState>(m_state | ssInEmptyElement));
                        return SetToken(TokenByElementName(name));
                    case XmlNodeType_Text:
                        return SetToken(dctText);
                    case XmlNodeType_EndElement:
                        return SetToken(dctEndElement);
                    case XmlNodeType_CDATA:
                        return SetToken(dclCData);
                    case XmlNodeType_Whitespace:
                        return SetToken(dclWhiteSpace);
                    case XmlNodeType_ProcessingInstruction:
                    case XmlNodeType_Comment:
                        return Scan(); // skip silently
                    default:
                        return SetToken(dctInvalid);
                }
            }

            if((m_state & ssInAttributes) != 0)
            {
                if((m_state & ssPastFirstAttribute) == 0)
                {
                    hr = m_reader->MoveToFirstAttribute();
                }
                else
                {
                    hr = m_reader->MoveToNextAttribute();
                }

                if(FAILED(hr)) 
                    goto Error;

                if(hr == S_FALSE)
                {
                    auto inEmptyElement = (m_state & ssInEmptyElement) != 0;

                    // No attribute was read
                    SetState(ssNormal);
                    if(inEmptyElement)
                    {
                        return SetToken(dctEndElement);
                    }

                    return Scan();
                }

                SetState(static_cast<ScanState>(m_state | ssPastFirstAttribute));

                LPCWSTR name;
                hr = m_reader->GetLocalName(&name, NULL);
                if(FAILED(hr)) 
                    goto Error;

                return SetToken(TokenByAttributeName(name));
            }

            // Not supposed to get there
            Assert(true);

        Error:
            Assert(FAILED(hr));
            SetLastError(hr);
            return SetToken(dctEOF);
        }

        dcToken Token()
        {
            return m_token;
        }

        LPCWSTR Value()
        {
            LPCWSTR value = L"";
            m_reader->GetValue(&value, NULL);
            return value;
        }

        bool IsEmptyElement()
        {
            return (m_state & ssInEmptyElement) != 0;
        }

        HRESULT GetLastError()
        {
            return m_lastError;
        }

        uint ErrorLine()
        {
            return m_errorLine;
        }

        charcount_t ErrorPos()
        {
            return m_errorPos;
        }

        LPCWSTR GetQualifiedName()
        {
            LPCWSTR name = L"";
            m_reader->GetQualifiedName(&name, NULL);
            return name;
        }

        bool IsAttribute(dcToken token)
        {
            switch(token)
            {
            case dctName:
            case dctType:
            case dctValue:
            case dctHelpKeyword:
            case dctLocid:
            case dctDomElement:
            case dctElementDomElement:
            case dctElementType:
            case dctOptional:
            case dctExternalid:
            case dctExternalFile:
            case dctStatic:
            case dctUnknownAttribute:
                return true;
            }
            return false;
        }

        bool BooleanValue()
        {
            LPCWSTR textValue = Value();

            if(wcscmp(textValue, L"true") == 0)
            {
                return true;
            }

            return false;
        }

    private:
        void SetState(ScanState state)
        {
            m_state = state;
        }

        dcToken SetToken(dcToken token)
        {
            m_token = token;
            return Token();
        }

        void SetLastError(HRESULT hr)
        {
            m_lastError = hr;
            uint line = 0;
            uint pos = 0;
            if(IsXmlError(hr) && m_reader && SUCCEEDED(m_reader->GetLineNumber(&line)) && SUCCEEDED(m_reader->GetLinePosition(&pos)))
            {
                m_errorLine = line;
                m_errorPos = pos;
            }
            else
            {
                m_errorLine = 0;
                m_errorPos = 0;
            }
        }

        dcToken TokenByElementName(LPCWSTR elementName)
        {
            if (elementName)
            {
                switch (elementName[0])
                {
                case 'b':
                    if(wcscmp(elementName, L"br") == 0)
                    {
                        return dctBr;
                    }
                    break;
                case 'c':
                    if(wcscmp(elementName, L"compatibleWith") == 0)
                    {
                        return dctCompatibleWith;
                    }
                    break;
                case 'd':
                    if(wcscmp(elementName, L"doccomment") == 0)
                    {
                        return dctRoot;
                    }
                    if(wcscmp(elementName, L"deprecated") == 0)
                    {
                        return dctDeprecated;
                    }
                    break;
                case 'f':
                    if(wcscmp(elementName, L"field") == 0)
                    {
                        return dctField;
                    }
                    break;
                case 'p':
                    if(wcscmp(elementName, L"param") == 0)
                    {
                        return dctParam;
                    }
                    break;
                case 'r':
                    if(wcscmp(elementName, L"returns") == 0)
                    {
                        return dctReturns;
                    }
                    break;
                case 's':
                    if(wcscmp(elementName, L"signature") == 0)
                    {
                        return dctSignature;
                    }
                    if(wcscmp(elementName, L"summary") == 0)
                    {
                        return dctSummary;
                    }
                    break;
                case 'v':
                    if(wcscmp(elementName, L"value") == 0)
                    {
                        return dctValueElement;
                    }
                    if(wcscmp(elementName, L"var") == 0)
                    {
                        return dctVar;
                    }
                    break;
                }
            }
            return dctUnknownElement;
        }


       dcToken TokenByAttributeName(LPCWSTR attributeName)
        {
            if (attributeName)
            {
                // NOTE: keep alphabetic order
                switch(attributeName[0])
                {
                case 'd':
                    if(wcscmp(attributeName, L"domElement") == 0)
                    {
                        return dctDomElement;
                    }
                    break;
                case 'e':
                    if(wcscmp(attributeName, L"elementDomElement") == 0)
                    {
                        return dctElementDomElement;
                    }
                    if(wcscmp(attributeName, L"elementType") == 0)
                    {
                        return dctElementType;
                    }
                    if(wcscmp(attributeName, L"externalFile") == 0)
                    {
                        return dctExternalFile;
                    }
                    if(wcscmp(attributeName, L"externalid") == 0)
                    {
                        return dctExternalid;
                    }
                    break;
                case 'h':
                    if(wcscmp(attributeName, L"helpKeyword") == 0)
                    {
                        return dctHelpKeyword;
                    }
                    break;
                case 'l':
                    if(wcscmp(attributeName, L"locid") == 0)
                    {
                        return dctLocid;
                    }
                    break;
                case 'm':
                    if(wcscmp(attributeName, L"minVersion") == 0)
                    {
                        return dctMinVersion;
                    }
                    break;
                case 'n':
                    if(wcscmp(attributeName, L"name") == 0)
                    {
                        return dctName;
                    }
                    break;
                case 'o':
                    if(wcscmp(attributeName, L"optional") == 0)
                    {
                        return dctOptional;
                    }
                    break;
                case 'p':
                    if(wcscmp(attributeName, L"platform") == 0)
                    {
                        return dctPlatform;
                    }
                    break;
                case 's':
                    if(wcscmp(attributeName, L"static") == 0)
                    {
                        return dctStatic;
                    }
                    break;
                case 't':
                    if(wcscmp(attributeName, L"type") == 0)
                    {
                        return dctType;
                    }
                    break;
                case 'v':
                    if(wcscmp(attributeName, L"value") == 0)
                    {
                        return dctValue;
                    }
                    break;
                }
            }
            return dctUnknownAttribute;
        }
    }; // End scanner

    Scanner m_scanner;
    ArenaAllocator* m_alloc;
public:

    VsDocCommentsParser() : m_alloc(nullptr) { }

    HRESULT ParseFuncDocComments(ArenaAllocator* alloc, LPCWSTR text, FunctionDocComments** funcDocComments)
    {
        return Parse<FunctionDocComments>(alloc, text, [&]() -> FunctionDocComments* { return ParseFuncDocComments(); }, funcDocComments);
    }

    HRESULT ParseVarDocComments(ArenaAllocator* alloc, LPCWSTR text, VarDocComments** varDocComments)
    {
        return Parse<VarDocComments>(alloc, text, [&]() -> VarDocComments* { return ParseVarDocComments(); }, varDocComments);
    }

    HRESULT ParseFieldDocComments(ArenaAllocator* alloc, LPCWSTR text, FieldDocComments** fieldDocComments)
    {
        return Parse<FieldDocComments>(alloc, text, [&]() -> FieldDocComments* { return ParseFieldDocComments(); }, fieldDocComments);
    }

private:
    template<typename TResult, typename TParseAction>
    HRESULT Parse(ArenaAllocator* alloc, LPCWSTR text, TParseAction parseAction, TResult** out)
    {
        Assert(alloc != NULL);
        Assert(text != NULL);
        Assert(out != NULL);

        HRESULT hr = S_OK;
    
        hr = m_scanner.SetText(text);
        IfFailedReturn(hr);

        m_alloc = alloc;

        auto token = m_scanner.Scan();
        Assert(token == Scanner::dctRoot);
        
        auto result = parseAction();

        auto scannerError = m_scanner.GetLastError();
        if (FAILED(scannerError))
        {
            if(IsXmlError(scannerError))
            {
                if(result)
                {
                    // The error is due to malformed XML. Attach scanner error to the result.
                    result->parseError = Anew(m_alloc, ParseError, scannerError, m_scanner.ErrorLine(), m_scanner.ErrorPos());
                }
            }
            else
            {
                // This is not an XML error, report it
                result = nullptr;
                hr = scannerError;
            }
        }

        *out = result;

        return hr;
    }

    FunctionDocComments* ParseFuncDocComments()
    {
        auto result = Anew(m_alloc, FunctionDocComments, m_alloc);
        result->implicitSignature = Anew(m_alloc, FunctionDocComments::Signature, m_alloc);
        bool valueTagFound = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctSummary:
                    if (!valueTagFound)
                    {
                        TextBuffer* description = TextBuffer::New(m_alloc);
                        ParseSummary(description, &(result->implicitSignature->locid));
                        if (description->Length() > 0)
                            result->implicitSignature->description = description->Sz();
                    }
                    break;
                case Scanner::dctSignature: 
                    if (!valueTagFound) result->signatures.Add(ParseSignature());
                    break;
                case Scanner::dctParam:
                    if (!valueTagFound) result->implicitSignature->params.Add(ParseParam());
                    break;
                case Scanner::dctReturns:
                    if (!valueTagFound) result->implicitSignature->returnValue = ParseReturns();
                    break;
                case Scanner::dctDeprecated:
                    if (!valueTagFound) result->implicitSignature->deprecated = ParseDeprecated();
                    break;
                case Scanner::dctCompatibleWith:
                    if (!valueTagFound) result->implicitSignature->compatibleWith.Add(ParseCompatibleWith());
                    break;
                case Scanner::dctField:
                    if (!valueTagFound) result->fields.Add(ParseField());
                    break;
                case Scanner::dctValueElement: 
                    // <value> is used for getters, fill in the implicit signature with all the information from 
                    // the value tag in the return value.
                    result->signatures.Clear();
                    result->fields.Clear();
                    result->implicitSignature = ParsePropertySignature();
                    valueTagFound = true;
                    break;
                default:
                    // Ignore unexpected tokens
                    break;
            }
        }
        return result;
    }

    FunctionDocComments::Signature* ParsePropertySignature ()
    {
        auto signature = Anew(m_alloc, FunctionDocComments::Signature, m_alloc);
        signature->returnValue = Anew(m_alloc, FunctionDocComments::ReturnValue, m_alloc);

        auto description = TextBuffer::New(m_alloc);
        bool domElement = false;
        bool elementDomElement = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctType:
                    signature->returnValue->type = TokenValue();
                    break;
                case Scanner::dctDomElement:
                    domElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementDomElement:
                    elementDomElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementType:
                    signature->returnValue->elementType = TokenValue();
                    break;
                case Scanner::dctHelpKeyword:
                    signature->helpKeyword = TokenValue();
                    break;
                case Scanner::dctLocid:
                    signature->locid = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownToken(description);
                    break;
            }
        }

End:
        if (description->Length() > 0)
            signature->description = description->Sz();
        if (signature->returnValue->type == nullptr && domElement)
            signature->returnValue->type = HTMLELMENT;
        if (signature->returnValue->elementType && elementDomElement)
            signature->returnValue->elementType = HTMLELMENT;
        return signature;
    }

    VarDocComments* ParseVarDocComments()
    {
        VarDocComments* result = nullptr;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctVar:
                    result = ParseVar();
                    break;
                default:
                    // Ignore unexpected tokens
                    break;
            }
        }
        return result;
    }

    VarDocComments* ParseVar()
    {
        auto varDoc = Anew(m_alloc, VarDocComments, m_alloc);
        auto description = TextBuffer::New(m_alloc);
        bool domElement = false;
        bool elementDomElement = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctExternalFile:
                    varDoc->externalFile = TokenValue();
                    break;
                case Scanner::dctExternalid:
                    varDoc->externalid  = TokenValue();
                    break;
                case Scanner::dctType:
                    varDoc->type = TokenValue();
                    break;
                case Scanner::dctDomElement:
                    domElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementDomElement:
                    elementDomElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementType:
                    varDoc->elementType = TokenValue();
                    break;
                case Scanner::dctHelpKeyword:
                    varDoc->helpKeyword = TokenValue();
                    break;
                case Scanner::dctLocid:
                    varDoc->locid = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownToken(description);
                    break;
            }
        }
End:
        if (description->Length() > 0)
            varDoc->description = description->Sz();
        if (varDoc->type == nullptr && domElement)
            varDoc->type = HTMLELMENT;
        if (varDoc->elementType == nullptr && elementDomElement)
            varDoc->elementType = HTMLELMENT;

        return varDoc;
    }

    FieldDocComments* ParseFieldDocComments()
    {
        FieldDocComments* result = nullptr;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctField:
                    result = ParseField();
                    break;
                default:
                    // Ignore unexpected tokens
                    break;
            }
        }
        return result;
    }

    FieldDocComments* ParseField()
    {
        auto fieldDoc = Anew(m_alloc, FieldDocComments, m_alloc);
        auto description = TextBuffer::New(m_alloc);
        bool domElement = false;
        bool elementDomElement = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctExternalFile:
                    fieldDoc->externalFile = TokenValue();
                    break;
                case Scanner::dctExternalid:
                    fieldDoc->externalid  = TokenValue();
                    break;
                case Scanner::dctType:
                    fieldDoc->type = TokenValue();
                    break;
                case Scanner::dctName:
                    fieldDoc->name = TokenValue();
                    break;
                case Scanner::dctDomElement:
                    domElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementDomElement:
                    elementDomElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementType:
                    fieldDoc->elementType = TokenValue();
                    break;
                case Scanner::dctHelpKeyword:
                    fieldDoc->helpKeyword = TokenValue();
                    break;
                case Scanner::dctLocid:
                    fieldDoc->locid = TokenValue();
                    break;
                case Scanner::dctStatic:
                    fieldDoc->isStatic = m_scanner.BooleanValue();
                    break;
                case Scanner::dctValue:
                    fieldDoc->value = TokenValue();
                    break;
                case Scanner::dctDeprecated:
                    fieldDoc->deprecated = ParseDeprecated();
                    break;
                case Scanner::dctCompatibleWith:
                    fieldDoc->compatibleWith.Add(ParseCompatibleWith());
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownToken(description);
                    break;
            }
        }
End:
        if (description->Length() > 0)
            fieldDoc->description = description->Sz();
        if (fieldDoc->type == nullptr && domElement)
            fieldDoc->type = HTMLELMENT;
        if (fieldDoc->elementType == nullptr && elementDomElement)
            fieldDoc->elementType = HTMLELMENT;

        return fieldDoc;
    }

    FunctionDocComments::Signature* ParseSignature()
    {
        auto signature = Anew(m_alloc, FunctionDocComments::Signature, m_alloc);

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctExternalFile:
                    signature->externalFile = TokenValue();
                    break;
                case Scanner::dctExternalid:
                    signature->externalid  = TokenValue();
                    break;
                case Scanner::dctHelpKeyword:
                    signature->helpKeyword = TokenValue();
                    break;
                case Scanner::dctSummary:
                    {
                        TextBuffer* description = TextBuffer::New(m_alloc);
                        ParseSummary(description, &(signature->locid));
                        if (description->Length() > 0)
                            signature->description = description->Sz();
                    }
                    break;
                case Scanner::dctParam:
                    signature->params.Add(ParseParam());
                    break;
                case Scanner::dctReturns:
                    signature->returnValue = ParseReturns();
                    break;
                case Scanner::dctDeprecated:
                    signature->deprecated = ParseDeprecated();
                    break;
                case Scanner::dctCompatibleWith:
                    signature->compatibleWith.Add(ParseCompatibleWith());
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    // Ignore unexpected tokens
                    // Still need to parse through the unknown token so we don't misinterpret a 
                    // dctEndElement token that belongs to some skipped element.
                    ParseUnknownTokenNoText();
                    break;
            }
        }

End:
        return signature;
    }

    FunctionDocComments::Param* ParseParam()
    {
        auto param = Anew(m_alloc, FunctionDocComments::Param);
        auto paramDescription = TextBuffer::New(m_alloc);
        bool domElement = false;
        bool elementDomElement = false;
        bool knownTagFound = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctName:
                    param->name = TokenValue();
                    break;
                case Scanner::dctType:
                    param->type = TokenValue();
                    break;
                case Scanner::dctValue:
                    param->value = TokenValue();
                    break;
                case Scanner::dctDomElement:
                    domElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementDomElement:
                    elementDomElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementType:
                    param->elementType = TokenValue();
                    break;
                case Scanner::dctLocid:
                    if (!knownTagFound) param->locid = TokenValue();
                    break;
                case Scanner::dctOptional:
                    param->optional = m_scanner.BooleanValue();
                    break;
                case Scanner::dctSummary:
                    paramDescription->Clear();
                    param->locid = nullptr;
                    ParseSummary(paramDescription, &(param->locid));
                    knownTagFound = true;     // if a summary tag is found use it as description
                    break;
                case Scanner::dctSignature:
                    if (!knownTagFound)
                    {
                        paramDescription->Clear(); // first known tag, clear description
                        param->locid = nullptr;
                    }
                    param->signature = ParseSignature();
                    knownTagFound = true;
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    if (!knownTagFound) 
                    {
                        ParseUnknownToken(paramDescription);
                    }
                    else
                    {
                        // Still need to parse through the unknown token so we don't misinterpret a 
                        // dctEndElement token that belongs to some skipped element.
                        ParseUnknownTokenNoText();
                    }

                    break;
            }
        }

End:
        if (paramDescription->Length() > 0)
            param->description = paramDescription->Sz();
        if (param->type == nullptr && domElement)
            param->type = HTMLELMENT;
        if (param->elementType == nullptr && elementDomElement)
            param->elementType = HTMLELMENT;
        return param;
    }

    void ParseSummary(TextBuffer* summary, LPCWSTR* locid)
    {
        Assert(summary != nullptr);
        Assert(locid != nullptr);

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctLocid:
                    *locid = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    return;
                default:
                    ParseUnknownToken(summary);
                    break;
            }
        }
    }

    FunctionDocComments::ReturnValue* ParseReturns()
    {
        auto returnValue = Anew(m_alloc, FunctionDocComments::ReturnValue, m_alloc);
        auto description = TextBuffer::New(m_alloc);
        bool domElement = false;
        bool elementDomElement = false;

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                 case Scanner::dctExternalFile:
                    returnValue->externalFile = TokenValue();
                    break;
                case Scanner::dctExternalid:
                    returnValue->externalid  = TokenValue();
                    break;
                case Scanner::dctType:
                    returnValue->type = TokenValue();
                    break;
                case Scanner::dctValue:
                    returnValue->value = TokenValue();
                    break;
                case Scanner::dctDomElement:
                    domElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementDomElement:
                    elementDomElement = m_scanner.BooleanValue();
                    break;
                case Scanner::dctElementType:
                    returnValue->elementType = TokenValue();
                    break;
                case Scanner::dctHelpKeyword:
                    returnValue->helpKeyword = TokenValue();
                    break;
                case Scanner::dctLocid:
                    returnValue->locid = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownToken(description);
                    break;
            }
        }

End:
        if (description->Length() > 0)
            returnValue->description = description->Sz();
        if (returnValue->type == nullptr && domElement)
            returnValue->type = HTMLELMENT;
        if (returnValue->elementType == nullptr && elementDomElement)
            returnValue->elementType = HTMLELMENT;
        return returnValue;
    }

    Deprecated* ParseDeprecated()
    {
        auto deprecated = Anew(m_alloc, Deprecated);
        auto message = TextBuffer::New(m_alloc);

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctType:
                    deprecated->type = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownToken(message);
                    break;
            }
        }

End:
        if (message->Length() > 0)
            deprecated->message = message->Sz();
        return deprecated;
    }

    CompatibleWith* ParseCompatibleWith()
    {
        auto compatibleWith = Anew(m_alloc, CompatibleWith);

        Scanner::dcToken token;
        while((token = m_scanner.Scan()) != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctPlatform:
                    compatibleWith->platform = TokenValue();
                    break;
                case Scanner::dctMinVersion:
                    compatibleWith->minVersion = TokenValue();
                    break;
                case Scanner::dctEndElement:
                    goto End;
                default:
                    ParseUnknownTokenNoText();
                    break;
            }
        }

End:
        return compatibleWith;
    }

    void ParseUnknownToken(TextBuffer* text)
    {
       Assert(text != nullptr);

       Scanner::dcToken token = m_scanner.Token();

       switch (token)
       {
        case Scanner::dclWhiteSpace:
        case Scanner::dctText:
            AddEscapedText(text, TokenValue());
            break;
        case Scanner::dclCData:
            ParseCData(text);
            break;
        case Scanner::dctRoot:
        case Scanner::dctParam:
        case Scanner::dctSummary:
        case Scanner::dctSignature:
        case Scanner::dctReturns:
        case Scanner::dctField:
        case Scanner::dctVar:
        case Scanner::dctBr:
        case Scanner::dctValueElement:
        case Scanner::dctDeprecated:
        case Scanner::dctUnknownElement:
            ParseUnknownElement(text);
            break;
        }
    }

    void ParseUnknownElement(TextBuffer* text)
    {
        Assert(text != nullptr);

        LPCWSTR elementName = m_scanner.GetQualifiedName(); 
        bool isEmptyElement = m_scanner.IsEmptyElement();
        Scanner::dcToken token;

        text->Add(L"<");
        text->Add(elementName);

        // parse attributes
        token = m_scanner.Scan();
        while((token != Scanner::dctEOF) && m_scanner.IsAttribute(token))
        {
                text->Add(L" ");
                text->Add(m_scanner.GetQualifiedName());
                text->Add(L"=\"");
                AddEscapedText(text, TokenValue());
                text->Add(L"\"");

                token = m_scanner.Scan();
        }

        if (isEmptyElement)
        {
            text->Add(L"/>");

            Assert(token == Scanner::dctEndElement);
            return;
        }

        text->Add(L">");
        // parse children

        while(token != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctText:
                case Scanner::dclWhiteSpace:
                    AddEscapedText(text, TokenValue());
                    break;
                case Scanner::dclCData:
                    ParseCData(text);
                    break;
                case Scanner::dctRoot:
                case Scanner::dctParam:
                case Scanner::dctReturns:
                case Scanner::dctSummary:
                case Scanner::dctSignature:
                case Scanner::dctField:
                case Scanner::dctVar:
                case Scanner::dctValueElement:
                case Scanner::dctUnknownElement:
                case Scanner::dctBr:
                case Scanner::dctDeprecated:
                    ParseUnknownElement(text);
                    break;
                case Scanner::dctEndElement:
                    text->Add(L"</");
                    text->Add(elementName);
                    text->Add(L">");
                    return;
                default:
                    // Ignore unexpected tokens
                    break;
            };

            token = m_scanner.Scan();
        }
    }

    void ParseUnknownTokenNoText()
    {
       Scanner::dcToken token = m_scanner.Token();

       switch (token)
       {
        case Scanner::dctRoot:
        case Scanner::dctParam:
        case Scanner::dctSummary:
        case Scanner::dctSignature:
        case Scanner::dctReturns:
        case Scanner::dctField:
        case Scanner::dctVar:
        case Scanner::dctBr:
        case Scanner::dctValueElement:
        case Scanner::dctDeprecated:
        case Scanner::dctUnknownElement:
            ParseUnknownElementNoText();
            break;
        default:
            // Skip all other tokens
            break;
        }
    }

    void ParseUnknownElementNoText()
    {
        Scanner::dcToken token;

        token = m_scanner.Scan();

        // parse children
        while(token != Scanner::dctEOF)
        {
            switch(token)
            {
                case Scanner::dctRoot:
                case Scanner::dctParam:
                case Scanner::dctReturns:
                case Scanner::dctSummary:
                case Scanner::dctSignature:
                case Scanner::dctField:
                case Scanner::dctVar:
                case Scanner::dctValueElement:
                case Scanner::dctUnknownElement:
                case Scanner::dctBr:
                case Scanner::dctDeprecated:
                    ParseUnknownElementNoText();
                    break;
                case Scanner::dctEndElement:
                    return;
                default:
                    // Skip all other tokens
                    break;
            };

            token = m_scanner.Scan();
        }
    }

    void ParseCData(TextBuffer* text)
    {
        Assert(text != nullptr);

        text->Add(L"<![CDATA[");
        text->Add(TokenValue());
        text->Add(L"]]>");
    }

    LPCWSTR TokenValue()
    {
        return String::Copy(m_alloc, m_scanner.Value());
    }

    void AddEscapedText(TextBuffer* buffer, LPCWSTR text)
    {
        LPCWSTR copyBegin = text;
        int copyLength = 0;
        LPCWSTR replaceText = nullptr;

        if (text == nullptr)
        {
            return;
        }

        while (*text)
        {
            switch (*text)
            {
            case '\"':
                replaceText = L"&quot;";
                break;
            case '\'':
                replaceText = L"&apos;";
                break;
            case '>':
                replaceText = L"&gt;";
                break;
            case '<':
                replaceText = L"&lt;";
                break;
            case '&':
                replaceText = L"&amp;";
                break;
            }

            if (replaceText)
            {
                if (copyLength > 0) buffer->Add(copyBegin, copyLength);
                buffer->Add(replaceText);
                copyLength = 0;
                copyBegin = ++ text;
                replaceText = nullptr;
                continue;
            }
            copyLength ++;
            text ++;
        }

        if (copyLength > 0)
        {
            buffer->Add(copyBegin, copyLength);
        }
    }
};

FunctionDocComments::Param* FunctionDocComments::Signature::FindParam(LPCWSTR name)
{
    for (int i = 0; i<params.Count(); i++)
    {
        auto param = params.Item(i);
        if (param->name && wcscmp(param->name, name) == 0)
        {
            return param;
        }
    }
    return nullptr;
}

HRESULT Authoring::ParseFuncDocComments(
    __in ArenaAllocator* alloc,
    __in LPCWSTR docText,
    __in CommentType commentType,
    __out_ecount(1) FunctionDocComments** funcDoc)
{
    Assert(alloc != nullptr);
    Assert(funcDoc != nullptr);
    Assert(docText != nullptr);

    if (commentType == commenttypeVSDoc)
    {
        VsDocCommentsParser parser;
        return parser.ParseFuncDocComments(alloc, docText, funcDoc);
    }
    else if (commentType == commenttypeJSDoc)
    {
        JsDocCommentsParser parser;
        return parser.ParseFuncDocComments(alloc, docText, funcDoc);
    }
    else
    {
        *funcDoc = nullptr;
        return S_OK;
    }
}

HRESULT Authoring::ParseVarDocComments(
    __in ArenaAllocator* alloc, 
    __in LPCWSTR docText, 
    __in CommentType commentType,
    __out_ecount(1) VarDocComments** varDoc)
{
    Assert(alloc != nullptr);
    Assert(docText != nullptr);
    Assert(varDoc != nullptr);
    if (commentType == commenttypeVSDoc)
    {
        VsDocCommentsParser parser;
        return parser.ParseVarDocComments(alloc, docText, varDoc);
    }
    else if (commentType == commenttypeJSDoc)
    {
        JsDocCommentsParser parser;
        return parser.ParseVarDocComments(alloc, docText, varDoc);
    }
    else
    {
        *varDoc = nullptr;
        return S_OK;
    }
}

HRESULT Authoring::ParseFieldDocComments(
    __in ArenaAllocator* alloc, 
    __in LPCWSTR fieldName,
    __in LPCWSTR docText, 
    __in CommentType commentType,
    __in bool isGlobalVariableAsField,
    __out_ecount(1) FieldDocComments** fieldDoc)
{
    Assert(alloc != nullptr);
    Assert(docText != nullptr);
    Assert(fieldDoc != nullptr);
    if (commentType == commenttypeVSDoc)
    {
        VsDocCommentsParser parser;
        return parser.ParseFieldDocComments(alloc, docText, fieldDoc);
    }
    else if (commentType == commenttypeJSDoc)
    {
        JsDocCommentsParser parser;
        return parser.ParseFieldDocComments(alloc, fieldName, docText, isGlobalVariableAsField, fieldDoc);
    }
    else
    {
        *fieldDoc = nullptr;
        return S_OK;
    }
}

HRESULT Authoring::ParseTypeDefinitionComments(__in ArenaAllocator* alloc, __in LPCWSTR docText, __in CommentType commentType, TypeDefintionSetDocComments** typeDefintionSetDocComments)
{
    Assert(alloc != nullptr);
    Assert(docText != nullptr);
    Assert(typeDefintionSetDocComments != nullptr);
    if (commentType == commenttypeVSDoc)
    {
        AssertMsg(false, "This is not currently supported!");
        return E_NOTIMPL;
    }
    else if (commentType == commenttypeJSDoc)
    {
        JsDocCommentsParser parser;
        return parser.ParseTypeDefinitionComments(alloc, docText, typeDefintionSetDocComments);
    }
    else
    {
        *typeDefintionSetDocComments = nullptr;
        return S_OK;
    }
}
