// SCA validation for SharedArrayBuffer

this.WScript.LoadScriptFile("..\\..\\core\\test\\UnitTestFramework\\UnitTestFramework.js");

function serialize(rootObject, transferArgs) {
    
    var toReturn = SCA.serialize(rootObject, { context: "samethread" } , undefined, transferArgs);

    transferArgs.forEach(function (arg) {
        if (arg instanceof SharedArrayBuffer) {
            assert.isTrue(arg.byteLength > 0);
        }
    });

    return toReturn;
}

function postMessage(rootObject, transferArgs) {
    var blob = serialize(rootObject, transferArgs);
    return SCA.deserialize(blob);
}

var tests = [
    {
        name: "Validate that SharedArrayBuffer should not be in transfer list",
        body: function () {
            var view1 = new Int8Array(new SharedArrayBuffer(4));
            try { postMessage(view1.buffer, [view1.buffer]); }
            catch (e) { print(e.number)}
        }
    },
    {
        name: "Validate that SharedArrayBuffer sharing functionality",
        body: function() {
              var view1 = new Int8Array(new SharedArrayBuffer(4));
              assert.areEqual(view1.byteLength, 4, "Ensuring that the view's length is 4 before postMessage");
              var buff = postMessage(view1.buffer, []);
              assert.isTrue(buff instanceof SharedArrayBuffer, "The SharedArrayBuffer will be received");
              var view2 = new Int8Array(buff);
              assert.areEqual(view1.byteLength, view2.byteLength, "The buffer is shared so both views length should be same");
              assert.areEqual(view1.buffer, view2.buffer, "The buffer is shared so both views buffer should be same");
              Atomics.store(view1, 1, 20);
              assert.areEqual(Atomics.load(view1, 1), Atomics.load(view2, 1), "Changing the value on the buffer should reflect on both views");
        }
    },
    {
        name: "Mix bag of transferring and sharing buffer",
        body: function() {
              var a1 = new Int8Array(new SharedArrayBuffer(4));
              var a2 = new Int8Array(new ArrayBuffer(4));
              var a3 = new Int8Array(new SharedArrayBuffer(4));
              var a4 = new Int32Array(a3.buffer);
              a1[0] = 10; 
              a3[0] = 20;
              var buffArray = postMessage([a1.buffer, a2.buffer, a3.buffer], [a2.buffer]);

              var b1 = new Int8Array(buffArray[0]);
              var b2 = new Int8Array(buffArray[1]);
              var b3 = new Int8Array(buffArray[2]);
              assert.areEqual(Atomics.load(b1, 0), 10, "The first buffer is shared so the views before and after are same");
              assert.areEqual(Atomics.load(b3, 0), 20, "The third buffer is shared so the views before and after are same");
              Atomics.add(b1, 0, 30);
              Atomics.add(b3, 0, 30);
              assert.areEqual(Atomics.load(a1, 0), 40, "Changing value in b1 should reflect on the a1");
              assert.areEqual(Atomics.load(a3, 0), 50, "Changing value in b3 should reflect on the a3");
              assert.areEqual(Atomics.load(a4, 0), 50, "a3 and a4 are sharing the same buffer so a4 also have 50");
              assert.areEqual(a2.length, 0, "second buffer is cloned so a1 is detached");
              assert.areEqual(b2.length, 4, "Ensuring that the buffer is transferred correctly");
        }
    },
];


testRunner.runTests(tests, {
	verbose : WScript.Arguments[0] != "summary"
});
