
function createForwardingHandler() {
        return {
            get: function (target, name, receiver) { return Reflect.get(target, name, receiver); },
            set: function (target, name, value, receiver) { return Reflect.set(target, name, value, receiver); },
            has: function (target, name) { return Reflect.has(target, name); },
            apply: function (target, receiver, args) { return Reflect.apply(target, receiver, args); },
            construct: function (target, args) { return Reflect.construct(target, args); },
            getOwnPropertyDescriptor: function (target, name) { return Reflect.getOwnPropertyDescriptor(target, name); },
            defineProperty: function (target, name, desc) { return Reflect.defineProperty(target, name, desc); },
            getPrototypeOf: function (target) { return Reflect.getPrototypeOf(target); },
            setPrototypeOf: function (target, newProto) { return Reflect.setPrototypeOf(target, newProto); },
            deleteProperty: function (target, name) { return Reflect.deleteProperty(target, name); },
            enumerate: function (target) { return Reflect.enumerate(target); },
            preventExtensions: function (target) { return Reflect.preventExtensions(target); },
            isExtensible: function (target) { return Reflect.isExtensible(target); },
            ownKeys: function (target) { return Reflect.ownKeys(target); }
        };
    };

handler = createForwardingHandler();


Debug.setAutoProxyName("handler");
WScript.LoadScriptFile("stringify-replacerfunc.js", "self");
