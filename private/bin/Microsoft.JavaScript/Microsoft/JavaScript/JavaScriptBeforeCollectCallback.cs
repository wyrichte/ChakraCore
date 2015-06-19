// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JavaScriptBeforeCollectCallback.cs" company="Microsoft Corporation">
//   Copyright (C) Microsoft. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.JavaScript
{
    using System;

    /// <summary>
    ///     A callback called before collection.
    /// </summary>
    /// <param name="callbackState">The state passed to SetBeforeCollectCallback.</param>
    public delegate void JavaScriptBeforeCollectCallback(IntPtr callbackState);
}
