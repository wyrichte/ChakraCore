// <copyright file="PrincipalHelper.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.Security.Principal;

    static public class PrincipalHelper
    {
        /// <summary>
        /// Indicates whether the program is running as administrator
        /// </summary>
        /// <returns>true if so, false otherwise</returns>
        public static bool IsRunningAsAdmin()
        {
            WindowsPrincipal currentPrincipal = new WindowsPrincipal(WindowsIdentity.GetCurrent());

            bool retValue = currentPrincipal.IsInRole(WindowsBuiltInRole.Administrator);

            return retValue;
        }
    }
}
