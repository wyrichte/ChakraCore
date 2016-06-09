//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

class DummyModule: public CAtlExeModuleT<DummyModule>
{
} _Module;

void PrintUsage()
{
    _tprintf(_T("\nUsage: jdtest [-c dbg_commands] [-q] [-v] [-debug] command\n"));
}

int _cdecl _tmain(int argc, __in_ecount(argc) _TCHAR* argv[])
{
    string dbgCommands;
    string cmdLine;

    bool quiet = false;
    bool verbose = false;

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
                if(argv[i] == string(_T("-?")))
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

    if (spDebugger)
    {
        spDebugger->SetQuiet(quiet);
        spDebugger->SetVerbose(verbose);

        spDebugger->DebugLaunch(const_cast<LPTSTR>(cmdLine.c_str()));
    }

    return (!spDebugger || spDebugger->HasFailure()) ? -1 : 0;
}
