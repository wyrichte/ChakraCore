#pragma once

namespace JsDiag
{
    //
    // Low level support for getting stack frames, just a wrapper over IDebug... API.
    //
    class StackFrameHelper
    {
        CComPtr<IDebugControl4> m_debugControl;
    public:
        StackFrameHelper(IDebugClient* debugClient) 
        {
            Assert(debugClient);
            HRESULT hr = debugClient->QueryInterface(__uuidof(IDebugControl4), (LPVOID*)&m_debugControl);
            CheckHR(hr);
        }

        void GetStackFrames(ULONG64 frameOffset, ULONG64 stackOffset, ULONG64 instructionOffset, DEBUG_STACK_FRAME* frames, ULONG frameCount, PULONG frameCountFilled)
        {
            HRESULT hr = m_debugControl->GetStackTrace(frameOffset, stackOffset, instructionOffset, frames, frameCount, frameCountFilled);
            CheckHR(hr);
        }
    };

} // namespace JsDiag.
