var b1 = Boolean(true);
b1.__defineSetter__("something", function() {});

var b2 = Boolean(true);
b2.__defineGetter__("something else", function() {});

// Above shouldn't cause AV
WScript.Echo('Pass');
