//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once

namespace JsDiag
{
    namespace Serialization
    {
        // The type of serialization field.
        // Currently there are 3 bits to store FieldType.
        enum FieldType
        {
            Byte = 1,         // The value is 1 byte long.
            LDInt,            // 1 byte for length of the value, then variable-length length bytes long.
            LengthDefined,    // LDInt for length of the data, then variable-length data -- used for such as strings, nested types, etc.
            ItemCountDefined, // LDInt for item count, then variable-length data.
        };

        // The stream used for de/serialization.
        struct ISerializationStream
        {
            // Write content of the buf to the stream.
            virtual void Write(const void* buf, size_t byteCount) = 0;

            // Read into buf from the stream.
            virtual void Read(void* buf, size_t byteCount) = 0;

            // Whether stream has reached its end/end of file.
            virtual bool IsEOF() = 0;
        };

        // Parameter object passed though all de/serialization chain.
        // Can be used 
        struct SerializationParameters
        {
        };

        typedef BYTE FieldIndex;

        // Base class for all serializable classes.
        // Note: all classes must also have public default ctor.
        struct ISerializable
        {
            // Returns the number of bytes needed for this instance to serialize.
            virtual int Serialize(ISerializationStream* stream, SerializationParameters* params) = 0;

            // Returns the number of bytes deserialized. Note: this must be accurate; return 0 if none of the fields were recognized.
            virtual int Deserialize(FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, ISerializationStream* stream, SerializationParameters* params) = 0;
        };

        //
        // Serializer that uses binary format that is extensible, i.e. 
        // v1 serializer can read format produced by v2 serializer which may have fields
        // that the serializer doesn't know about and just ignores them.
        //
        // Goals:
        // - Decouple versions of serializer (client) and deserializer (server) so that different versions can work together
        //   without need to check/special case the version and like this:
        //   if (version == 1) ...
        //   else if (version == 2) ...
        // - Reasonable space (XML is too chatty), reasonable speed (XML is too slow) and reasonable diagnosability (not too cryptic).
        //
        // Design ideas:
        // - Serialization stream represents a message (struct that implements ISerializable), which has data fields.
        //   - The fields can be integers (byte, int, int64, etc), strings, nested structs, arrays of nested, etc.
        // - The entry point is Serializer::Serialize and Deserialize -- both operate on one top-level message.
        // - For each message the Serializer calls into ISerializable to serialize soecific field, which calls Serializer primitives
        //   to serialize specific field type (e.g. integer), etc.
        // - Each field is serialized as key-value pair:
        //   - The key contains field index (everything except 3 low bits) and field type (3 low bits):
        //     - Field index is what makes the format extensible, like tag name in XML.
        //     - Field type (see FieldType enum) denotes the way the field is de/serialized.
        //     - The key would normally be 1 byte with hi bit set to 0, but for field index > 15 we use key as LDInt with hi-bit set to 1.
        //   - The value is actual value of the field.
        //   - For top-level message we don't encode the key.
        // - LDInt is core type for integers.
        //   - Only valuable bytes are serialized (e.g. for int32 that has non-zero only 1st byte we serialize only that one byte). Save space.
        //   - Serialized as 1 byte length which tells how long in bytes the value is, then the value.
        //   - Normally we don't serialize 0 values (as this is the default), but in certain cases we do (e.g. nested length),
        //     in this case 0 LDInt is serialized as 00 (length = 0, no value).
        //   - There could be different ways, e.g. verints, etc. 
        //     Our choice is to avoid to much decryption but at the same time 
        //     use reasonable space.
        // - Why use ISerializable/ISerialization stream:
        //   - Well, we could use templates everywhere, but usin interfaces is much more readable, and perf penalty
        //     for virtual function call is not super important here.
        // - During deserialation we should always be able to skip a field unrecognized by ISerialializable message.
        //
        // Typical scenarios where extensibility kicks in:
        // - Added field:
        //   - Old deserializer, new version of message that has a field that wasn't in old one. 
        //     In this case old message just ignores the field with unsupported field index.
        // - Removed field:
        //   - Old deserializer, in new version of message a field was deleted so that it's not serialized.
        //     In this case old message isn't called to deserialize the field and the field gets default value.
        //
        // Guidelines:
        // - If you need to remove a field from the message, just don't serialize it.
        // - if you want to add a new field to the message, always use filed index greater than max current one (add to the end), 
        //   don't reuse 'holes' from removed field.
        //
        // For examples refer to unit tests.
        //
        class ExtensibleBinarySerializer
        {
            typedef unsigned int Length;
            static const Length Length_MAX = static_cast<Length>(-1);

        public:
            // All of these return the # of bytes needed to serialize.
            // If specified stream is NULL, no serialization happens, but still the length needed to serialize is returned.
            // Serialize a message to the stream.
            // Parameters:
            // - data: the message to serialize.
            // - stream: the stream to serialize to.
            // - params: extensible/parameter object that is kept through the session and can be used to pass through some data.
            //   For instance, a message can set some options to affect deserailziation of nested messages.
            //   Or, the serializer can pass a cache of lengths of nested instances to avoid getting length for same instance more than once.
            // - returns the number of bytes serialized to the stream.
            static int Serialize(ISerializable* data, ISerializationStream* stream, SerializationParameters* params);

            // Below are serialation primitives, call these from implementation of ISerialilable to serialize the fields.
            template <typename T> static int SerializeInteger(T value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeBool(bool value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeString(LPCWSTR value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeNested(ISerializable* value, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);
            template <typename TItem> static int SerializeArrayOfNested(Length elementCount, TItem* items, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);

            // All of these return the # of bytes deserialized.
            // Deserialize a message.
            // Main entry point of deserialization.
            // Parameters:
            // - stream: the stream to deserialize from.
            // - params: extensible/parameter object that is kept through the session and can be used to pass through some data.
            //   For instance, a message can set some options to affect deserailziation of nested messages.
            //   Or, the serializer can pass a cache of lengths of nested instances to avoid getting length for same instance more than once.
            // Return value:
            // - returns the number of bytes deserialized from the stream.
            static int Deserialize(ISerializationStream* stream, SerializationParameters* params, ISerializable* value);

            // Below are deserialation primitives, call these from implementation of ISerialilable to deserialize the fields.
            template <typename T> static int DeserializeInteger(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, T* value);
            static int DeserializeBool(FieldType fieldType, ISerializationStream* stream, SerializationParameters* params, bool* value);
            static int DeserializeString(ISerializationStream* stream, SerializationParameters* params, LPCWSTR* value);
            static int DeserializeNested(ISerializationStream* stream, SerializationParameters* params, ISerializable* value);
            template <typename TItem> static int DeserializeArrayOfNested(ISerializationStream* stream, SerializationParameters* params, int itemCount, TItem* items);

        private:
            static int SerializeKey(FieldType fieldType, FieldIndex fieldIndex, ISerializationStream* stream, SerializationParameters* params);
            template <typename T> static int SerializeLDInt(T value, ISerializationStream* stream, SerializationParameters* params, BYTE lengthMask = 0);
            template <typename T> static int SerializeLDIntValueHelper(T value, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeLDIntLengthHelper(BYTE valueLength, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeLengthHelper(int length, ISerializationStream* stream, SerializationParameters* params);
            static int SerializeNestedLengthAndValueHelper(ISerializable* value, ISerializationStream* stream, SerializationParameters* params);
            template <typename TItem> static int SerializeArrayOfNestedValueHelper(Length elementCount, TItem* items, ISerializationStream* stream, SerializationParameters* params, int* actualElementCount);

            static int DeserializeKey(ISerializationStream* stream, SerializationParameters* params, FieldType* fieldType, FieldIndex* fieldIndex);
            template <typename T> static int DeserializeLDInt(ISerializationStream* stream, SerializationParameters* params, T* value);
            template <typename T> static int DeserializeLDIntValue(BYTE byteCount, ISerializationStream* stream, SerializationParameters* params, T* value);
            static int DeserializeStructHelper(ISerializationStream* stream, SerializationParameters* params, Length length, ISerializable* value);
            static int DeserializeStructValueHelper(ISerializable* serializable, FieldIndex fieldIndex, FieldType fieldType, int arrayItemCount, 
                int remainingStructBytes, ISerializationStream* stream, SerializationParameters* params);
            static int DeserializeLengthHelper(ISerializationStream* stream, SerializationParameters* params, Length* value);
            static int DeserializeSkippedField(FieldType fieldType, int arrayItemCount, ISerializationStream* stream,  SerializationParameters* params);
        };
    }
}
