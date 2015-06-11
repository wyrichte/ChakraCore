// --------------------------------------------------------------------------------------------------------------------
// <copyright file="JavaScriptException.cs" company="Microsoft Corporation">
//   Copyright (C) Microsoft. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Microsoft.JavaScript
{
    using System;
    using System.Runtime.Serialization;

    /// <summary>
    ///     An exception returned from the Chakra engine.
    /// </summary>
    [Serializable]
    public class JavaScriptException : Exception
    {
        /// <summary>
        /// The error code.
        /// </summary>
        private readonly JavaScriptErrorCode code;

        /// <summary>
        ///     Initializes a new instance of the <see cref="JavaScriptException"/> class. 
        /// </summary>
        /// <param name="code">The error code returned.</param>
        public JavaScriptException(JavaScriptErrorCode code) :
            this(code, "A fatal exception has occurred in a JavaScript runtime")
        {
        }

        /// <summary>
        ///     Initializes a new instance of the <see cref="JavaScriptException"/> class. 
        /// </summary>
        /// <param name="code">The error code returned.</param>
        /// <param name="message">The error message.</param>
        public JavaScriptException(JavaScriptErrorCode code, string message) :
            base(message)
        {
            this.code = code;
        }

        /// <summary>
        ///     Initializes a new instance of the <see cref="JavaScriptException"/> class. 
        /// </summary>
        /// <param name="info">The serialization info.</param>
        /// <param name="context">The streaming context.</param>
        protected JavaScriptException(SerializationInfo info, StreamingContext context) :
            base(info, context)
        {
        }

        /// <summary>
        ///     Gets the error code.
        /// </summary>
        public JavaScriptErrorCode ErrorCode
        {
            get { return this.code; }
        }
    }
}