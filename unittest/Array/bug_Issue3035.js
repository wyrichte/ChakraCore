//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

var objectSize = 16*1024*1024; // create 16MB Object
var result = "";

function testLeak(leakFunc, idx) {
	var arr = [];
	for(var i = 0; i < 100; i++) {
		arr[i] = [];
        leakFunc(arr[i]);
	}

    CollectGarbage();

    var ws = WScript.GetWorkingSet();
    if (ws.workingSet > 300000000) {
        result += idx + "\n" + JSON.stringify(ws) + "\n";
    }
}

var tests = [
    (arr)=>{
        arr[0] = new ArrayBuffer(objectSize);
        arr.splice(0, 1);
    },
    (arr)=>{
        arr[0] = new ArrayBuffer(objectSize);
        arr[4] = 0;
        arr.splice(0);
    },
    (arr)=>{
        arr[0] = new ArrayBuffer(objectSize);
        arr[8] = 0;
        arr[-9] = 0;
        arr.splice(-9);
    },
    (arr)=>{
        arr[3] = new ArrayBuffer(objectSize);
        arr[5] = 0;
        arr.shift();
        arr.shift();
        arr.shift();
        arr.shift();
    },
    (arr)=>{
        arr[3] = new ArrayBuffer(objectSize);
        arr[5] = 0;
        arr.reverse();
        arr.shift();
        arr.shift();
        arr.shift();
    },
    (arr)=>{
        arr[0] = new ArrayBuffer(objectSize);
        arr.unshift(0, 1, 2, 3);
        arr[4] = null;
    },
];

tests.forEach(testLeak);
if (result === "") {
    console.log("Pass");
} else {
    console.log(result);
}
