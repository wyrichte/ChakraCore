//
// SCA tests for Map and Set objects
//
var tests_mapset = [
    { root: new Map() },
    { root: new Set() },
    {
        get: function () {
            var root = new Map();
            root.set(1, 2);
            root.set("a", "b");
            root.set({ }, { });
            return root;
        }
    },
    {
        get: function () {
            var root = new Set();
            root.add(1);
            root.add("a");
            root.add({ });
            return root;
        }
    },
    {
        get: function () {
            var root = new Map();
            root.a = { };
            root.b = { x: 5 };
            root.c = 10;
            root.set(root.a, root.b);
            root.set(root.b, root.a);
            root.set(root, root.c);
            root.set(root.b.x, root);
            return root;
        }
    },
    {
        get: function () {
            var root = new Set();
            root.a = { };
            root.b = { x: 5 };
            root.c = 10;
            root.add(root.a);
            root.add(root.b);
            root.add(root);
            root.add(root.b.x);
            return root;
        }
    },
];

