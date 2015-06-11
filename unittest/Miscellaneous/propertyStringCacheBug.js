//
// Blue 559508

var a = Object.create(this);
// or:
// var a=function(){};
// a.__proto__ = this;

// Filter out "WScript" as it doesn't support enumeration.
for (var i in this) {
    if (i != "WScript") {
        for (var j in this[i]) {
            if (j != "WScript") {
                  try {
                    z;
                  }
                  catch(e) {}; // DOM reject invalid call to getter.
                }
        }
    }
}

self.onmessage = function(e) {
    self.postMessage("pass");
    self.close();
}
