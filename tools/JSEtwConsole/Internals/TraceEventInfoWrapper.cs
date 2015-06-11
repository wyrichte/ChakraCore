// <copyright file="TraceEventInfoWrapper.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains interop definitions
// based on http://code.msdn.microsoft.com/EventTraceWatcher
// </summary>

namespace JsEtwConsole.Internals
{
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;
    using System.Text;
    using InteropTypes = Interop.Types;
    using InteropMethods = Interop.NativeMethods;

    /// <summary>
    /// Represents the event wrapper
    /// </summary>
    internal sealed class TraceEventInfoWrapper : IDisposable
    {
        #region Constants
        /// <summary>
        /// Constant for default property name
        /// </summary>
        public const string DefaultPropertyEventDataName = "EventData";
        #endregion // Constants

        #region Fields
        /// <summary>
        /// Base address of the native TraceEventInfo structure.
        /// </summary>
        [SuppressMessage("Microsoft.Reliability", "CA2006:UseSafeHandleToEncapsulateNativeResources", Justification = "tools code")]
        private IntPtr address;

        /// <summary>
        /// Managed representation of the native TraceEventInfo structure.
        /// </summary>
        private InteropTypes.TraceEventInfo traceEventInfo;

        /// <summary>
        /// True if the event has a schema with well defined properties.
        /// </summary>
        private bool hasProperties;

        /// <summary>
        /// Marshalled array of EventPropertyInfo objects.
        /// </summary>
        private InteropTypes.EventPropertyInfo[] eventPropertyInfoArray;
        #endregion // Fields

        #region Lifetime Methods
        /// <summary>
        /// Initializes a new instance of the <see cref="TraceEventInfoWrapper"/> class.
        /// </summary>
        /// <param name="eventRecord">event record to initialize on</param>
        internal TraceEventInfoWrapper(InteropTypes.EventRecord eventRecord)
        {
            this.Initialize(eventRecord);
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="TraceEventInfoWrapper"/> class.
        /// </summary>
        ~TraceEventInfoWrapper()
        {
            this.ReleaseMemory();
        }
        #endregion // Lifetime Methods

        #region Properties

        #region Internal Properties
        /// <summary>
        /// Gets the event name
        /// </summary>
        internal string EventName
        {
            get;
            private set;
        }
        #endregion // Internal Properties

        #endregion // Properties

        #region Methods

        #region Public Methods
        /// <summary>
        /// Dispose logic
        /// </summary>
        public void Dispose()
        {
            this.ReleaseMemory();
            GC.SuppressFinalize(this);
        }
        #endregion // Public Methods

        #region Internal / Private Methods
        /// <summary>
        /// Get all properties of the given event
        /// </summary>
        /// <param name="eventRecord">event to be parsed</param>
        /// <returns>all properties as propertybag</returns>
        internal PropertyBag GetProperties(InteropTypes.EventRecord eventRecord)
        {
            PropertyBag properties = new PropertyBag();

            if (this.hasProperties)
            {
                int offset = 0;

                for (int i = 0; i < this.traceEventInfo.TopLevelPropertyCount; i++)
                {
                    InteropTypes.EventPropertyInfo topLevelDescriptor = this.eventPropertyInfoArray[i];
                    string topLevelPropertyName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + topLevelDescriptor.NameOffset));

                    object value = null;
                    string mapName = null;
                    int length = 0;
                    IntPtr dataPtr;
                    uint arraySize = 1;

                    if (IsPropertyHasZeroLength(topLevelDescriptor, properties))
                    {
                        // String with zero length (empty string).
                        continue;
                    }

                    if ((topLevelDescriptor.Flags & InteropTypes.PropertyFlags.PropertyParamCount) == InteropTypes.PropertyFlags.PropertyParamCount)
                    {
                        // If this field is an array, the number of elements in the array is stored at another index.
                        InteropTypes.EventPropertyInfo countDescriptor = this.eventPropertyInfoArray[topLevelDescriptor.CountPropertyIndex];
                        string countName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + countDescriptor.NameOffset));

                        // Hopefully we already decoded the count field (propertyName). Look it up in PropertyBag.
                        if (properties.TryGetValue(countName, out value))
                        {
                            arraySize = (uint)value;
                        }
                    }

                    for (uint k = 0; k < arraySize; k++)
                    {
                        if ((topLevelDescriptor.Flags & Interop.Types.PropertyFlags.PropertyStruct) == InteropTypes.PropertyFlags.PropertyStruct)
                        {
                            int firstMember = topLevelDescriptor.StructTypeValue.StructStartIndex;
                            int lastMember = topLevelDescriptor.StructTypeValue.StructStartIndex + topLevelDescriptor.StructTypeValue.NumOfStructMembers;
                            PropertyBag structValue = new PropertyBag();

                            for (int j = firstMember; j < lastMember; j++)
                            {
                                InteropTypes.EventPropertyInfo fieldDescriptor = this.eventPropertyInfoArray[j];

                                if (IsPropertyHasZeroLength(fieldDescriptor, structValue))
                                {
                                    // String with zero length (empty string).
                                    continue;
                                }

                                string fieldName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + fieldDescriptor.NameOffset));
                                dataPtr = new IntPtr(eventRecord.UserData.ToInt64() + offset);
                                object field = this.ReadPropertyValue(fieldDescriptor, dataPtr, out mapName, out length);
                                offset += length;

                                // If we have a map name, return both map name and map value as a pair.
                                if (!string.IsNullOrEmpty(mapName))
                                {
                                    field = new KeyValuePair<string, object>(mapName, field);
                                }

                                if (ShouldFilterEventField(fieldName, fieldDescriptor.NonStructTypeValue.InType, value) == false)
                                {
                                    structValue.Add(fieldName, field);
                                }
                            }

                            value = structValue;
                        }
                        else
                        {
                            dataPtr = new IntPtr(eventRecord.UserData.ToInt64() + offset);
                            value = this.ReadPropertyValue(topLevelDescriptor, dataPtr, out mapName, out length);
                            offset += length;

                            // If we have a map name, return both map name and map value as a pair.
                            if (!string.IsNullOrEmpty(mapName))
                            {
                                value = new KeyValuePair<string, object>(mapName, value);
                            }
                        }

                        if (ShouldFilterEventField(topLevelPropertyName, topLevelDescriptor.NonStructTypeValue.InType, value) == false)
                        {
                            if (arraySize > 1)
                            {
                                properties.Add(topLevelPropertyName + "[" + k + "]", value);
                            }
                            else
                            {
                                properties.Add(topLevelPropertyName, value);
                            }
                        }
                    }
                }

                if (offset < eventRecord.UserDataLength)
                {
                    // There is some extra information not mapped.
                    IntPtr dataPtr = new IntPtr(eventRecord.UserData.ToInt64() + offset);
                    int length = eventRecord.UserDataLength - offset;
                    byte[] array = new byte[length];
                    Marshal.Copy(dataPtr, array, 0, length);

                    string str = "0x" + BitConverter.ToString(array).Replace("-", string.Empty);

                    properties.Add("__ExtraPayload", str);
                }
            }
            else
            {
                // get the byte[], and try to parse it as string (best effort basis)
                byte[] buffer = new byte[eventRecord.UserDataLength];
                Marshal.Copy(eventRecord.UserData, buffer, 0, eventRecord.UserDataLength);

                string str = string.Empty;

                bool isInvalidString = false;
                try
                {
                    // convert to string (unicode 1st)
                    str = Encoding.Unicode.GetString(buffer) ?? string.Empty;

                    // check for any invalid characters (unprintable)
                    foreach (char c in str)
                    {
                        if (c < ' ' && c != '\0' && c != '\t' && c != '\r' && c != '\n')
                        {
                            isInvalidString = true;
                            break;
                        }
                    }
                }
                catch
                {
                    // swallow, fail safe
                    isInvalidString = true;
                }

                if (isInvalidString)
                {
                    // has inprintable chars -- show hexString (from ASCII)
                    str = Encoding.Default.GetString(buffer) ?? string.Empty;
                    str = "0x" + BitConverter.ToString(buffer).Replace("-", string.Empty);
                }
                else
                {
                    // clean (trim)
                    str = str.TrimEnd(" \0\r\n".ToCharArray());
                }

                properties.Add(DefaultPropertyEventDataName, str);
            }

            return properties;
        }

        /// <summary>
        ///  If the current property has PropertyParamLength field, then it returns true if the value of it is 0,  o/w false.
        /// </summary>
        /// <param name="descriptor">Current property info</param>
        /// <param name="properties">The property bag where current property will be added</param>
        /// <returns>return true if length is zero</returns>
        public bool IsPropertyHasZeroLength(InteropTypes.EventPropertyInfo descriptor, PropertyBag properties)
        {
            if ((descriptor.Flags & InteropTypes.PropertyFlags.PropertyParamLength) == InteropTypes.PropertyFlags.PropertyParamLength)
            {
                // Reading the field which contains the length field.
                // Only top level string support : if we need to add this support

                InteropTypes.EventPropertyInfo lengthDescriptor = this.eventPropertyInfoArray[descriptor.LengthPropertyIndex];
                string lengthName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + lengthDescriptor.NameOffset));

                object value;
                // Try get the length field, and if that is zero, filter this property.
                if (properties.TryGetValue(lengthName, out value) && ((uint)value == 0))
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Indicates fields which should be filtered
        /// </summary>
        /// <param name="fieldName">Name of the ETW event field.</param>
        /// <param name="inType">the inType mentioned for current field.</param>
        /// <param name="value">Value of the ETW event field.</param>
        /// <returns>true if the field should be filtered out</returns>
        public static bool ShouldFilterEventField(string fieldName, InteropTypes.TdhInType inType, object value)
        {
            // Do not filter any events if verbose output is enabled.
            if (Program.EnableFileVerboseOutput == true)
            {
                return false;
            }

            // If field is pointer type, filter it out.
            if (inType == InteropTypes.TdhInType.Pointer)
            {
                return true;
            }

            // If field is a well-known verbose name, filter it out.
            if (string.Compare(fieldName, "DocumentId", StringComparison.OrdinalIgnoreCase) == 0)
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Initialize based on a given eventRecord
        /// </summary>
        /// <param name="eventRecord">event to initialize on</param>
        private void Initialize(InteropTypes.EventRecord eventRecord)
        {
            int size = 0;
            const uint BufferTooSmall = 122;
            const uint ElementNotFound = 1168;

            int error = InteropMethods.TdhGetEventInformation(ref eventRecord, 0, IntPtr.Zero, IntPtr.Zero, ref size);
            if (error == ElementNotFound)
            {
                // Nothing else to do here.
                this.hasProperties = false;
                return;
            }

            this.hasProperties = true;

            if (error != BufferTooSmall)
            {
                throw new Win32Exception(error, "TdhGetEventInformation");
            }

            // Get the event information (schema)
            this.address = Marshal.AllocHGlobal(size);
            this.traceEventInfo = new InteropTypes.TraceEventInfo();
            error = InteropMethods.TdhGetEventInformation(ref eventRecord, 0, IntPtr.Zero, this.address, ref size);
            if (error != 0)
            {
                throw new Win32Exception(error, "TdhGetEventInformation");
            }

            // Marshal the first part of the trace event information.
            Marshal.PtrToStructure(this.address, this.traceEventInfo);

            // Marshal the second part of the trace event information, the array of property info.
            int actualSize = Marshal.SizeOf(this.traceEventInfo);
            if (size != actualSize)
            {
                int structSize = Marshal.SizeOf(typeof(InteropTypes.EventPropertyInfo));
                int itemsLeft = (size - actualSize) / structSize;

                this.eventPropertyInfoArray = new InteropTypes.EventPropertyInfo[itemsLeft];
                long baseAddress = this.address.ToInt64() + actualSize;
                for (int i = 0; i < itemsLeft; i++)
                {
                    IntPtr structPtr = new IntPtr(baseAddress + (i * structSize));
                    InteropTypes.EventPropertyInfo info = new InteropTypes.EventPropertyInfo();
                    Marshal.PtrToStructure(structPtr, info);
                    this.eventPropertyInfoArray[i] = info;
                }
            }

            // Get the opcode name
            if (this.traceEventInfo.OpcodeNameOffset > 0)
            {
                this.EventName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + this.traceEventInfo.OpcodeNameOffset));
            }
        }

        /// <summary>
        /// Read the property value
        /// </summary>
        /// <param name="info">info instance</param>
        /// <param name="dataPtr">data pointer</param>
        /// <param name="mapName">mapping name</param>
        /// <param name="length">length of the property returned</param>
        /// <returns>matching object returned</returns>
        private object ReadPropertyValue(InteropTypes.EventPropertyInfo info, IntPtr dataPtr, out string mapName, out int length)
        {
            length = info.LengthPropertyIndex;

            if (info.NonStructTypeValue.MapNameOffset != 0)
            {
                mapName = Marshal.PtrToStringUni(new IntPtr(this.address.ToInt64() + info.NonStructTypeValue.MapNameOffset));
            }
            else
            {
                mapName = string.Empty;
            }

            switch (info.NonStructTypeValue.InType)
            {
                case InteropTypes.TdhInType.Null:
                    break;
                case InteropTypes.TdhInType.UnicodeString:
                    {
                        string str = Marshal.PtrToStringUni(dataPtr);
                        length = (str.Length + 1) * sizeof(char);
                        return str;
                    }

                case InteropTypes.TdhInType.AnsiString:
                    {
                        string str = Marshal.PtrToStringAnsi(dataPtr);
                        length = str.Length + 1;
                        return str;
                    }

                case InteropTypes.TdhInType.Int8:
                    return (sbyte)Marshal.ReadByte(dataPtr);
                case InteropTypes.TdhInType.UInt8:
                    return Marshal.ReadByte(dataPtr);
                case InteropTypes.TdhInType.Int16:
                    return Marshal.ReadInt16(dataPtr);
                case InteropTypes.TdhInType.UInt16:
                    return (uint)Marshal.ReadInt16(dataPtr);
                case InteropTypes.TdhInType.Int32:
                    return Marshal.ReadInt32(dataPtr);
                case InteropTypes.TdhInType.UInt32:
                    return (uint)Marshal.ReadInt32(dataPtr);
                case InteropTypes.TdhInType.Int64:
                    return Marshal.ReadInt64(dataPtr);
                case InteropTypes.TdhInType.UInt64:
                    return (ulong)Marshal.ReadInt64(dataPtr);
                case InteropTypes.TdhInType.Float:
                    break;
                case InteropTypes.TdhInType.Double:
                    break;
                case InteropTypes.TdhInType.Boolean:
                    return (bool)(Marshal.ReadInt32(dataPtr) != 0);
                case InteropTypes.TdhInType.Binary:
                    break;
                case InteropTypes.TdhInType.Guid:
                    return new Guid(
                           Marshal.ReadInt32(dataPtr),
                           Marshal.ReadInt16(dataPtr, 4),
                           Marshal.ReadInt16(dataPtr, 6),
                           Marshal.ReadByte(dataPtr, 8),
                           Marshal.ReadByte(dataPtr, 9),
                           Marshal.ReadByte(dataPtr, 10),
                           Marshal.ReadByte(dataPtr, 11),
                           Marshal.ReadByte(dataPtr, 12),
                           Marshal.ReadByte(dataPtr, 13),
                           Marshal.ReadByte(dataPtr, 14),
                           Marshal.ReadByte(dataPtr, 15));
                case InteropTypes.TdhInType.Pointer:
                    IntPtr pointer = Marshal.ReadIntPtr(dataPtr);
                    return pointer.ToString("X");
                case InteropTypes.TdhInType.FileTime:
                    break;
                case InteropTypes.TdhInType.SystemTime:
                    break;
                case InteropTypes.TdhInType.SID:
                    break;
                case InteropTypes.TdhInType.HexInt32:
                    break;
                case InteropTypes.TdhInType.HexInt64:
                    break;
                case InteropTypes.TdhInType.CountedString:
                    break;
                case InteropTypes.TdhInType.CountedAnsiString:
                    break;
                case InteropTypes.TdhInType.ReversedCountedString:
                    break;
                case InteropTypes.TdhInType.ReversedCountedAnsiString:
                    break;
                case InteropTypes.TdhInType.NonNullTerminatedString:
                    break;
                case InteropTypes.TdhInType.NonNullTerminatedAnsiString:
                    break;
                case InteropTypes.TdhInType.UnicodeChar:
                    break;
                case InteropTypes.TdhInType.AnsiChar:
                    break;
                case InteropTypes.TdhInType.SizeT:
                    break;
                case InteropTypes.TdhInType.HexDump:
                    break;
                case InteropTypes.TdhInType.WbemSID:
                    break;
                default:
                    Debugger.Break();
                    break;
            }

            throw new ArgumentOutOfRangeException("info.NonStructTypeValue.InType = " + info.NonStructTypeValue.InType.ToString());
        }

        /// <summary>
        /// Release the memory used
        /// </summary>
        private void ReleaseMemory()
        {
            if (this.address != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(this.address);
            }
        }
        #endregion // Internal / Private Methods

        #endregion // Methods
    }
}