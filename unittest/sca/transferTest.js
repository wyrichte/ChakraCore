function serialize(rootObject, transferArgs) {
    var toReturn = SCA.serialize(rootObject, { context: "samethread" } , undefined, transferArgs);

    transferArgs.forEach(function (arg) {
        try {
            if (arg.byteLength === 'undefined' || arg.byteLength !== 0) {
                WScript.Echo('ArrayBuffer.byteLength should be 0 since it is detached.');
            }
        } catch (ex) 
        {
            WScript.Echo("Not expected to throw while trying to access ArrayBuffer.prototype.byteLength of detached buffer." + arg.byteLength);
        }
    });

    return toReturn;
}

function deserialize(data) {
    return SCA.deserialize(data);
}

function doTest(root, transferArgs) {
    function e(s) { return eval(s); }
    //I am evaling over the transferArgs, in order to do a simple "path" lookup based on root variable.
    var toPass = transferArgs.map(e);
    var lengths = toPass.map(function (arg) { return arg.byteLength; });
    root = deserialize(serialize(root, toPass));
    var received = transferArgs.map(e);
    for (var i = 0; i < received.length; i++) {
        if (lengths[i] != received[i].byteLength) {
            throw new Error("Returning buffer lengths don't match.");
        }
    }
    return root;
}


var obj = { test: 1, array: new Uint8Array(10) };

var returned = doTest(obj, ["root.array.buffer"]);

try {
    if (obj.array.length === 'undefined' || obj.array.length !== 0) {
        WScript.Echo('TypedArray.prototype.length should be 0 since it is detached.');
    }
} catch (ex) {
    WScript.Echo("Not expected to throw while trying to access TypedArray.prototype.length of detached buffer." + obj.array.length);
}


doTest(obj = { array: new Uint8Array(returned.array.buffer), arrayReturned: returned.array }, ["root.array.buffer"]);
try {
    if (obj.array.length === 'undefined' || obj.array.length !== 0) {
        WScript.Echo('TypedArray.prototype.length should be 0 since it is detached.');
    }
    if (obj.arrayReturned.length === 'undefined' || obj.arrayReturned.length !== 0) {
        WScript.Echo('TypedArray.prototype.length should be 0 since it is detached.');
    }
} catch (ex) {
    WScript.Echo("Not expected to throw while trying to access TypedArray.prototype.length of detached buffer." + obj.array.length);
    WScript.Echo("Not expected to throw while trying to access TypedArray.prototype.length of detached buffer." + obj.arrayReturned.length);
}
try{
    if (obj.array.byteOffset === 'undefined' || obj.array.byteOffset !== 0) {
        WScript.Echo('TypedArray.prototype.byteOffset should be 0 since it is detached.');
    }
    if (obj.arrayReturned.byteOffset === 'undefined' || obj.arrayReturned.byteOffset !== 0) {
        WScript.Echo('TypedArray.prototype.byteOffset should be 0 since it is detached.');
    }
} catch (ex) {
    WScript.Echo("Not expected to throw while trying to access TypedArray.prototype.length of detached buffer." + obj.array.byteOffset);
    WScript.Echo("Not expected to throw while trying to access TypedArray.prototype.length of detached buffer." + obj.arrayReturned.byteOffset);
}

doTest(null, ["new Uint8Array(10).buffer"]);
doTest(null, ["new DataView(new ArrayBuffer(100)).buffer"]);

// Bug# 1369156 repro scenario
var projectionArray = { test: 1, array: new Debug.createProjectionArrayBuffer(1000)};
var projectionArrayReturned = doTest(projectionArray, ["root.array"]);

CollectGarbage();
CollectGarbage();

//Here we got a couple of security-based tests
//Goal is to start an operation on a typed array and get back to usercode and neuter the typed array

function neuter(val) {
    if (val.isNeutered) {
        return;
    }

    val.isNeutered = true;
    deserialize(serialize(null, [val]));
}

function normalV(val) {
    return function () {
        return val;
    }
}

function neuteringV(val) {
    return function (array) {
        return {
            valueOf: function () {
                neuter(array.buffer);
                return val;
            }
        };
    };
}

function performTest(func) {
    try{
        var array = new Uint8Array(100);
        var args = [];
        if (arguments.length > 1) {
            for (var i = 1; i < arguments.length; i++) {
                args.push(arguments[i](array));
            }
        }
        func.apply(array, args)
    } catch(ex) {
        WScript.Echo(ex);
    }
}
function neuterTests() {
    performTest(Uint8Array.prototype.set, neuteringV(5), neuteringV(10));
    performTest(Uint8Array.prototype.set, neuteringV(obj.array.buffer) /*neutered*/, neuteringV(10));
    performTest(Uint8Array.prototype.subarray, neuteringV(5), neuteringV(10));
    performTest(Uint8Array.prototype.subarray, neuteringV(5), normalV(10));
}

//interpreted
neuterTests();
//jitted 
neuterTests();

// Reenable with OSG:VSO 598499: SCA Neutering and ES6 TypedArray Built-Ins
//if (Uint8Array.prototype.find !== undefined || Array.prototype.find) {
//    WScript.Echo("We most likely merged with a branch that has ES6 changes, should expand on this testing. Ping anborod and sanyamc.");
//}