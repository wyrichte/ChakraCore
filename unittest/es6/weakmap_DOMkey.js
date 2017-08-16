/// <reference path="../../core/test/UnitTestFramework/UnitTestFramework.js" />

var tests = {
    test01: {
        name: "WeakMap should allow FastDOM objects as keys",
        body: function () {
            var weakmap = new WeakMap();

            var fd = document.getElementById('foo');

            weakmap.set(fd, 1);

            assert.isTrue(weakmap.has(fd), "weakmap has FastDOM object fd as a key");
            assert.isTrue(weakmap.get(fd) === 1, "weakmap maps FastDOM object fd to 1");
            assert.isTrue(weakmap.delete(fd), "delete FastDOM object fd from weakmap");

            assert.isFalse(weakmap.has(fd), "weakmap no longer has FastDOM object fd as a key");
            assert.isFalse(weakmap.delete(fd), "cannot delete FastDOM object fd from weakmap because it is not a key anymore");
        }
    },

    test02: {
        name: "WeakMap should allow HostDispatch objects as keys",
        body: function () {
            var weakmap = new WeakMap();

            var hd = window.location;

            weakmap.set(hd, 1);

            assert.isTrue(weakmap.has(hd), "weakmap has HostDispatch object hd as a key");
            assert.isTrue(weakmap.get(hd) === 1, "weakmap maps HostDispatch object hd to 1");
            assert.isTrue(weakmap.delete(hd), "delete HostDispatch object hd from weakmap");

            assert.isFalse(weakmap.has(hd), "weakmap no longer has HostDispatch object hd as a key");
            assert.isFalse(weakmap.delete(hd), "cannot delete HostDispatch object hd from weakmap because it is not a key anymore");
        }
    },
};

testRunner.run(tests);
