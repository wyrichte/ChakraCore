//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
    namespace Serialization
    {
        template <typename T> 
        int ExtensibleBinarySerializer::SerializeInteger(T value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            if (value)
            {
                int length = 0;
                if ((value & 0xFF) == value)
                {
                    // Optimization: 1-byte values are serialized as FieldType::Byte,
                    // but note that the key still can be LDInt if fieldIndex is fairly large.
                    length += SerializeKey(FieldType::Byte, fieldIndex, stream, params);
                    length += SerializeLDIntValueHelper(static_cast<BYTE>(value), stream, params);
                }
                else
                {
                    length += SerializeKey(FieldType::LDInt, fieldIndex, stream, params);
                    length += SerializeLDInt(value, stream, params);
                }
                return length;
            }
            return 0;
        }

        // 
        // Serialize an array of ISerializable.
        // Notes: 
        // - Why we pass TItem* and not ISerializable* -- because we need to access item[i], for which correct length of the item must be avaialble.
        // - Arrays of other types are possible but are not implemented/not needed yet.
        //
        template <typename TItem> 
        int ExtensibleBinarySerializer::SerializeArrayOfNested(
            Length elementCount, TItem* items, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params)
        {
            int byteCount = 0;
            if (elementCount != 0)
            {
                Assert(items);

                // Serialize:
                // - key: type = length defined, index = fieldIndex.
                // - item count (as length).
                // - nested items key which is same for all items (type = length-delimited, index = 0).
                // - for each item (as nested):
                //   - length (as it's nested)
                //   - value
                byteCount += SerializeKey(FieldType::ItemCountDefined, fieldIndex, stream, params);
                byteCount += SerializeLengthHelper(elementCount, stream, params);
                byteCount += SerializeKey(FieldType::LengthDefined, 0, stream, params);

                for (unsigned int i = 0; i < elementCount; ++i)
                {
                    ISerializable* item = static_cast<ISerializable*>(&items[i]);
                    Assert(item);
                    byteCount += SerializeNestedLengthAndValueHelper(item, stream, params);
                }
            }

            return byteCount;
        }

        // Serialize LDInt value (no key): just length and value.
        template <typename T> 
        int ExtensibleBinarySerializer::SerializeLDInt(T value, ISerializationStream* stream, SerializationParameters* params, BYTE lengthMask /* = 0 */)
        {
            int length = 0;
            BYTE nonZeroByteCount = static_cast<BYTE>(SerializeLDIntValueHelper(value, NULL, params)) | lengthMask;
            length += SerializeLDIntLengthHelper(nonZeroByteCount, stream, params);
            length += SerializeLDIntValueHelper(value, stream, params);
            return length;
        }

        // Serialize only non-zero byte count.
        template <typename T> 
        int ExtensibleBinarySerializer::SerializeLDIntValueHelper(T value, ISerializationStream* stream, SerializationParameters* /*ADCAT params */)
        {
            T tmpVal = value;
            const int size = sizeof(T);
            BYTE buf[size];

            int i;
            for (i = size - 1; i >= 0 && tmpVal != 0; --i)
            {
                BYTE b = tmpVal & 0xFF;
                if (stream)
                {
                    buf[i] = b;
                }
                tmpVal >>= 8;
            }
            // Now buf[i + 1] is the value written last. Note: i can be ngative, that's fine.

            int nonZeroElementCount = size - i - 1;
            if (stream)
            {
                stream->Write(buf + i + 1, nonZeroElementCount);
            }

            Assert(nonZeroElementCount <= 8);
            return nonZeroElementCount;
        }

        template <typename T> 
        int ExtensibleBinarySerializer::DeserializeInteger(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, T* value)
        {
            switch (fieldType)
            {
            case FieldType::Byte:
                {
                    BYTE byteValue;
                    stream->Read(&byteValue, sizeof(BYTE));
                    *value = byteValue;
                    return sizeof(BYTE);
                }
    
            case FieldType::LDInt:
                return DeserializeLDInt(stream, params, value);

            default:
                DiagException::Throw(E_UNEXPECTED);
            }
        }

        template <typename T> 
        int ExtensibleBinarySerializer::DeserializeLDInt(ISerializationStream* stream, SerializationParameters* params, T* value)
        {
            int length = 0;
            BYTE byteCount;
            stream->Read(&byteCount, sizeof(BYTE));
            length += sizeof(BYTE);

            length += DeserializeLDIntValue(byteCount, stream, params, value);
            return length;
        }

        template <typename T> 
        int ExtensibleBinarySerializer::DeserializeLDIntValue(BYTE byteCount, ISerializationStream* stream, SerializationParameters* /*ADCAT params */, T* pValue)
        {
            // Note: byteCount == 0 is a valid case.
            
            if (byteCount > sizeof(T))
            {
                // LDInt would not fit into supplied T
                DiagException::Throw(E_INVALIDARG); // Note: we could also have configurable option for this, e.g. ignore the field.
            }

            AutoPtr<BYTE> buf = new(oomthrow) BYTE[byteCount];
            stream->Read(buf, byteCount);

            T value = 0;
            for (int i = 0; i < byteCount; ++i)
            {
                value = (value << 8) + buf[i];
            }

            *pValue = value;
            return byteCount;
        }

        //
        // Deserialized array of TItem, each of which is ISerializable*.
        // Note: when this is called, stream position is at item count, just passed over array key.
        //
        template <typename TItem> 
        int ExtensibleBinarySerializer::DeserializeArrayOfNested(ISerializationStream* stream, SerializationParameters* params, int itemCount, TItem* items)
        {
            int byteCount = 0;
            if (itemCount > 0)
            {
                // Note: the key (next) is same for all items.
                FieldType fieldType;
                FieldIndex fieldIndex;
                byteCount += DeserializeKey(stream, params, &fieldType, &fieldIndex);
                Assert(fieldType == FieldType::LengthDefined);

                for (int i = 0; i < itemCount; ++i)
                {
                    TItem* item = &items[i];
                    int tmpByteCount = DeserializeNested(stream, params, item);
                    byteCount += tmpByteCount;
                }
            }
            return byteCount;
        }
    }  // namespace Serialization.
}
