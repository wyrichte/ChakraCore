var a = 1;
var b = Debug.parseFunction('c = a');
if( b.toString() == "c = a") {print("pass"); }
else {print("fail"); }