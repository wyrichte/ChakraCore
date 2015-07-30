//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

bool g_autoBreakpoints = false;
bool g_targetedTest = false;
bool g_dynamicAttach = false;
LPCTSTR g_dbgBaselineFilename = nullptr;

class DummyModule: public CAtlExeModuleT<DummyModule>
{
} _Module;

void PrintUsage()
{
    _tprintf(_T("\nUsage: jdtest [-c dbg_commands] [-q] [-v] [-debug] command\n"));
}

int _cdecl _tmain(int argc, __in_ecount(argc) _TCHAR* argv[])
{
    ULONG targetPid = 0;
    bool attach = false;
    string dbgCommands;
    string cmdLine;

    TCHAR eventCmd[] = _T("-event:");
    TCHAR baselineCmd[] = _T("-dbgbaseline:");
    TCHAR baselineCmdWithoutColon[] = _T("-dbgbaseline");
    string inspectMaxStringLengthCmd(_T("-inspectmaxstringlength:"));
    
    _putenv("JDTEST=1");

    bool quiet = false;
    bool verbose = false;
    bool debugEnabled = false;
    int inspectMaxStringLength = 0;

    // Parse command line
    {
        bool doneOptions = false;
        for (int i = 1; i < argc; i++)
        {
            if (!doneOptions)
            {
                // parse known options
                if(_tcsicmp (argv[i], _T("-c")) == 0)
                {
                    if (i + 1 < argc)
                    {
                        dbgCommands = argv[++i];
                    }
                    else
                    {
                        PrintUsage();
                        return -1;
                    }
                    continue;
                }
                if(_tcsicmp (argv[i], _T("-q")) == 0)
                {
                    quiet = true;
                    continue;
                }
                if(_tcsicmp (argv[i], _T("-v")) == 0)
                {
                    verbose = true;
                    continue;
                }
                 if(_tcsicmp (argv[i], _T("-debug")) == 0)
                {
                    debugEnabled = true;
                    continue;
                }
                if(_tcsicmp (argv[i], _T("-auto")) == 0)
                {
                    g_autoBreakpoints = true;
                    quiet = true;
                    continue;
                }
                 if(_tcsicmp (argv[i], _T("-targeted")) == 0)
                {
                    g_targetedTest = true;
                    quiet = true;
                    continue;
                }                  
                else if(_tcsnicmp(argv[i], inspectMaxStringLengthCmd.c_str(), inspectMaxStringLengthCmd.length()) == 0)
                {
                    inspectMaxStringLength = StrToInt(argv[i] + inspectMaxStringLengthCmd.length());
                    continue;
                }
                else if(_tcsicmp (argv[i], _T("-dynamicattach")) == 0)
                {
                    g_dynamicAttach = true;
                    continue;
                }
                else if(!_tcsicmp(argv[i], baselineCmdWithoutColon))
                {
                    g_dbgBaselineFilename = nullptr;
                    continue;
                }
                else if(!_tcsnicmp(argv[i], baselineCmd, _tcslen(baselineCmd)))
                {
                    g_dbgBaselineFilename = argv[i] + _tcslen(baselineCmd);
                    if(_tcslen(g_dbgBaselineFilename) == 0)
                    {
                        _tprintf(_T("ERROR: must specify a baseline file\n"));
                        return 1;
                    }
                    continue;
                }
                else if(!_tcsnicmp(argv[i], eventCmd, _tcslen(eventCmd)))
                {
                    // Switch format: -event:jdtest1234

                    attach = true;

                    TCHAR* ptr = argv[i] + _tcslen(eventCmd);
                    TCHAR jdtest[] = _T("jdtest");
                    if(!_tcsnicmp(ptr, jdtest, _tcslen(jdtest)))
                    {
                        ptr += _tcslen(jdtest);

                        targetPid = _ttoi(ptr);
                    }
                    else
                    {
                        // This should never be used by the user
                        _tprintf(_T("ERROR: incorrect value passed to -event switch\n"));
                        return 1;
                    }
                    continue;
                }
                else if(argv[i] == string(_T("-?")))
                {
                    PrintUsage();
                    return 0;
                }

                // Unknown arg, assume options end
                doneOptions = true;
            }

            // For now construct simple command line, assuming no " in args.
            cmdLine = cmdLine + (cmdLine.empty() ? _T("") : _T(" "))
                + _T("\"") + argv[i] + _T("\"");
        }
    }

    CComPtr<SimpleDebugger> spDebugger;
    SimpleDebugger::Create(dbgCommands, &spDebugger);

    if(g_dynamicAttach && !g_targetedTest && !g_autoBreakpoints)
    {
        _tprintf(_T("ERROR: -dynamicattach must be combined with -auto or -targeted\n"));
        return 1;
    }

    if (spDebugger)
    {
        spDebugger->SetQuiet(quiet);
        spDebugger->SetVerbose(verbose);

        if (inspectMaxStringLength > 0)
        {
            spDebugger->SetInspectMaxStringLength(inspectMaxStringLength);
        }

        if(attach)
        {
            spDebugger->Attach(targetPid);
        }
        else
        {
            spDebugger->DebugLaunch(const_cast<LPTSTR>(cmdLine.c_str()));
        }
    }
    return (!spDebugger || spDebugger->HasFailure()) ? -1 : 0;
}
