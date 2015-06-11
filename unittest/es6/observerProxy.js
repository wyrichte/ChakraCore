// JavaScript source code
var observeHandler = {};
var reflectHandler = {};
var handler;
var savedLogResult = new Array();
var print;
if (this.WScript) {
    print = WScript.Echo;
}
else {
    print = console.log;
}
function LogResult(text) {
    savedLogResult.push(text);
    print(text);
}

function VerifyThis(thisVar) {
    if (thisVar != observeHandler) {
        throw "invalid this pointer in trap function";
    }
}

function IsUsingReflect() {
    return handler == reflectHandler;
}

observeHandler.myName = 'observeHandler';
observeHandler.sourceBreak = Debug.sourceDebugBreak;

observeHandler.getOwnPropertyDescriptor = function (obj, key) {
    LogResult("getOwnPropertyDescriptor trap " + key.toString());
    VerifyThis(this);
    if (this.trapGetOwnPropertyDescriptor) {
        return this.trapGetOwnPropertyDescriptor(obj, key);
    }
    return Object.getOwnPropertyDescriptor(obj, key);
}

observeHandler.has = function (obj, key) {
    LogResult('has trap ' + typeof obj + '.' + key.toString());
    VerifyThis(this);
    if (this.trapHas) {
        return this.trapHas(obj, key);
    }
    return key in obj
};

observeHandler.getPrototypeOf = function (obj) {
    LogResult('getPrototypeOf trap');
    VerifyThis(this);
    if (this.trapGetPrototypeOf) {
        return this.trapGetPrototypeOf(obj);
    }
    return Object.getPrototypeOf(obj);
}

observeHandler.setPrototypeOf = function (obj, proto) {
    consle.log('setPrototypeOf trap');
    VerifyThis(this);
    if (this.traSetPrototypeOf) {
        return this.trapSetPrototypeOf(obj, proto);
    }
    obj.__proto__ = proto;
}

observeHandler.isExtensible = function (obj) {
    LogResult('isExtensible trap');
    VerifyThis(this);
    if (this.trapIsExtensible) {
        return this.trapIsExtensible(obj);
    }
    return Object.isExtensible(obj);
}

observeHandler.preventExtensions = function (obj) {
    LogResult('preventExtensions trap');
    VerifyThis(this);
    if (this.trapPreventExtensions) {
        return this.trapPreventExtensions(obj);
    }
    return Object.preventExtensions(obj);
}

observeHandler.defineProperty = function (obj, name, descObj) {
    LogResult('defineProperty trap ' + name.toString());
    VerifyThis(this);
    if (this.trapDefineProperty) {
        return this.trapDefineProperty(obj, name, descObj);
    }
    return Object.defineProperty(obj, name, descObj);
}

observeHandler.get = function (obj, name, receiver) {
    LogResult('get trap ' + name.toString() + " on " + typeof obj + " receiver " + typeof receiver);
    VerifyThis(this);
    if (this.trapGet) {
        return this.trapGet(obj, name, receiver);
    }
    return obj[name];
}

observeHandler.set = function (obj, name, value, receiver) {
    LogResult('set trap ' + name.toString());
    VerifyThis(this);
    if (this.trapSet) {
        return this.trapSet(obj, name, value, receiver);
    }
    obj[name] = value;
    return (obj[name] == value);
}

observeHandler.deleteProperty = function (obj, name) {
    LogResult('delete trap ' + name.toString());
    VerifyThis(this);
    if (this.trapDeleteProperty) {
        return this.trapDeleteProperty(obj, name);
    }
    return delete obj[name];
}

observeHandler.enumerate = function (obj) {
    LogResult('enumerate trap');
    VerifyThis(this);
    if (this.trapEnumerate) {
        return this.trapEnumerate(obj);
    }
    var arrayItems = [];
    var count = 0;
    for (var j in obj) {
        arrayItems[count++] = j;
    }
    print(arrayItems.length);
    return this.iteratorFromArray(arrayItems);
}

observeHandler.iteratorFromArray = function (arrayList) {
    var retVal = {};
    retVal._arrayItems = arrayList;
    retVal._current = -1;
    print('iterator array length ' + arrayList.length);
    retVal.next = function () {
        retVal._current++;
        var result;
        if (retVal._current >= retVal._arrayItems.length) {
            result = { done: true, value: undefined };
        }
        else {
            result = { done: false, value: retVal._arrayItems[retVal._current] };
        }
        return result;
    }
    return retVal;
};

observeHandler.ownKeys = function (obj) {
    LogResult('ownKey trap');
    VerifyThis(this);
    if (this.trapOwnKeys) {
        return this.trapOwnKeys(obj);
    }
    var arr = Object.getOwnPropertyNames(obj);
    var sym = Object.getOwnPropertySymbols(obj);
    for (var oneSym in sym)
    {
        arr.push(sym[oneSym]);
    }
    return arr;
}

observeHandler.apply = function (func, thisArg, args) {
    LogResult('call trap' + args.length);
    VerifyThis(this);
    if (this.trapApply) {
        return this.trapApply(func, thisArg, args);
    }
    return func.apply(thisArg, args);
}

observeHandler.construct = function (func, args) {
    LogResult('construct trap');
    VerifyThis(this);
    if (this.trapConstruct) {
        return this.trapConstruct(func, args);
    }
    return new func(...args);
}

function VerifyReflectThis(thisVar) {
    if (thisVar != reflectHandler) {
        throw "invalid this pointer in trap function";
    }
}

reflectHandler.getOwnPropertyDescriptor = function (obj, key) {
    LogResult("getOwnPropertyDescriptor trap " + key.toString());
    VerifyReflectThis(this);
    if (this.trapGetOwnPropertyDescriptor) {
        return this.trapGetOwnPropertyDescriptor(obj, key);
    }
    return Reflect.getOwnPropertyDescriptor(obj, key);
}

reflectHandler.has = function (obj, key) {
    LogResult('has trap ' + typeof obj + '.' + key.toString());
    VerifyReflectThis(this);
    if (this.trapHas) {
        return this.trapHas(obj, key);
    }
    return Reflect.has(obj, key);
};

reflectHandler.getPrototypeOf = function (obj) {
    LogResult('getPrototypeOf trap');
    VerifyReflectThis(this);
    if (this.trapGetPrototypeOf) {
        return this.trapGetPrototypeOf(obj);
    }
    return Reflect.getPrototypeOf(obj);
}

reflectHandler.setPrototypeOf = function (obj, proto) {
    LogResult('setPrototypeOf trap');
    VerifyReflectThis(this);
    if (this.traSetPrototypeOf) {
        return this.trapSetPrototypeOf(obj, proto);
    }
    Reflect.setPrototypeOf(obj, proto);
}

reflectHandler.isExtensible = function (obj) {
    LogResult('isExtensible trap');
    VerifyReflectThis(this);
    if (this.trapIsExtensible) {
        return this.trapIsExtensible(obj);
    }
    return Reflect.isExtensible(obj);
}

reflectHandler.preventExtensions = function (obj) {
    LogResult('preventExtensions trap');
    VerifyReflectThis(this);
    if (this.trapPreventExtensions) {
        return this.trapPreventExtensions(obj);
    }
    return Reflect.preventExtensions(obj);
}

reflectHandler.defineProperty = function (obj, name, descObj) {
    LogResult('defineProperty trap ' + name.toString());
    VerifyReflectThis(this);
    if (this.trapDefineProperty) {
        return this.trapDefineProperty(obj, name, descObj);
    }
    return Reflect.defineProperty(obj, name, descObj);
}

reflectHandler.get = function (obj, name, receiver) {
    LogResult('get trap ' + name.toString() + " on " + typeof obj + " receiver " + typeof receiver);
    VerifyReflectThis(this);
    if (this.trapGet) {
        return this.trapGet(obj, name, receiver);
    }
    return Reflect.get(obj, name, receiver);
}

reflectHandler.set = function (obj, name, value, receiver) {
    LogResult('set trap ' + name.toString());
    VerifyReflectThis(this);
    if (this.trapSet) {
        return this.trapSet(obj, name, value, receiver);
    }
    return Reflect.set(obj, name, value, receiver);
}

reflectHandler.deleteProperty = function (obj, name) {
    LogResult('delete trap ' + name.toString());
    VerifyReflectThis(this);
    if (this.trapDeleteProperty) {
        return this.trapDeleteProperty(obj, name);
    }
    return Reflect.deleteProperty(obj, name);
}

reflectHandler.enumerate = function (obj) {
    LogResult('enumerate trap');
    VerifyReflectThis(this);
    if (this.trapEnumerate) {
        return this.trapEnumerate(obj);
    }
    return Reflect.enumerate(obj);
}

reflectHandler.ownKeys = function (obj) {
    LogResult('ownKey trap');
    VerifyReflectThis(this);
    if (this.trapOwnKeys) {
        return this.trapOwnKeys(obj);
    }
    return Reflect.ownKeys(obj);
}

reflectHandler.apply = function (func, thisArg, args) {
    LogResult('call trap' + args.length);
    VerifyReflectThis(this);
    if (this.trapApply) {
        return this.trapApply(func, thisArg, args);
    }
    return Reflect.apply(func, thisArg, args);
}

reflectHandler.construct = function (func, args) {
    LogResult('construct trap');
    VerifyReflectThis(this);
    if (this.trapConstruct) {
        return this.trapConstruct(func, args);
    }
    return Reflect.construct(func, args);
}


var target;
var observerProxy;
function initialize(style)
{
    target = function () {
        print('target func');
        this.myName = "proxy target";
        return this;
    }
    if (style == 'Reflect') {
        observerProxy = new Proxy(target, reflectHandler);
        handler = reflectHandler;
    }
    else {
        observerProxy = new Proxy(target, observeHandler);
        handler = observeHandler;
    }
}