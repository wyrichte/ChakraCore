//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#pragma once
#ifdef EDIT_AND_CONTINUE

class EditAndContinue
{
public:
    //-----------------------------------------------------------------------------
    // Dispatch a call synchronously in given thread through PDM.
    //-----------------------------------------------------------------------------
    template <class Fn>
    static HRESULT Dispatch(IDebugBitCorrectApplicationThread* pThread, const Fn& fn)
    {
        class DebugThreadCallDispatcher :
            public Js::ComObjectBase<IDebugThreadCall, DebugThreadCallDispatcher>
        {
        private:
            const Fn* m_fn;

        public:
            void Init(const Fn* fn)
            {
                m_fn = fn;
            }

            STDMETHOD(ThreadCallHandler)(
                DWORD_PTR dwParam1,
                DWORD_PTR dwParam2,
                DWORD_PTR dwParam3)
            {
                return (*m_fn)();
            }
        };

        CComPtr<DebugThreadCallDispatcher> spDispatcher;
        HRESULT hr = DebugThreadCallDispatcher::CreateInstance(&spDispatcher);
        if (SUCCEEDED(hr))
        {
            spDispatcher->Init(&fn);
            hr = pThread->SynchronousCallIntoThread(spDispatcher, 0, 0, 0);
        }
        return hr;
    }

    static HRESULT InitializeScriptEdit(ScriptEngine * scriptEngine, IActiveScriptEdit ** scriptEdit);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    static void InitializeEditTest(Js::ScriptContext * scriptContext);
#endif
};

#endif
