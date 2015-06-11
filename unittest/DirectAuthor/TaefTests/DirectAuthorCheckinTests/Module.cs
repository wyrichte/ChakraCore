//----------------------------------------------------------------------------------------------------------------------
// <copyright file="Module.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
// <summary>Defines the Module type.</summary>
//----------------------------------------------------------------------------------------------------------------------
namespace DirectAuthorCheckinTests
{
    using System;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class Module
    {
        [AssemblyInitialize]
        [TestProperty("Parallel", "true")]
        public static void RunModuleSetup(Object context)
        {
        }

        private Module()
        {
        }
    }
}