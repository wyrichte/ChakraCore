var HETest = {};

Object.defineProperty(HETest, "testprop", { enumerable: true, configurable: true, set: function () { }, get: function () { return 'string literal (testprop)'; } });
Object.defineProperty(HETest, "100", { enumerable: true, configurable: true, set: function () { }, get: function () { return 'string literal (100)'; } });
Object.defineProperty(HETest, "200", { enumerable: true, configurable: true, value: "string literal (200)" });
Object.defineProperty(HETest, "300", { enumerable: true, configurable: true, set: function () { }, get: function () { return 'string literal (300)'; } });
Object.defineProperty(HETest, "400", { enumerable: true, configurable: true, get: function () { return 'string literal (400)'; } });
Object.defineProperty(HETest, "testprop2", { enumerable: true, configurable: true, set: function () { }, get: function () { return 'string literal (testprop2)'; } });
Object.defineProperty(HETest, "500", { enumerable: true, configurable: true, set: function () { } });
Object.defineProperty(HETest, "600", { enumerable: true, configurable: true, set: undefined, get: function() { return 'string literal (600)'; } });

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);
