//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// Testing the functions are implemented in the script code in the runtime.

function top()
{
    var wm = new WeakMap();

    function foo()
    {
        var a = {};
        wm.set(a,1);
    } 
    foo();

    var col = new Intl.Collator("en-US");
    Intl.__proto__ = {a:1};

    var map = new Map();
    var set = new Set();
    set.add(map);
    map.set(set,wm);
    map;                             /**bp:locals(1);**/
    anotherExample();
}
function anotherExample()
{
var a = [];
a.push(1);
a.forEach(function () {
    a; /**bp:stack()**/
});

var coll = Intl.Collator();
var a = {
    toString: function () {
        return "a"; /**bp:stack()**/
    }
}
coll.compare(a, "b");

}

WScript.Attach(top);
WScript.Echo("Pass");