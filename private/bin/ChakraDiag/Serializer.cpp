//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#include "StdAfx.h"

// TODO: Clean this warning up
#pragma warning(disable:4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data

namespace JsDiag
{
    namespace Serialization
    {
        int ExtensibleBinarySerializer::Serialize(ISerializable* data, ISerializationStream* stream, SerializationParameters* params)
        {
            return data->Serialize(stream, params);
        }

        int ExtensibleBinarySerializer::SerializeBool(bool value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            return SerializeInteger<BYTE>(value ? 1 : 0, fieldIndex, stream, params);
        }

        int ExtensibleBinarySerializer::SerializeString(LPCWSTR value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            // Don't serialize if string is NULL or empty.
            // The idea is to save space and be consistent with scenario of missing field (which gets default value during deserialization).
            if (value)
            {
                // Serialize:
                // - key
                // - length in bytes
                // - all chars exlcuding NULL terminator.
                size_t strByteCount = wcslen(value) * sizeof(char16);
                if (strByteCount)
                {
                    int length = SerializeKey(FieldType::LengthDefined, fieldIndex, stream, params);
                    length += SerializeLengthHelper(strByteCount, stream, params);

                    if (stream)
                    {
                        stream->Write(value, strByteCount);    // We don't write the NULL terminator.
                    }
                    length += strByteCount;

                    return length;
                }
            }

            return 0;
        }

        // Serialize as length-delimited:
        // - key
        // - length of the value (LDInt)
        // - value
        int ExtensibleBinarySerializer::SerializeNested(ISerializable* value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            // Don't serialize NULL value but don't check what's inside for perf.
            // It's valid that nested length could be 0.
            if (value)
            {
                int length = 0;
                length += SerializeKey(FieldType::LengthDefined, fieldIndex, stream, params);
                length += SerializeNestedLengthAndValueHelper(value, stream, params);
                return length;
            }

            return 0;
        }

        int ExtensibleBinarySerializer::SerializeLDIntLengthHelper(BYTE valueLength, ISerializationStream* stream, SerializationParameters* /* params */)
        {
            // Note that if get got here and valueLength == 0 we still need to put this zero to the stream, 
            // as e.g. ew may be serializing length of nested (as LDInt).
            if (stream)
            {
                stream->Write(&valueLength, sizeof(BYTE));
            }
            return sizeof(BYTE);
        }

        // Helper to serialize the value of nested.
        // Serialize:
        // - value length in bytes
        // - value
        int ExtensibleBinarySerializer::SerializeNestedLengthAndValueHelper(ISerializable* value, ISerializationStream* stream, SerializationParameters* params)
        {
            int nestedLength = value->Serialize(nullptr, params);
            int length = SerializeLengthHelper(nestedLength, stream, params);
            if (length > 0)
            {
                length += value->Serialize(stream, params);
            }
            return length;
        }

        // Serialize the key.
	    // - The key can be one of:
	    //	 0....XXX -- (optimization for small keys) one byte (< 16 fields). XXX is for data type.
	    //	 1.......|........ ... -- LDInt: 1st byte except reserved high bit is key length, the rest is actual key value.
        int ExtensibleBinarySerializer::SerializeKey(FieldType fieldType, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            AssertMsg(fieldIndex == (fieldIndex & 0x01FF), "In the serializer we don't currently support keys longer than 2 bytes (currently FieldIndex is BYTE, so we are fine).");
            if (fieldIndex == (fieldIndex & 0x0F))
            {
                // Optimization: the key fits into just 1 byte.
                BYTE key = static_cast<BYTE>((fieldIndex << 3) | fieldType);
                if (stream)
                {
                    stream->Write(&key, sizeof(key));
                }
                return sizeof(key);
            }
            else
            {
                short key = static_cast<short>((fieldIndex << 3) | fieldType);
                int length = SerializeLDInt(key, stream, params, 0x80);
                return length;
            }
        }

        int ExtensibleBinarySerializer::Deserialize(ISerializationStream* stream, SerializationParameters* params, ISerializable* value)
        {
            return DeserializeStructHelper(stream, params, Length_MAX, value);
        }

        int ExtensibleBinarySerializer::DeserializeNested(ISerializationStream* stream, SerializationParameters* params, ISerializable* value)
        {
            Length length;
            int byteCount = DeserializeLengthHelper(stream, params, &length);
            // Note: length could be 0, that's valid case.

            byteCount += DeserializeStructHelper(stream, params, length, (ISerializable*)value);
            return byteCount;
        }

        //
        // Deserialize a struct.
        // Note: state of reader when we enter here: just passed over the field key of the whole struct.
        //
        int ExtensibleBinarySerializer::DeserializeStructHelper(ISerializationStream* stream, SerializationParameters* params, Length length, ISerializable* value)
        {
            Assert(stream);
            // Note: it is valid case when length == 0, we serialize the length but just don't deserialize anything here.

            unsigned int byteCount = 0;
            while (byteCount < length && !stream->IsEOF())
            {
                FieldType fieldType;
                FieldIndex fieldIndex;
                byteCount += DeserializeKey(stream, params, &fieldType, &fieldIndex);
                Length arrayItemCount = 0;
                if (fieldType == FieldType::ItemCountDefined)
                {
                    byteCount += DeserializeLengthHelper(stream, params, &arrayItemCount);
                }

                int remainingStructBytes = length == Length_MAX ? Length_MAX : length - byteCount;
                int tmpByteCount = DeserializeStructValueHelper(value, fieldIndex, fieldType, arrayItemCount, remainingStructBytes, stream, params);
                if (tmpByteCount == 0)
                {
                    break;
                }
                byteCount += tmpByteCount;
            }
            AssertMsg(byteCount == length || stream->IsEOF() || length == Length_MAX, 
                "Deserialized byte count doesn't match the nested type length.");

            return byteCount;
        }

        int ExtensibleBinarySerializer::DeserializeKey(ISerializationStream* stream, SerializationParameters* params, 
            FieldType* fieldType, FieldIndex* fieldIndex)
        {
            Assert(fieldType);
            Assert(fieldIndex);

            int length = 0;
            BYTE firstByte;
            stream->Read(&firstByte, sizeof(BYTE));
            length += 1;

            if ((firstByte & 0x80) == 0)
            {
                // The key is 1 byte.
                *fieldType = static_cast<FieldType>(firstByte & 0x07);
                *fieldIndex = firstByte >> 3;
            }
            else
            {
                // The key is LDInt, with firstByte being the length in bytes, which has reserved high bit set to 1.
                firstByte &= 0x7F;
                short value;
                length += DeserializeLDIntValue(firstByte, stream, params, &value);
                *fieldType = static_cast<FieldType>(value & 0x07);
                *fieldIndex = static_cast<FieldIndex>(value >> 3);
            }

            return length;
        }
                
        int ExtensibleBinarySerializer::DeserializeBool(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, bool* value)
        {
            BYTE tmpValue;
            int byteCount = DeserializeInteger(fieldType, stream, params, &tmpValue);
            *value = tmpValue != 0;
            return byteCount;
        }

        int ExtensibleBinarySerializer::DeserializeString(ISerializationStream* stream, SerializationParameters* params, LPCWSTR* value)
        {
            Length strByteCount;
            int byteCount = DeserializeLengthHelper(stream, params, &strByteCount);
            AssertMsg(strByteCount % 2 == 0, "The string must've been serialized with even # of bytes since it's UNICODE string.");
            int strLength = strByteCount / 2;
            
            char16* str = (char16*)malloc(sizeof(char16) * (strLength + 1)); // Extra 1 char is for NULL-terminator. Use malloc/free for strings.
            stream->Read(str, strLength * sizeof(char16));
            str[strLength] = _u('\0');

            byteCount += strByteCount;
            *value = str;

            return byteCount;
        }

        int ExtensibleBinarySerializer::DeserializeStructValueHelper(
            ISerializable* serializable, FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, int remainingStructBytes, ISerializationStream* stream, SerializationParameters* params)
        {
            int byteCount = serializable->Deserialize(fieldIndex, fieldType, arrayItemCount, stream, params);
            if (byteCount == 0)
            {
                byteCount = DeserializeSkippedField(fieldType, arrayItemCount, stream, params);
                if (byteCount == 0)
                {
                    // Unknown field type which must've been serialized by v.next serializer.
                    // Since we always add new fields to the end of the type, it is safe to skip to just beyond the type.
                    if (remainingStructBytes != Length_MAX)
                    {
                        // Advance stream by the remaining bytes, just beyond nested type.
                        AutoArrayPtr<BYTE> buf = new(oomthrow) BYTE[remainingStructBytes];
                        stream->Read(buf, remainingStructBytes);
                        byteCount += remainingStructBytes;
                    }
                    // Else we are are deserialising top level type, just stop.
                }
            }
            return byteCount;
        }

        int ExtensibleBinarySerializer::SerializeLengthHelper(int lengthInBytes, ISerializationStream* stream, SerializationParameters* params)
        {
            Assert(lengthInBytes >= 0);
            Length byteCount = static_cast<Length>(lengthInBytes);            
            return SerializeLDInt(byteCount, stream, params);
        }

        int ExtensibleBinarySerializer::DeserializeLengthHelper(ISerializationStream* stream, SerializationParameters* params, Length* value)
        {
            return DeserializeInteger<Length>(FieldType::LDInt, stream, params, value);
        }

        // Restore the stream for skipped fields.
        // Returns: the number of bytes deserialized (and advances the stream by this number) or 0 if the field is unknown.
        // A field can be skipped due to the following reasons:
        // 1) deserializer is old (v1), message is newer (v2) but message type is old, though in newer message the field was deleted.
        //    In this case since the field type is known, we just skip it. 
        // 2) unknown field type: deserilizer is old, field type in the message was introduced later.
        //    In this case since we always introduce new fields to the end of the message, there would be no known fields,
        //    so what we do is just skip to the end of the type.
        // Advance the stream and return the number of bytes advanced by. Ignore the value in the stream.
        //static
        int ExtensibleBinarySerializer::DeserializeSkippedField(FieldType skippedFieldType, int arrayItemCount, ISerializationStream* stream,  SerializationParameters* params)
        {
            int byteCount = 0;
            int advanceStreamByCount = 0;
            switch (skippedFieldType)
            {
            case FieldType::Byte:
                byteCount = advanceStreamByCount = sizeof(BYTE);
                break;

            case FieldType::LDInt:
                byteCount = advanceStreamByCount = sizeof(int);
                break;

            case FieldType::LengthDefined:
                Length length;
                byteCount += DeserializeLengthHelper(stream, params, &length);
                byteCount += length;
                advanceStreamByCount += length;
                break;

            case FieldType::ItemCountDefined:
                {
                    int allItemsByteCount = 0;

                    FieldType fieldType;
                    FieldIndex fieldIndex;
                    allItemsByteCount += DeserializeKey(stream, params, &fieldType, &fieldIndex);
                    Length nestedArrayItemCount = 0;
                    if (fieldType == FieldType::ItemCountDefined)
                    {
                        byteCount += DeserializeLengthHelper(stream, params, &nestedArrayItemCount);
                    }

                    // For each item/value, get the length and advance the stream.
                    bool areAllRecognized = true;
                    for (int i = 0; i < arrayItemCount; ++i)
                    {
                        int itemByteCount = DeserializeSkippedField(fieldType, nestedArrayItemCount, stream, params);
                        if (itemByteCount == 0)
                        {
                            areAllRecognized = false;
                            break;
                        }
                        allItemsByteCount += itemByteCount;
                    }

                    if (areAllRecognized)
                    {
                        byteCount += allItemsByteCount;
                    }
                    break;
                }
            }

            if (advanceStreamByCount != 0)
            {
                AutoArrayPtr<BYTE> buf = new(oomthrow) BYTE[advanceStreamByCount];
                stream->Read(buf, advanceStreamByCount);
            }
            return byteCount;
        }

    } // namespace Serialization.
}
