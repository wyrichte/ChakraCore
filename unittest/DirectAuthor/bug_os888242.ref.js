function createForwardingHandler(obj) {
        return {
            get: function (target, name, receiver) { return Reflect.get(obj, name, receiver); },
            set: function (target, name, value, receiver) { return Reflect.set(obj, name, value, receiver); },
            has: function (target, name) { return Reflect.has(obj, name); },
            apply: function (target, receiver, args) { return Reflect.apply(obj, receiver, args); },
            construct: function (target, args) { return Reflect.construct(obj, args); },
            getOwnPropertyDescriptor: function (target, name) { return Reflect.getOwnPropertyDescriptor(obj, name); },
            defineProperty: function (target, name, desc) { return Reflect.defineProperty(obj, name, desc); },
            getPrototypeOf: function (target) { return Reflect.getPrototypeOf(obj); },
            setPrototypeOf: function (target, newProto) { return Reflect.setPrototypeOf(obj, newProto); },
            deleteProperty: function (target, name) { return Reflect.deleteProperty(obj, name); },
            enumerate: function (target) { return Reflect.enumerate(obj); },
            preventExtensions: function (target) { return Reflect.preventExtensions(obj); },
            isExtensible: function (target) { return Reflect.isExtensible(obj); },
            ownKeys: function (target) { return Reflect.ownKeys(obj); }
        };
    };
var o = { foo : "Bar" };    
var x = new Proxy({}, createForwardingHandler(o));