#include <windows.h>
#include "ScriptProjectionHost.hxx"

HRESULT CreateScriptProjectionHost(
  __in IActiveScript * pScriptEngine,
  __in IActiveScriptProjection * scriptProjection,
  __in_opt IScriptHostContextEvent *hostEventCallback,
  __in_opt IWebPlatformPriorityContext *priorityContext,
  __in_opt Windows::UI::Core::ICoreDispatcher *coreDispatcher,
  __in BOOL fIsWinRTEnabled,
  __in_ecount_opt(applicationObjectsToExposeCount) NativeObjectEntry* applicationObjectsToExpose,
  __in size_t applicationObjectsToExposeCount)
{
  return E_NOTIMPL;
}

HRESULT CreateScriptProjectionHost(
  __in IActiveScript * pScriptEngine,
  __in IActiveScriptProjection * scriptProjection,
  __in_ecount(applicationObjectsToExposeCount) NativeObjectEntry* applicationObjectsToExpose,
  __in size_t applicationObjectsToExposeCount)
{
  return E_NOTIMPL;
}
