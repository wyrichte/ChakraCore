//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Shared
{
    internal static class NativeMethods
    {
        public const int S_OK = 0x0;
        public const int S_FALSE = 0x1;

        internal static class BOOL
        {
            internal const int FALSE = 0;
            internal const int TRUE = 1;

            // TODO: {AlexGav} Make this an extension method to Boolean?
            internal static int ToBOOL(bool value)
            {
                return value ? TRUE : FALSE;
            }
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
        public static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true, ExactSpelling = true)]
        public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [SuppressMessage("Microsoft.Usage", "CA1801")] /* unused hresult parameter in release builds */
        public static void AssertOnFail(int hresult)
        {
            Debug.Assert(hresult == S_OK || hresult == S_FALSE, "Failed hresult");
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool FreeLibrary(IntPtr hModule);
    }
}
