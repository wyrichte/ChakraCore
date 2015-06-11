// <copyright file="NativeMethods.cs" company="Microsoft">
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
    using InteropTypes = Interop.Types;

    /// <summary>
    /// Represents interop definitions
    /// </summary>
    [SuppressMessage("Microsoft.StyleCop.CSharp.DocumentationRules", "SA1402:FileMayOnlyContainASingleClass", Justification = "interop code")]
    [SuppressMessage("Microsoft.StyleCop.CSharp.DocumentationRules", "SA1600:ElementsMustBeDocumented", Justification = "interop code")]
    internal static class NativeMethods
    {
        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateEvent(
            InteropTypes.SECURITY_ATTRIBUTES SecurityAttributes, 
            bool IsManualReset, 
            bool InitialState, 
            string Name);

        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr OpenEvent(
            uint DesiredAccess,
            bool InheritHandle,
            string Name);

        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool SetEvent(
            IntPtr Handle);

        [DllImport("kernel32", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern bool ResetEvent(
            IntPtr Handle);

        [DllImport("Kernel32")]
        public static extern bool SetConsoleCtrlHandler(InteropTypes.HandlerRoutine Handler, bool Add);

        [DllImport("advapi32.dll", ExactSpelling = true, EntryPoint = "OpenTraceW", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern ulong OpenTrace(ref InteropTypes.EventTraceLogfile logfile);

        [DllImport("advapi32.dll", ExactSpelling = true, EntryPoint = "ProcessTrace")]
        public static extern int ProcessTrace(
            ulong[] HandleArray,
            uint HandleCount,
            IntPtr StartTime,
            IntPtr EndTime);

        [DllImport("advapi32.dll", ExactSpelling = true, EntryPoint = "CloseTrace")]
        public static extern int CloseTrace(ulong traceHandle);

        [DllImport("tdh.dll", ExactSpelling = true, EntryPoint = "TdhGetEventInformation")]
        public static extern int TdhGetEventInformation(
            ref InteropTypes.EventRecord Event,
            uint TdhContextCount,
            IntPtr TdhContext,
            [Out] IntPtr eventInfoPtr,
            ref int BufferSize);

        [DllImport("tdh.dll", ExactSpelling = true, EntryPoint = "TdhLoadManifest")]
        public static extern int TdhLoadManifest(
            [MarshalAs(UnmanagedType.LPWStr)] string manifest);

        [DllImport("tdh.dll", ExactSpelling = true, EntryPoint = "TdhUnloadManifest")]
        public static extern int TdhUnloadManifest(
            [MarshalAs(UnmanagedType.LPWStr)] string manifest);
    }
}
