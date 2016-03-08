#pragma once
extern HINSTANCE g_jscript9dll;
extern DllGetClassObjectPtr pfDllGetClassObject;
extern CRITICAL_SECTION PrintCS;
extern LARGE_INTEGER g_frequency;
HRESULT CreateNewEngine(ScriptSite ** scriptSite);
DWORD WINAPI ThreadProc(LPVOID param);
extern HANDLE hProcess;

void Fail(__in __nullterminated char *reason);
void ParseArg(__in __nullterminated char16 *arg);
void Usage();

