function write(v) { WScript.Echo(v + ""); }

function PrintDescriptor(name, propDesc) {
    write(name + ":configurable : " + propDesc.configurable);
    write(name + ":enumerable   : " + propDesc.enumerable);
    write(name + ":writable     : " + propDesc.writable);    
    write(name + ":getter       : " + propDesc.get);
    write(name + ":setter       : " + propDesc.set);    
    write(name + ":value        : " + propDesc.value);
}

function exceptToString(ee) {
    if (ee instanceof TypeError) return "TypeError";
    if (ee instanceof ReferenceError) return "ReferenceError";
    if (ee instanceof EvalError) return "EvalError";
    if (ee instanceof SyntaxError) return "SyntaxError";
    return "Unknown Error";
}

(function Test1() {
    "use strict";
    var str = "function.caller get";
        
    try {
        Test1.caller;
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

(function Test2() {
    "use strict";
    var str = "function.caller set";
        
    try {
        Test2.caller = 10;
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

(function Test3() {
    "use strict";
    var str = "function.arguments get";
        
    try {
        Test3.arguments;
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

(function Test4() {
    "use strict";
    var str = "function.arguments set";
        
    try {
        Test4.arguments = 10;
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

(function Test5() {
    "use strict";
    var str = "function.arguments.get/set and funciton.caller.get/set are equal/strictEqual to each other";

    // Properties on the function object. 
    var callerGet = Object.getOwnPropertyDescriptor(function() {}, 'caller').get;
    var callerSet = Object.getOwnPropertyDescriptor(function() {}, 'caller').set;
    var argumentsGet = Object.getOwnPropertyDescriptor(function() {}, 'arguments').get;
    var argumentsSet = Object.getOwnPropertyDescriptor(function() {}, 'arguments').set;

    // Properties on the arguments object. 
    var argumentsCallerGet = Object.getOwnPropertyDescriptor(arguments, 'caller').get;
    var argumentsCallerSet = Object.getOwnPropertyDescriptor(arguments, 'caller').set;
    var argumentsCalleeGet = Object.getOwnPropertyDescriptor(arguments, 'callee').get;
    var argumentsCalleeSet = Object.getOwnPropertyDescriptor(arguments, 'callee').set;
        
    write("Return: " + 
      (callerGet == argumentsGet && callerSet == argumentsSet && callerGet == callerSet &&
       argumentsCallerGet == argumentsCalleeGet && argumentsCallerSet == argumentsCalleeSet && 
       argumentsCallerGet == argumentsCallerSet && callerGet == argumentsCallerGet).toString() + " " +
      (callerGet === argumentsGet && callerSet === argumentsSet && callerGet === callerSet &&
       argumentsCallerGet === argumentsCalleeGet && argumentsCallerSet === argumentsCalleeSet && 
       argumentsCallerGet === argumentsCallerSet && callerGet === argumentsCallerGet).toString() + ": " +
      str);
})();

(function Test6() {
    var str = "funciton.caller's value is a strict mode function";

    function foo() {
        "use strict";
        return goo();
    }

    function goo() {
        return goo.caller;  // Expected: TypeError, as the caller (foo) is a strict mode function -- ES5.15.3.5.4.
    }

    try {
        foo();
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

/* TODO
(function Test5() {
    "use strict";
    var str = "function.caller get";
        
    try {
        PrintDescriptor("Test5.caller", Object.getOwnPropertyDescriptor(Test5, "caller"));
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();

(function Test6() {
    "use strict";
    var str = "function.arguments get";
        
    try {
        PrintDescriptor("Test6.arguments", Object.getOwnPropertyDescriptor(Test6, "arguments"));
    } catch (e) {
        write("Exception: " + str + " " + exceptToString(e));
        return;
    }
    write("Return: " + str);
})();
*/
