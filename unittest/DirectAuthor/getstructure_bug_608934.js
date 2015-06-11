// Verifying that GetStructure is creating the display graph correctly.

/**ref:..\\..\\Lib\\Author\\References\\domWeb.js**/
/**ref:..\\..\\Lib\\Author\\References\\libhelp.js**/

if (typeof Namespace == 'undefined') var Namespace = {};
if (!Namespace.Manager) Namespace.Manager = {};

Namespace.Manager = {
    Register: function (namespace) {
        namespace = namespace.split('.');

        if (!window[namespace[0]]) window[namespace[0]] = {};

        var strFullNamespace = namespace[0];
        for (var i = 1; i < namespace.length; i++) {
            strFullNamespace += "." + namespace[i];
            eval("if(!window." + strFullNamespace + ")window." + strFullNamespace + "={};");
        }
    }
};

// Register our Namespace
Namespace.Manager.Register("PietschSoft.Utility.Class");

// Add the Triplet class to the namespace created above
PietschSoft.Utility.Class.Triplet = function (one, two, three) {
    this.First = one;
    this.Second = two;
    this.Third = three;
}

// Create an instance of our Triplet class
var myTriplet = new PietschSoft.Utility.Class.Triplet("1", "2", "3");

/**gs:**/