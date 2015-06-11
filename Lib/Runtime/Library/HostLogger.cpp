//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#ifdef TEST_LOG

namespace Js
{
        LONG HostLogger::numInstances = -1;

        HostLogger::HostLogger(ThreadContext *threadContext) : m_pThreadContext(threadContext), m_logfile(NULL)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));
            currentInstance = InterlockedIncrement(&HostLogger::numInstances);
            m_pDispIdMap = Anew(threadContext->GetThreadAlloc(), DispIdMap, threadContext);
            OpenLogFile();
        }

        HostLogger::~HostLogger()
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));
            CloseLogFile();
        }
       
        void HostLogger::OpenLogFile()
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));

            wchar_t filename[_MAX_PATH];
            swprintf_s(filename, _countof(filename), L"%s.%d", (LPCWSTR)Js::Configuration::Global.flags.HostLogging, currentInstance);

            if(_wfopen_s(&m_logfile, filename, L"wt") != 0 || m_logfile == NULL)
            {
                Js::Throw::FatalInternalError();
            }
        }

        void HostLogger::DumpType(Var value) 
        {
            if (!TaggedNumber::Is(value) &&
                JavascriptOperators::IsUndefinedObject(value, RecyclableObject::FromVar(value)->GetLibrary()->GetUndefined()))
            {
                fprintf(m_logfile, "undefined:");
            }
            else
            {
                TypeId id = JavascriptOperators::GetTypeId(value);
                switch (id) {
                    case TypeIds_String:
                        fprintf(m_logfile, "String:");
                        break;
                    case TypeIds_Integer:
                    case TypeIds_Number:
                    case TypeIds_Int64Number:
                    case TypeIds_UInt64Number:
                        fprintf(m_logfile, "Number:");                
                        break;
                    case TypeIds_Boolean:
                        fprintf(m_logfile, "Boolean:");                
                        break;
                    case TypeIds_Function:
                        fprintf(m_logfile, "[object Function]:");
                        break;
                    case TypeIds_Array:
                    case TypeIds_ES5Array:
                        fprintf(m_logfile, "[object Array]:");
                        break;
                    case TypeIds_Date:
                    case TypeIds_WinRTDate:
                        fprintf(m_logfile, "[object Date]:");
                        break;
                    case TypeIds_RegEx:
                        fprintf(m_logfile, "[object RegExp]:");
                        break;
                    case TypeIds_VariantDate:
                        fprintf(m_logfile, "[object VariantDate]:");
                        break;
                    case TypeIds_Symbol:
                        fprintf(m_logfile, "Symbol:");
                        break;
                    case TypeIds_HostDispatch:
                    case TypeIds_HostObject:
                    case TypeIds_Object:
                        fprintf(m_logfile, "[object Object]:");
                        break;
                    case TypeIds_Null:
                        fprintf(m_logfile, "null:");
                        break;
                    case TypeIds_Undefined:
                        fprintf(m_logfile, "undefined:");
                        break;
                    case TypeIds_ArrayIterator:
                        fprintf(m_logfile, "[object Array Iterator]:");
                        break;
                    case TypeIds_MapIterator:
                        fprintf(m_logfile, "[object Map Iterator]:");
                        break;
                    case TypeIds_SetIterator:
                        fprintf(m_logfile, "[object Set Iterator]:");
                        break;
                    case TypeIds_StringIterator:
                        fprintf(m_logfile, "[object String Iterator]:");
                        break;
                    case TypeIds_Generator:
                        fprintf(m_logfile, "[object Generator]:");
                        break;
                    default:
                        AssertMsg(false, "Unknown type");
                }
            }
        }

        void HostLogger::DumpValue(Var value)
        {
            if (!TaggedNumber::Is(value) &&
                JavascriptOperators::IsUndefinedObject(value, RecyclableObject::FromVar(value)->GetLibrary()->GetUndefined()))
            {
                fprintf(m_logfile, "undefined");
            }
            else
            {
                TypeId id = JavascriptOperators::GetTypeId(value);
                ScriptContext* scriptContext = TaggedNumber::Is(value) ? nullptr : RecyclableObject::FromVar(value)->GetScriptContext();
                double val = 0.0;
                switch (id) {
                    case TypeIds_String:
                        fprintf(m_logfile, "\"%S\"", JavascriptConversion::ToString(value, scriptContext)->GetSz());
                        break;
                    case TypeIds_Number:
                        val = JavascriptConversion::ToNumber(value, scriptContext);
                        if(_isnan(val))
                        {
                            fprintf(m_logfile, "NaN");
                        }
                        else
                        {
                            fprintf(m_logfile, "%f", val);
                        }
                        break;
                    case TypeIds_Integer:
                        fprintf(m_logfile, "%d", JavascriptConversion::ToInt32(value, scriptContext));
                        break;
                    case TypeIds_Int64Number:
                        {
                        __int64 result = JavascriptConversion::ToInt64(value, scriptContext);
                        fprintf(m_logfile, "%I64d", result);
                        break;
                        }
                    case TypeIds_UInt64Number:
                        {
                        unsigned __int64 result = JavascriptConversion::ToUInt64(value, scriptContext);
                        fprintf(m_logfile, "%I64u", result);
                        break;
                        }
                    case TypeIds_Boolean:
                        fprintf(m_logfile, "%d", JavascriptConversion::ToBoolean(value, scriptContext));
                        break;
                    case TypeIds_Function:
                        fprintf(m_logfile, "%s", "[object Function]");
                        break;
                    case TypeIds_Array:
                    case TypeIds_ES5Array:
                        fprintf(m_logfile, "%s", "[object Array]");
                        break;
                    case TypeIds_Date:
                    case TypeIds_WinRTDate:
                        fprintf(m_logfile, "%s", "[object Date]");
                        break;
                    case TypeIds_VariantDate:
                        fprintf(m_logfile, "%s", "[object VariantDate]");
                        break;
                    case TypeIds_Symbol:
                        fprintf(m_logfile, "%S", JavascriptSymbol::FromVar(value)->GetValue()->GetBuffer());
                        break;
                    case Js::TypeIds_WithScopeObject:
                        AssertMsg(false, "WithScopeObjects should not be exposed");
                        break;
                    case TypeIds_HostDispatch:
                    case TypeIds_HostObject:
                    case TypeIds_Object:
                    case TypeIds_ActivationObject:
                        fprintf(m_logfile, "%s", "[object Object]");
                        break;
                    case TypeIds_RegEx:
                        fprintf(m_logfile, "%s", "[object RegExp]");
                        break;
                    case TypeIds_Null:
                        fprintf(m_logfile, "%s", "null");
                        break;
                    case TypeIds_Undefined:
                        fprintf(m_logfile, "undefined");
                        break;
                    case TypeIds_ArrayIterator:
                        fprintf(m_logfile, "%s", "[object Array Iterator]");
                        break;
                    case TypeIds_MapIterator:
                        fprintf(m_logfile, "%s", "[object Map Iterator]");
                        break;
                    case TypeIds_SetIterator:
                        fprintf(m_logfile, "%s", "[object Set Iterator]");
                        break;
                    case TypeIds_StringIterator:
                        fprintf(m_logfile, "%s", "[object String Iterator]");
                        break;
                    case TypeIds_Generator:
                        fprintf(m_logfile, "%s", "[object Generator]");
                        break;
                    case TypeIds_Promise:
                        fprintf(m_logfile, "%s", "[object Promise]");
                        break;
                    default:
                        AssertMsg(false, "Unknown type");
                }
            }
        }

        void HostLogger::AddCachedPropertyName(void *disp, DISPID id, const wchar_t *name)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));
            m_pDispIdMap->AddItem(disp, id, name);
        }

        void HostLogger::LogInvokeCall(void *disp, DISPID id, ulong flags)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));

            const wchar_t *propName = m_pDispIdMap->LookupItem(disp, id);
            if(propName == NULL)
            {
                propName = L"UNKNOWN_NAME";
            }
            fprintf(m_logfile, "\n");
            fprintf(m_logfile, "call: %S\n", propName);
            fprintf(m_logfile, "flags: 0x%x ( ", flags);
            if(flags & DISPATCH_METHOD)
                fprintf(m_logfile,"DISPATCH_METHOD ");
            if(flags & DISPATCH_PROPERTYPUT)
                fprintf(m_logfile,"DISPATCH_PROPERTYPUT ");
            if(flags & DISPATCH_PROPERTYGET)
                fprintf(m_logfile,"DISPATCH_PROPERTYGET ");
            if(flags & DISPATCH_PROPERTYPUTREF)
                fprintf(m_logfile,"DISPATCH_PROPERTYPUTREF ");
            fprintf(m_logfile, ")\n");
        }

        void HostLogger::LogInvokeCall(void *disp, DISPID id, ulong flags, Js::Var value)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));

            LogInvokeCall(disp, id, flags);
            fprintf(m_logfile, "arg_count: 1\n");
            fprintf(m_logfile, "arg0: ");
            DumpVar(value);
        }

        void HostLogger::LogInvokeCall(void *disp, DISPID id, ulong flags, Js::Arguments args)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));

            LogInvokeCall(disp, id, flags);
            fprintf(m_logfile, "arg_count: %d\n", args.Info.Count - 1);
            for(uint i = 1; i < args.Info.Count; ++i)
            {
                fprintf(m_logfile, "arg%d: ", i - 1);
                DumpVar(args.Values[args.Info.Count - i]);
            }
        }

        void HostLogger::LogInvokeResult(Js::Var result)
        {
            Assert(Js::Configuration::Global.flags.IsEnabled(Js::HostLoggingFlag));

            fprintf(m_logfile, "result: ");
            DumpVar(result);
        }
        void HostLogger::DumpVar(Var value)
        {
            DumpType(value);
            DumpValue(value);
            fprintf(m_logfile, "\n");
        }

        void HostLogger::CloseLogFile()
        {
            if(m_logfile)
                fclose(m_logfile);
        }


        DispIdMap::DispIdMap(ThreadContext *threadContext) : threadContext(threadContext)
        {
            // create the IDispatch->DISPIDCache cache
            propertyNameCache = Anew(threadContext->GetThreadAlloc(), propertyNameCache_t,
                    threadContext->GetThreadAlloc(), 
                    17
                );
        }

        void DispIdMap::AddItem(void* pdex, DISPID id, const wchar_t* str)
        {
            if(!propertyNameCache->ContainsKey(pdex))
            {
                // if we haven't seen this IDispatch[Ex] pointer yet, create a new cache for it
                DISPIDCache_t* newCache = Anew(threadContext->GetThreadAlloc(), DISPIDCache_t,
                        threadContext->GetThreadAlloc(),
                        17
                    );
                propertyNameCache->Add(pdex, newCache);
            }

            // we don't own the property name, so make a copy to save
            wchar_t *name = (wchar_t*)threadContext->GetThreadAlloc()->Alloc((wcslen(str)+1)*sizeof(wchar_t));;
            wcscpy_s(name, wcslen(str)+1, str);

            // cache the {pdex, id, name} tuple
            propertyNameCache->Lookup(pdex, NULL)->Add(id, name);
        }

        const wchar_t* DispIdMap::LookupItem(void* pdex, DISPID id)
        {
            const wchar_t *result;

            DISPIDCache_t *nameCache = propertyNameCache->Lookup(pdex, NULL);
            if(nameCache == NULL)
            {
                result = NULL;
            }
            else
            {
                result = nameCache->Lookup(id, NULL);
            }

            return result;
        }
}

#endif