//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "stdafx.h"

namespace JsDiag
{
    WerFrame::WerFrame():
        FrameBase(NULL), ReturnAddress(NULL), InstructionPointer(NULL), StackPointer(NULL), IsInlineFrame(false),
        Uri(NULL), FunctionName(NULL), Row(0), Col(0) {}

    WerFrame::~WerFrame()
    { 
        // Use free() to release string memory. These were allocated by strdup() or malloc().
        free((void*)this->Uri);
        free((void*)this->FunctionName);
    }

    void WerFrame::Initialize(ULONG64 frameBase, ULONG64 retAddr, ULONG64 ip, ULONG64 sp, bool isInline, LPCWSTR uri, LPCWSTR func, ULONG row, ULONG col)
    {
        this->FrameBase = frameBase;
        this->ReturnAddress = retAddr;
        this->InstructionPointer = ip;
        this->StackPointer = sp;
        this->IsInlineFrame = isInline;

        this->Uri = uri;            // Take ownsership.
        this->FunctionName = func;  // Take ownsership.
        this->Row = row;
        this->Col = col;
    }

    int WerFrame::Serialize(ISerializationStream* stream, SerializationParameters* params)
    {
        int length = 0;
        length += Serializer::SerializeInteger(this->FrameBase, FRAMEBASE, stream, params);
        length += Serializer::SerializeInteger(this->ReturnAddress, RETURNADDRESS, stream, params);
        length += Serializer::SerializeInteger(this->InstructionPointer, INSTRUCTIONPOINTER, stream, params);

#ifdef _M_X64
        length += Serializer::SerializeInteger(this->StackPointer, STACKPOINTER, stream, params);
#endif
        // Note: currently for non-x64 we don't serialize StackPointer. Skip to save size.

        length += Serializer::SerializeBool(this->IsInlineFrame, ISINLINEFRAME, stream, params);
        length += Serializer::SerializeString(this->Uri, URI, stream, params);
        length += Serializer::SerializeString(this->FunctionName, FUNCTIONNAME, stream, params);
        length += Serializer::SerializeInteger(this->Row, ROW, stream, params);
        length += Serializer::SerializeInteger(this->Col, COL, stream, params);
        return length;
    }

    int WerFrame::Deserialize(FieldIndex fieldIndex, FieldType fieldType, int /* itemCount */, ISerializationStream* stream, SerializationParameters* params)
    {
        switch (fieldIndex)
        {
        case FRAMEBASE:
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->FrameBase);
        case RETURNADDRESS:
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->ReturnAddress);
        case INSTRUCTIONPOINTER: 
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->InstructionPointer);
        case STACKPOINTER: 
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->StackPointer);
        case ISINLINEFRAME:
            return Serializer::DeserializeBool(fieldType, stream, params, &this->IsInlineFrame);
        case URI:
            return Serializer::DeserializeString(stream, params, &this->Uri);
        case FUNCTIONNAME:
            return Serializer::DeserializeString(stream, params, &this->FunctionName);
        case ROW:
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->Row);
        case COL:
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->Col);
        }
        return 0;
    }

    WerStack::WerStack(ULONG threadId /*= 0*/, ULONG frameCount /*= 0*/, WerFrame* frames /*= NULL*/):
        ThreadId(threadId),
        FrameCount(frameCount),
        Frames(frames) // Take ownsership
    {}

    WerStack::~WerStack()
    {
        delete[] this->Frames;
    }

    WerStack& WerStack::operator=(WerStack& stack)
    {
        this->ThreadId = stack.ThreadId;
        this->FrameCount = stack.FrameCount;
        this->Frames = stack.Frames; // Take ownership

        stack.FrameCount = 0;
        stack.Frames = NULL;

        return *this;
    }

    int WerStack::Serialize(ISerializationStream* stream, SerializationParameters* params)
    {
        int length = 0;
        length += Serializer::SerializeInteger(this->ThreadId, THREADID, stream, params);
        length += Serializer::SerializeArrayOfNested(this->FrameCount, this->Frames, FRAMES, stream, params);
        return length;
    }

    int WerStack::Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params)
    {
        switch (fieldIndex)
        {
        case THREADID:
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->ThreadId);
        case FRAMES:
            this->FrameCount = arrayItemCount;
            this->Frames = new(oomthrow) WerFrame[arrayItemCount];
            return Serializer::DeserializeArrayOfNested<WerFrame>(stream, params, arrayItemCount, this->Frames);
        }
        return 0;
    }

    WerMessage::WerMessage(ULONG stackCount /*= 0*/, WerStack* stacks /*= NULL*/):
        MagicCookie(0),
        Version(0),
        StackCount(stackCount),
        Stacks(stacks), // Take ownsership
        ErrorHr(S_OK),
        ErrorCode(DiagErrorCode::NONE)
    {
    }

    WerMessage::WerMessage(HRESULT errorHr, DiagErrorCode errorCode) :
        MagicCookie(0),
        Version(0),
        StackCount(0),
        Stacks(nullptr),
        ErrorHr(errorHr),
        ErrorCode(errorCode)
    {
    }

    WerMessage::~WerMessage()
    {
        delete[] this->Stacks;
    }

    //
    // Validate this message contains valid magic cookie, throw otherwise. Used for deserialization validation.
    //
    void WerMessage::ValidateMagicCookie() const
    {
        if (this->MagicCookie != JS_MAGICCOOKIE)
        {
            DiagException::Throw(E_FAIL); // fail magic cookie validation, bad data stream
        }
    }

    int WerMessage::Serialize(ISerializationStream* stream, SerializationParameters* params)
    {
        // For serialization, set correct magic cookie and version explictly
        this->MagicCookie = JS_MAGICCOOKIE;
        this->Version = JS_VERSION;

        int length = 0;
        length += Serializer::SerializeInteger(this->MagicCookie, MAGICCOOKIE, stream, params); // NOTE: MagicCookie must be serialized first.
        length += Serializer::SerializeInteger(this->Version, VERSION, stream, params);

        if (this->ErrorHr != S_OK)
        {
            length += Serializer::SerializeInteger(static_cast<ULONG>(this->ErrorHr), ERRORHR, stream, params);
            length += Serializer::SerializeInteger(static_cast<ULONG>(this->ErrorCode), ERRORCODE, stream, params);
        }

        if (this->StackCount > 0)
        {
            length += Serializer::SerializeArrayOfNested(this->StackCount, this->Stacks, STACKS, stream, params);
        }

        return length;
    }

    int WerMessage::Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params)
    {
        switch (fieldIndex)
        {
        case MAGICCOOKIE:
            {
                int length = Serializer::DeserializeInteger(fieldType, stream, params, &this->MagicCookie);
                ValidateMagicCookie(); // validate magic cookie immediately
                return length;
            }

        case VERSION:
            ValidateMagicCookie(); // magic cookie must have been read
            return Serializer::DeserializeInteger(fieldType, stream, params, &this->Version);

        case STACKS:
            ValidateMagicCookie(); // Magic cookie must have been read. Validate it first to avoid reading potential wrong/expensive stack frames.
            this->StackCount = arrayItemCount;
            this->Stacks = new(oomthrow) WerStack[arrayItemCount];
            return Serializer::DeserializeArrayOfNested(stream, params, arrayItemCount, this->Stacks);

        case ERRORHR:
            ValidateMagicCookie(); // magic cookie must have been read
            {
                ULONG data;
                int length = Serializer::DeserializeInteger(fieldType, stream, params, &data);
                this->ErrorHr = static_cast<HRESULT>(data);
                return length;
            }

        case ERRORCODE:
            ValidateMagicCookie(); // magic cookie must have been read
            {
                ULONG data;
                int length = Serializer::DeserializeInteger(fieldType, stream, params, &data);
                this->ErrorCode = static_cast<DiagErrorCode>(data);
                return length;
            }
        }

        return 0;
    }
}
