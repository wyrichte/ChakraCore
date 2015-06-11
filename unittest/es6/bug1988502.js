let arr = [3, 5, 7];
let copy = WScript.CopyOnWrite(arr);
let result = [];
for (var i of copy) {
    result.push(i);
}

if (result.length === 3 && result[0] === 3 && result[1] === 5 && result[2] === 7) {
    WScript.Echo("passed");
} else {
    WScript.Echo("failed");
}
