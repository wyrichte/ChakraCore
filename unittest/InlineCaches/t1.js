/*
  This crashes if GetProperty doesn't correctly invalidate typeWithoutProperty inline-cache field.
*/

var a = { w: 1 };
var b = { w: 2 };

a.id = 1;
a.name = 2;
x = a.id;                // if this only updates local.type and not typeWithoutProperty we end up with incorrect cached transition
b.id = 3;
y = b.name || 2;

WScript.Echo("ok");