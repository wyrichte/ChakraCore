//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var platform = Debug.EngineInterface.Intl;
var failed = false;

var arr = ["en-CA"];
Object.defineProperty(arr, "1", { get: function () { return "en-US"; } });
var result = Intl.DateTimeFormat.supportedLocalesOf(arr);

if (result[0] !== arr[0] || result[1] !== arr[1]) {
    failed = true;
}

if (new Intl.DateTimeFormat(arr).resolvedOptions().locale !== "en-CA") {
    failed = true;
}

if (!platform.builtInGetArrayLength([1, 2, 3]) === 3) {
    failed = true;
}

if (!failed) {
    WScript.Echo("Pass");
} else {
    WScript.Echo("Fail");
}
