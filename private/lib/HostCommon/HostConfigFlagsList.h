//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#ifdef FLAG
FLAG(bool, BVT,                             "BVT mode. Skip non-script output", false)
FLAG(bool, HtmlKeepAlive,                   "Keep alive after page loaded. Exit on WScript.Quit() call. Used with -html only", false)
FLAG(int,  HtmlOnScriptError,               "Set OnSriptError host action: 1 - Continue script (default). 0 - Stop script. Other: Trident handles it", 1)
FLAG(bool, HtmlVersion,                     "Let Trident decide script version based on html page. Don't use default latest version. Used with -html only", false)
FLAG(bool, HtmlVisible,                     "Show jshost html window. Implies -HtmlKeepAlive.", false)
FLAG(bool, HtmlUseShdocvw,                  "Use Shdocvw.", false)
FLAG(bool, IgnoreScriptErrorCode,           "Don't return error code on script error", false)
FLAG(bool, EnableDebug,                     "Run in debug mode, allowing attach", false)
FLAG(bool, Break,                           "Stop in JS debugger on initial script execution. Requires EnableDebug.", false)
FLAG(bool, AsyncBreak,                      "Async break into debugger every 10 milliseconds. Requires EnableDebug", false)
FLAG(BSTR, DebugLaunch,                     "Create the test debugger and execute test in the debug attached mode", NULL)
FLAG(bool, Auto,                            "Runs the test debugger in automatic breakpoint mode", false)
FLAG(bool, Targeted,                        "Runs the test debugger in targeted testing mode", false)
FLAG(int,  InspectMaxStringLength,          "Max string length to dump in locals inspection", 16)
FLAG(bool, DumpLocalsOnDebuggerBp,          "Dump locals on 'debugger' statement", false)
FLAG(bool, Configurable,                    "Makes projection objects configurable", false)
FLAG(BSTR, Serialized,                      "If source is UTF8, deserializes from bytecode file", NULL)
FLAG(bool, RecreateByteCodeFile,            "If source is UTF8, serializes to a bytecode file", true)
FLAG(BSTR, GenerateLibraryByteCodeHeader,   "Generate bytecode header file from library code", NULL)
FLAG(BSTR, GenerateValidPointersMapHeader,  "Generate valid pointers map header file", NULL)
FLAG(bool, EnableDelegateWrapper,           "Support IDelegateWrapper in host", false)
FLAG(bool, HostManagedSource,               "Sources passed to script engine are host managed.", false)
FLAG(bool, DebugDumpText,                   "Dump logs on the command prompt.", false)
FLAG(bool, DumpRecyclerStats,               "Dump the memory and recycler stats after the final GC", false)
FLAG(bool, AutomaticSourceRundown,          "Does a PerformSourceRundown call prior to calling Attach (without needing to use WScript.PerformSourceRundown).", false)
FLAG(bool, LogProfilerVerbose,              "Log verbose script profiler events", true)
FLAG(bool, LogLineColumnProfileInfo,        "Logs line/column information for each function when running with the profiler attached.", true)
FLAG(bool, LogProfilerCallTree,             "Log script profiler function call tree.", false)
FLAG(bool, NoLibraryStackFrameDebugger,     "Disable registering library stack frame debugger options", false)
FLAG(bool, VerifyShortAndFullNameValues,    "Check that /**bp:evaluate()**/ short name expressions are equal to full name ones.", true)
FLAG(bool, PerformUTF8BoundaryTest,         "Performs a serialize/deserialize test on utf8 boundary aligned source, source length must be a factor of 4096.", false)
FLAG(bool, DiagnosticsEngine,               "Create default engine as diagnostics engine (simulate the engine used to run F12 JS code).", false)
FLAG(bool, EnableMiscWScriptFunctions,      "Some functions on WScript are disabled, this is for enabling them.", false)
FLAG(bool, EnableExtendedErrorMessages,		"JSHost will by default print short error mesages, this is for enabling the long format.", false)
FLAG(bool, EvalRestriction,                 "Puts the script engine into Eval restricted mode, to restrict eval access use WScript.RegisterEvalApprover(func).", false)
FLAG(bool, EvalIsAllowed,                   "If EvalRestriction flag is specified, this flag is used to determine the default value of 'IsEvalAllowed' API call that will be done on the host.", false)
FLAG(bool, MemProtectHeapTest,              "Specific test for memory protect heap", false)
FLAG(bool, EnableOutOfProcJIT,				"JIT jobs should be run in a separate process", false)
#undef FLAG
#endif
#ifdef FLAGA2
FLAGA2(DT, DebugLaunch, BSTR, Targeted, bool, "Combining DebugLaunch and Targeted flags")
#undef FLAGA2
#endif
