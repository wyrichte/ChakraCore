/**ref:..\\..\\Lib\\Author\\References\\libhelp.js**/

Boolean.prototype.foo = 20;
var b = {};
Reflect.deleteProperty({},'a')./**ml:foo**/;
Reflect.has({}, 'a')./**ml:foo**/;
Reflect.isExtensible({})./**ml:foo**/;
Reflect.set({}, 'a', 20)./**ml:foo**/;
Reflect.preventExtensions(b)./**ml:foo**/;

var obj = {a:20};
Reflect.setPrototypeOf(obj, Object.prototype)./**ml:foo**/;
Reflect.defineProperty(obj, 'b', {value: 2})./**ml:foo**/;
