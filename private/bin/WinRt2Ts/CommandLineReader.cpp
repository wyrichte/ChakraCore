//---------------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "CommandLineReader.h"

using namespace std;

Configuration CommandLineReader::Read(int argc, wchar_t* argv[])
{
    Configuration config;
    for (int i = 1; i < argc; i++)
    {
        auto& argument = argv[i];

        if (argument[0] == L'@')
        {
            auto fileName = &argument[1];
            wifstream responseFileStream(fileName);

            while (!responseFileStream.eof())
            {
                if (!responseFileStream.good())
                {
                    throw CommandLineFileError(fileName);
                }

                wstring parameter;
                getline(responseFileStream, parameter);

                if (responseFileStream.good())
                {
                    ApplyParameter(config, parameter);
                }
            }
        }
        else
        {
            ApplyParameter(config, wstring(argument));
        }
    }

    if (config.winmds.empty())
    {
        throw MalformedCommandLineError();
    }

    return config;
}

void CommandLineReader::ApplyParameter(Configuration& config, const wstring& parameter)
{
    wstring lowercaseParam = parameter;
    transform(lowercaseParam.begin(), lowercaseParam.end(), lowercaseParam.begin(), towlower);
    switch (m_parameterState)
    {
    case ParameterState::None:
    {
        if (lowercaseParam[0] == L'-')
        {
            if (lowercaseParam.compare(1, lowercaseParam.size() - 1, L"outfilepath") == 0)
            {
                m_parameterState = ParameterState::ExpectingOutFilePath;
            }
            else if (lowercaseParam.compare(1, lowercaseParam.size() - 1, L"enableversioningallassemblies") == 0)
            {
                config.enableVersioningAllAssemblies = true;
            }
            else if (lowercaseParam.compare(1, lowercaseParam.size() - 1, L"enableversioningwindowsassemblies") == 0)
            {
                config.enableVersioningWindowsAssemblies = true;
            }
            else if (lowercaseParam.compare(1, lowercaseParam.size() - 1, L"emitanyforunresolvedtypes") == 0)
            {
                config.emitAnyForUnresolvedTypes = true;
            }
            else
            {
                throw CommandLineArgumentError(parameter);
            }
        }
        else
        {
            const wchar_t winMdExtension[] = L".winmd";
            const auto winMdExtensionLength = ARRAYSIZE(winMdExtension) - 1; // sizeof includes the null terminator
            if (lowercaseParam.size() >= winMdExtensionLength && lowercaseParam.compare(lowercaseParam.size() - winMdExtensionLength, winMdExtensionLength, winMdExtension) == 0)
            {
                config.winmds.push_back(lowercaseParam);
            }
            else
            {
                throw CommandLineArgumentError(parameter);
            }
        }
    }
    break;
    case ParameterState::ExpectingOutFilePath:
    {
        config.outFilePath = lowercaseParam;
        m_parameterState = ParameterState::None;
    }
    break;
    }
}
