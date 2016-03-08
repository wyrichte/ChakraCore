// Copyright (C) Microsoft. All rights reserved.
#include "stdafx.h"


#include "ScriptDirectUnitTests.h"

// Hack: this test doesn't use ATL. DiagAssertion.h requests to be before ATL headers, but
// it conflicts with Verifier.Assert in this project.
#undef ATLASSERT

#include "ntassert.h"
#include "DiagAssertion.h"
#include "thrownew.h"
#include "BasePtr.h"
#include "AutoPtr.h"
#include "DiagAutoPtr.h"
#include "DiagException.h"
#include "Serializer.h"
#include "Serializer.inl"

namespace JsDiag
{
    using namespace JsDiag::Serialization;
    typedef ExtensibleBinarySerializer Serializer;

    static char* c_passedOutcome = "PASSED";
    static char* c_failedOutcome = "FAILED";

    class MemoryStream : public ISerializationStream
    {
        BYTE* m_buffer;
        size_t m_length;
        size_t m_pos;

    public:
        MemoryStream(int length) : m_length(length), m_buffer(new BYTE[length]), m_pos(0) {}
        ~MemoryStream() { delete[] m_buffer; }

        // Write content of the buf to the stream.
        virtual void Write(const void* buf, size_t byteCount) override
        {
            CheckForOverrun(byteCount);
            memcpy(m_buffer + m_pos, buf, byteCount);
            m_pos += byteCount;
        }

        // Read from the stream into buf.
        virtual void Read(void* buf, size_t byteCount) override
        {
            CheckForOverrun(byteCount);
            memcpy(buf, m_buffer + m_pos, byteCount);
            m_pos += byteCount;
        }

        virtual bool IsEOF() override
        {
            return m_pos >= m_length;
        }

        void ResetPos()
        {
            m_length = m_pos;
            m_pos = 0;
        }

        // Returns hexadecimal string representing all bytes, like AB0C5D...
        // The bytes are from beginning of the stream up to, but not including, m_pos.
        // Returned value must be released by delete[].
        char* ToByteString()
        {
            // m_pos points to the position after last written byte.
            char* result = new char[2 * (m_pos + 1)];
            result[m_pos] = L'\0';

            BYTE* src = m_buffer;
            char* dst = result;
            for (unsigned int i = 0; i < m_pos; ++i)
            {
                sprintf_s(dst, 3, "%02X", *src);  // Each bytechar is 2 chars.
                ++src;
                dst += 2;
            }

            return result;
        }

    private:
        void CheckForOverrun(size_t byteCount)
        {
            if (m_pos + byteCount > m_length)
            {
                DiagException::Throw(E_INVALIDARG);
            }
        }
    }; // MemoryStream.

    // Helper class, main purpose it to isolate char16* from integers so that SerialiableOneValue can compile with both.
    template <typename T>
    struct SerializeHelper
    {
        static int Serialize(T value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            return Serializer::SerializeInteger(value, fieldIndex, stream, params);
        }
        static int Deserialize(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, T* value)
        {
            return Serializer::DeserializeInteger(fieldType, stream, params, value);
        }
        static void DisposeValue(T value) {}
    };

    template <>
    struct SerializeHelper<LPCWSTR>
    {
        static int Serialize(LPCWSTR value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            return Serializer::SerializeString(value, fieldIndex, stream, params);
        }
        static int Deserialize(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, LPCWSTR* value)
        {
            return Serializer::DeserializeString(stream, params, value);
        }
        static void DisposeValue(LPCWSTR value) { delete[] value; }
    };

    // T = byte, short, int, int64, string.
    template <typename T>
    struct SerializableOneValue : ISerializable
    {
        T m_value;
        Serialization::FieldIndex m_fieldIndex;
        SerializableOneValue(): m_fieldIndex(1) { ZeroMemory(&m_value, sizeof(m_value)); }
        SerializableOneValue(T value, Serialization::FieldIndex fieldIndex = 1): m_value(value), m_fieldIndex(fieldIndex) {}
        ~SerializableOneValue() { SerializeHelper<T>::DisposeValue(m_value); }
        virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
        {
            return SerializeHelper<T>::Serialize(m_value, m_fieldIndex, stream, params);
        }
        virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int itemCount, ISerializationStream* stream, SerializationParameters* params) override
        {
            if (fieldIndex == m_fieldIndex)
            {
                return SerializeHelper<T>::Deserialize(fieldType, stream, params, &m_value);
            }
            DiagException::Throw(E_UNEXPECTED);
        }
    };

    void TestByte()
    {
        MemoryStream stream(8);
        SerializableOneValue<BYTE> data(0xA7);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> str = stream.ToByteString();
        bool isPass1 = _stricmp("09A7", str) == 0;   // 1 byte key, type = Byte, value = A7.

        stream.ResetPos();
        SerializableOneValue<BYTE> data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = data.m_value == data1.m_value;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestByte - ") + outcome, isPass1 && isPass2);
    };

    void Test2ByteInt32()
    {
        MemoryStream stream(8);
        SerializableOneValue<int> data(0xAC00);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> str = stream.ToByteString();
        bool isPass1 = _stricmp("0A02AC00", str) == 0;   // 1 byte key, type = LDInt, value = ACBD.

        stream.ResetPos();
        SerializableOneValue<int> data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = data.m_value == data1.m_value;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("Test2ByteInt32 - ") + outcome, isPass1 && isPass2);
    };

    void TestInt64()
    {
        MemoryStream stream(16);
        SerializableOneValue<__int64> data(0xABCDEFABCFEFABCD);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> str = stream.ToByteString();
        bool isPass1 = _stricmp("0A08ABCDEFABCFEFABCD", str) == 0;  // 1 byte key, type = LDInt, value = ...

        stream.ResetPos();
        SerializableOneValue<__int64> data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = data.m_value == data1.m_value;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestInt64 - ") + outcome, isPass1 && isPass2);
    };

    // The value is small buf field index is 16 which makes it boundary case, 1st value which doesn't fit into 1 byte key
    // but resulting LDInt value fits into 1 byte. 
    void TestFieldIndexIs16()
    {
        MemoryStream stream(8);
        Serialization::FieldIndex fieldIndex = 16;
        SerializableOneValue<int> data(0xAC, fieldIndex);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> str = stream.ToByteString();
        bool isPass1 = _stricmp("8181AC", str) == 0;
        //                 hint: KLKV

        stream.ResetPos();
        SerializableOneValue<int> data1(0, fieldIndex);
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = data.m_value == data1.m_value;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestFieldIndexIs16 - ") + outcome, isPass1 && isPass2);
    }

    // The value is small buf field index is large enough so that the key doesn't fit in 1 byte and LDInt value doesn't fit into 1 byte either. 
    void TestBigFieldIndex()
    {
        MemoryStream stream(8);
        Serialization::FieldIndex fieldIndex = 32;
        SerializableOneValue<int> data(0xAC, fieldIndex);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> str = stream.ToByteString();
        bool isPass1 = _stricmp("820101AC", str) == 0;
        //                 hint: KL<KV>.. - where KL is key length, KV is key value.

        stream.ResetPos();
        SerializableOneValue<int> data1(0, fieldIndex);
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = data.m_value == data1.m_value;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestBigFieldIndex - ") + outcome, isPass1 && isPass2);
    }

    void TestString()
    {
        MemoryStream stream(32);
        LPCWSTR str = _u("hello");
        SerializableOneValue<LPCWSTR> data(_wcsdup(str));
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> serializedBytes = stream.ToByteString();
        bool isPass1 = _stricmp("0B010A680065006C006C006F00", serializedBytes) == 0;   // key is 1 byte, type = LD, value = ...

        stream.ResetPos();
        SerializableOneValue<LPCWSTR> data1(0);
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = wcscmp(data.m_value, data1.m_value) == 0;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestString - ") + outcome, isPass1 && isPass2);
    }

    void TestNested()
    {
        struct Outer : ISerializable
        {
            int m_i1;
            SerializableOneValue<int> m_inner;
            int m_i2;

            Outer(): m_i1(0), m_inner(0), m_i2(0) {}
            Outer(int i1, int i2, int innerValue): m_i1(i1), m_i2(i2), m_inner(innerValue) {}
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeInteger(m_i1, 1, stream, params);
                length += Serializer::SerializeNested(&m_inner, 2, stream, params);
                length += Serializer::SerializeInteger(m_i2, 3, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int itemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                case 1:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i1);
                case 2:
                    return Serializer::DeserializeNested(stream, params, &m_inner);
                case 3:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i2);
                default:
                    DiagException::Throw(E_UNEXPECTED);
                }
            }
        };
        MemoryStream stream(16);

        Outer data(0xBC, 0xBD, 0xBE);
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> serializedBytes = stream.ToByteString();
        bool isPass1 = _stricmp("09BC13010209BE19BD", serializedBytes) == 0;
        //                 hint: <i1>NK<NL><in><i2>- where NK is nested key, NL is nested length (LDInt)

        stream.ResetPos();
        Outer data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = 
            data.m_i1 == data1.m_i1 && 
            data.m_inner.m_value == data1.m_inner.m_value && 
            data.m_i2 == data1.m_i2;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestNested - ") + outcome, isPass1 && isPass2);
    } // TestNested.

    void TestArrayOfNested()
    {
        struct Outer : ISerializable
        {
            typedef SerializableOneValue<int> ItemType;
            int m_count;
            ItemType* m_items;
            int m_i;

            Outer(): m_count(0), m_items(NULL), m_i(0) {}
            Outer(int count, int i): m_count(count), m_i(i), m_items(NULL) {}
            ~Outer() { delete[] m_items; }
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeArrayOfNested(m_count, m_items, 1, stream, params);
                length += Serializer::SerializeInteger(m_i, 2, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                case 1:
                    if (arrayItemCount == 0)
                    {
                        DiagException::Throw(E_UNEXPECTED);
                    }
                    m_count = arrayItemCount;
                    m_items = new ItemType[arrayItemCount];
                    return Serializer::DeserializeArrayOfNested(stream, params, arrayItemCount, m_items);
                case 2:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i);
                default:
                    DiagException::Throw(E_UNEXPECTED);
                }
            }
        };

        MemoryStream stream(32);
        Outer data(2, 0xBA);
        data.m_items = new Outer::ItemType[data.m_count];
        for (int i = 0; i < data.m_count; ++i)
        {
            data.m_items[i].m_value = 0xBA + i + 1;
        }
        Serializer::Serialize(&data, &stream, NULL);
        AutoArrayPtr<char> serializedBytes = stream.ToByteString();
        bool isPass1 = _stricmp("0C010203010209BB010209BC11BA", serializedBytes) == 0;
        //                 hint: AK<IC>CK<NL>NKNV<NL>NKNV<mi> - where AK is array key, IC is item count (LDInt), CK is common key,
        //                 hint:             <item#1><item#2>   NK is nested key, NL is nested length (LDInt), mi is m_i.

        stream.ResetPos();
        Outer data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass2 = 
            data.m_count == data1.m_count && 
            data.m_items[0].m_value == data1.m_items[0].m_value && 
            data.m_items[1].m_value == data1.m_items[1].m_value && 
            data.m_i == data1.m_i;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestArrayOfNested - ") + outcome, isPass1 && isPass2);
    } // TestNestedArray.

    void TestDefaultValueInt()
    {
        MemoryStream stream(4);
        SerializableOneValue<int> data(0);
        int length = Serializer::Serialize(&data, &stream, NULL);
        bool isPass = length == 0;

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDefaultValueInt - ") + outcome, isPass);
    }

    void TestDefaultValueString()
    {
        MemoryStream stream(8);
        int length;

        // NULL and emtpry string is not serialized.
        SerializableOneValue<LPCWSTR> data1(NULL);
        length = Serializer::Serialize(&data1, &stream, NULL);
        bool isPass1 = length == 0;

        SerializableOneValue<LPCWSTR> data2(_wcsdup(_u("")));
        length = Serializer::Serialize(&data2, &stream, NULL);
        bool isPass2 = length == 0;

        char* outcome = isPass1 && isPass2 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDefaultValueString - ") + outcome, isPass1 && isPass2);
    }

    void TestDefaultValueNested()
    {
        struct Outer : ISerializable
        {
            typedef SerializableOneValue<int> Inner;
            Inner* m_inner;

            Outer(): m_inner(NULL) {}
            Outer(Inner* inner): m_inner(inner) {}
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                return Serializer::SerializeNested(m_inner, 1, stream, params);
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int itemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                return Serializer::DeserializeNested(stream, params, m_inner);
            }
        };

        MemoryStream stream(4);
        int length;

        // Inner = NULL.
        Outer data;
        length = Serializer::Serialize(&data, &stream, NULL);
        bool isPass1 = length == 0;

        // Inner has int which is 0. Serialize with length = 0.
        Outer::Inner inner(0);
        Outer data1(&inner);
        Serializer::Serialize(&data1, &stream, NULL);
        AutoArrayPtr<char> serializedBytes = stream.ToByteString();
        bool isPass2 = _stricmp("0b00", serializedBytes) == 0;

        stream.ResetPos();
        Outer data2;
        length = Serializer::Deserialize(&stream, NULL, &data2);
        bool isPass3 = length == 2 && data2.m_inner == NULL;

        char* outcome = isPass1 && isPass2 &&isPass3 ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDefalutValueNested - ") + outcome, isPass1 && isPass2);
    }

    void TestDefaultArrayOfNested()
    {
        struct Outer : ISerializable
        {
            typedef SerializableOneValue<int> ItemType;
            int m_count;
            ItemType* m_items;

            Outer(): m_count(0), m_items(NULL) {}
            Outer(int count): m_count(count), m_items(NULL) {}
            ~Outer() { delete[] m_items; }
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeInteger(m_count, 1, stream, params);
                length += Serializer::SerializeArrayOfNested(m_count, m_items, 2, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                case 1:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_count);
                case 2:
                    if (arrayItemCount == 0)
                    {
                        DiagException::Throw(E_UNEXPECTED);
                    }
                    m_items = new ItemType[arrayItemCount];
                    return Serializer::DeserializeArrayOfNested(stream, params, arrayItemCount, m_items);
                default:
                    DiagException::Throw(E_UNEXPECTED);
                }
            }
        };

        // Array with 0 items.
        MemoryStream stream(32);
        Outer data(2);
        int length = Serializer::Serialize(&data, &stream, NULL);
        bool isPass = length == 0;

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDefalutArrayOfNested - ") + outcome, isPass);
    }

    // Scenario: added field (we need to skip unrecognized field in serializer):
    // - serialized a message with field 1.
    // - deserialized message does not know about field 1.
    // Note that the "deleted field" is naturally fine, because the message just would not get called with that field.
    void TestDeletedField()
    {
        struct Outer : ISerializable
        {
            SerializableOneValue<int> m_inner;
            int m_i;

            Outer(): m_inner(0), m_i(0) {}
            Outer(int innerValue, int i): m_inner(innerValue), m_i(i) {}
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeNested(&m_inner, 1, stream, params);
                length += Serializer::SerializeInteger(m_i, 2, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                // In the version of the message being deserialised field #1 is not present, thus there is no "case 1:".
                case 2:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i);
                default:
                    return 0;
                }
            }
        };

        MemoryStream stream(16);
        Outer data(0xBC, 0xBD);
        Serializer::Serialize(&data, &stream, NULL);

        stream.ResetPos();
        Outer data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass = data.m_i == data1.m_i;

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDeletedField - ") + outcome, isPass);
    }

    void TestDeletedFieldIsArrayOfNested()
    {
        struct Outer : ISerializable
        {
            typedef SerializableOneValue<int> ItemType;
            int m_count;
            ItemType* m_items;
            int m_i;

            Outer(): m_count(0), m_items(NULL), m_i(0) {}
            Outer(int count, int i): m_count(count), m_i(i), m_items(NULL) {}
            ~Outer() { delete[] m_items; }
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeArrayOfNested(m_count, m_items, 1, stream, params);
                length += Serializer::SerializeInteger(m_i, 2, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                // In new version of the message m_count and m_items were deleted, but old client still sends these.
                case 2:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i);
                }
                return 0;
            }
        };

        MemoryStream stream(32);
        Outer data(2, 0xBA);
        data.m_items = new Outer::ItemType[data.m_count];
        for (int i = 0; i < data.m_count; ++i)
        {
            data.m_items[i].m_value = 0xBA + i + 1;
        }
        int length = Serializer::Serialize(&data, &stream, NULL);

        stream.ResetPos();
        Outer data1;
        int length1 = Serializer::Deserialize(&stream, NULL, &data1);

        bool isPass =
            length == length1 && 
            data1.m_count == 0 &&
            data1.m_items == NULL &&
            data.m_i == data1.m_i;

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDeletedFieldIsArrayOfNested - ") + outcome, isPass);
    }

    // Scenario is either:
    // - deserializing new version of message, but old serializer still writes a field which is deleted in new version.
    // - deserializer is old, and new version of message serialized a filed of new type unknown to deserializer.
    //   Note that in this case new fields are always added to the end, so if we skip the end we should be fine.
    void TestDeletedFieldIsOfUnknownType()
    {
        struct Outer : ISerializable
        {
            struct Inner : ISerializable
            {
                virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
                {
                    int length = 0;
                    BYTE key = 0x0D;    // FieldType(5) is not there yet => make it 0000 1101.
                    if (stream) stream->Write(&key, sizeof(key));
                    length += sizeof(key);

                    const int bufSize = 3;
                    BYTE buf[bufSize];
                    memset(buf, 0xAA, bufSize);
                    if (stream) stream->Write(buf, bufSize);
                    length += bufSize;

                    return length;
                }
                virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
                {
                    return 0;
                }
            };
            Inner m_inner;
            int m_i;

            Outer(): m_i(0) {}
            Outer(int i): m_i(i) {}
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeNested(&m_inner, 1, stream, params);
                length += Serializer::SerializeInteger(m_i, 2, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                // In the version of the message being deserialised field #1 is not present, thus there is no "case 1:".
                case 2:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &m_i);
                default:
                    return 0;
                }
            }
        };

        MemoryStream stream(16);
        Outer data(0xBC);
        Serializer::Serialize(&data, &stream, NULL);

        stream.ResetPos();
        Outer data1;
        Serializer::Deserialize(&stream, NULL, &data1);
        bool isPass = data.m_i == data1.m_i;

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestDeletedFieldIsOfUnknownType - ") + outcome, isPass);
    }

    void TestTypicalWerCase()
    {
        struct WerFrame : public ISerializable
        {
            __int64 EffectiveFrameBase;
            __int64 InstructionPointer;
            __int64 ReturnAddress;
            int Row;
            int Col;
            LPCWSTR FunctionName;
            LPCWSTR Uri;
            bool IsInlineFrame;

            WerFrame()  : EffectiveFrameBase(NULL), InstructionPointer(NULL), ReturnAddress(NULL),
                Row(0), Col(0), FunctionName(NULL), Uri(NULL), IsInlineFrame(0) {}
            ~WerFrame() 
            { 
                free((void*)this->Uri);
                free((void*)this->FunctionName);
            }
            void Initialize(__int64 effBP, __int64 ip, __int64 retAddr, int row, int col, char16* func, char16* uri, bool isInline)
            {
                this->EffectiveFrameBase = effBP; 
                this->InstructionPointer = ip;
                this->ReturnAddress = retAddr;
                this->Row = row;
                this->Col = col;
                this->FunctionName = func;  // Take ownsership.
                this->Uri = uri;            // Take ownsership.
                this->IsInlineFrame = isInline;
            }
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeInteger(this->EffectiveFrameBase, 1, stream, params);
                length += Serializer::SerializeInteger(this->InstructionPointer, 2, stream, params);
                length += Serializer::SerializeInteger(this->ReturnAddress, 3, stream, params);
                length += Serializer::SerializeInteger(this->Row, 4, stream, params);
                length += Serializer::SerializeInteger(this->Col, 5, stream, params);
                length += Serializer::SerializeString(this->FunctionName, 6, stream, params);
                length += Serializer::SerializeString(this->Uri, 7, stream, params);
                length += Serializer::SerializeInteger(static_cast<BYTE>(this->IsInlineFrame), 8, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int itemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                case 1:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->EffectiveFrameBase);
                case 2: 
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->InstructionPointer);
                case 3:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->ReturnAddress);
                case 4:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->Row);
                case 5:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->Col);
                case 6:
                    return Serializer::DeserializeString(stream, params, &this->FunctionName);
                case 7:
                    return Serializer::DeserializeString(stream, params, &this->Uri);
                case 8:
                    {
                        BYTE tmpValue;
                        int byteCount = Serializer::DeserializeInteger(fieldType, stream, params, &tmpValue);
                        this->IsInlineFrame = tmpValue != 0;
                        return byteCount;
                    }
                }
                return 0;
            }
            bool Equals(const WerFrame& other)
            {
                return 
                    this->EffectiveFrameBase == other.EffectiveFrameBase &&
                    this->InstructionPointer == other.InstructionPointer &&
                    this->ReturnAddress == other.ReturnAddress &&
                    this->Row == other.Row &&
                    this->Col == other.Col &&
                    wcscmp(this->FunctionName, other.FunctionName) == 0 &&
                    wcscmp(this->Uri, other.Uri) == 0 &&
                    this->IsInlineFrame == other.IsInlineFrame;
            }
        };

        struct Message : public ISerializable
        {
            unsigned short MagicCookie;
            LPCWSTR Version;
            int FrameCount;
            WerFrame* Frames;   // Consequitive array of frames.

            Message() : MagicCookie(0xADBE), Version(NULL), Frames(NULL) {}
            Message(unsigned short magic, char16* version, int frameCount, WerFrame* frames) : 
                MagicCookie(magic), Version(version), FrameCount(frameCount), Frames(frames) {}
            ~Message() 
            { 
                delete[] this->Frames; 
                free((void*)this->Version);
            }
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) override
            {
                int length = 0;
                length += Serializer::SerializeInteger(this->MagicCookie, 1, stream, params);
                length += Serializer::SerializeString(this->Version, 2, stream, params);
                length += Serializer::SerializeArrayOfNested(this->FrameCount, this->Frames, 3, stream, params);
                return length;
            }
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) override
            {
                switch (fieldIndex)
                {
                case 1:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->MagicCookie);
                case 2:
                    return Serializer::DeserializeString(stream, params, &this->Version);
                case 3:
                    this->FrameCount = arrayItemCount;
                    this->Frames = new WerFrame[arrayItemCount];
                    return Serializer::DeserializeArrayOfNested<WerFrame>(stream, params, arrayItemCount, this->Frames);
                case 4:
                    return Serializer::DeserializeInteger(fieldType, stream, params, &this->FrameCount);
                }
                return 0;
            }
            bool Equals(const Message& other)
            {
                return
                    this->MagicCookie == other.MagicCookie &&
                    this->FrameCount == other.FrameCount &&
                    this->Frames != NULL && this->Frames->Equals(*other.Frames);
            }
        };

        MemoryStream stream(256);
        int frameCount = 2;
        WerFrame* frames = new WerFrame[frameCount];
        frames[0].Initialize(0xACBDCF, 0xACBDC0, 0xACBDC1, 256, 5, _wcsdup(_u("foo")), _wcsdup(_u("http://some")), false);
        frames[1].Initialize(0xCBDCAF, 0xCBDCA0, 0, 2048, 1, _wcsdup(_u("bar")), _wcsdup(_u("http://another")), true);
        Message message(0xACBE, _wcsdup(_u("1.0")), 2, frames);
        Serializer::Serialize(&message, &stream, NULL);

        stream.ResetPos();
        Message message1;
        Serializer::Deserialize(&stream, NULL, &message1);
        bool isPass = message1.Equals(message);

        char* outcome = isPass ? c_passedOutcome : c_failedOutcome;
        Print(std::string("TestTypicalWerCase - ") + outcome, isPass);
    }

} // namespace JsDiag.

using namespace JsDiag;

void RunScriptDiagSerializationTest() 
{
    try
    {
        TestByte();
        Test2ByteInt32();
        TestInt64();
        TestFieldIndexIs16();
        TestBigFieldIndex();
        TestString();
        TestNested();
        TestArrayOfNested();
        TestDefaultValueInt();
        TestDefaultValueString();
        TestDefaultValueNested();
        TestDeletedField();
        TestDeletedFieldIsArrayOfNested();
        TestDeletedFieldIsOfUnknownType();
        TestTypicalWerCase();
    }
    catch (const DiagException& ex)
    {
        std::ostringstream oss;
        oss << "DiagException was thrown: HR=";
        oss << hex << ex.hr;
        Print(oss.str(), 0);
    }
    catch (...)
    {
        Print(std::string("Unknown was thrown!"), 0);
    }
}

