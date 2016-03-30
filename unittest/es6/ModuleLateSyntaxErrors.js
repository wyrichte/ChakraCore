//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

WScript.LoadModule('import { fakebinding } from "ModuleComplexExports.js";', 'samethread');
WScript.LoadModule('import { fakebinding as localname } from "ModuleComplexExports.js";', 'samethread');
WScript.LoadModule('import _default from "ModuleSimpleExport.js";', 'samethread');
WScript.LoadModule(`eval('import foo from "ValidExportStatements.js";');`, 'samethread');
WScript.LoadModule("eval('export default function() { }');", 'samethread');
