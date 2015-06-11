var HETest = {};

HETest.obj = {};

HETest.obj['string key'] = 'string value';
HETest.obj[Symbol.iterator] = 500;
HETest.obj[Symbol('my symbol')] = function() { return 100; }
HETest.obj[Symbol()] = undefined;
HETest.obj[Symbol()] = 'blank symbol description';
HETest.obj['a symbol'] = Symbol('symbol description');
HETest.obj[Symbol('symbol as key')] = Symbol('symbol as value');
HETest.obj['a symbol object'] = Object(Symbol('symbol object value'));

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);