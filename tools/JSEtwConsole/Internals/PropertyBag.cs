// <copyright file="PropertyBag.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>
// <summary>
// Contains a property bag class
// based on http://code.msdn.microsoft.com/EventTraceWatcher
// </summary>

namespace JsEtwConsole.Internals
{
    using System;
    using System.Collections.Generic;
    using System.Runtime.Serialization;
    using System.Text;

    /// <summary>
    /// Represents a property bag class
    /// </summary>
    [Serializable]
    public class PropertyBag : Dictionary<string, object>
    {
        #region Lifetime Methods
        /// <summary>
        /// Initializes a new instance of the <see cref="PropertyBag"/> class.
        /// </summary>
        public PropertyBag()
            : base(StringComparer.OrdinalIgnoreCase)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="PropertyBag"/> class.
        /// </summary>
        /// <param name="info">A System.Runtime.Serialization.SerializationInfo object containing the information 
        /// required to serialize the System.Collections.Generic.Dictionary&gt;TKey,TValue&lt;.</param>
        /// <param name="context">A System.Runtime.Serialization.StreamingContext structure containing the source 
        /// and destination of the serialized stream associated with the System.Collections.Generic.Dictionary&gt;TKey,TValue&lt;.</param>
        protected PropertyBag(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }

        /// <summary>
        /// Write this PropertyBag out to a string.
        /// Format is (key1=value1;key2=value2)
        /// PropertyBag supports nested PropertyBag objects.
        /// </summary>
        /// <returns>String representation of this PropertyBag</returns>
        public override string ToString() 
        {
            StringBuilder sb = new StringBuilder();
            bool first = true;

            if (this.Count > 0)
            {
                sb.Append("(");
                foreach (KeyValuePair<string, object> kv in this)
                {
                    if (!first)
                    {
                        sb.Append(';');
                    }
                    first = false;

                    sb.Append(kv.Key);
                    sb.Append('=');

                    string kvValue = kv.Value == null ? "<null>" : kv.Value.ToString();
                    kvValue = kvValue.Replace("{", "{{");
                    kvValue = kvValue.Replace("}", "}}");

                    sb.Append(kvValue);
                }
                sb.Append(")");
            }

            return sb.ToString();
        }
        #endregion // Lifetime Methods
    }
}
