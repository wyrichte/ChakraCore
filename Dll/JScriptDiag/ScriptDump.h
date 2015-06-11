//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

namespace JsDiag
{
    using namespace Serialization;
    typedef ExtensibleBinarySerializer Serializer;

    //
    // Represents a serializable JS stack frame. Used to serialize JS stack frame info into dump stream (e.g. by WER dump writer),
    // and later to deserialize from dump stream to reconstruct stack info (e.g. by debugger).
    //
    struct WerFrame: public ISerializable
    {
        ULONG64 FrameBase;
        ULONG64 ReturnAddress;
        ULONG64 InstructionPointer;
        ULONG64 StackPointer;
        bool IsInlineFrame;

        LPCWSTR Uri;
        LPCWSTR FunctionName;
        ULONG Row;
        ULONG Col;

        // NOTE: For compatibility, only add new field index at the end. See comment of ExtensibleBinarySerializer.
        enum Fields { NONE = 0, FRAMEBASE, RETURNADDRESS, INSTRUCTIONPOINTER, STACKPOINTER, ISINLINEFRAME, URI, FUNCTIONNAME, ROW, COL };

        WerFrame();
        ~WerFrame();
        void Initialize(ULONG64 frameBase, ULONG64 retAddr, ULONG64 ip, ULONG64 sp, bool isInline, LPCWSTR uri, LPCWSTR func, ULONG row, ULONG col);

        virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override;
        virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int itemCount, ISerializationStream* stream, SerializationParameters* params) override;

    private:
        WerFrame(const WerFrame&); // disable
        const WerFrame& operator=(const WerFrame&); // disable
    };

    //
    // Represents a serializable JS thread stack. Contains all JS stack frames of the thread.
    //
    struct WerStack: public ISerializable
    {
        ULONG ThreadId;
        ULONG FrameCount;
        WerFrame* Frames;   // Consequitive array of frames.

        // NOTE: For compatibility, only add new field index at the end. See comment of ExtensibleBinarySerializer.
        enum Fields { NONE = 0, THREADID, FRAMES };

        WerStack(ULONG threadId = 0, ULONG frameCount = 0, WerFrame* frames = NULL);
        ~WerStack();

        // WARNING: This assignment operator passes contained WerFrames. Source WerStack is used as temporary WerFrame item holder.
        WerStack& operator=(WerStack& stack);

        virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override;
        virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override;

    private:
        WerStack(const WerStack&); // disable
    };

    //
    // The root serializable object for JS stack info. This is serialized to a user dump stream and later deserialized to root object by dump reader.
    // Contains one or more JS thread stacks.
    //
    struct WerMessage : public ISerializable
    {
        static const USHORT JS_MAGICCOOKIE = 0xADBE;
        static const BYTE   JS_VERSION_MAJOR = 1;
        static const BYTE   JS_VERSION_MINOR = 0;
        static const USHORT JS_VERSION = MAKEWORD(JS_VERSION_MINOR, JS_VERSION_MAJOR); // HIBYTE: major, LOBYTE: minor

        USHORT MagicCookie;
        USHORT Version;
        ULONG StackCount;
        WerStack* Stacks;   // Consequitive array of stacks.

        // A WerMessage might alternatively contain internal error info to indicate what happened during dump capture time.
        HRESULT ErrorHr;
        DiagErrorCode ErrorCode;

        // NOTE: For compatibility, only add new field index at the end. See comment of ExtensibleBinarySerializer.
        enum Fields { NONE = 0, MAGICCOOKIE, VERSION, STACKS, ERRORHR, ERRORCODE };

        WerMessage(ULONG stackCount = 0, WerStack* stacks = NULL);
        WerMessage(HRESULT errorHr, DiagErrorCode errorCode);
        ~WerMessage();

        void ValidateMagicCookie() const;
        virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override;
        virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override;

    private:
        WerMessage(const WerMessage&); // disable
        const WerMessage& operator=(const WerMessage&); // disable
    };

    //
    // A list helper which owns contained pointers to objects.
    //
    template <class T>
    class AutoList
    {
    private:
        CAtlArray<T*> m_items;

    public:
        ~AutoList()
        {
            Clear();
        }

        void Add(T* item)
        {
            m_items.Add(item);
        }

        size_t GetCount() const
        {
            return m_items.GetCount();
        }

        bool IsEmpty() const
        {
            return m_items.IsEmpty();
        }

        void Clear()
        {
            Map([this](size_t i, T* item)
            {
                m_items.RemoveAt(i); // Make sure we won't delete it again
                delete item;
            });

            m_items.RemoveAll();
        }

        template <class Func>
        void Map(Func f)
        {
            for (size_t i = 0; i < GetCount(); i++)
            {
                f(i, m_items[i]);
            }
        }
    };

}
