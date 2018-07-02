var tests = [
    {
        name: "Non-enumerable DOM objects are ignored",
        body: function () {
            let DOMobj = document.getElementById("body");
            let obj = {...DOMobj};

            assert.areEqual(0, Object.keys(obj).length);
        }
    },
    {
        name: "Enumerable DOM objects can be spread",
        body: function () {
            let DOMobj = document.getElementById("top").childNodes;
            let obj = {...DOMobj};

            assert.areEqual(3, Object.keys(obj).length);
            assert.strictEqual(DOMobj[0], obj[0]);
            assert.strictEqual(DOMobj[1], obj[1]);
            assert.strictEqual(DOMobj[2], obj[2]);

            DOMobj = document.getElementById("firstdiv").dataset;
            obj = {...DOMobj};

            assert.areEqual(3, Object.keys(obj).length);
            assert.areEqual("hi", obj.hello);
            assert.areEqual("world", obj.world);
            assert.areEqual("foo", obj.foo);

            DOMobj = document.getElementById("firstdiv").attributes;
            obj = {...DOMobj};

            assert.areEqual(4, Object.keys(obj).length);
            assert.strictEqual(DOMobj[0], obj[0]);
            assert.strictEqual(DOMobj[1], obj[1]);
            assert.strictEqual(DOMobj[2], obj[2]);
            assert.strictEqual(DOMobj[3], obj[3]);
        }
    },
];

testRunner.runTests(tests);