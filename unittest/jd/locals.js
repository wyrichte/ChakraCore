// test locals

// Test reg slot locals
(function f1(){

    var n = null;
    var u = undefined;
    var a;
    
    /**bp:locals(2, LOCALS_TYPE)**/
}).apply({});

// Test slot array locals
(function f2(){
    var n;
    var u;
    var n1 = null;
    var n2;
    
    (function(){
        n = null;
        u = undefined;
    })();
    
    /**bp:locals(2, LOCALS_TYPE)**/
}).apply({});

// Test activation object locals
(function f3(){
    var n = null;
    var u = undefined;

    eval("");

    /**bp:locals(2, LOCALS_TYPE)**/
}).apply({});

WScript.Echo("pass");