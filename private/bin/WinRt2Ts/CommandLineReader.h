//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "CommandLineError.h"

struct Configuration
{
    std::vector<std::wstring> winmds;
    std::wstring outFilePath;
    bool enableVersioningAllAssemblies = false;
    bool enableVersioningWindowsAssemblies = false;
    bool emitAnyForUnresolvedTypes = false;
};

class CommandLineReader
{
public:
    Configuration Read(int argc, wchar_t* argv[]);

private:
    void ApplyParameter(Configuration& config, const std::wstring& parameter);

private:
    enum class ParameterState
    {
        None,
        ExpectingOutFilePath
    };

    ParameterState m_parameterState = ParameterState::None;
};
