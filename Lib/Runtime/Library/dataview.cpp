//----------------------------------------------------------------------------
// Copyright (C) by Microsoft Corporation.  All rights reserved.
// 
//  Implements typed array. 
//----------------------------------------------------------------------------
#include "stdafx.h"

namespace Js
{

    Var DataView::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        AssertMsg(args.Info.Count > 0, "Should always have implicit 'this'");

        Assert(!(callInfo.Flags & CallFlags_New) || args[0] == null);
        uint32 byteLength = 0;
        int32 mappedLength;
        int32 offset = 0;
        double numberOffset = 0;
        ArrayBuffer* arrayBuffer = NULL;
        DataView* dataView;

        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"arrayBuffer");
        }

#ifdef ENABLE_PROJECTION
        // Currently the only reason we check for an external object is projection related, so it remains under conditional compilation.
        RecyclableObject* jsArraySource = NULL;
        if (JavascriptOperators::IsObject(args[1]) && JavascriptConversion::ToObject(args[1], scriptContext, &jsArraySource))
        {
            JavascriptLibrary::ArrayBufferFromExternalObject pfArrayBufferFromExternalObject = jsArraySource->GetLibrary()->getpfArrayBufferFromExternalObject();
            // If the function pointer is null then Projection has not been initialized yet and we are not interested in this object.
            if (pfArrayBufferFromExternalObject != nullptr && jsArraySource->IsExternal())
            {
                OUTPUT_TRACE(TypedArrayPhase, L"Suspected Projection ArrayBuffer source - attempting to get buffer\n");
                // Check for potential external object ArrayBuffer
                HRESULT hr = (*pfArrayBufferFromExternalObject)(jsArraySource, &arrayBuffer);
                switch (hr)
                {
                case S_OK:
                case S_FALSE:
                    // Both of these cases will be handled by the arrayBuffer null check.
                    break;

                default:
                    // Any FAILURE HRESULT or unexpected HRESULT.
                    JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_InvalidArugment, L"arrayBuffer");
                    break;
                }
            }
        }
#endif
        //2.    If Type(buffer) is not Object, throw a TypeError exception.
        //3.    If buffer does not have an [[ArrayBufferData]] internal slot, throw a TypeError exception.
        if (arrayBuffer == nullptr)
        {
            if (ArrayBuffer::Is(args[1]))
            {
                arrayBuffer = ArrayBuffer::FromVar(args[1]);
            }
            else
            {
                JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"arrayBuffer");
            }
        }

        //4.    Let numberOffset be ToNumber(byteOffset).
        //5.    Let offset be ToInteger(numberOffset).
        //6.    ReturnIfAbrupt(offset).
        //7.    If numberOffset <> offset or offset < 0, throw a RangeError exception.
        if (args.Info.Count > 2)
        {
            Var secondArgument = args[2];
            numberOffset = JavascriptConversion::ToNumber(secondArgument, scriptContext);
            offset = JavascriptConversion::ToInt32(numberOffset, scriptContext);

            if (offset < 0 || 
                numberOffset != offset)
            {
                JavascriptError::ThrowRangeError(
                    scriptContext, JSERR_DataView_InvalidArugment, L"offset");
            }
        }

        //8.    If IsDetachedBuffer(buffer) is true, throw a TypeError exception.
        if (arrayBuffer->IsDetached())
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }

        //9.    Let bufferByteLength be the value of buffer's[[ArrayBufferByteLength]] internal slot.
        //10.   If offset > bufferByteLength, throw a RangeError exception.
       
        byteLength = arrayBuffer->GetByteLength();
        if ((uint32)offset > byteLength)
        {
            JavascriptError::ThrowRangeError(
                scriptContext, JSERR_DataView_InvalidArugment, L"offset");
        }

        //11.   If byteLength is undefined, then
        //      a.  Let viewByteLength be bufferByteLength - offset.
        //12.   Else,
        //      a.  Let viewByteLength be ToLength(byteLength).
        //      b.  ReturnIfAbrupt(viewLength).
        //      c.  If offset + viewByteLength > bufferByteLength, throw a RangeError exception.
        if (args.Info.Count > 3)
            {
                Var thirdArgument = args[3];
                // TODO: Change length of ArrayObject from uint32 to uint64?
                mappedLength = (int32)JavascriptConversion::ToLength(thirdArgument, scriptContext);
                AssertMsg(mappedLength >= 0, "Mapped Length of dataview should never be less than 0.");

                // mappedLength and offset will always be >= 0, so no need to typecast to uint64
                if ((uint32)(mappedLength + offset) > byteLength)
                {
                    JavascriptError::ThrowRangeError(
                        scriptContext, JSERR_DataView_InvalidArugment, L"length");
                }
            }
        else
        {
            mappedLength = byteLength - offset;
        }

        //13.   Let O be OrdinaryCreateFromConstructor(NewTarget, "%DataViewPrototype%", [[DataView]], [[ViewedArrayBuffer]], [[ByteLength]], [[ByteOffset]]).
        //14.   ReturnIfAbrupt(O).
        //15.   Set O's[[DataView]] internal slot to true.
        //16.   Set O's[[ViewedArrayBuffer]] internal slot to buffer.
        //17.   Set O's[[ByteLength]] internal slot to viewByteLength.
        //18.   Set O's[[ByteOffset]] internal slot to offset.
        //19.   Return O.
        dataView = scriptContext->GetLibrary()->CreateDataView(arrayBuffer, offset, mappedLength);
        return dataView;
    }

    DataView::DataView(ArrayBuffer* arrayBuffer, uint32 byteoffset, uint32 mappedLength, DynamicType* type)
        : ArrayBufferParent(type, mappedLength, arrayBuffer),
          byteOffset(byteoffset)
    {
        buffer = arrayBuffer->GetBuffer() + byteoffset;
    }

    BOOL DataView::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_DataView;
    }

    Var DataView::EntryGetInt8(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"offset");
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<int8>(offset, L"DataView.prototype.GetInt8", FALSE);
    }

    Var DataView::EntryGetUint8(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset");
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<uint8>(offset, L"DataView.prototype.GetUint8", FALSE);
    }

    Var DataView::EntryGetInt16(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<int16>(offset, L"DataView.prototype.GetInt16", isLittleEndian);
    }

    Var DataView::EntryGetUint16(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<uint16>(offset, L"DataView.prototype.GetUint16", isLittleEndian);
    }

    Var DataView::EntryGetUint32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<uint32>(offset, L"DataView.prototype.GetUint32", isLittleEndian);
    }

    Var DataView::EntryGetInt32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValue<int32>(offset, L"DataView.prototype.GetInt32", isLittleEndian);
    }

    Var DataView::EntryGetFloat32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValueWithCheck<float>(offset, L"DataView.prototype.GetFloat32", isLittleEndian);
    }

    Var DataView::EntryGetFloat64(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 2)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset");
        }
        if (args.Info.Count > 2)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[2], scriptContext);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        return dataView->GetValueWithCheck<double>(offset, L"DataView.prototype.GetFloat64", isLittleEndian);
    }

    Var DataView::EntrySetInt8(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        int8 value = JavascriptConversion::ToInt8(args[2], scriptContext);
        dataView->SetValue<int8>(offset, value, L"DataView.prototype.SetInt8");
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetUint8(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        uint8 value = JavascriptConversion::ToUInt8(args[2], scriptContext);
        dataView->SetValue<uint8>(offset, value, L"DataView.prototype.SetUint8");
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetInt16(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        int16 value = JavascriptConversion::ToInt16(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<int16>(offset, value, L"DataView.prototype.SetInt16", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetUint16(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument, L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        uint16 value = JavascriptConversion::ToUInt16(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<uint16>(offset, value, L"DataView.prototype.SetUint16", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetInt32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        int32 value = JavascriptConversion::ToInt32(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<int32>(offset, value, L"DataView.prototype.SetInt32", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetUint32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        uint32 value = JavascriptConversion::ToUInt32(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<uint32>(offset, value, L"DataView.prototype.SetUint32", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetFloat32(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView,  L"offset or value");
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument);
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        float value = JavascriptConversion::ToFloat(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<float>(offset, value, L"DataView.prototype.SetFloat32", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntrySetFloat64(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();
        BOOL isLittleEndian = FALSE;

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }
        if (args.Info.Count < 3)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_DataView_NeedArgument,  L"offset or value");
        }
        DataView* dataView = DataView::FromVar(args[0]);
        uint32 offset = JavascriptConversion::ToUInt32(args[1], scriptContext);
        double value = JavascriptConversion::ToNumber(args[2], scriptContext);
        if (args.Info.Count > 3)
        {
            isLittleEndian = JavascriptConversion::ToBoolean(args[3], scriptContext);
        }
        dataView->SetValue<double>(offset, value, L"DataView.prototype.SetFloat64", isLittleEndian);
        return scriptContext->GetLibrary()->GetUndefined();
    }

    Var DataView::EntryGetterBuffer(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        ArrayBuffer* arrayBuffer = dataView->GetArrayBuffer();

        if (arrayBuffer == nullptr)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayBufferObject);
        }

        return arrayBuffer;
    }

    Var DataView::EntryGetterByteLength(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        ArrayBuffer* arrayBuffer = dataView->GetArrayBuffer();

        if (arrayBuffer == nullptr)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayBufferObject);
        }
        else if (arrayBuffer->IsDetached())
        {
            return TaggedInt::ToVarUnchecked(0);
        }

        return JavascriptNumber::ToVar(dataView->GetLength(), scriptContext);
    }

    Var DataView::EntryGetterByteOffset(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        Assert(!(callInfo.Flags & CallFlags_New)); 

        if (args.Info.Count == 0 || !DataView::Is(args[0]))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_This_NeedDataView);
        }

        DataView* dataView = DataView::FromVar(args[0]);
        ArrayBuffer* arrayBuffer = dataView->GetArrayBuffer();

        if (arrayBuffer == nullptr)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedArrayBufferObject);
        } 
        else if (arrayBuffer->IsDetached())
        {
            return TaggedInt::ToVarUnchecked(0);
        }

        return JavascriptNumber::ToVar(dataView->GetByteOffset(), scriptContext);
    }

    BOOL DataView::SetItemWithAttributes(uint32 index, Var value, PropertyAttributes attributes)
    {
        AssertMsg(false, "We don't need a DataView to serve as object's internal array");
        return FALSE;
    }

#ifdef _M_ARM
        // Provide template specilization (only) for memory access at unaligned float/double address which causes data alignment exception otherwise.
        template<>
        Var DataView::GetValueWithCheck<float>(uint32 byteOffset, wchar_t *funcName, BOOL isLittleEndian = FALSE)
        {
            return this->GetValueWithCheck<float, float UNALIGNED*>(byteOffset, isLittleEndian, funcName);
        }

        template<>
        Var DataView::GetValueWithCheck<double>(uint32 byteOffset, wchar_t *funcName, BOOL isLittleEndian = FALSE)
        {
            return this->GetValueWithCheck<double, double UNALIGNED*>(byteOffset, isLittleEndian, funcName);
        }

        template<>
        void DataView::SetValue<float>(uint32 byteOffset, float value, wchar_t *funcName, BOOL isLittleEndian = FALSE)
        {
            this->SetValue<float, float UNALIGNED*>(byteOffset, value, isLittleEndian, funcName);
        }

        template<>
        void DataView::SetValue<double>(uint32 byteOffset, double value, wchar_t *funcName, BOOL isLittleEndian = FALSE)
        {
            this->SetValue<double, double UNALIGNED*>(byteOffset, value, isLittleEndian, funcName);
        }
#endif

}