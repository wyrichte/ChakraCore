//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once
namespace Projection
{
    class WinRTPromiseEngineInterfaceExtensionObject : public Js::EngineExtensionObjectBase
    {
    public:
        WinRTPromiseEngineInterfaceExtensionObject(Js::ScriptContext* scriptContext);
        void Initialize();
#if DBG
        void DumpByteCode() override;
#endif
        static void __cdecl InitializePromiseNativeInterfaces(Js::DynamicObject* promiseNativeInterfaces, Js::DeferredTypeHandlerBase * typeHandler, Js::DeferredInitializeMode mode);
        Js::Var GetPromiseConstructor(_In_ Js::ScriptContext * scriptContext);

        class EntryInfo
        {
        public:
            static Js::NoProfileFunctionInfo Promise_EnqueueTask;
        };
        static Js::Var EntryPromise_EnqueueTask(Js::RecyclableObject *function, Js::CallInfo callInfo, ...);

    private:
        Js::DynamicObject* promiseNativeInterfaces;
        Js::FunctionBody* promiseByteCode;
        void EnsurePromiseByteCode(_In_ Js::ScriptContext * scriptContext);
    };
}
