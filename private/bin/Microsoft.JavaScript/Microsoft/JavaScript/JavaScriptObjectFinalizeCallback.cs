// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JavaScriptObjectFinalizeCallback.cs" company="Microsoft Corporation">
//   Copyright (C) Microsoft. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.JavaScript
{
    using System;

    /// <summary>
    ///     A finalization callback.
    /// </summary>
    /// <param name="data">
    ///     The external data that was passed in when creating the object being finalized.
    /// </param>
    public delegate void JavaScriptObjectFinalizeCallback(IntPtr data);
}
