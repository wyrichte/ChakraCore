var resolve, reject;
var promise = new Promise(function(s,f) { resolve = s; reject = f; });

promise.then(function(v) { }, function(e) { });
