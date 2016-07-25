//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "Errors.h"

class CommandLineError : public ErrorBase
{
protected:
    CommandLineError(unsigned int code) : ErrorBase(code) {}
};

class CommandLineArgumentError : public CommandLineError
{
public:
    CommandLineArgumentError(const std::wstring& argument) :
        m_argument(argument),
        CommandLineError(ErrorCodeCommandLineArgument)
    {
    }

    std::wstring Description() override
    {
        return L"Unrecognized command line parameter " + m_argument;
    }

private:
    const std::wstring m_argument;
};

class CommandLineFileError : public CommandLineError
{
public:
    CommandLineFileError(const std::wstring& filePath) :
        m_filePath(filePath),
        CommandLineError(ErrorCodeCommandLineFile)
    {
    }

    std::wstring Description() override
    {
        return L"Cannot read from file " + m_filePath;
    }

private:
    const std::wstring m_filePath;
};

class MalformedCommandLineError : public CommandLineError
{
public:
    MalformedCommandLineError() :
        CommandLineError(ErrorCodeMalformedCommandLine)
    {
    }

    std::wstring Description() override
    {
        return L"Must specify at least one input .winmd file";
    }
};