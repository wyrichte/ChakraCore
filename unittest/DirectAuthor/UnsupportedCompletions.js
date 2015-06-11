// Spported Completions
|var my_function2 =| function(){|    
    var object2 =| 22;
    var x12, x22,x32=3,x42, x52=5;|

    (|function(p1, p2, p3) {|

        }|())

    try {|
       x12 = 1;|
    } catch (error){|
        return error;
    }|
};



// Unspported Completions
var m|y_function| = function(|){    
    var o|bject| ={| };
    var x1|,| x2|,x3=3,|x4, x5=5;

    (function(|p|1, p2|, p3 /* comments */|) {

        }())

    try {
       x1 = 1;
    } catch (|error|)|
//Comment
|{
        return error;
    }
};

var y1, y2| /* comment */| ,| y3|;