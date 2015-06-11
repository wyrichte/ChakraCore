//
// test index property name inspection
//

this[200] = 200; // on global

(function() {
    var i = 100;
    
    // Test arguments
    arguments[i] = i;
    i++;

    // Put test objects in an array
    var that = [new Boolean(), new Number(1), new String("strobj"), function () { }, new Error("ErrorObj"), /regex/, new Date(), Set.prototype].map(
        function (v) {
            v[i] = i;
            i++;
            return v;
        });

    i; /**bp:locals(2);**/

}).apply({}, ["arg0", "arg1"]);

this;
/**bp:locals(1);**/

WScript.Echo("pass");
