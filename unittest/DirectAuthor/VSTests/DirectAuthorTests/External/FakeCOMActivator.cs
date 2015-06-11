//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.JavaScript.LanguageService.Shared;

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Engine
{
    [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("00000001-0000-0000-C000-000000000046")]
    [SuppressMessage("Microsoft.StyleCop.CSharp.NamingRules", "*")]
    internal interface IClassFactory
    {
        [PreserveSig]
        int CreateInstance(IntPtr pUnkOuter, ref Guid riid, out IntPtr ppvObject);        
        int LockServer(int fLock);
    }

    [SuppressMessage("Microsoft.StyleCop.CSharp.NamingRules", "*")]
    internal class FakeComActivator
    {
        internal delegate int DllGetClassObject([In, MarshalAs(UnmanagedType.LPStruct)] Guid ClassId, [In, MarshalAs(UnmanagedType.LPStruct)] Guid riid, out IntPtr ppvObject);

        public static IntPtr CoCreateFromFile(string filenameOfServer, Guid clsidOfServer, Guid riid, out IntPtr pvObject)
        {
            Guid iidIUnknown = new Guid("00000001-0000-0000-C000-000000000046");
            IntPtr hmod = NativeMethods.LoadLibrary(filenameOfServer);            

            if (hmod == IntPtr.Zero)
            {
                // TODO: Discuss how to report catastrophic errors (like being unable to load JScript9ls.dll)
                var errorCode = Marshal.GetLastWin32Error();
                Marshal.ThrowExceptionForHR(errorCode);
            }

            IntPtr fptr = NativeMethods.GetProcAddress(hmod, "DllGetClassObject");
            DllGetClassObject delegateForDllGetClassObject = (DllGetClassObject)System.Runtime.InteropServices.Marshal.GetDelegateForFunctionPointer(fptr, typeof(DllGetClassObject));
            IntPtr pClassFactory = IntPtr.Zero;
            delegateForDllGetClassObject(clsidOfServer, iidIUnknown, out pClassFactory);
            IClassFactory classFactory = (IClassFactory)Marshal.GetTypedObjectForIUnknown(pClassFactory, typeof(IClassFactory));
            NativeMethods.AssertOnFail(classFactory.CreateInstance(IntPtr.Zero, ref riid, out pvObject));
            Marshal.Release(pClassFactory);
            Marshal.ReleaseComObject(classFactory);
            //Marshal.Release(fptr);
            return hmod;
        }
    }
}
