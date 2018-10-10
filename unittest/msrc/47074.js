//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

for (var x = 0; x < 10; x++) {
    f();
}
f(this);
function f(i0) {
    try {
        f0 = function () {
            f0 = function () {
                i0[0] = 1 & i0;
            };
            f0();
        };
        for (var x = 0; x < 10; x++) {
            f0();
        }
    } catch (e) {
    }
}

console.log("pass");
