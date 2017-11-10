//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

//
// Windows Blue Bugs: Bug 589877: CAS:WebCrawler: AV: UNKNOWN: jscript9.dll!Js::FunctionInfo::GetOriginalEntryPoint
//
var i=0;
function foo() {
    var d = new Date();
    var t = d.toLocaleTimeString();
    i++;
}
foo();

WScript.StartProfiling(foo);
WScript.Attach(foo);
WScript.Detach(foo);
WScript.StopProfiling(function() { WScript.Echo("pass"); });
