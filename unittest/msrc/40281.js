var a = Array(65536);
var arr_arr = Array(12288);
function netuer(buffer) {
  SCA.deserialize(SCA.serialize(buffer, { context: 'samethread' }, undefined, [buffer]));
}
type = new Uint32Array(16777216);
function f(c, d) {
  type[c] = d;
}
value2 = {};
value2.valueOf = function () {
  netuer(type.buffer);
  for (var i = 0; i < arr_arr.length; ++i) {
    arr_arr[i] = a.slice();
  }
  return 4294967295;
};
for (var j = 0; j < 131072; j++) {
  f();
}
f(9, value2);
for (var i = 0; arr_arr; i++) {
  arr_arr[i].length = 0;
}
print("pass");