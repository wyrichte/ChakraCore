#define REGISTER_PROXY_DLL

#define USE_STUBLESS_PROXY //defined only with MIDL switch /Oicf
#define ENTRY_PREFIX JsHostScriptSitePrx

#include "JsHostScriptSite_p.c"
#include "JsHostScriptSite_i.c"
#include "JsHostScriptSite_dlldata.c"
