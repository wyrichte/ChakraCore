//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

class ErrorBase : public std::exception
{
public:
    unsigned int Code()
    {
        return m_code;
    }

    virtual std::wstring Description() = 0;

protected:
    ErrorBase(unsigned int code) :
        m_code(code)
    {
    }

private:
    unsigned int m_code;
};

const unsigned int ErrorCodeUnrecognizedType = 10001;
const unsigned int ErrorCodeUnexpectedProperty = 10002;
const unsigned int ErrorCodeCommandLineArgument = 10003;
const unsigned int ErrorCodeCommandLineFile = 10004;
const unsigned int ErrorCodeMalformedCommandLine = 10005;
const unsigned int ErrorCodeMetadataDispenser = 10006;
const unsigned int ErrorCodeUnknown = 10007;
const unsigned int ErrorCodeWinmdRead = 10008;
