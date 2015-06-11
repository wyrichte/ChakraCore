//-----------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
using System;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.VisualStudio.JavaScript.LanguageService.Shared
{
    [SuppressMessage("Microsoft.StyleCop.CSharp.NamingRules", "*")]
    internal static class Guids
    {
        public static readonly Guid IID_IAuthorTokenEnumerator = new Guid(GuidStrings.IAuthorTokenEnumeratorGuid);
        public static readonly Guid IID_IAuthorColorizeText = new Guid(GuidStrings.IAuthorColorizeTextGuid);
        public static readonly Guid IID_IAuthorServices = new Guid(GuidStrings.IAuthorServicesGuid);
        public static readonly Guid IID_IAuthorFileReader = new Guid(GuidStrings.IAuthorFileReaderGuid);
        public static readonly Guid IID_IAuthorFileContext = new Guid(GuidStrings.IAuthorFileContextGuid);
        public static readonly Guid IID_IAuthorFileAuthoring = new Guid(GuidStrings.IAuthorFileAuthoringGuid);
        public static readonly Guid IID_IAuthorDiagnostics = new Guid(GuidStrings.IAuthorDiagnosticsGuid);
        public static readonly Guid CLSID_JScript9LS = new Guid(0xf13098a9, 0xcec8, 0x471e, 0x8e, 0x43, 0xd0, 0xbd, 0x93, 0x12, 0x62, 0x3);

        public static readonly Guid LanguageServiceGuid = new Guid(GuidStrings.LanguageServiceGuid);
        public static readonly Guid DebuggerScriptLanguageGuid = new Guid("F7FA31DA-C32A-11D0-B442-00A0244A1DD2");
    }  
}
