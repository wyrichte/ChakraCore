// <copyright file="JScript9Etw.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <date>2013-06-26</date>

namespace JsEtwConsole
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;

    /// <summary>
    /// Represents the wrapper around JScript9 ETW provider
    /// </summary>
    public class JScript9Etw
    {
        #region Constants
        /// <summary>
        /// Trace guid for Flushing in JScript9
        /// </summary>
        public static readonly Guid TraceGuid = EtwProviders.TraceGuidJScript9Provider;
        #endregion // Constants

        #region Lifetime Methods
        /// <summary>
        /// Initializes static members of the <see cref="JScript9Etw"/> class.
        /// </summary>
        static JScript9Etw()
        {
            Logger = new EtwLog(JScript9Etw.TraceGuid);
        }
        #endregion // Lifetime Methods

        #region Events
        #endregion // Events

        #region Properties

        #region Public Properties
        #endregion // Public Properties

        #region Private Properties
        /// <summary>
        /// Gets or Sets the ETW logger
        /// </summary>
        public static EtwLog Logger
        {
            get;
            private set;
        }
        #endregion // Private Properties

        #endregion // Properties

        #region Methods

        #region Public Methods
        #endregion // Public Methods

        #region Private Methods
        #endregion // Private Methods

        #endregion // Methods
    }
}
