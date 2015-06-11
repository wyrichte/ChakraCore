function transfer(a) {
    SCA.deserialize(SCA.serialize(null, { context: "samethread" }, undefined, [a]));
}

var transferBuffer = null;
var transferrer = {
    valueOf: function() {
        transfer(transferBuffer);
        return 0;
    }
};

var nullTransferrers = [0, "0"];

function runTest(test) {
    WScript.Echo(test.name + " - " + test.description);
    for(var i = 0; i < 3; ++i) {
        var a = new Int32Array(2);
        a[0] = 1;
        a[1] = 2;
        try {
            WScript.Echo(test(a, i));
        }
        catch(ex) {
            WScript.Echo(ex.name + ": " + ex.message);
        }
    }
    WScript.Echo();
}

function test0(a) {
    var x = a[0];
    transfer(a.buffer);
    var y = a[1];
    return [x, y];
}
test0.description = "Array segment length load elimination";
runTest(test0);

function test1(a) {
    var x = a[0];
    transfer(a.buffer);
    var y = a[0];
    return [x, y];
}
test1.description = "Array segment length load elimination, CSE";
runTest(test1);

function test2(a) {
    var x = 0;
    for(var j = 0; j < 2; ++j) {
        x += a[j];
        transfer(a.buffer);
    }
    var y = a[1];
    return [x, y];
}
test2.description = "Array segment length hoisting";
runTest(test2);

function test3(a) {
    var x = a[0];
    transfer(a.buffer);
    var y = a[1];
    return [x, y];
}
test3.description = "Call, array segment length load elimination";
runTest(test3);

function test4(a) {
    var x = a[0];
    transfer(a.buffer);
    var y = a[0];
    return [x, y];
}
test4.description = "Call, array segment length load elimination, CSE";
runTest(test4);

function test5(a) {
    var x = 0;
    for(var j = 0; j < 2; ++j) {
        x += a[j];
        transfer(a.buffer);
    }
    var y = a[1];
    return [x, y];
}
test5.description = "Call, array segment length hoisting";
runTest(test5);

function test6(a, i) {
    var x = a[0];
    transferBuffer = a.buffer;
    x += +transferrer;
    var y = a[1];
    return [x, y];
}
test6.description = "Known implicit call, array segment length load elimination";
runTest(test6);

function test7(a, i) {
    var x = a[0];
    transferBuffer = a.buffer;
    x += +transferrer;
    var y = a[0];
    return [x, y];
}
test7.description = "Known implicit call, array segment length load elimination, CSE";
runTest(test7);

function test8(a, i) {
    var x = 0;
    for(var j = 0; j < 2; ++j) {
        x += a[j];
        transferBuffer = a.buffer;
        x += +transferrer;
    }
    var y = a[1];
    return [x, y];
}
test8.description = "Known implicit call, array segment length hoisting";
runTest(test8);

function test9(a, i) {
    var x = a[0];
    transferBuffer = a.buffer;
    x += i < 2 ? nullTransferrers[i] : transferrer;
    var y = a[1];
    return [x, y];
}
test9.description = "Unknown implicit call, array segment length load elimination";
runTest(test9);

function test10(a, i) {
    var x = a[0];
    transferBuffer = a.buffer;
    x += i < 2 ? nullTransferrers[i] : transferrer;
    var y = a[0];
    return [x, y];
}
test10.description = "Unknown implicit call, array segment length load elimination, CSE";
runTest(test10);

function test11(a, i) {
    var x = 0;
    for(var j = 0; j < 2; ++j) {
        x += a[j];
        transferBuffer = a.buffer;
        x += i < 2 ? nullTransferrers[i] : transferrer;
    }
    var y = a[1];
    return [x, y];
}
test11.description = "Unknown implicit call, array segment length hoisting";
runTest(test11);
