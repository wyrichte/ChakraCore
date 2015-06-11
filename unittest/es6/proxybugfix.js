var write = WScript.Echo;

write("Test case 1");

var x = { hello: "World" };

function obj()
{
    return x;
}

function createForwardingHandler(obj) {
        return {
            get: function (target, name, receiver) { return Reflect.get(obj(), name, receiver); },
            set: function (target, name, value, receiver) { return Reflect.set(obj(), name, value, receiver); },
            has: function (target, name) { return Reflect.has(obj(), name); },
            apply: function (target, receiver, args) { return Reflect.apply(obj(), receiver, args); },
            construct: function (target, args) { return Reflect.construct(obj(), args); },
            getOwnPropertyDescriptor: function (target, name) { return Reflect.getOwnPropertyDescriptor(obj(), name); },
            defineProperty: function (target, name, desc) { return Reflect.defineProperty(obj(), name, desc); },
            getPrototypeOf: function (target) { return Reflect.getPrototypeOf(obj()); },
            setPrototypeOf: function (target, newProto) { return Reflect.setPrototypeOf(obj(), newProto); },
            deleteProperty: function (target, name) { return Reflect.deleteProperty(obj(), name); },
            enumerate: function (target) { return Reflect.enumerate(obj()); },
            preventExtensions: function (target) { return Reflect.preventExtensions(obj()); },
            isExtensible: function (target) { return Reflect.isExtensible(obj()); },
            ownKeys: function (target) { return Reflect.ownKeys(obj()); }
        };
    };

handler = createForwardingHandler(obj);
var y = new Proxy({}, handler);
write(JSON.stringify(y));

write("Test case 2");
function test0() {
  var ary = new Array();
  ary[2] = 1;
  Object.defineProperty(Array.prototype, '1', {
      get: function () {
          return 100;
    }
  });
  ary.shift();
  WScript.Echo(ary[1]);
}

var ProtoObjFactory = function (p) {
    var bar = function () { };
    var newObj = new bar();
    newObj.__proto__ = p; return newObj;
};
var EquivObjFactory = function (p) {
    var newObj = Object(p); return newObj;
};
function SumOfProps(obj) {
    var sum = 0;
    for (var p in obj) {
        sum = obj;
    }
    return sum;
}
function test1() {
    var protoObj0 = {};
    var func0 = function () { };
    var protoObj0 = ProtoObjFactory(protoObj0);
    var protoObj1 = ProtoObjFactory(protoObj0);
    var eqObj5 = EquivObjFactory(protoObj0);
    protoObj1[1] = 1;
    WScript.Echo(SumOfProps(eqObj5));
}

Debug.setAutoProxyName(test0);
test0();
test1();
