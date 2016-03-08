//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//
// This object provides the ability to the tools to make use of 'diagnosticsScript' object.
// The idea behind that is we will get extend the tools facing object to provide direct API from the runtime.
// for eg. diagnosticsScript.getStackTrace(maxCount)
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class DiagnosticsScriptObject : public DynamicObject
    {
    public:
        DiagnosticsScriptObject(DynamicType * type);
        static DiagnosticsScriptObject * New(Recycler * recycler, DynamicType * type);

        DEFINE_VTABLE_CTOR(DiagnosticsScriptObject, DynamicObject);
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(DiagnosticsScriptObject);

        class EntryInfo
        {
        public:
            static FunctionInfo GetStackTrace;
            static FunctionInfo DebugEval;

#ifdef EDIT_AND_CONTINUE
            static FunctionInfo EditSource;
#endif
        };

        static Var EntryGetStackTrace(RecyclableObject* function, CallInfo callInfo, ...);
        static Var EntryDebugEval(RecyclableObject* function, CallInfo callInfo, ...);

#ifdef EDIT_AND_CONTINUE
        static Var EntryEditSource(RecyclableObject* function, CallInfo callInfo, ...);
#endif

        template <size_t N>
        static void SetPropertyStatic(RecyclableObject* obj, const char16(&propertyName)[N], Var value,
            ScriptContext* scriptContext);
        template <size_t N>
        static void SetPropertyStatic(RecyclableObject* obj, const char16(&propertyName)[N], BSTR value,
            ScriptContext* scriptContext);

        PropertyId GetFunctionNameId() const { return functionNameId; }
        PropertyId GetUrlId() const { return urlId; }
        PropertyId GetDocumentId() const { return documentId; }
        PropertyId GetLineId() const { return lineId; }
        PropertyId GetColumnId() const { return columnId; }

    private:
        static DynamicObject* GetActiveScopeObject(ScriptContext* targetScriptContext, bool* isStrict);

    private:
        PropertyId functionNameId;
        PropertyId urlId;
        PropertyId documentId;
        PropertyId lineId;
        PropertyId columnId;
    };
}
