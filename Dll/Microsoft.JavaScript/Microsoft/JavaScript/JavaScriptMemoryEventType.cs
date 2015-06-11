// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JavaScriptMemoryEventType.cs" company="Microsoft Corporation">
//   Copyright (C) Microsoft. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.JavaScript
{
    /// <summary>
    ///     Allocation callback event type.
    /// </summary>
    public enum JavaScriptMemoryEventType
    {
        /// <summary>
        ///     Indicates a request for memory allocation.
        /// </summary>
        Allocate = 0,

        /// <summary>
        ///     Indicates a memory freeing event.
        /// </summary>
        Free = 1,

        /// <summary>
        ///     Indicates a failed allocation event.
        /// </summary>
        Failure = 2
    }
}
