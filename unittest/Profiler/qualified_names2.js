// Validation of defineproperty(ies) full name against the script profiler.

var a = 10;
var foo1 = { };
Object.defineProperty(foo1, "bar", { enumerable: true, configurable: true, 
                                     get: function() { 
                                            a++;
                                            return a;
                                        },
                                     set: function () {
                                            a++;
                                        }                                         
                                    });
k = foo1.bar;
foo1.bar = 31;

Object.defineProperties(foo1, {
  "property1": {
  enumerable: true,
    get: function() { 
        a++;
        return a;
    },
     set: function () {
        a++;     
    }                                         
  },
  "property2": {
   enumerable: true,
    get: function() { 
        a++;
        return a;
    },
     set: function () {
        a++;     
    }                                         
  }
});

var k = foo1.property1;
foo1.property1 = 10;
k = foo1.property2;
foo1.property2 = 32;

function temp() {}
WScript.StartProfiling(temp);