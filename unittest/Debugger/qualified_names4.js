// Validation of defineproperty(ies) names against the debugger.

var a = 10;
var foo1 = { };
Object.defineProperty(foo1, "bar", { enumerable: true, configurable: true, 
                                     get: function() { 
                                            a++;
                                            return a;       /**bp:stack()**/
                                        },
                                     set: function () {
                                            a++;        /**bp:stack()**/
                                        }                                         
                                    });
k = foo1.bar;
foo1.bar = 31;

Object.defineProperties(foo1, {
  "property1": {
  enumerable: true,
    get: function() { 
        a++;
        return a;       /**bp:stack()**/
    },
     set: function () {
        a++;        /**bp:stack()**/
    }                                         
  },
  "property2": {
   enumerable: true,
    get: function() { 
        a++;
        return a;       /**bp:stack()**/
    },
     set: function () {
        a++;        /**bp:stack()**/
    }                                         
  }
});

var k = foo1.property1;
foo1.property1 = 10;
k = foo1.property2;
foo1.property2 = 32;

function TrimStackTracePath(line) {
    return line && line.replace(/\(.+unittest.debugger./ig, "(");
}

// Validating the error.stack.
Object.defineProperty(foo1, "genError", { enumerable: true, configurable: true, 
                                     get: function() { 
                                            try{
                                                abc.def = 10;
                                            }
                                            catch(e) {
                                                WScript.Echo(TrimStackTracePath(e.stack));
                                            }
                                            return 10;
                                        }
                                    });
k = foo1.genError;

function anotherNameFunc() {
    a++; /**bp:stack()**/
    return 10;  
}

Object.defineProperty(foo1, "bar2", {get : anotherNameFunc,
                                    set : function setFunction(newValue){ 
                                        newValue;
                                        bValue = newValue; /**bp:stack()**/
                                        bValue++;
                               }});


k = foo1.bar2;
foo1.bar2 = 20;

