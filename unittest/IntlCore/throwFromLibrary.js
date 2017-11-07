//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

/**exception(resume_ignore):stack();**/

function testFirstChanceException() {
    var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
}

testFirstChanceException();

WScript.Echo("pass");
