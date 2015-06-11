// <copyright file="Interop.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains interop definitions
// based on http://code.msdn.microsoft.com/EventTraceWatcher
// </summary>

namespace JsEtwConsole.Internals.Interop
{
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    /// <summary>
    /// Represents interop type definitions
    /// </summary>
    [SuppressMessage("Microsoft.StyleCop.CSharp.MaintainabilityRules", "SA1401:FieldsMustBePrivate", Justification = "interop code")]
    [SuppressMessage("Microsoft.StyleCop.CSharp.DocumentationRules", "SA1600:ElementsMustBeDocumented", Justification = "interop code")]
    [SuppressMessage("Microsoft.StyleCop.CSharp.DocumentationRules", "SA1602:EnumerationItemsMustBeDocumented", Justification = "interop code")]
    internal static class Types
    {
        internal delegate void EventRecordCallback([In] ref EventRecord eventRecord);

        // A delegate type to be used as the handler routine 
        // for SetConsoleCtrlHandler.
        internal delegate bool HandlerRoutine(CtrlTypes CtrlType);

        // An enumerated type for the control messages
        // sent to the handler routine.
        internal enum CtrlTypes
        {
            CTRL_C_EVENT = 0,
            CTRL_BREAK_EVENT,
            CTRL_CLOSE_EVENT,
            CTRL_LOGOFF_EVENT = 5,
            CTRL_SHUTDOWN_EVENT
        }

        [Flags]
        internal enum PropertyFlags
        {
            PropertyStruct = 0x1,
            PropertyParamLength = 0x2,
            PropertyParamCount = 0x4,
            PropertyWBEMXmlFragment = 0x8,
            PropertyParamFixedLength = 0x10
        }

        internal enum TdhInType : ushort
        {
            Null,
            UnicodeString,
            AnsiString,
            Int8,
            UInt8,
            Int16,
            UInt16,
            Int32,
            UInt32,
            Int64,
            UInt64,
            Float,
            Double,
            Boolean,
            Binary,
            Guid,
            Pointer,
            FileTime,
            SystemTime,
            SID,
            HexInt32,
            HexInt64,  // End of winmeta intypes
            CountedString = 300, // Start of TDH intypes for WBEM
            CountedAnsiString,
            ReversedCountedString,
            ReversedCountedAnsiString,
            NonNullTerminatedString,
            NonNullTerminatedAnsiString,
            UnicodeChar,
            AnsiChar,
            SizeT,
            HexDump,
            WbemSID
        }

        internal enum TemplateFlags
        {
            TemplateEventDdata = 1,
            TemplateUserData = 2
        }

        internal enum DecodingSource
        {
            DecodingSourceXmlFile,
            DecodingSourceWbem,
            DecodingSourceWPP
        }

        internal enum TdhOutType : ushort
        {
            Null,
            String,
            DateTime,
            Byte,
            UnsignedByte,
            Short,
            UnsignedShort,
            Int,
            UnsignedInt,
            Long,
            UnsignedLong,
            Float,
            Double,
            Boolean,
            Guid,
            HexBinary,
            HexInt8,
            HexInt16,
            HexInt32,
            HexInt64,
            PID,
            TID,
            PORT,
            IPV4,
            IPV6,
            SocketAddress,
            CimDateTime,
            EtwTime,
            Xml,
            ErrorCode,              // End of winmeta outtypes
            ReducedString = 300,    // Start of TDH outtypes for WBEM
            NoPrint
        }

        [Flags]
        internal enum SyncObjectAccess : uint
        {
            DELETE = 0x00010000,
            READ_CONTROL = 0x00020000,
            WRITE_DAC = 0x00040000,
            WRITE_OWNER = 0x00080000,
            SYNCHRONIZE = 0x00100000,
            EVENT_ALL_ACCESS = 0x001F0003,
            EVENT_MODIFY_STATE = 0x00000002,
            MUTEX_ALL_ACCESS = 0x001F0001,
            MUTEX_MODIFY_STATE = 0x00000001,
            SEMAPHORE_ALL_ACCESS = 0x001F0003,
            SEMAPHORE_MODIFY_STATE = 0x00000002,
            TIMER_ALL_ACCESS = 0x001F0003,
            TIMER_MODIFY_STATE = 0x00000002,
            TIMER_QUERY_STATE = 0x00000001
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct SECURITY_ATTRIBUTES {
            internal uint Length;
            internal IntPtr SecurityDescriptor;
            internal bool InheritHandle;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct Win32TimeZoneInfo
        {
            internal int Bias;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            internal char[] StandardName;
            internal SystemTime StandardDate;
            internal int StandardBias;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            internal char[] DaylightName;
            internal SystemTime DaylightDate;
            internal int DaylightBias;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct SystemTime
        {
            internal short Year;
            internal short Month;
            internal short DayOfWeek;
            internal short Day;
            internal short Hour;
            internal short Minute;
            internal short Second;
            internal short Milliseconds;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct TraceLogfileHeader
        {
            internal uint BufferSize;
            internal uint Version;
            internal uint ProviderVersion;
            internal uint NumberOfProcessors;
            internal long EndTime;
            internal uint TimerResolution;
            internal uint MaximumFileSize;
            internal uint LogFileMode;
            internal uint BuffersWritten;
            internal Guid LogInstanceGuid;
            internal IntPtr LoggerName;
            internal IntPtr LogFileName;
            internal Win32TimeZoneInfo TimeZone;
            internal long BootTime;
            internal long PerfFreq;
            internal long StartTime;
            internal uint ReservedFlags;
            internal uint BuffersLost;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct EventTraceHeader
        {
            internal ushort Size;
            internal ushort FieldTypeFlags;
            internal uint Version;
            internal uint ThreadId;
            internal uint ProcessId;
            internal long TimeStamp;
            internal Guid Guid;
            internal uint KernelTime;
            internal uint UserTime;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct EventTrace
        {
            internal EventTraceHeader Header;
            internal uint InstanceId;
            internal uint ParentInstanceId;
            internal Guid ParentGuid;
            internal IntPtr MofData;
            internal uint MofLength;
            internal uint ClientContext;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct EventTraceLogfile
        {
            [MarshalAs(UnmanagedType.LPWStr)]
            internal string LogFileName;
            [MarshalAs(UnmanagedType.LPWStr)]
            internal string LoggerName;
            internal long CurrentTime;
            internal uint BuffersRead;
            internal uint ProcessTraceMode;
            internal EventTrace CurrentEvent;
            internal TraceLogfileHeader LogfileHeader;
            internal IntPtr BufferCallback;
            internal uint BufferSize;
            internal uint Filled;
            internal uint EventsLost;
            [MarshalAs(UnmanagedType.FunctionPtr)]
            internal EventRecordCallback EventRecordCallback;
            internal uint IsKernelTrace;
            internal IntPtr Context;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct EtwEventDescriptor
        {
            internal ushort Id;
            internal byte Version;
            internal byte Channel;
            internal byte Level;
            internal byte Opcode;
            internal ushort Task;
            internal ulong Keyword;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct EventHeader
        {
            internal ushort Size;
            internal ushort HeaderType;
            internal ushort Flags;
            internal ushort EventProperty;
            internal uint ThreadId;
            internal uint ProcessId;
            internal long TimeStamp;
            internal Guid ProviderId;
            internal EtwEventDescriptor EventDescriptor;
            internal ulong ProcessorTime;
            internal Guid ActivityId;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct EventRecord
        {
            internal EventHeader EventHeader;
            internal EtwBufferContext BufferContext;
            internal ushort ExtendedDataCount;
            internal ushort UserDataLength;
            internal IntPtr ExtendedData;
            internal IntPtr UserData;
            internal IntPtr UserContext;

            [StructLayout(LayoutKind.Sequential)]
            internal struct EtwBufferContext
            {
                internal byte ProcessorNumber;
                internal byte Alignment;
                internal ushort LoggerId;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        internal sealed class TraceEventInfo
        {
            internal Guid ProviderGuid;
            internal Guid EventGuid;
            internal EtwEventDescriptor EventDescriptor;
            internal DecodingSource DecodingSource;
            internal uint ProviderNameOffset;
            internal uint LevelNameOffset;
            internal uint ChannelNameOffset;
            internal uint KeywordsNameOffset;
            internal uint TaskNameOffset;
            internal uint OpcodeNameOffset;
            internal uint EventMessageOffset;
            internal uint ProviderMessageOffset;
            internal uint BinaryXmlOffset;
            internal uint BinaryXmlSize;
            internal uint ActivityIDNameOffset;
            internal uint RelatedActivityIDNameOffset;
            internal uint PropertyCount;
            internal uint TopLevelPropertyCount;
            internal TemplateFlags Flags;
        }

        // see http://msdn.microsoft.com/en-us/library/windows/desktop/aa964763(v=vs.85).aspx
        [StructLayout(LayoutKind.Explicit)]
        internal sealed class EventPropertyInfo
        {
            [FieldOffset(0)]
            internal PropertyFlags Flags;
            [FieldOffset(4)]
            internal uint NameOffset;
            [FieldOffset(8)]
            internal NonStructType NonStructTypeValue;
            [FieldOffset(8)]
            internal StructType StructTypeValue;
            [FieldOffset(16)]
            internal ushort CountPropertyIndex;
            [FieldOffset(18)]
            internal ushort LengthPropertyIndex;
            [FieldOffset(20)]
            private uint reserved;

            [StructLayout(LayoutKind.Sequential)]
            internal struct NonStructType
            {
                internal TdhInType InType;
                internal TdhOutType OutType;
                internal uint MapNameOffset;
            }

            [StructLayout(LayoutKind.Sequential)]
            internal struct StructType
            {
                internal ushort StructStartIndex;
                internal ushort NumOfStructMembers;
                private uint padding;
            }
        }

    }
}
