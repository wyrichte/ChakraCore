//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    // Specialized Error Object, containing WinRT specific information.
    // Not applicable for non-Windows platforms
    // TODO: Move this out of ChakraCore
#ifdef _WIN32
    class JavascriptErrorDebug : public JavascriptError
    {
    protected:
        DEFINE_VTABLE_CTOR(JavascriptErrorDebug, JavascriptError);
    public:
        JavascriptErrorDebug(IErrorInfo* perrinfo, DynamicType* type, BOOL isExternalError = FALSE) :
          JavascriptError(type, isExternalError)
        {
            Assert(perrinfo);
            pErrorInfo = perrinfo;
            restrictedStrings.referenceStr = nullptr;
            restrictedStrings.restrictedErrStr = nullptr;
            restrictedStrings.capabilitySid = nullptr;
        }

        // For differentiating between JavascriptError and JavascriptErrorDebug
        bool HasDebugInfo();

        void Finalize(bool isShutdown) override;
        void Dispose(bool isShutdown) override;

        static void __declspec(noreturn) MapAndThrowErrorWithInfo(ScriptContext* scriptContext, HRESULT hr);
#ifdef ENABLE_PROJECTION
        static void ClearErrorInfo(ScriptContext* scriptContext);
#endif

        void SetRestrictedErrorStrings(RestrictedErrorStrings * proerrstr)
        {
            Assert(proerrstr);
            restrictedStrings.referenceStr = proerrstr->referenceStr;
            restrictedStrings.restrictedErrStr = proerrstr->restrictedErrStr;
            restrictedStrings.capabilitySid = proerrstr->capabilitySid;
        }

        BSTR GetRestrictedErrorString()
        {
            if (restrictedStrings.restrictedErrStr)
            {
                return SysAllocString(restrictedStrings.restrictedErrStr);
            }
            return nullptr;
        }

        BSTR GetRestrictedErrorReference()
        {
            if (restrictedStrings.referenceStr)
            {
                return SysAllocString(restrictedStrings.referenceStr);
            }
            return nullptr;
        }

        BSTR GetCapabilitySid()
        {
            if (restrictedStrings.capabilitySid)
            {
                return SysAllocString(restrictedStrings.capabilitySid);
            }
            return nullptr;
        }

        IErrorInfo * GetRestrictedErrorInfo() { return pErrorInfo; }

        JavascriptError* CreateNewErrorOfSameType(JavascriptLibrary* targetJavascriptLibrary) override;

        void SetErrorInfo();

    private:
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptErrorDebug);
        static void GetErrorTypeFromNumber(HRESULT hr, ErrorTypeEnum * errorTypeOut);
        static HRESULT GetExcepAndErrorInfo(ScriptContext* scriptContext, HRESULT hrReturned, EXCEPINFO *pexcepinfo, RestrictedErrorStrings * proerrstr, IErrorInfo ** pperrinfo);
        RestrictedErrorStrings restrictedStrings; // WinRT specific error strings
        IErrorInfo * pErrorInfo; // reference to the original IErrorInfo object
        static __declspec(thread)  char16 msgBuff[512];

        static void SetErrorMessage(JavascriptError *pError, HRESULT errCode, PCWSTR varDescription, ScriptContext* scriptContext);
        static void __declspec(noreturn) SetMessageAndThrowError(ScriptContext * scriptContext, JavascriptError * pError, int32 hCode, EXCEPINFO * pei);
    };

    template <> bool VarIsImpl<JavascriptErrorDebug>(RecyclableObject* object);
#endif
}
