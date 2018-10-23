
// Exhibiting WScript.postMessage behaviour among other threads
// Use WScript.Done to stop process any more messages.

var sc1 = WScript.LoadScript(`
onmessage = function(e) {
    e.data.msg += ',sc1';
    WScript.postMessage(undefined, e.data);
}
`, "crossthread");

var sc2 = WScript.LoadScript(`
onmessage = function(e) {
    e.data.msg += ',sc2';
    WScript.postMessage(undefined, e.data);
}
`, "crossthread");

var obj = {msg : "hello"};
WScript.postMessage(sc1, obj);
var iteration = 0;
onmessage = function (e) {
  var obj = e.data;
  if (iteration++ >= 10) {
      if (obj.msg == "hello,sc1,sc2,sc1,sc2,sc1,sc2,sc1,sc2,sc1,sc2,sc1") {
        print('pass');
      } else {
        print('failed');
        print(obj.msg);
      }
      WScript.Done();
  }
  else {
      if (iteration % 2 == 1) {
            WScript.postMessage(sc2, obj);
      } else {
            WScript.postMessage(sc1, obj);
      }
  }
}
