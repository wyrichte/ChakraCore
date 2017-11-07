(function() {
    var a = [1, 2];
    for (var i = 0; i < 100 * 1024; i++) {
        a.push(i);
    }
    delete a[0]; // Make a missing item
    var protoObj = [11];
    Object.defineProperty(protoObj, '0', {
        get : function () {
            Object.setPrototypeOf(a, Array.prototype);
            a.splice(0);        // head seg is now length=0
            return 42;
        },
        configurable : true
    });
    Object.setPrototypeOf(a, protoObj);
    var b = a.slice(); // This will invoke getter at '0'

})();

(function() {
    var a = [1, 2];
    for (var i = 0; i < 100 * 1024; i++) {
        a.push(i);
    }
    delete a[0]; // Make a missing item
    var protoObj = [11];
    Object.defineProperty(protoObj, '0', {
        get : function () {
            Object.setPrototypeOf(a, Array.prototype);
            a.splice(0);        // head seg is now length=0
            a.length = 100000;  // increase the length of the array to 100000 so to access head segment
            return 42;
        },
        configurable : true
    });
    Object.setPrototypeOf(a, protoObj);
    var b = a.slice(); // This will invoke getter at '0'

})();

print("pass");

