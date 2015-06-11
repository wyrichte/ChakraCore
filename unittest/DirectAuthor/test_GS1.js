
var obj = {
    x1:10
    };
var test1Val;
Object.defineProperty(obj, "test1", {get : function(){ return test1Val; },
                               set : function(newValue){ test1Val = newValue; },
                              enumerable : true,
                               configurable : true});

var F1 = function () {
    this.x2 = 10;
}
var test2Val;
Object.defineProperty(F1, "test2", {get : function(){ return test2Val; },
                               set : function(newValue){ test2Val = newValue; },
                              enumerable : true,
                              configurable: true
});

intellisense.MyFooBar = function (arg1, arg2) {
}

/**gs:**/
