#pragma once

#define QI_IMPL(name, intf)\
    if (IsEqualIID(riid, name))\
{   \
    *ppvObj = static_cast<intf *>(this); \
    static_cast<intf *>(this)->AddRef();\
    return NOERROR;\
}\

#define QI_IMPL_INTERFACE(intf) \
    QI_IMPL(__uuidof(intf), intf)

#define IfNullReturnError(EXPR, ERROR) do { if (!(EXPR)) { return (ERROR); } } while(FALSE)
#define IfFailedReturn(EXPR) do { hr = (EXPR); if (FAILED(hr)) { return hr; }} while(FALSE)
#define IfFailedGoLabel(expr, label) if (FAILED(expr)) { goto label; }
#define IfFailedGo(expr) IfFailedGoLabel(expr, LReturn)

#define DEBUGPRINT(...) do { if(g_debugPrint) printf(__VA_ARGS__); } while(0)

// The following will assign to hr
#define IfFailGo(expr) IfFailedGoLabel(hr = (expr), Error)

#define ASSERT(x) do { if(!(x)) { printf("ASSERTION failed: %s\n", #x); } } while(0)

#include <stdio.h>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include <float.h>
#include <atlbase.h>
#include <dispex.h>
#include "activscp.h"
#include "activscp_private.h"
#include "edgescriptdirect.h"
#include <psapi.h>
#ifdef ALLOW_V8
#include "v8.h"
#endif
#include "jsrt.h"


extern int g_debugPrint;

const CLSID CLSID_JScript  = { 0xf414c260, 0x6ac0, 0x11cf, 0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58 };
const CLSID CLSID_Chakra = { 0x1b7cd997, 0xe5ff, 0x4932, 0xa7, 0xa6, 0x2a, 0x9e, 0x63, 0x6d, 0xa3, 0x85 };
typedef HRESULT (__stdcall *DllGetClassObjectPtr)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);

class AutoCriticalSection 
{
public:
    AutoCriticalSection(CRITICAL_SECTION* cs) {m_cs = cs; EnterCriticalSection(cs); };
    ~AutoCriticalSection() { LeaveCriticalSection(m_cs); }
private:
    CRITICAL_SECTION* m_cs;
};

// forward decls
class ScriptSite;

#include "engine.h"
#include "scriptsite.h"
#include "jsperf.h"
#include "perftest.h"

